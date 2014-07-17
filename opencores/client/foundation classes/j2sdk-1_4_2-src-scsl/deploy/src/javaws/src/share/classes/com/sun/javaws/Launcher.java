/*
 * @(#)Launcher.java	1.127 03/03/06
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws;
import java.util.Properties;
import java.io.*;
import java.net.*;
import java.security.*;
import java.util.*;
import java.lang.reflect.*;
import java.awt.*;
import java.awt.event.*;
import java.applet.*;
import java.lang.reflect.InvocationTargetException;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.SwingUtilities;
import javax.swing.UIManager;
import javax.swing.UnsupportedLookAndFeelException;
import java.util.jar.JarFile;
import java.util.jar.Manifest;
import com.sun.javaws.ui.general.GeneralUtilities;
import com.sun.javaws.debug.*;
import com.sun.javaws.jnl.*;
import com.sun.javaws.util.VersionString;
import com.sun.javaws.util.VersionID;
import com.sun.javaws.util.URLUtil;
import com.sun.javaws.cache.DownloadProtocol;
import com.sun.javaws.cache.InstallCache;
import com.sun.javaws.cache.DiskCacheEntry;
import com.sun.javaws.security.AppPolicy;
import com.sun.javaws.security.KeyStoreManager;
import com.sun.javaws.security.JavaWebStartSecurity;
import com.sun.javaws.ui.console.Console;
import com.sun.javaws.exceptions.*;
import com.sun.javaws.Main;
import javax.jnlp.ServiceManager;
import javax.jnlp.UnavailableServiceException;
import com.sun.jnlp.BasicServiceImpl;
import com.sun.jnlp.ExtensionInstallerServiceImpl;
import com.sun.jnlp.JNLPClassLoader;
import com.sun.jnlp.AppletContainer;
import com.sun.jnlp.AppletContainerCallback;
/*
 * Given a LaunchDescriptor, the class takes care of launching
 * the given application. This might involve downloading JREs, and
 * optional packages
 */
public class Launcher implements Runnable {
       
    /**
     *  Reference to download progress window
     */
    private DownloadWindow _downloadWindow = null;
     
    /**
     * The console. This will be null if the user doesn't want to see the
     * console.
     */
    private Console _console = null;
    private LaunchDesc _launchDesc;
    private String[] _args;
    private JAuthenticator _ja;
    
    /** Main entry point for launching the give app. If the didInstall flag is true, then
     *  this is the second time around with the LaunchDesc. Thus, we should not do any
     *  downloads nor spawn a separate JVM. This might lead to an infinit loop.
     */
    public Launcher(LaunchDesc ld, String[] args) {
	_launchDesc = ld;
	_args = args;

	ThreadGroup _systemTG;
        for ( _systemTG = Thread.currentThread().getThreadGroup();
              _systemTG.getParent() != null; 
              _systemTG = _systemTG.getParent()
	    ); 
 
        ThreadGroup applicationThreadGroup =
            new ThreadGroup(_systemTG, "javawsApplicationThreadGroup");

        Thread javawsApplicationMain =
            new Thread(applicationThreadGroup, this, "javawsApplicationMain");

	javawsApplicationMain.start();
    }
    
    private void removeTempJnlpFile(LaunchDesc ld) {
	DiskCacheEntry dce = null;

	try {
	    dce =  DownloadProtocol.getCachedLaunchedFile(ld.getLocation());
	} catch (JNLPException jnlpe) {
	    Debug.ignoredException(jnlpe);
	}

	if (dce == null) return;

	File cachedJnlp = dce.getFile();

	if (cachedJnlp != null && JnlpxArgs.shouldRemoveArgumentFile()) {
	    //remove temp file
	    new File(_args[0]).delete();

	    // mark removeArgumentFile to false
	    JnlpxArgs.setShouldRemoveArgumentFile(String.valueOf(false)); 
	    
	    //replace args[0] to point to this cached jnlp file
	    _args[0] = cachedJnlp.getPath();
	    
	}
    }

    public void run() {
	
        // We never trust LaunchDesc's with <jnlp href=".." unless we have downloaded them
        // ourselves. Make sure to get it from the cache, if we got one.
	LaunchDesc ld = LaunchDownload.getLaunchDescFromCache(_launchDesc);
        
	// remove temp jnlp file if it is in the cache already
	removeTempJnlpFile(ld);

        // Tracing
        if (Globals.TraceStartup) Debug.println(ld.toString());
        
        // See if the JNLP file properties contains any debugging properties
        if (ld.getResources() != null) {
	    Globals.getDebugOptionsFromProperties(
			ld.getResources().getResourceProperties());
        }
        
        /**
	 * Regardles of property we will create a Console the property
	 * will decide if the user wants to expose the Console.
	 */
        if (_console == null) {
	    _console = new Console();
        }
        
        // Create download window. This might not be shown. But it is created
        // here, so it can be used as a owner for dialogs.
        _downloadWindow = new DownloadWindow();
        
        /**
	 * We initialize the dialog to pop up for user authentication
	 * to password prompted URLs.
	 */
        ConfigProperties cp = ConfigProperties.getInstance();
	if (cp.getJAuthenticator() != ConfigProperties.JA_NONE) {
            _ja = JAuthenticator.getInstance(_downloadWindow);
            Authenticator.setDefault(_ja);
        }
        
        // The handling of this LaunchDesc might result in downloading of
        // a new one, if there is an updated LaunchDesc. We don't want
        // to get into an infinite loop here - so if the Web server keeps
        // returning a newer one - we just disable caching of the JNLP
        // file and launches
        int tries = 0;
        boolean isFinalTry = false;
        do {
	    isFinalTry = (tries == 3);
	    _downloadWindow.setLaunchDesc(ld, true);
	    ld = handleLaunchFile(ld, _args, !isFinalTry);
	    tries++;
        } while(ld != null && !isFinalTry);
        
    }
    
    private LaunchDesc handleLaunchFile(LaunchDesc ld, String[] args, boolean checkForJNLPUpdate) {
        // Up front check on JNLP Spec version
        VersionString version = new VersionString(ld.getSpecVersion());
        VersionID supportedVersion = new VersionID("1.0");
        if (!version.contains(supportedVersion)) {
	    JNLPException.setDefaultLaunchDesc(ld);
	    handleJnlpFileException(ld, new LaunchDescException(ld,
								Resources.getString("launch.error.badjnlversion",
										    ld.getSpecVersion()), null));
        }
        
        // Check that at least some resources was specified
        if (ld.getResources() == null) {
	    handleJnlpFileException(ld, new LaunchDescException(ld,
								Resources.getString("launch.error.noappresources",
										    ld.getSpecVersion()), null));
        }
        
        // Check that a JRE is specified
        if (!ld.isJRESpecified()) {
	    handleJnlpFileException(ld, new LaunchDescException(ld,
								Resources.getString("launch.error.missingjreversion"), null));
        }
        
        if (ld.isApplicationDescriptor()) {
	    // Applications and Applets
	    return handleApplicationDesc(ld, args, false, checkForJNLPUpdate);
        } else if (ld.getLaunchType() == LaunchDesc.INSTALLER_DESC_TYPE) {
	    // Installers
	    return handleApplicationDesc(ld, args, true, checkForJNLPUpdate);
        } else {
	    // All others
	    JNLPException.setDefaultLaunchDesc(ld);
	    handleJnlpFileException(ld, new MissingFieldException(
					ld.getSource(),
					"<application-desc>|<applet-desc>"));
        }
        return null;
    }
    
    private LaunchDesc handleApplicationDesc(LaunchDesc ld,
					    String[] args,
					    boolean isInstaller,
					    boolean checkForJNLPUpdate)
    {
        // Initialize exception with the launch descriptor
        JNLPException.setDefaultLaunchDesc(ld);
        
        ConfigProperties cp = ConfigProperties.getInstance();
	int timeout;
        Frame owner = _downloadWindow.getFrame();
        
        // Get location and LocalApplication Properties for JNLP file
        // Descriptor home is either the <jnlp href="..."> attribute or the URL for
        // the main JAR file. If neither is specified, we signal an error
        URL jnlpUrl = ld.getCanonicalHome();
        if (jnlpUrl == null) {
	    LaunchErrorDialog.show(owner,
				   new LaunchDescException( ld,
							   Resources.getString("launch.error.nomainjar"), null));
        }
        LocalApplicationProperties lap = null;
        if (!isInstaller) {
	    lap = InstallCache.getCache().getLocalApplicationProperties(jnlpUrl, ld);
        } else {
	    // An extension should always be invoked by a a file entry that points to
	    // the cache. We can get the URL and version id from it.
	    lap = InstallCache.getCache().getLocalApplicationProperties(args[0], ld);
	    jnlpUrl = lap.getLocation();
        }

        if (Globals.TraceStartup) {
	    Debug.println("LaunchDesc location: " + jnlpUrl + ", version: " + lap.getVersionId());
        }
        
        // Step One: Check if we need to download resources.
        boolean offlineMode = Globals.isOffline();
        // This call installs all extension descriptors in the launch descriptor as a
        // side-effect. If it succeeds.
        boolean isCached = LaunchDownload.isInCache(ld);
        // Determine if a JRE is already available
        ConfigProperties.JREInformation jreInfo = LaunchSelection.selectJRE(ld);
        
	timeout = cp.getRapidUpdateTimeout();
	
        // Need to update if either:
        //  - the application is not cached
        //  - no local JRE is found
        //  - the preferences is to always do the check (except when offline)
        //  - the application properties says that we should do an update (except when offline)
        //  - Installer needs progress window
        boolean forceUpdate =
	    (!isCached)  ||
	    (jreInfo == null) ||
	    (!offlineMode && (cp.getForceUpdateCheck() || lap.forceUpdateCheck() ||
				  ((new RapidUpdateCheck()).doUpdateCheck(ld, lap, timeout)))) ||
	    (isInstaller);
        
        if (Globals.TraceStartup) {
	    Debug.println("Offline mode: " + offlineMode);
	    Debug.println("IsInCache: " + isCached);
	    Debug.println("forceUpdate: " + forceUpdate);
	    Debug.println("Installed JRE: " + jreInfo);
	    Debug.println("IsInstaller: " + isInstaller);
        }
        
        if (forceUpdate && offlineMode) {
	    LaunchErrorDialog.show(owner, new OfflineLaunchException());
        }
        
        //
        // Download all resources
        //
        
	// Keep track of all JNLP files for installers
        ArrayList installFiles = new ArrayList();
        
        if (forceUpdate) {
	    LaunchDesc newLd = downloadResources(ld,
						 jreInfo == null,
						 checkForJNLPUpdate,
						 installFiles);
	    // Check if we got a new LaunchDesc
	    if (newLd != null) {
		// downloaded JNLP file into cache, remove temp file
		removeTempJnlpFile(ld);
		return newLd;
	    }
	    
	    // Reset force update
	    if (lap.forceUpdateCheck()) {
		lap.setForceUpdateCheck(false);
		try { lap.store(); }
		catch(IOException ioe) { Debug.ignoredException(ioe); }
	    }
        }

        SplashScreen.generateCustomSplash(owner, ld, forceUpdate);
        
        //
        // Run all installers if any
        //
        if (!installFiles.isEmpty()) {
	    if (isInstaller) {
		// FIXIT: Installers should not have installers
	    }
	    executeInstallers(installFiles);
	    // Force reexecution of JRE
	    jreInfo = null;
        }
        
        // Let progress window show launching behavior
        if (_downloadWindow.getFrame() != null) {
	    String title = Resources.getString("launch.launchApplication");
	    if (ld.getLaunchType() == LaunchDesc.INSTALLER_DESC_TYPE) {
		title = Resources.getString("launch.launchInstaller");
	    }
	    _downloadWindow.showLaunchingApplication(title);
        }
        
        
        // Detemine what JRE to use, we have not found want yet
        if (jreInfo == null) {
	    // Reread config properties with information about new JREs
	    cp.refresh();
	    jreInfo = LaunchSelection.selectJRE(ld);
	    if (jreInfo == null) {
		LaunchErrorDialog.show(null,
				       new LaunchDescException(ld,
							       Resources.getString("launch.error.missingjreversion"),
							       null));
	    }
        }
        
        // Running on wrong JRE or if we did an install, then relaunch JVM
        JREDesc selectedJRE = ld.getResources().getSelectedJRE();
	long minHeap = selectedJRE.getMinHeap();
	long maxHeap = selectedJRE.getMaxHeap();
        boolean heapSizeMatches = JnlpxArgs.isCurrentRunningJREHeap(
				minHeap, maxHeap);
	Properties props = ld.getResources().getResourceProperties();
	boolean securePropsMatch = JnlpxArgs.isSecurePropsMatch(props);

	// For debuggeeType == JWS or debuggeeType == JNL, we always force
	// the relaunch of a new (next) JRE, even if for instance the version
	// of the currently running JRE is the same as the version required
	// to run the JNL app; the forced relaunch is required since there is
	// no other way to switch from running in debugging mode to running
	// in non-debugging mode (JWS) or vice versa (JNL).
	boolean jpdaMotivatedForcedRelaunch =
			(JPDA.getDebuggeeType() == JPDA.JWS	||
			 JPDA.getDebuggeeType() == JPDA.JNL);

	if (jpdaMotivatedForcedRelaunch || !jreInfo.isCurrentRunningJRE() || 
	    !installFiles.isEmpty() || !heapSizeMatches || !securePropsMatch) {
	    // JRE is installed. Launch separate process for this JVM
	    try {
		execProgram(jreInfo, args, minHeap, maxHeap, props);
	    } catch(IOException ioe) {
		LaunchErrorDialog.show(null,
				       new JreExecException(jreInfo.getInstalledPath(), ioe));
	    }

	    // do not remove tmp file if it is a relaunch
	    if (JnlpxArgs.shouldRemoveArgumentFile()) { 
		JnlpxArgs.setShouldRemoveArgumentFile(String.valueOf(false)); 
	    }	    

	    Main.systemExit(0); 
        }
        
        // Remove argument file if neccesary
        JnlpxArgs.removeArgumentFile(args);
        
        if (Globals.TraceStartup) {
	    Debug.println("continuing launch in this VM");
        }
        continueLaunch(owner, lap, offlineMode, jnlpUrl, ld, forceUpdate);
        
        // No need to redownload
        return null;
    }
    
    private static class EatInput implements Runnable {
	private InputStream _is;
	
	EatInput(InputStream is) {
	    _is = is;
	}
	
	public void run() {
	    byte[] buffer = new byte[1024];
	    try {
		while(true) {
		    int n = _is.read(buffer);
		}
	    } catch(IOException ioe) { /* just ignore */ }
	}
	
	private static void eatInput(InputStream is) {
	    EatInput eater = new EatInput(is);
	    new Thread(eater).start();
	}
    }
    
    
    private void executeInstallers(ArrayList installFiles) {
        
        // Let progress window show launching behavior
        if (_downloadWindow.getFrame() != null) {
	    String title = Resources.getString("launch.launchInstaller");
	    _downloadWindow.showLaunchingApplication(title);
	    // We will just show this window a short while, and then remove
	    // it. The installer is going to pop up pretty quickly.
	    new Thread(new Runnable() {
			public void run() {
			    try {
				Thread.sleep(5000);
			    } catch(Exception ioe) { /* ignore */ };
			    _downloadWindow.getFrame().setVisible(false);
			}}).start();
        }
        
        for(int i = 0; i < installFiles.size(); i++) {
	    File jnlpFile = (File)installFiles.get(i);
	    
	    try {
		LaunchDesc ld = LaunchDescFactory.buildDescriptor(jnlpFile);
		LocalApplicationProperties lap = InstallCache.getCache().
		getLocalApplicationProperties(jnlpFile.getPath(), ld);
		// proceed with normal installation
		if (Globals.TraceExtensions) {
		    Debug.println("Installing extension: " + jnlpFile);
		}
		
		String[] args = new String[]{ jnlpFile.getAbsolutePath() };
		
		// Determine JRE to run on
		ConfigProperties.JREInformation jreInfo = LaunchSelection.selectJRE(ld);
		if (jreInfo == null) {
		    _downloadWindow.getFrame().setVisible(true);
		    // No JRE to run application (FIXIT: Wrong exception)
		    LaunchErrorDialog.show(null,
					   new LaunchDescException(ld,
								   Resources.getString("launch.error.missingjreversion"),
								   null));
		}
		
		// remeber whether to removeArgumentFile for the application
		boolean removeArgumentFile = JnlpxArgs.shouldRemoveArgumentFile();

		// Exec installer and wait for it to complete
		// should not remove installer JNLP file in cache
		JnlpxArgs.setShouldRemoveArgumentFile("false");
                Properties props = ld.getResources().getResourceProperties(); 
		Process p = execProgram(jreInfo, args, -1, -1, props);
		EatInput.eatInput(p.getErrorStream());
		EatInput.eatInput(p.getInputStream());
		p.waitFor();
		
		// reset removeArgumentFile flag for this application
		JnlpxArgs.setShouldRemoveArgumentFile(String.valueOf(removeArgumentFile));

		// Validate that installation succeded
		lap.refresh();
		if (lap.isRebootNeeded()) {
		    boolean doboot = false;
		    ExtensionInstallHandler eih =
			ExtensionInstallHandler.getInstance();
		    if (eih != null &&
			eih.doPreRebootActions(_downloadWindow.getFrame())) {
		    	doboot = true;
		    }
		    // set locally installed to be true
		    lap.setLocallyInstalled(true);
		    lap.setRebootNeeded(false);
		    lap.store();
		    if (doboot && eih.doReboot()) {
			Main.systemExit(0);
		    }
		}
		if (!lap.isLocallyInstalled()) {
		    _downloadWindow.getFrame().setVisible(true);
		    // Installation failed
		    LaunchErrorDialog.show(null, new LaunchDescException(ld,
									 Resources.getString("Launch.error.installfailed"), null));
		}
	    } catch(JNLPException je) {
		_downloadWindow.getFrame().setVisible(true);
		LaunchErrorDialog.show(null, je);
	    } catch(IOException io) {
		_downloadWindow.getFrame().setVisible(true);
		LaunchErrorDialog.show(null, io);
	    } catch(InterruptedException iro) {
		_downloadWindow.getFrame().setVisible(true);
		LaunchErrorDialog.show(null, iro);
	    }
        }
    }

    static private Process execProgram(ConfigProperties.JREInformation jreInfo, 
				String[] args, long minHeap, long maxHeap, 
				Properties props) throws IOException {
        String javacmd = jreInfo.getInstalledPath();
        Debug.jawsAssert(javacmd.length() != 0, "must exist");
        
        String[] jnlpxs = JnlpxArgs.getArgumentList(javacmd, minHeap, maxHeap, props);
        int cmdssize = 1 + jnlpxs.length + args.length;
        String[] cmds = new String[cmdssize];
        int pos = 0;
        cmds[pos++] = javacmd;
        for(int i = 0; i < jnlpxs.length; i++) cmds[pos++] = jnlpxs[i];
	
        for(int i = 0; i < args.length; i++) cmds[pos++] = args[i];
        
	// (possibly) insert any JPDA arguments into command line
	cmds = JPDA.JpdaSetup(cmds, jreInfo);

        if (Globals.TraceStartup) {
	    Debug.println("Launching new JRE version: " + jreInfo);
	    for(int i = 0; i < cmds.length; i++) {
		Debug.println("cmd " + i + " : " + cmds[i]);
	    }
        }
        if (Globals.TCKHarnessRun) {
	    Debug.tckprintln(Globals.NEW_VM_STARTING);
        }
        return Runtime.getRuntime().exec(cmds);
    }

    private void continueLaunch(Frame owner, LocalApplicationProperties lap, 
	boolean offlineMode, URL jnlpUrl, LaunchDesc ld, boolean updated) {
	// Initialize KeyStoreManager. This object handles all certificates
        try {
	    KeyStoreManager.initialize();
        } catch(KeyStoreException ks) {
	    // Ignore fr now.
	    // This should probably be logged
        }
	
        // Initialize the App policy stuff
        AppPolicy policy = AppPolicy.createInstance(
					ld.getCanonicalHome().getHost());
	
        try {
	    // Check is resources in each JNLP file is signed and prompt for
	    // certificates. This will also check that each resources downloaded
	    // so far is signed by the same certificate.
	    LaunchDownload.checkSignedResources(ld);
	    
	    // Check signing of all JNLP files
	    LaunchDownload.checkSignedLaunchDesc(ld);
        }  catch(JNLPException je) {
	    LaunchErrorDialog.show(owner, je);
        } catch(IOException ioe) {
	    // This should be very uncommon
	    LaunchErrorDialog.show(owner, ioe);
        }
        
        // Step 3: Setup netloader
        final JNLPClassLoader netLoader =
	    JNLPClassLoader.createClassLoader(ld, policy);
	
        // Set context classloader to the JNLP classloader, so look up of classes
        // will start in that loader. This is important since this thread will later
        // call into the main() of the application.
        Thread.currentThread().setContextClassLoader(netLoader);
	
        // Step 4: Setup security
	System.setSecurityManager(new JavaWebStartSecurity());
	
	// set the context class loader of the AWT EventQueueThread.
	try {
	    SwingUtilities.invokeAndWait(new Runnable() {
			public void run() {
			    Thread.currentThread().setContextClassLoader(netLoader);
			    // set the LookAndFeel for the app context
			    try {
				UIManager.setLookAndFeel(UIManager.getLookAndFeel());
			    } catch (UnsupportedLookAndFeelException e) {
				e.printStackTrace();
	    			Debug.ignoredException(e);
			    }
			}
		    });
	} catch (InterruptedException ignore) {
	    Debug.ignoredException(ignore);
	} catch (InvocationTargetException ignore) {
	    Debug.ignoredException(ignore);
	}
	
	
        // Step 5: Load main class
        String className = null;
        Class mainClass= null;
        try {
	    className = LaunchDownload.getMainClassName(ld, true);
	    if (Globals.TraceStartup) {
		Debug.println("Main-class: " + className);
	    }
	    // Lookup class
	    mainClass = netLoader.loadClass(className);
	    if (this.getClass().getPackage().equals(mainClass.getPackage())) {
                throw new ClassNotFoundException(className); 
            }   
        } catch(ClassNotFoundException cnfe) {
	    LaunchErrorDialog.show(owner, cnfe);
        } catch(IOException ioe) {
	    LaunchErrorDialog.show(owner, ioe);
        } catch(JNLPException je) {
	    LaunchErrorDialog.show(owner, je);
        }
        
        // At this point the user will have accepted to run the code.
        // If the code requested full access, then the AppPolicy object will
	// already have put up a dialog causing the user to confirm the launch.
        //
        
        // If no explicit codebase is given, then the codebase URL is
	// the base URL for the JNLP file
        URL codebase =  ld.getCodebase();
        if (codebase == null) {
	    codebase = URLUtil.getBase(jnlpUrl);
        }
        
        // Step 6: Show the console if necessary.
        if (_console != null) {
	    _console.show(ld.getInformation().getTitle());
        }
        
        // Step 7: Setup JNLP API
	try {
	    BasicServiceImpl.initialize(codebase,
					offlineMode,
					BrowserSupport.isWebBrowserSupported());
	     	   
	    // Setup ExtensionInstallation Service if needed
	    if (ld.getLaunchType() == LaunchDesc.INSTALLER_DESC_TYPE) {
		String installDir = InstallCache.getCache().getNewExtensionInstallDirectory();
		// Setup the install context
		ExtensionInstallerServiceImpl.initialize(installDir,
							 lap,
							 _downloadWindow);
	    }
        } catch(IOException ioe) {
	    LaunchErrorDialog.show(owner, ioe);
        }
	 	  
        // Step 8: User has accepted to launch the application. GO TO IT!
        try {
	    
	    // Check for desktop integration
	    if (ld.getLaunchType() != LaunchDesc.INSTALLER_DESC_TYPE && ld.getLocation() != null) {
		notifyLocalInstallHandler(ld, lap, updated);
	    }
	    
	    // Execute main class
	    DownloadWindow dw = _downloadWindow;
	    _downloadWindow = null; // Reset reference, so loading delegates will be disabled
	    if (Globals.TCKHarnessRun) {
		Debug.tckprintln(Globals.JNLP_LAUNCHING);
	    }
	    executeMainClass(ld, mainClass, dw);
        } catch(SecurityException se) {
	    // This would be an application-level security exception
	    LaunchErrorDialog.show(owner, se);
        } catch(IllegalAccessException iae) {
	    LaunchErrorDialog.show(owner, iae);
        } catch(IllegalArgumentException iae) {
	    LaunchErrorDialog.show(owner, iae);
        } catch(InstantiationException ie) {
	    LaunchErrorDialog.show(owner, ie);
        } catch(InvocationTargetException ite) {
	    LaunchErrorDialog.show(owner, ite.getTargetException());
        } catch(NoSuchMethodException nsme) {
	    LaunchErrorDialog.show(owner, nsme);
	}
    }
    
    private boolean _shownDownloadWindow = false;
    
    private LaunchDesc downloadResources(LaunchDesc ld,
					boolean downloadJRE,
					boolean downloadJNLPFile,
					ArrayList installFiles) {
        
        // Show loading progress window. We only do this the first
        // time through. If we are downloading a JNLP file, we might
        // come here twice.
        if (!_shownDownloadWindow) {
	    _shownDownloadWindow = true;
	    _downloadWindow.buildIntroScreen();
	    _downloadWindow.showLoadingProgressScreen();
	    _downloadWindow.getFrame().show();
	    SplashScreen.hide();
        }
        
        // Download all needed resources
        try {
	    // First check for updated JNLP File
	    if (downloadJNLPFile) {
		LaunchDesc updatedLaunchDesc = LaunchDownload.getUpdatedLaunchDesc(ld);
		if (updatedLaunchDesc != null) {
		    // Found updated one, return with this, so we can restart the download
		    // process
		    return updatedLaunchDesc;
		}
	    }
	    
	    if (downloadJRE) {
		LaunchDownload.downloadJRE(ld, _downloadWindow, installFiles);
	    }
	    
	    LaunchDownload.downloadExtensions(ld, _downloadWindow, 0, installFiles);
	    
	    // Check that the JNLP restrictions given in the spec. Applications with sandbox
	    // security is only allowed to referer back to a specific host and not use native
	    // libraries
	    LaunchDownload.checkJNLPSecurity(ld);
	    
	    // Download all eagerly needed resources (what is in the cache is not ok, should
	    // do a check to the server)
	    LaunchDownload.downloadEagerorAll(ld, false, _downloadWindow, false);
	    
        } catch(SecurityException se) {
	    // This error should be pretty uncommon. Most would have already been wrapped
	    // in a JNLPException by the downloadJarFiles method.
	    LaunchErrorDialog.show(_downloadWindow.getFrame(), se);
        } catch(JNLPException je) {
	    LaunchErrorDialog.show(_downloadWindow.getFrame(), je);
        } catch(IOException ioe) {
	    LaunchErrorDialog.show(_downloadWindow.getFrame(), ioe);
        }
        return null;
    }
    
    /**
     * This invokes <code>installIfNecessaryFromLaunch</code> on
     * the LocalInstallHandler. This will also update the state of
     * the <code>LocalApplicationProperties</code>.
     */
    private void notifyLocalInstallHandler(LaunchDesc ld, 
		LocalApplicationProperties lap, boolean updated) {
        if (lap == null) return;
        lap.setLastAccessed(new Date());
        lap.incrementLaunchCount();

        // Notify the LocalInstallHandler
        LocalInstallHandler lih = LocalInstallHandler.getInstance();
        
	if (lih != null && lih.isLocalInstallSupported()) {
	    Window frame = null;
	    if (_downloadWindow != null) {
	        frame = _downloadWindow.getFrame();
	    }
	    if (lap.isLocallyInstalled()) {
		if (updated) {
		    lih.uninstall(ld, lap);
		    lih.install(frame, ld, lap);
		}
	    } else {
	        lih.installIfNecessaryFromLaunch(frame, ld, lap);
	    }
        }
        
        // Save the LocalApplicationProperties state.
        try {
	    lap.store();
        } catch (IOException ioe) {
	    // We could warn the user
	    if (Globals.TraceStartup) {
		Debug.println("Couldn't save LAP: " + ioe);
	    }
        }
    }
    
    /** Executes the mainclass */
    private void executeMainClass(LaunchDesc ld, Class mainclass, DownloadWindow dw)
        throws IllegalAccessException, InstantiationException,
        InvocationTargetException, NoSuchMethodException  {
        
        if (ld.getLaunchType() == LaunchDesc.APPLET_DESC_TYPE) {
	    executeApplet(ld, mainclass, dw);
        } else {
	    executeApplication(ld, mainclass, dw);
        }
    }
    
    /** Execute launchDesc for application */
    private void executeApplication(LaunchDesc ld, Class mainclass, 
		DownloadWindow dw) throws 
		IllegalAccessException, InstantiationException, 
		NoSuchMethodException, InvocationTargetException   {
        
        String[] args = null;
        if (ld.getLaunchType() == LaunchDesc.INSTALLER_DESC_TYPE) {
	    dw.reset();
	    // Fixed argument for installer
	    args = new String[] { "install" };
        } else {
	    // Remove Splash window
	    dw.disposeWindow();
	    SplashScreen.hide();
	    
	    // Get arguments for application
	    args = ld.getApplicationDescriptor().getArguments();
        }
        
        // Lose reference to aid GC
        dw = null;
        
        Object[] wrappedArguments = { args };
        
        // Find a static main(String[] args) method
        Class[] main_type = { (new String[0]).getClass() };
        Method mainMethod = mainclass.getMethod("main", main_type);
        // Check that method is static
        if (!Modifier.isStatic(mainMethod.getModifiers())) {
	    // FIXIT: This should be a JNLP exception
	    throw new NoSuchMethodException(Resources.getString("launch.error.nonstaticmainmethod"));
        }
        mainMethod.setAccessible(true);
	
        // Invoke main method
        mainMethod.invoke(null, wrappedArguments);
    }
    
    /** Execute launchDesc for applet */
    private void executeApplet(LaunchDesc ld, Class appletClass, 
		DownloadWindow dw) throws 
		IllegalAccessException, InstantiationException   {
        AppletDesc ad = ld.getAppletDescriptor();
        int height = ad.getWidth();
        int width  = ad.getHeight();
        
        Applet applet = null;
        applet = (Applet)appletClass.newInstance();
        
        // Remove all content from window
        SplashScreen.hide();
        if (dw.getFrame() == null) {
	    // Build the spash screen
	    dw.buildIntroScreen();
	    dw.showLaunchingApplication(ld.getInformation().getTitle());
        }
        final JFrame mainFrame = dw.getFrame();
        // Force classloading
        boolean check = BrowserSupport.isWebBrowserSupported();
        
        // New applet container stub
        AppletContainerCallback callback = new AppletContainerCallback() {
	    /** Use BasicService to show document */
	    public void showDocument(URL url) {
		BrowserSupport.showDocument(url);
	    }
	    
	    /** Resize frame */
	    public void relativeResize(Dimension delta) {
		Dimension d = mainFrame.getSize();
		d.width  += delta.width;
		d.height += delta.height;
		mainFrame.setSize(d);
	    }
        };
        
        URL codebase = BasicServiceImpl.getInstance().getCodeBase();
        URL documentbase = ad.getDocumentBase();
        // Documentbase defaults to codebase if not specified
        if (documentbase == null) documentbase = codebase;
        
        // Build GUI for Applet
        final AppletContainer ac = new AppletContainer(callback,
						       applet,
						       ad.getName(),
						       documentbase,
						       codebase,
						       height,
						       width,
						       ad.getParameters());
       


        // We want to override the default WindowListener added inside the
        // DownloadWindow object and add a new WindowListener which calls
        // applet's stop() & destroy() method whenever window is closed.

        mainFrame.removeWindowListener(dw);
        mainFrame.addWindowListener(new WindowAdapter()
        {
             public void windowClosing (WindowEvent event)
             {
              ac.stopApplet();
              }
         });
 
        dw.clearWindow();
        // Update gui for frame
        mainFrame.setTitle(ld.getInformation().getTitle());
        Container parent = mainFrame.getContentPane();
        parent.setLayout(new BorderLayout());
        parent.add("Center", ac);
        mainFrame.pack();
        Dimension d = ac.getPreferredFrameSize(mainFrame);
        mainFrame.setSize(d);
        // Force repaint of frame
        mainFrame.getRootPane().revalidate();
        mainFrame.getRootPane().repaint();
        if (!mainFrame.isVisible()) mainFrame.show(); // Make sure to show it
        // Start applet
        ac.startApplet();
    }
    
    private void handleJnlpFileException (LaunchDesc ld, Exception exception) {
	/* purge the bad jnlp file from the cache: */
        DiskCacheEntry dce = null;
        try {
	    dce = DownloadProtocol.getCachedLaunchedFile(ld.getLocation());
	    if (dce != null) {
		InstallCache.getDiskCache().removeEntry(dce);
	    }
        } catch (JNLPException jnlpe) {
	    Debug.ignoredException(jnlpe);
        }
	Frame parent = ((_downloadWindow == null) ?
			    null : _downloadWindow.getFrame());
        LaunchErrorDialog.show(parent, exception);
    }
    
    private class RapidUpdateCheck extends Thread {
        private LaunchDesc _ld;
        private LocalApplicationProperties _lap;
        private boolean _updateAvailable;
        private boolean _checkCompleted;
        private Object _signalObject = null;
        
        public RapidUpdateCheck() {
	    _ld = null;
	    _signalObject = new Object();
        }
        
        private boolean doUpdateCheck(LaunchDesc ld, 
				LocalApplicationProperties lap, int timeout) {
	    _ld = ld;
	    _lap = lap;
	    boolean ret = false;

	    synchronized (_signalObject) {
		_updateAvailable = false;
		_checkCompleted = false;
		start();
	        do {
		  if (ld.getInformation().supportsOfflineOperation()) {
		    try {
		        /* wait up to timeout milli's, no answer = no Update*/
			_signalObject.wait(timeout);
			ret = _updateAvailable ;
		    } catch (InterruptedException e) {
			ret = false;
		    }
		  } else {
		    try {
		        /* wait up to timeout milli's, no answer =
			 **                    show dialog and wait some more
			 */
			_signalObject.wait(timeout);
			ret = _updateAvailable || !_checkCompleted;
		    } catch (InterruptedException e) {
			ret = true;
		    }
		  }
	        } while ((_ld.isHttps() && !_checkCompleted)	||
			(_ja != null &&
		         _ja.isChallanging())); // don't cover the JAuthenticator
					        // challange window with either
					        // the DownbloadWindow or App
	    }
	    return ret;
        }
        
	
        public void run() {
	    boolean available = false;
	    try {
		available = LaunchDownload.isUpdateAvailable(_ld);
	    } catch (FailedDownloadingResourceException fdre) {
		if (_ld.isHttps()) {
		    Throwable thr = fdre.getWrappedException();
		    if ((thr != null) && (thr instanceof 
				javax.net.ssl.SSLHandshakeException)) {
		        // user chose not to accept Https cert ...
		        Main.systemExit(0);
		    }
		}
		Debug.ignoredException(fdre);
	    } catch (JNLPException je) {
		// Just ignore
		Debug.ignoredException(je);
	    }
	    
	    synchronized (_signalObject) {
		_updateAvailable = available;
		_checkCompleted = true;
		_signalObject.notify();
	    }
	    
	    if (_updateAvailable) {
		// Store info. in local application properties, so the next time we can
		// force an update
		_lap.setForceUpdateCheck(true);
		try {
		    _lap.store();
		} catch (IOException ioe) {
		    Debug.ignoredException(ioe);
		}
	    }
        }
    }
}



