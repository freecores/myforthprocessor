/*
 * @(#)WinInstallHandler.java	1.41 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws;

import com.sun.javaws.debug.Globals;
import com.sun.javaws.debug.Debug;
import com.sun.javaws.cache.*;
import com.sun.javaws.jnl.*;
import com.sun.javaws.ui.general.GeneralUtilities;
import com.sun.javaws.exceptions.JNLPException;
import java.awt.*;
import java.awt.event.*;
import java.awt.image.*;
import java.io.*;
import java.net.*;
import javax.swing.*;

/**
 * Instances of InstallHandler are used to handle installing/uninstalling
 * of Applications. Handling installing/uninstalling is platform specific,
 * and therefore not all platforms may support any sort of additional
 * install options. The instance to use for installing can be located
 * via the class method <code>getInstallHandler</code>. This method
 * will look up the class from the property
 * <code>javaws.installer.className</code>, and create it if non-null. A
 * null return value from <code>getInstallHandler</code> indicates the
 * current platform does not support any install options.
 *
 * @version 1.8 03/02/00
 */
public class WinInstallHandler extends LocalInstallHandler {
    
    // Used in LocalApplicationProperties, gives the name the shortcut
    // was created as, which may differ from the real name (if a shortcut
    // already existed with that name).
    private static final String INSTALLED_DESKTOP_SHORTCUT_KEY =
        "windows.installedDesktopShortcut";
    private static final String INSTALLED_START_MENU_KEY =
        "windows.installedStartMenuShortcut";


    // This is the size of the result image we will create.
    private static final int IMAGE_SIZE = 32;
    // The type of image that is obtained from the file.
    private static final int IMAGE_TYPE = InformationDesc.ICON_SIZE_MEDIUM;
    private static final int IMAGE_KIND = IconDesc.ICON_KIND_DEFAULT;
    public static final int TYPE_DESKTOP = 1;
    public static final int TYPE_START_MENU = 2;

   
    /**
     * Path in registry to find shortcuts.
     */  
    private static final String REG_SHORTCUT_PATH =
        "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders";    /** 
     * Key under REG_SHORTCUT_PATH to find desktop shotcut paths.
     */  
    private static final String REG_DESKTOP_PATH_KEY = "Desktop";
    /**
     * Key under REG_SHORTCUT_PATH to find start menu shotcut paths.
     */
    private static final String REG_START_MENU_PATH_KEY = "Programs";
    
    /**
     * Extension of shortcut path files.
     */
    private static final String SHORTCUT_EXTENSION = ".lnk";

    private static final int MAX_PATH = 200;
    
    /**
     * Set to true when the paths for shortcuts and start menu have been
     * looked up.
     */
    private boolean _loadedPaths = false;
    /**
     * Path where desktop shortcuts should be placed.
     */
    private String _desktopPath;
    /**
     * Path where start menu shortcuts should be placed.
     */
    private String _startMenuPath;
     
    
    static {
        NativeLibrary.getInstance().load();
    }
    

    /**
     * Invoked when an application is first launched. This should determine
     * if the user should be prompted for an install, and if necessary do
     * the install.
     */
    public void installIfNecessaryFromLaunch(final Window owner, 
		final LaunchDesc desc, final LocalApplicationProperties lap) {
                 
        int whenInstall = ConfigProperties.getInstance().getWhenInstall();
	switch (whenInstall) {
	    case ConfigProperties.INSTALL_NEVER:
		return;
	    case ConfigProperties.INSTALL_ALWAYS:
		install(owner, desc, lap);
		return;
	    case ConfigProperties.INSTALL_ASK:
	    default:
		if (lap.getAskedForInstall()) {
		    return; // already asked
		}
		break;		// OK - go ahead and ask
	}
        new Thread(new Runnable() {
            public void run() {
	        int ret = GeneralUtilities.showLocalInstallDialog(
				    owner, desc.getInformation().getTitle());
	        if (Globals.TraceLocalAppInstall) {
	            Debug.println("showLocalInstallDialog returned: "+ret);
	        }
	        switch (ret) {
	            case GeneralUtilities.INSTALL_YES:
		        install(owner, desc, lap);
		        lap.setAskedForInstall(true);
		        break;
	            case GeneralUtilities.INSTALL_NO:		
		        lap.setAskedForInstall(true);
		        break;
	            case GeneralUtilities.INSTALL_ASK:
		        lap.setAskedForInstall(false);
		        break;
	        }	
		save(lap);
            }
	}).start();
    }

    /**
     *  Determine if this platform supports Local Application Installation.
     *  We will want to return true for the window managers that we support
     *  in install and uninstall above.
     */
    public boolean isLocalInstallSupported() {
	return true;
    } 

    /**
     * Installs an Application in start menu and desktop
     * <code>lp</code>.
     */
    public void install(Window owner, LaunchDesc desc, 
		LocalApplicationProperties lap) {
        
        install(desc, lap);
    }

    /**
     * Uninstalls a previously installed Application identified by
     * <code>lp</code>.
     */
    public void uninstall(LaunchDesc desc, LocalApplicationProperties lap) {
        // Never show an error.
        uninstall(desc, lap, false);
    }

/*******************************************************/

    private void save(LocalApplicationProperties lap) {
        // Save the LocalApplicationProperties state.
        try {
	    lap.store();
        } catch (IOException ioe) {
	    // We could warn the user
	    if (Globals.TraceLocalAppInstall) {
		Debug.println("Couldn't save LAP: " + ioe);
	    }
        }
    }
    
    private void uninstall(LaunchDesc desc, LocalApplicationProperties lap,
			   boolean showError) {
        if (lap == null) {
	    if (Globals.TraceLocalAppInstall) {
		Debug.println("No LAP for uninstall, bailing!");
	    }
	    return;
        }
	
        String path;
        boolean failed = false;
	
        if ((path = lap.get(INSTALLED_START_MENU_KEY)) != null) {
            if (!uninstallShortcut(path)) {
                failed = true;
            } else {
                lap.put(INSTALLED_START_MENU_KEY, null);
            }
        }
        if ((path = lap.get(INSTALLED_DESKTOP_SHORTCUT_KEY)) != null) {
            if (!uninstallShortcut(path)) {
                failed = true;
            } else {
                lap.put(INSTALLED_DESKTOP_SHORTCUT_KEY, null);
            }
        }

        // Even if we failed, mark the application as not installed.
        lap.setLocallyInstalled(false);
        save(lap);
        if (failed && showError) {
	    uninstallFailed(desc);
        }
    }
    
    /**
     * Returns true if the passed if application has a valid title.
     */
    private boolean hasValidTitle(LaunchDesc desc) {
        if (desc == null) {
	    return false;
        }
        InformationDesc iDesc = desc.getInformation();
	
        if (iDesc == null || iDesc.getTitle() == null) {
	    if (Globals.TraceLocalAppInstall) {
		Debug.println("Invalid: No title!");
	    }
	    return false;
        }
        return true;
    }
    
    private void install(LaunchDesc desc, LocalApplicationProperties lap) {

        if (!hasValidTitle(desc)) {
            if (Globals.TraceLocalAppInstall) {
                Debug.println("No valid title for install, bailing!");
            }
            return;
        }

        if (isApplicationInstalled(desc)) {
	    if (!shouldInstallOverExisting(desc)) {
		return;
	    }
        }
        String iPath;
	int ret = 0;

        try {
	    iPath = lap.getIndirectLauchFilePath(true);
        } catch (IOException ioe) {
	    iPath = null;
	    if (Globals.TraceLocalAppInstall) {
		Debug.println("Couldn't determine path for install: " + ioe);
	    }
        }
        if (iPath == null) {
	    installFailed(desc, ret);
	    return;
        }
        ret = handleInstall(desc, lap, iPath, TYPE_DESKTOP); 
	if (ret != 0) {
            installFailed(desc, ret);
            return;
	}

        ret = handleInstall(desc, lap, iPath, TYPE_START_MENU);
	if (ret != 0) {
	    uninstall(desc, lap, false);
            installFailed(desc, ret);
	    return;	 
        }

	lap.setLocallyInstalled(true);
	save(lap);
    }
    
    /**
     * Returns true if the user wants to install over the existing
     * application.
     */
    private boolean shouldInstallOverExisting(final LaunchDesc desc) {
        final int result[] = { JOptionPane.NO_OPTION };
        Runnable jOptionPaneRunnable = new Runnable() {
	    public void run() {
		result[0] = GeneralUtilities.showOptionDialog (null, 
		    Resources.getString("windowsInstall.applicationAlreadyInstalled",
		        getInstallName(desc)), 
		    Resources.getString("windowsInstall.applicationAlreadyInstalledTitle"),
		    JOptionPane.YES_NO_OPTION,  
		    JOptionPane.QUESTION_MESSAGE);
	    }
        };
	
        invokeRunnable(jOptionPaneRunnable);
        return (result[0] == JOptionPane.YES_OPTION);
    }
    
    /**
     * Invoked when the install fails.
     */
    // err is the error code from native windows call
    private void installFailed(final LaunchDesc desc, final int err) {
        Runnable jOptionPaneRunnable = new Runnable() {
		public void run() {
		    if (err == -7 ) {
			// failed in IPersistFile::Save method
			GeneralUtilities.showMessageDialog(null, Resources.getString("windowsInstall.invalidFilename", getInstallName(desc)), Resources.getString("windowsInstall.installFailedTitle"), JOptionPane.ERROR_MESSAGE);

		    } else {
			GeneralUtilities.showMessageDialog(null, 
							   Resources.getString("windowsInstall.installFailed", 
									       getInstallName(desc)),
							   Resources.getString("windowsInstall.installFailedTitle"),
							   JOptionPane.ERROR_MESSAGE);
		    }
		}
		
	    };
	
        invokeRunnable(jOptionPaneRunnable);
    }
    
    private void invokeRunnable(Runnable runner) {
        if (SwingUtilities.isEventDispatchThread()) {
	    runner.run();
        }
        else {
	    try {
		SwingUtilities.invokeAndWait(runner);
	    } catch (InterruptedException ie) {
	    } catch (java.lang.reflect.InvocationTargetException ite) {}
        }
    }
    
    /**
     * Invoked when the install fails.
     */
    private void uninstallFailed(final LaunchDesc desc) {
        Runnable jOptionPaneRunnable = new Runnable() {
	    public void run() {
		GeneralUtilities.showMessageDialog(null, 
		    Resources.getString("windowsInstall.uninstallFailed", 
					 getInstallName(desc)),
		    Resources.getString("windowsInstall.uninstallFailedTitle"),
		    JOptionPane.ERROR_MESSAGE);
	    }
        };
	
        invokeRunnable(jOptionPaneRunnable);
    }
    
    
    /**
     * Invokes the appropriate native method to handle the installation.
     * Returns true if successful in installing.
     */
    private int handleInstall(LaunchDesc lp,LocalApplicationProperties lap,
				  String arg, int type) {
        InformationDesc iDesc = lp.getInformation();
        String iconPath = getIconPath(lp);
        String name = getInstallName(lp);
        String description = iDesc.getDescription(InformationDesc.DESC_SHORT);
        String key;
        String path;
        int retValue;
	
        arg = "\"@" + arg + "\"";
	
        if (type == TYPE_DESKTOP) {
	    path = getDesktopPath(lp);
	    name = getDesktopName(lp);
	    retValue = installShortcut(path, name, description,
				       ConfigProperties.getStartCommand(),
				       arg, null, iconPath);
	    key = INSTALLED_DESKTOP_SHORTCUT_KEY;
        }
        else {
	    path = getStartMenuPath(lp);
	    name = getStartMenuName(lp);
	    retValue = installShortcut(path, name, description,
				       ConfigProperties.getStartCommand(),
				       arg, null, iconPath);
	    key = INSTALLED_START_MENU_KEY;
        }
	
        if (retValue == 0) {
	    lap.put(key, path);
	    if (Globals.TraceLocalAppInstall) {
		Debug.println("Installed desktop shortcut as: " + name);
	    }
        }
        else {
	    if (Globals.TraceLocalAppInstall) {
		Debug.println("Installed desktop shortcut failed!");
	    }
        }
        return retValue;
    }
    
    /**
     * This returns true if there is currently an application registered
     * under the name <code>name</code>.
     */
    private boolean isApplicationInstalled(LaunchDesc ld) {
        loadPathsIfNecessary();
	         
        // Check for Desktop shortcut.
        String path = getDesktopPath(ld);
        if (path == null) {
	    return true;
        }
        if (new File(path).exists()) {
	    if (Globals.TraceLocalAppInstall) {
		Debug.println("Shortcut exists, bailing");
	    }
	    return true;
        }
	
        // Start menu shortcut
        path = getStartMenuPath(ld);
        if (path == null) {
	    return true;
        }
        if (new File(path).exists()) {
	    if (Globals.TraceLocalAppInstall) {
		Debug.println("Start menu exists, bailing");
	    }
	    return true;
        }
	
        return false;
    }
    
    /**
     * Returns the path to the icon to use for the application identified
     * by <code>ld</code>. This will download and create a new BMP if
     */
    private String getIconPath(LaunchDesc ld) {
        if (Globals.TraceLocalAppInstall) {
	    Debug.println("Getting icon path");
        }
	
        // Check for .ico file.
        IconDesc id = ld.getInformation().getIconLocation(IMAGE_TYPE,
							  IMAGE_KIND);
	File tempIco = null;

        if (id != null) {
	    try {
		// Download image (cached version is ok)
		DiskCacheEntry iEntry = DownloadProtocol.getResource(
		    id.getLocation(),
		    id.getVersion(),
		    DownloadProtocol.IMAGE_DOWNLOAD,
		    true, null);
		
		// Check if mapped image file exist
		File  mEntry = iEntry.getMappedBitmap();
		
		if (mEntry == null || !mEntry.exists()) {
		    mEntry = null;
		    
		    Image awtImage = CacheUtilities.getSharedInstance().
			loadImage(iEntry.getFile().getPath());
		    		
		    // Covert image to .ico file
		    tempIco = saveICOfile(awtImage);

		    if (Globals.TraceLocalAppInstall) {
			Debug.println("updating ICO: " + tempIco);
		    }
		    
		    if (tempIco != null) {
			mEntry = InstallCache.getDiskCache().putMappedImage(id.getLocation(),
										id.getVersion(),
										tempIco);
			tempIco = null; // No delete of temp. file (it got renamed)
		    }
		}
		
		if (mEntry != null) {
		    return mEntry.getPath();
		}
	    } catch (IOException ioe) {
		if (Globals.TraceLocalAppInstall) {
		    Debug.println("exception creating BMP: " + ioe);
		}
	    } catch (JNLPException je) {
		if (Globals.TraceLocalAppInstall) {
		    Debug.println("exception creating BMP: " + je);
		}
	    }
	}
	if (tempIco != null) tempIco.delete();

	String icoPath = ConfigProperties.getInstance().getApplicationHome() + 
			 File.separator + "JavaCup.ico";

	tempIco = new File(icoPath);

	if (tempIco.exists()) {
	    return tempIco.getPath();
	}

	// this create the default button icon if no image is supplied
	tempIco = saveICOfile(Resources.getIcon("JavaCup.image").getImage());
	
	// DEBUG
	if (Globals.TraceIcoCreation)
	    Debug.println("default button ico path: " + tempIco.getPath());
	
	// return ico path if icon is created successfully
	if (tempIco != null) {
	    return tempIco.getPath();
	}
	else {
	    return null;
	}
    }
     
    /**
     * Encodes the passed in image into a windows icon (.ico) and saves
     * it into a temp file. The path to the tremporary file is returned.
     */    
    private File saveICOfile(Image awtImage) {

	FileOutputStream fos = null;
	File result = null;
	try {
	    result = File.createTempFile("javaws", ".ico");
	    fos = new FileOutputStream(result);
	    
	    IcoEncoder encoder = new IcoEncoder(fos, awtImage);
	    
	    encoder.encode();
	    fos.close();
	    return result;
	} catch (Throwable th) {}
	
	if (fos != null) {
	    try {
		fos.close();
	    } catch (IOException ioe) {}
	}
	if (result != null) result.delete(); // Cleanup in case of error
	return null;

    }
       
    
    /**
     * Returns the name to install under.
     */
    private String getInstallName(LaunchDesc desc) {
	return desc.getInformation().getTitle();
    }
    
    private String getDesktopName(LaunchDesc ld) {
	return Resources.getString("windowsInstall.desktopShortcutName",
				       getInstallName(ld));
    }
    
    private String getStartMenuName(LaunchDesc ld) {
	return Resources.getString("windowsInstall.startMenuShortcutName",
				       getInstallName(ld));
    }
    
    /**
     * Returns the path desktop shortcuts are to be placed in.
     */
    private String getDesktopPath(LaunchDesc ld) {
	String path = getDesktopPath();
	if (path != null) {
	    String name = getDesktopName(ld);	 
	    if (name != null) {
		path = path + name;
	    }
	    if (path.length() > MAX_PATH) {
		// truncate MAX_PATH
		path = path.substring(0, MAX_PATH);
	    }    
	    path = path + SHORTCUT_EXTENSION;        	    
	}
	return path;
    }
    
    /**
     * Returns the path desktop shortcuts are to be placed in.
     */
    private String getStartMenuPath(LaunchDesc ld) {
	String path = getStartMenuPath();
	if (path != null) {
	    String name = getStartMenuName(ld);
	    if (name != null) {
		path = path + name;
	    }
	    if (path.length() > MAX_PATH) {
		// truncate MAX_PATH
		path = path.substring(0, MAX_PATH);
	    }    
	    path = path + SHORTCUT_EXTENSION;        
	    
	}
	return path;
    }
    
    /**
     * Returns the path desktop shortcuts are to be placed in.
     */
    private String getDesktopPath() {
	loadPathsIfNecessary();
	return _desktopPath;
    }
    
    /**
     * Returns the path start menu shortcuts are to be placed in.
     */
    private String getStartMenuPath() {
	loadPathsIfNecessary();
	return _startMenuPath;
    }
    
    /**
     * Loads the paths for shortcuts if the paths haven't already been
     * loaded.
     */
    private void loadPathsIfNecessary() {
	if (!_loadedPaths) {
	    _desktopPath = WinRegistry.getString(WinRegistry.HKEY_CURRENT_USER,
						 REG_SHORTCUT_PATH,
						 REG_DESKTOP_PATH_KEY);
	    if (_desktopPath != null && _desktopPath.length() > 0 &&
		_desktopPath.charAt(_desktopPath.length() - 1) != '\\') {
		_desktopPath += '\\';
	    }
	    _startMenuPath = WinRegistry.getString(WinRegistry.
						       HKEY_CURRENT_USER,
						   REG_SHORTCUT_PATH,
						   REG_START_MENU_PATH_KEY);
	    if (_startMenuPath != null && _startMenuPath.length() > 0 &&
		_startMenuPath.charAt(_startMenuPath.length() - 1) != '\\') {
		_startMenuPath += '\\';
	    }
	    _loadedPaths = true;
	    if (Globals.TraceLocalAppInstall) {
		Debug.println("Start path: " + _startMenuPath + " desktop " +
				  _desktopPath);
	    }
	}
    }
    
    /**
     * Uninstalls the shortcut, returning true if either the file didn't exist,
     * or the file exists and it was deleted.
     */
    private boolean uninstallShortcut(String path) {
	File f = new File(path);
	
	if (f.exists()) {
	    return f.delete();
	}
	return true;
    }
    
    private native int installShortcut(String path, String appName,
					   String description,
					   String appPath,
					   String args,String directory,
					   String iconPath);
}
