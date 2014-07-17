/*
 * @(#)Player.java	1.19 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.ui.player;

import com.sun.javaws.*;
import com.sun.javaws.cache.*;
import com.sun.javaws.debug.*;
import com.sun.javaws.jnl.*;
import com.sun.javaws.ui.prefs.*;
import com.sun.javaws.ui.general.*;
import com.sun.javaws.ui.console.Console;
import com.sun.javaws.exceptions.*;
import java.awt.*;
import java.awt.event.*;
import java.beans.*;
import java.io.*;
import java.net.*;
import java.text.*;
import java.util.*;
import javax.accessibility.*;
import javax.swing.*;
import javax.swing.border.*;
import javax.swing.event.*;
import com.sun.javaws.util.GeneralUtil;
import com.sun.javaws.JAuthenticator;
import com.sun.javaws.net.*;


/**
 * @version 1.79 03/23/01
 *
 * The player UI. This class focuses on controlling the UI associated with
 * the player. The player shows three sets of JNL files: files in the
 * current cache, and files from a web page. The loading and
 * managing of  the JNL files is handled by an instance of EntryManager.
 * This class will install the necessary listeners on the EntryManager and
 * update the display as the EntryManager changes.
 * <p>
 * The UI is centered around a JComboBox, and an IconList. The selection in the
 * combo box picks which group of Applications (i.e.: which EntryManager) to
 * display in the IconList.  The Application selected in the IconList is then
 * (optionally) described in the InfoPanel below.
 *
 */
public class Player implements CacheImageLoaderCallback {
    
    /**
     * Used as an entry in _imageMap to indicate the image is being loaded.
     */
    private static final Object LOADING_OBJECT = new Object();
    /**
     * Used as a client property to indicate the EntryManager that should be
     * used.
     */
    private static final String ENTRY_CLIENT_PROPERTY = "EntryClientProperty";
    /**
     * Used as a client property to indicate the class name of the
     * EntryManager that should be used.
     */
    private static final String ENTRY_CLASS_CLIENT_PROPERTY = "EntryClassClientProperty";
    

    private static int DEFAULT_VIEWMENU_ITEM_COUNT = 4;
    private static int ADDENTRY_VIEWMENU_ITEM_INDEX = 0;

    private static int MAX_MENUITEM_LENGTH = 25;

    /**
     * The Image that is used if the jnl file has no image entry.
     */
    private Image _defaultImage;
    /**
     * Set to true when the default Image has been loaded.
     */
    private boolean _loadedDefaultImage;
    
    /**
     * Maps from an image location (IconDesc) to the image to show in the
     * _iconPanel. These are lazily loaded on another thread via the
     * ImageLoader. A value may also be an instance of LOADING_OBJECT,
     * to indicate either the Image is being loaded, or the image
     * couldn't be loaded.
     */
    private HashMap _imageMap;
    
    /**
     * Listens for changes on the current EntryManager (_currentManager).
     */
    private EntryHandler _entryListener;
    
    /**
     * True if we are currently offline.
     */
    private boolean _isOffline;
    
    /**
     * CachedEntryManager.
     */
    private EntryManager _cachedManager;
    /**
     * The currently selected EntryManager.
     */
    private EntryManager _currentManager;
    
    /**
     * Non-null while the preferences panel is being displayed.
     */
    private PrefsPanel _prefsPanel;
    
    /**
     * Handles invoking install/uninstall in a separate thread.
     */
    private InstallRunnable _installer;

    // ArrayList for the BookmarkEntry pair (name, URL)
    private ArrayList _bookmarkEntryList;

    // UI widgets
    private JComponent _viewPanel;
    private JComponent _iconPanel;
    private JComponent _buttonPanel;
    private JComponent _bottomPanel;
    private JComponent _urlPanel;
    
    // within _viewPanel
    private JLabel _applicationsLabel;
    private JLabel _viewNameLabel;
    private JLabel _urlLabel;
    
    // within iconPanel
    private IconList _iconList;
    
    // within buttonPanel
    private JButton _detailButton;
    private JButton _miniButton;
    private JButton _launchButton;
    private boolean _moreDroppedDown;
    
    // within bottomPanel
    private JPanel _infoPanel;
    private JPanel _statusPanel;
    
    private RemoteURL _remote;

    // within _statusPanel
    private JPanel _systemIndicatorPanel;
    private JLabel _statusLabel;
    
    // within infoPanel
    private JLabel _applicationTitle;
    private JLabel _madeByLabel;
    private JLabel _madeByInfoLabel;
    private JLabel _homePageLabel;
    private JButton _homePageButton;
    private JLabel _descriptionLabel;
    private JTextArea _descriptionTextArea;
    private IndicatorsPanel _appIndicatorPanel;
    private Icon _updateAvailable, _updateUnavailable, _online, _offline,
	_inCache, _notInCache, _signed, _unsigned, _netNo, _netYes, 
	_moreIcon, _lessIcon;
    private Font _appTitleFont;
    private Font _defaultFont;
    
    //
    private JMenuItem _installMI;
    private JMenuItem _launchMI;
    private JMenuItem _removeMI;
    private boolean _removeMIAdded;
    private JMenu _appMenu;
    private JMenu _viewMenu;
    private JMenuItem _addEntryMI;
    private JMenuItem _editEntryMI;
    private JFrame _frame;
    private JViewport _iconViewport;
    
    private Vector _managerList;
    private int _selectedManager = 0;
    
    private static final int IMAGE_WIDTH = 32;
    private static final int IMAGE_HEIGHT = 32;

    private static Player _player;
 
    public static void main(String[] args) {
	
        SwingUtilities.invokeLater(new Runnable() {
		    public void run() {
			new Player();
		    }
		});
    }
    
    public static Player getPlayer() {
        return _player;
    }

 
    public Player() {
        _player = this;
	
	_bookmarkEntryList = ConfigProperties.getInstance().getBookmarks();

        _frame = new JFrame(Resources.getString("player.title"));
 
	// will only show is specified in config, also initializes logging.
	(new Console()).show(Resources.getString("player.title"));

        // We invoke this now to avoid the MediaTracker bogging down on
        // waiting for other images before loading the default
        getDefaultImage();
    
        _imageMap = new HashMap();
        
        _entryListener = new EntryHandler();
      
        // Create the default EntryManagers
	try {
	   
	    _cachedManager = new CacheEntryManager();
	 
	    _cachedManager.refresh();
	 
	} catch(Exception ioe) {
	    // Got an exception accessing the cache - Thus, it is likely to have been
	    // corruped. Show error dialog and offer to clean the cache
	    Object [] newArray = { Resources.getString("player.corruptedcache.msg") };
	    
	    Object [] opts = {
		Resources.getString("player.corruptedcache.exit"),
		    Resources.getString("player.corruptedcache.clearandexit")
	    };
	    
	    Frame parent = GeneralUtil.getActiveTopLevelFrame();
	    
	    int result = GeneralUtilities.showOptionDialog(parent, newArray,
			   Resources.getString("player.corruptedcache.title"),
			   JOptionPane.YES_NO_OPTION,
			   JOptionPane.QUESTION_MESSAGE, opts, opts[1]);
	    if (result == 1) {
		DiskCache dc = InstallCache.getDiskCache();
		dc.uninstallCache();
	    }
	    System.exit(0);
	}
	
        createComponents();

        layoutComponents();

        // Determine what EntryManager to initialy select
        int select = ConfigProperties.getPlayerManager();
	// removed favorites manager, so select should be either 0 (remote) 
	// or 1 (cached)
	if (select < 0 || select > 1) {
	    select = 1;
	}       
        if (select == 1 && _cachedManager.size() == 0) {
	    select = 0;
        }
	setSelectedManagerIndex(select);
        
        // Center and show the frame.
        _frame.pack();
        
        Rectangle bounds = _frame.getBounds();
        bounds.width += 32;
        if (bounds.width < 400) {
	    bounds.width = 400;
        }


	
        Dimension ssSize = Toolkit.getDefaultToolkit().getScreenSize();
        bounds.x = (ssSize.width - bounds.width) / 2;
        bounds.y = (ssSize.height - bounds.height) / 2;
	Rectangle savedBounds = ConfigProperties.getPlayerBounds();
	if (savedBounds != null) {
	    bounds = savedBounds;
	}
        _frame.setBounds(bounds);

        SplashScreen.hide();
        _frame.show();

        /**
	 * We initialize the dialog to pop up for user authentication
	 * to password prompted URLs.
	 */
        JAuthenticator ja = JAuthenticator.getInstance(_frame);
        Authenticator.setDefault(ja);

    }

    /**
     * Sets the current EntryManager 
     */
    void setSelectedManagerIndex(int selection) {
	if (selection < _managerList.size()) {
	    _selectedManager = selection;
	    ManagerListEntry entry = 
	    	(ManagerListEntry) _managerList.elementAt(_selectedManager);
	    _viewNameLabel.setText(entry.getManagerName());
	    updateEntryManager();	// will get to setCurrentManager
	}
    }

    
    /**
     * Sets the current EntryManager. Will update the display accordingly.
     */
    private void setCurrentManager(EntryManager manager) {
        if (_currentManager != manager) {
	    if (_currentManager != null) {
		_currentManager.removeChangeListener(_entryListener);
		_currentManager.removePropertyChangeListener(_entryListener);
		_currentManager.save();
	    }
	    _currentManager = manager;
	    if (manager != null) {
		if (manager.size() > 0) {
		    manager.setSelectedIndex(0);
		}
		if (_entryListener == null) {
		    _entryListener = new EntryHandler();
		}
		manager.addChangeListener(_entryListener);
		manager.addPropertyChangeListener(_entryListener);
		
		updateStatusLabel();
		
		if (manager.canRemove()) {
		    _removeMI.setText(manager.getRemoveString());
		    _removeMI.setMnemonic(
			Resources.getVKCode("player.removeMnemonic"));
		    if (!_removeMIAdded) {
			_appMenu.insert(_removeMI, (_installMI != null) ? 3:2);
			_removeMIAdded = true;
		    }
		}
		else if (_removeMIAdded) {
		    _appMenu.remove(_removeMI);
		    _removeMIAdded = false;
		}
	    }
	    updateUI(true);
        }
    }
    
    /**
     * Returns the current EntryManager.
     */
    EntryManager getCurrentManager() {
        return _currentManager;
    }      

    // display only upto MAX_MENUITEM_LENGTH in the pull-down menu item
    private String getMenuItemString(String menuString) {
	if (menuString.length() > MAX_MENUITEM_LENGTH) {
	    menuString = menuString.substring(0, MAX_MENUITEM_LENGTH);
	    menuString = menuString + "...";
	}
	return menuString;
    }

    /**
     * Updates the UI based on the selected JNL file.
     */
    private void updateUI(boolean newIconList) {
        EntryManager manager = getCurrentManager();
        if (manager == null) {
	    return;
        }

	if (manager instanceof HTMLEntryManager) {	 
	    _viewMenu.getItem(ADDENTRY_VIEWMENU_ITEM_INDEX).setEnabled(true);
	} else {	   	 
	    _viewMenu.getItem(ADDENTRY_VIEWMENU_ITEM_INDEX).setEnabled(false);
	}

        JNLEntry e = manager.getSelectedEntry();
        JButton defButton = null;
        if (e == null) {
	    _applicationTitle.setText(" ");
	    _madeByInfoLabel.setText(null);
	    _homePageButton.setText(null);
	    _descriptionTextArea.setText("");
	    _removeMI.setEnabled(false);
	    _launchMI.setEnabled(false);
	    _launchMI.setText(Resources.getString("player.launchMenuItem",""));
	    _homePageButton.setEnabled(false);
	    _launchButton.setEnabled(false);
	    _launchButton.setText(
                Resources.getString("player.launchButton") );
	    _launchButton.setDefaultCapable(false);
	    Component focus = SwingUtilities.findFocusOwner(_iconPanel);
	    if (_installMI != null) {
		_installMI.setEnabled(false);
	    }
        }
        else {
	    _applicationTitle.setText(getMenuItemString(e.getTitle()));	  
            int ret = _appTitleFont.canDisplayUpTo(getMenuItemString(e.getTitle()));	   
            // javadoc says canDisplayUpTo returns -1, but returns length()
            boolean ok = (ret < 0 || ret >= getMenuItemString(e.getTitle()).length());	    
            _applicationTitle.setFont((ok) ? _appTitleFont : _defaultFont);
	
	    _madeByInfoLabel.setText(e.getVendor());
	    _homePageButton.setText(e.getHome());
	    _descriptionTextArea.setText(e.getDescription());
	    _removeMI.setEnabled(manager.canRemove());


	    boolean launchable = (!_isOffline || (e.canRunOffline() &&
						  e.isCached() &&
			        LaunchDownload.isInCache(
						  e.getLaunchDescriptor())
						  ));
	    _launchMI.setEnabled(launchable);
	    _launchButton.setText(Resources.getString("player.launchButton"));
	    _launchButton.setEnabled(launchable);
	    _launchButton.setDefaultCapable(launchable);
	    if (launchable) {	
	        _launchMI.setText(Resources.getString("player.launchMenuItem",
			getMenuItemString(e.getTitle())));
	        defButton = _launchButton;
	    } else {
		Component focus = SwingUtilities.
		    findFocusOwner(_iconPanel);
	    }
	    	 
	    _homePageButton.setEnabled(!_isOffline && (e.getHome() != null));
	    
	    // Update the installer menu item.
	    updateInstallMI(e);
        }
        updateStatusLabel();
        if (newIconList) {
	    int selIndex = Math.max(0, manager.getSelectedIndex());
	    Vector iconCellList = new Vector();
	    int size = manager.size();
	    for (int listIndex=0; listIndex<size; listIndex++) {
		String name = manager.get(listIndex).getTitle();
		Image image = getImage(listIndex);
		ImageIcon icon = null;
		if (image != null) {
		    icon = new ImageIcon(image, name);
		}
		iconCellList.addElement(new IconCell(name, new ScaledImageIcon(
							 icon, IMAGE_WIDTH, IMAGE_HEIGHT)));
	    }
	    _iconList.setList(iconCellList, selIndex);
        } else {
	    _iconList.setSelectedIndex(manager.getSelectedIndex());
        }
        _appIndicatorPanel.setIcons(e);
        _frame.getRootPane().setDefaultButton(defButton);
    }

    private void updateInstallMI(JNLEntry entry) { 
        if (entry != null && _installMI != null) {
            LocalApplicationProperties lap = 
		entry.getLocalApplicationProperties();
            boolean enable = false;
            if (lap != null) {
                lap.refreshIfNecessary();
                if (lap.isLocallyInstalled()) {
                    _installMI.setText(
                        Resources.getString("player.uninstallMenuItem"));
                    _installMI.setMnemonic(
                        Resources.getVKCode("player.uninstallMnemonic"));
                } else {
                    _installMI.setText(
                        Resources.getString("player.installMenuItem"));
                    _installMI.setMnemonic(
                        Resources.getVKCode("player.installMnemonic"));
                }
                if (_installer == null || !_installer.isInstalling()) {
                    try {
                        String path;
                        path = lap.getIndirectLauchFilePath(true);
                        enable = true;
                    } catch (IOException ioe) {
                        enable = false; // no local jnlp file
                    }
                }
            }
            _installMI.setEnabled(enable);
	}
    }

    
    private class ScaledImageIcon implements Icon {
        ImageIcon _imageIcon;
        int _width;
        int _height;
	
        public ScaledImageIcon(ImageIcon imageIcon, int width, int height) {
	    _imageIcon = imageIcon;
	    _width = width;
	    _height = height;
        }
	
        public int getIconWidth() {
	    return _width;
        }
	
        public int getIconHeight() {
	    return _height;
        }
	
        public void paintIcon(Component c, Graphics g, int x, int y) {
	    g.drawImage(_imageIcon.getImage(), x , y, _width, _height, c);
        }
    }
    
    /**
     * Updates the status label. The status label shows the status of
     * the selected EntryManager (either loading or loaded).
     */
    private void updateStatusLabel() {
        EntryManager manager = getCurrentManager();
	
        if (_installer != null && _installer.isInstalling()) {
	    JNLEntry e;
	    LocalApplicationProperties lap;
	    String labelText = null;
	    if ((manager != null) &&
		    ((e = manager.getSelectedEntry()) != null) &&
		    ((lap = e.getLocalApplicationProperties()) != null)) {
		if (lap.isLocallyInstalled()) {
		    labelText = Resources.getString("player.uninstallingLabel");
		} else {
		    labelText = Resources.getString("player.installingLabel");
		}
	    }
	    
	    // if still installing ...
	    if (_installer.isInstalling()) {
		_statusLabel.setText(labelText);
	    }
	    
        }
        else {
	    if (manager != null) {
		if (manager.getState() == 0) {
		    _statusLabel.setText(
			Resources.getString("player.loadedLabel"));
		}
		else {
		    _statusLabel.setText(
			Resources.getString("player.loadingLabel"));
		}
	    }
        }
    }
    
    /**
     * Returns the image for the given string, or null.
     */
    private Image doLoadImage(String path) {
        MediaTracker tracker = new MediaTracker(_frame);
        Image image = Toolkit.getDefaultToolkit().getImage
	    (Player.class.getResource(path));
        tracker.addImage(image, 0);
        try {
	    tracker.waitForID(0, 0);
        } catch (InterruptedException e) {
	    Debug.ignoredException(e);
	    image = null;
        }
        if (image != null) {
	    if (tracker.statusID(0, false) != MediaTracker.COMPLETE) {
		image = null;
	    } else {
		tracker.removeImage(image, 0);
	    }
        }
        return image;
    }
    
    /**
     * Returns the default image, creating if necessary.
     */
    private Image getDefaultImage() {
        if (!_loadedDefaultImage) {
	    _defaultImage = doLoadImage("resources/defaultIcon.gif");
	    _loadedDefaultImage = true;
        }
        return _defaultImage;
    }
    
    
    /**
     * Returns the image for the JNLEntry at the specified index. If the
     * image hasn't been loaded yet this will NOT block the receiver. Instead
     * it will return the default image and message the ImageLoader to load
     * the image. Alternatively if the image was previously asked for,
     * and couldn't be loaded, this will return the default image.
     */
    private Image getImage(int index) {
        JNLEntry entry = getCurrentManager().get(index);
        IconDesc id = entry.getImageLocation();
        
        if (id != null) {
	    Object image = _imageMap.get(id);
	    
	    if (image == null && !_isOffline) {
		_imageMap.put(id, LOADING_OBJECT);
		CacheImageLoader.getInstance().loadImage(id, this);
		if (Globals.TracePlayerImageLoading) {
		    Debug.println("starting loading of image from " + id);
		}
	    }
	    else if (image instanceof Image) {
		return (Image)image;
	    }
        }
        return getDefaultImage();
    }

    // implementation of the two methods in CachedImageLoaderCallback:
    public void imageAvailable(IconDesc id, Image image, File file) {
        Player.this.finishedLoadingImage(id, image);
    }

    public void finalImageAvailable(IconDesc id, Image image, File file) {
    }


    
    /**
     * Invoked when an image has finished loading. This will redisplay
     * the IconDisplay.
     */
    private void finishedLoadingImage(IconDesc id, Image image) {
        _imageMap.put(id, image);
        getCurrentManager().setSelectedIndex(0);
        updateUI(true);
    }
    
    /**
     * Invoked when the set of JNLEntries the current EntryManager is
     * maintaining changes.
     */
    private void entriesChanged() {
        updateUI(true);
    }
    
    /**
     * Invoked when the selected changes. Will redisplay the necessary
     * UI widgets.
     */
    private void selectionChanged() {
        updateUI(false);
    }
    
    /**
     * Resets the current EntryManager based on _selectedManager
     * This gets the EntryManager from the objects client property
     * ENTRY_CLIENT_PROPERTY. If this is null, then
     * ENTRY_CLASS_CLIENT_PROPERTY is checked. The named class is then
     * instantiated and that instance is then set as the client property
     * ENTRY_CLIENT_PROPERTY. Once the EntryManager has been obtained,
     * setCurrentManager is invoked.
     */
    private void updateEntryManager() {
      
	ManagerListEntry entry = null;
	if (_selectedManager < _managerList.size()) {
	    entry = (ManagerListEntry) _managerList.elementAt(_selectedManager);
	}
	if (entry != null) {
	    EntryManager newManager = entry.getManager();
		
	    if (newManager == null) {
	        String managerClassName = entry.getManagerClassName();
	        try {
		    newManager = (EntryManager)Class.forName
		        (managerClassName).newInstance();
	        } catch (ClassNotFoundException cnfe) {
		    Debug.ignoredException(cnfe);
	        } catch (IllegalAccessException iae) {
		    Debug.ignoredException(iae);
	        } catch (InstantiationException ie) {
		    Debug.ignoredException(ie);
	        }
	        entry.setManager(newManager);
	        newManager.refresh();
	    }
	    if (newManager != getCurrentManager()) {
	        setCurrentManager(newManager);
	    }
	    if (_currentManager instanceof HTMLEntryManager) {
		// ensure the combo box display the current loaded URL	
		_remote.setSelectedItem(((HTMLEntryManager)_currentManager).getURL().toString());	 
	    } else {
		_remote.setSelectedItem("");
	    }
    	}
    }
    
    /**
     * Launches the selected JNLEntry.
     */
    private void launch() {
        JNLEntry entry = getCurrentManager().getSelectedEntry();
        if (entry != null) {
	    entry.launch();
	    _appIndicatorPanel.launched();
        }
    }
    
    /**
     * Removes the selected JNLEntry from the current EntryManager.
     */
    private void remove() {
        if (getCurrentManager() == _cachedManager) {
	    JNLEntry se = getCurrentManager().getSelectedEntry();
	    
        }
        getCurrentManager().remove(getCurrentManager().getSelectedIndex());
    }

    /**
     * Exists the app. This should be invoked instead of System.exit to
     * do the necessary clean up.
     */
    private void exit() {
        getCurrentManager().save();
        Rectangle rect = _frame.getBounds();

	// store the remote entry in the combo box (history)
	ConfigProperties.getInstance().setRemoteURLs(_remote.getRemoteURLs());

	// store the html entries
	ConfigProperties.getInstance().setBookmarks(_bookmarkEntryList);

        ConfigProperties.setPlayerBounds(rect);
        ConfigProperties.setPlayerManager(_selectedManager);
        ConfigProperties.setPlayerMode((_moreDroppedDown) ? 1 : 0);
        ConfigProperties.getInstance().store();
        System.exit(0);
    }
    
    //
    // Methods called in laying out the components
    //
    
    private void createComponents() {
        _frame.addWindowListener(new WindowAdapter() {
	    public void windowClosing(WindowEvent we) {
		exit();
	    }
	});
        
        _viewPanel = createGroupPanel();
        _iconPanel = createIconPanel();			   
        _buttonPanel = createButtonPanel();
        _bottomPanel = createBottomPanel();

	_urlPanel = createUrlPanel();

	setCompactMode();
	
        
        _applicationsLabel.setLabelFor(_iconList);
	
	_urlLabel.setLabelFor(_remote);

        createMenuBar();
    }
    
    // File 		- Preferences, Exit
    // Application	- Start, Remove from Cache
    // View 		- Remote, Downloaded
    // Help 		- About, Check for Update
    private void createMenuBar() {
        JMenuBar mb = new JMenuBar();
        JMenu menu = mb.add(new JMenu(Resources.getString("player.fileMenu")));
        menu.setMnemonic(Resources.getVKCode("player.fileMnemonic"));
        JMenuItem mi;
        
        mi = menu.add(Resources.getString("player.preferencesMenuItem"));
        mi.setMnemonic(Resources.getVKCode("player.preferencesMnemonic"));
        mi.addActionListener(new ActionListener() {
		    public void actionPerformed(ActionEvent e) {
			ConfigProperties cp = ConfigProperties.getInstance();
			cp.refreshIfNecessary();
			// Provide closure to update table, when window closes.
			if (_prefsPanel == null) {
			    _prefsPanel = new PrefsPanel(_frame, false);
			    _prefsPanel.addWindowListener(new WindowAdapter() {
					public void windowClosed(WindowEvent we) {
					    _prefsPanel = null;
					    getCurrentManager().refresh();
					    updateStatusLabel();
					}
				    });

			    // to work around swing bug #4189244, instead of
			    // just _prefsPanel.show(); - we wait a bit first:
			    // fix for 4636963
                	    javax.swing.Timer t = new javax.swing.Timer(20, new ActionListener() {
                    	        public void actionPerformed(ActionEvent ae) {
                        	    Player.this._prefsPanel.show();
				    // fix for bug #4385871: repaint ...
                        	    Player.this._prefsPanel.repaint();
                    	        }
                	    });
                	    t.setRepeats(false);
                	    t.start();
                        } else {
				GeneralUtilities.placeWindow(_prefsPanel);
			}
                        _prefsPanel.toFront();
                    }
                });
        // See if preference panel should be enabled
        mi.setEnabled(true);
        
        menu.addSeparator();
        
        mi = menu.add(Resources.getString("player.exitMenuItem"));
        mi.setMnemonic(Resources.getVKCode("player.exitMnemonic"));
        mi.addActionListener(new ActionListener() {
		    public void actionPerformed(ActionEvent e) {
			exit();
		    }
		});
        
        menu = mb.add(new JMenu(
			  Resources.getString("player.applicationMenu")));
        menu.setMnemonic(Resources.getVKCode("player.applicationMnemonic"));
        _appMenu = menu;
	_appMenu.addActionListener(new ActionListener() {
                    public void actionPerformed(ActionEvent ae) {
                        // shortcuts may be installed asynchronously, ...
			EntryManager manager = getCurrentManager();
			if (manager != null) {
			    updateInstallMI(manager.getSelectedEntry());
			}
                    }
               });
	
        _launchMI = menu.add(Resources.getString("player.launchMenuItem",""));
        _launchMI.setMnemonic(Resources.getVKCode("player.launchMnemonic"));


        menu.addSeparator();
        // Only add the install/uninstall menu item if the platform
        // supports it.
        LocalInstallHandler lih = LocalInstallHandler.getInstance();
        if (lih != null && lih.isLocalInstallSupported()) {
	    _installMI = menu.add(
	        Resources.getString("player.installMenuItem"));
	    _installMI.setMnemonic(
	    Resources.getVKCode("player.installMnemonic"));
	    _installMI.addActionListener(new ActionListener() {
		     public void actionPerformed(ActionEvent ae) {
		         installOrUninstall();
		     }
		});
        }
        // If you change order, also upate setCurrentManager
        _removeMI = menu.add("");
        _removeMIAdded = true;
	      	
        _launchMI.addActionListener(new ActionListener() {
		    public void actionPerformed(ActionEvent ae) {
			launch();
		    }
		});
        _removeMI.addActionListener(new ActionListener() {
		    public void actionPerformed(ActionEvent ae) {
			remove();
		    }
		});    
        
        menu = mb.add(new JMenu(
			  Resources.getString("player.viewMenu")));
        menu.setMnemonic(Resources.getVKCode("player.viewMnemonic"));
	_viewMenu = menu;

	_addEntryMI = menu.add(Resources.getString("player.addEntryMenuItem"));
	_addEntryMI.setMnemonic(Resources.getVKCode("player.addEntryMnemonic"));
	_addEntryMI.addActionListener(new ActionListener() {
	    public void actionPerformed(ActionEvent ae) {
		addBookmarkEntry();
	    }
	});
	_editEntryMI = menu.add(Resources.getString("player.editEntryMenuItem"));
	_editEntryMI.setMnemonic(Resources.getVKCode("player.editEntryMnemonic"));
	_editEntryMI.addActionListener(new ActionListener() {
	    public void actionPerformed(ActionEvent ae) {
		editBookmarkEntry();
	    }
	});
	menu.addSeparator();

	// add the Downloaded Applications entry
	ManagerListEntry mle = (ManagerListEntry)(_managerList.get(1));
	addViewMenuItem(mle);

	// add all the html entry loaded
	loadViewMenuBookmarkEntries();

        menu = mb.add(new JMenu(Resources.getString("player.helpMenu")));
        menu.setMnemonic(Resources.getVKCode("player.helpMnemonic"));
        mi = menu.add(Resources.getString("player.aboutMenuItem"));
        mi.setMnemonic(Resources.getVKCode("player.aboutMnemonic"));
        mi.addActionListener(new ActionListener() {
	    public void actionPerformed(ActionEvent e) {
		String buildID = Globals.getBuildID();
		GeneralUtilities.showAboutDialog(_frame,
	      	    Resources.getString("player.aboutMessageTitle"));
	    }
	});
	
        _frame.setJMenuBar(mb);
    }
    
    /**
     * This is invoked from the install/uninstall menu item. If the
     * selected application is currently installed, this should uninstall
     * it, otherwise it should install it.
     */
    private void installOrUninstall() {
        if (_installer == null) {
	    _installer = new InstallRunnable();
        }
        _installer.installOrUninstall(getCurrentManager().getSelectedEntry());
        updateUI(false);
    }
    
    /**
     * Invoked when the installer has finished installing something.
     */
    private void installCompleted() {
        updateUI(false);
    }
    
    private void layoutComponents() {
        //
        JPanel parent = new JPanel(new GridBagLayout());
        _frame.getContentPane().add(parent);
        GridBagConstraints cons = new GridBagConstraints();
        
        cons.gridx = 0;
        cons.weightx = 1;
        cons.fill = GridBagConstraints.HORIZONTAL;
        cons.anchor = GridBagConstraints.NORTHEAST;
	
	cons.gridy = 0;
        cons.weighty = 0;
        parent.add(_urlPanel, cons);

        //
        cons.gridy = 1;
        cons.weighty = 0;
        parent.add(_viewPanel, cons);
	
        //
        cons.gridy = 2;
        cons.weighty = 0;
        parent.add(_iconPanel, cons);
        
        //
        cons.gridy = 3;
        cons.weighty = 0;
        parent.add(_buttonPanel, cons);
	
        //
        cons.gridy = 4;
        cons.weighty = 0;
        parent.add(_bottomPanel, cons);
    }

    private JComponent createUrlPanel() {
	_urlLabel = new PlayerLabel(Resources.getString("player.locationLabel"));	
	_urlLabel.setDisplayedMnemonic(Resources.getVKCode("player.locationLabelMnemonic"));
	_remote = new RemoteURL();

	JPanel parent = new JPanel(new BorderLayout());
	_remote.setBorder(BorderFactory.createEmptyBorder(0,5,0,0));
	parent.add(_urlLabel, BorderLayout.WEST);
	parent.add(_remote, BorderLayout.CENTER);
	
	parent.setBorder(BorderFactory.createEmptyBorder(2,8,2,8));

	return parent;
    }
    
    private JComponent createGroupPanel() {
        JPanel parent = new JPanel(new BorderLayout());
	
        _applicationsLabel = new PlayerLabel(
	    Resources.getString("player.applicationsLabel"));
        _applicationsLabel.setDisplayedMnemonic(
	    Resources.getVKCode("player.applicationsLabelMnemonic"));

	_viewNameLabel = new PlayerLabel("");
	
        _managerList = new Vector();

        addManager(
	    Resources.getString("player.remoteApplications"),
	    null, "com.sun.javaws.ui.player.HTMLEntryManager",
	    Resources.getString("player.remoteMenuItem"), 
	    Resources.getVKCode("player.remoteMenuMnemonic"));
	
        addManager(
	    Resources.getString("player.cachedApplications"),
	    _cachedManager, null,
	    Resources.getString("player.localMenuItem"), 
	    Resources.getVKCode("player.localMenuMnemonic"));
	

        _systemIndicatorPanel = new JPanel() {
	    public Dimension getMinimumSize() {
		return getPreferredSize();
	    }
	    public Dimension getPreferredSize() {
		return new Dimension(40,20);
	    }
	    public void paint(Graphics g) {
		g.setColor(getBackground());
		g.fillRect(0,0,getWidth(),getHeight());
		Icon icon;
		if (_isOffline) {
		    icon = _offline;
		    this.setToolTipText(
			Resources.getString("player.notOnlineToolTipText"));
		} else {
		    icon = _online;
		    this.setToolTipText(
			Resources.getString("player.onlineToolTipText"));
		}
		if (icon != null) {
		    icon.paintIcon(this, g, 4, 2);
		}
	    }
	    public boolean isFocusTraversable() {
		return false;
	    }
	};
	
        _systemIndicatorPanel.addMouseListener(new MouseAdapter() {
            public void mouseClicked(MouseEvent e) {
                _isOffline = !_isOffline;
                _systemIndicatorPanel.repaint();
                updateUI(false);
            }
        });
	
        parent.add(_applicationsLabel, BorderLayout.WEST);
	_viewNameLabel.setBorder(BorderFactory.createEmptyBorder(0,10,0,10));
        parent.add(_viewNameLabel, BorderLayout.CENTER);
        parent.setBorder(BorderFactory.createEmptyBorder(2,12,2,12));
        return parent;
    }
    
    private class ManagerListEntry {
        String _managerName;
        EntryManager _manager;
        String _managerClassName;
	JMenuItem _menuItem;
	
        public ManagerListEntry(String managerName, EntryManager manager,
		String managerClassName, JMenuItem menuItem) {
	    _managerName = managerName;
	    _manager = manager;
	    _managerClassName = managerClassName;
	    _menuItem = menuItem;
        }
        public EntryManager getManager() {
	    return _manager;
        }
        public void setManager(EntryManager manager) {
	    _manager = manager;
        }
        public String getManagerClassName() {
	    return _managerClassName;
        }
	public String getManagerName() {
	    return _managerName;
	}
	public JMenuItem getMenuItem() {
	    return _menuItem;
	}
    }

    public void resetCacheEntryManager() {
	int len = _managerList.size();
        for (int i=0; i<len; i++) {
	    ManagerListEntry mle = (ManagerListEntry)(_managerList.get(i));
	    EntryManager manager = mle.getManager();
	    if (manager instanceof CacheEntryManager) {
		manager.refresh();
		break;
	    }
	}
    }
    
    public void resetHTMLEntryManager() {
        int len = _managerList.size();
        for (int i=0; i<len; i++) {
	    ManagerListEntry mle = (ManagerListEntry)(_managerList.get(i));
	    EntryManager manager = mle.getManager();
	    if (manager instanceof HTMLEntryManager) {
		mle.setManager(null);
		if (manager == getCurrentManager()) {
		    updateEntryManager();
		}
	    }
        }
    }
    
    private void addManager(String managerName, EntryManager manager, String 
	    entryManagerClassName, String menuItem, int mnemonic) {
	JMenuItem mi = new JMenuItem(menuItem);
	mi.setMnemonic(mnemonic);
	ManagerListEntry mle = new ManagerListEntry(managerName, manager, 
	                           entryManagerClassName, mi);
        _managerList.add(mle);
	if (_viewMenu != null) {
	    addViewMenuItem(mle);
	}
    }

    private void addViewMenuItem(ManagerListEntry entry) {
	if (_viewMenu != null) {	   
	    JMenuItem mi = (JMenuItem)(entry.getMenuItem());
	    _viewMenu.add(mi);	   	  
 	    mi.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
		    JMenuItem item = (JMenuItem) e.getSource();
        	    int len = _managerList.size();
        	    for (int i=0; i<len; i++) {
			ManagerListEntry mle = (ManagerListEntry)
				(_managerList.get(i));
			if (item.equals(mle.getMenuItem())) {
				setSelectedManagerIndex(i);
			}
		    }
		}
	    });
	}
    }
 
    
    
    private JComponent createIconPanel() {
        JPanel parent =  new JPanel(new BorderLayout());
        
        parent.setBorder(BorderFactory.createEmptyBorder(2,8,2,8));
	
        _iconList = new IconList();

	_iconList.addFocusListener(new FocusAdapter() {
	    public void focusGained(FocusEvent e) {
		_iconList.setFocus(true);
		int index = getCurrentManager().getSelectedIndex();
		if (index >= 0) {
		    //set selected border for index
		    _iconList.setIconBorder(index, true);
		}
            }
	    public void focusLost(FocusEvent e) {		   	
		_iconList.setFocus(false);
	        int index = getCurrentManager().getSelectedIndex();
		if (index >= 0) {
		    //unset selected border
		    _iconList.setIconBorder(index, false);
		}
	    }
	});
	
        _iconList.setVisibleRowCount(3);
        JScrollPane sp = new JScrollPane(_iconList,
					 ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED,
					 ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER) {
	    public Dimension getMinimumSize() {
		return getPreferredSize();
	    }
	    public Dimension getPreferredSize() {
		Insets insets = getInsets();
		Dimension ret = super.getPreferredSize();
		ret.height = insets.top + insets.bottom + 12 +
		    _iconList.getVisibleRowCount() *
		    _iconList.getFixedCellHeight();
		return ret;
	    }
        };
        sp.setViewportBorder(BorderFactory.createCompoundBorder(
				 BorderFactory.createEtchedBorder(),
				 BorderFactory.createEmptyBorder(4,4,4,4)));
	
        _iconViewport = sp.getViewport();
	sp.setBackground(_iconList.getBackground());
        parent.add(sp);
	
        _frame.addComponentListener(new ComponentAdapter() {
		    public void componentResized(ComponentEvent e) {
			int currentRows = _iconList.getVisibleRowCount();
			int fixed = _iconList.getFixedCellHeight();
			int height = _frame.getHeight();
			int prefHeight = (int)_frame.getPreferredSize().getHeight();                    if (height > prefHeight) {
			    int moreRows = (height - prefHeight) / fixed;
			    if (moreRows > 0) {
				_iconList.setVisibleRowCount(
				    currentRows + moreRows);
				_iconList.invalidate();
				_frame.validate();
			    }
			} else if (height < prefHeight) {
			    int lessRows = (prefHeight - height + fixed - 1) /
				fixed;
			    _iconList.setVisibleRowCount(Math.max
							     (1, currentRows - lessRows));
			    _iconList.invalidate();
			    _frame.validate();
			}
			int oldColumns = _iconList.getVisibleColumns();
			int newColumns = (_frame.getWidth() > 300) ? 2 : 1;
			if (oldColumns != newColumns) {
			    _iconList.setVisibleColumns(newColumns);
			}
			_iconList.invalidate();
			_frame.validate();
		    }
		} );
	
        return parent;
    }
    
    private JComponent createButtonPanel() {
        JPanel parent = new JPanel(new GridLayout(1,2));
        Border buttonBorder = BorderFactory.createCompoundBorder(
	    BorderFactory.createRaisedBevelBorder(),
	    BorderFactory.createEmptyBorder(2,8,2,8));
	
        _moreIcon = new ImageIcon(doLoadImage("resources/more.gif"));
        _lessIcon = new ImageIcon(doLoadImage("resources/less.gif"));
        Icon launchIcon = new ImageIcon( doLoadImage("resources/launch.gif"));
	
        _detailButton = new JButton(
	    Resources.getString("player.lessDetailButton"), _lessIcon);
        _detailButton.setMnemonic(
	    Resources.getVKCode("player.lessDetailMnemonic"));
	_detailButton.setHorizontalTextPosition(SwingConstants.LEADING);
        _detailButton.setEnabled(true);
	
	
        _miniButton = new JButton(Resources.getString("player.miniButton"));
        _miniButton.setMnemonic(
	    Resources.getVKCode("player.miniButtonMnemonic"));
	_miniButton.setHorizontalTextPosition(SwingConstants.LEADING);
        _miniButton.setEnabled(true);
	
        // ensure more and less buttons are the same size
	_detailButton.setText(
	    Resources.getString("player.moreDetailButton"));
        Dimension moreSize = _detailButton.getPreferredSize();
	_detailButton.setText(
	    Resources.getString("player.lessDetailButton"));
        Dimension lessSize = _detailButton.getPreferredSize();
        Dimension max = new Dimension(Math.max(moreSize.width, lessSize.width),
				      Math.max(moreSize.height, lessSize.height));
	Dimension miniSize = _miniButton.getPreferredSize();
        max = new Dimension(Math.max(miniSize.width, max.width),
                                      Math.max(miniSize.height, max.height));

        _detailButton.setPreferredSize(max);
        _miniButton.setPreferredSize(max);

	int mode = ConfigProperties.getPlayerMode();
        _moreDroppedDown = true;
	if (mode >= 0 && (mode & 1) == 0) {
	    _moreDroppedDown = false;
	}

        _detailButton.addActionListener(new ActionListener() {
		    public void actionPerformed(ActionEvent e) {
			Rectangle bounds  = _frame.getBounds();
			int height;
			int rows = _iconList.getVisibleRowCount();
			Dimension oldPref = _frame.getPreferredSize();
			
			if (_moreDroppedDown) {
                            _moreDroppedDown = false;
			    setCompactMode();
                            Dimension newPref = _frame.getPreferredSize();
                            Dimension newMin = _frame.getMinimumSize();
                            height = oldPref.height - newPref.height;
                            if (bounds.height - height > newMin.height) {
                                bounds.height -= height;
                            } else {
                                bounds.height = newMin.height;
                            }      
                            _frame.setBounds(bounds);
			} else {
			    _moreDroppedDown = true;
			    setCompactMode();
			    Dimension newPref = _frame.getPreferredSize();
			    height = newPref.height - oldPref.height;
			    if (bounds.height < newPref.height) {
				if (bounds.height + height < newPref.height) {
				    bounds.height += height;
				} else {
				    bounds.height = newPref.height;
				}
				_frame.setBounds(bounds);
			    }
			}
		    }
		});
	
        _miniButton.addActionListener(new ActionListener() {
		    public void actionPerformed(ActionEvent e) {
			Rectangle bounds  = _frame.getBounds();
			int rows = _iconList.getVisibleRowCount();
			int height;
			Dimension oldPref = _frame.getPreferredSize();
			
			_detailButton.setEnabled(true);
			if (_moreDroppedDown) {
			    _infoPanel.setVisible(false);
			    Dimension newPref = _frame.getPreferredSize();
			    Dimension newMin = _frame.getMinimumSize();
			    height = oldPref.height - newPref.height;
			    if (bounds.height - height > newMin.height) {
				bounds.height -= height;
			    } else {
				bounds.height = newMin.height;
			    }
			    _frame.setBounds(bounds);
			    _moreDroppedDown = false;
			    if (rows <= 1) {
				_miniButton.setEnabled(true);
			    }
			} else {
			    // Roll up to one row of Icons ...
			    _iconList.setVisibleRowCount(1);
			    Dimension newPref = _frame.getPreferredSize();
			    
			    bounds.height = newPref.height;
			    _frame.setBounds(bounds);
			}
                    }
                });
        
        _launchButton = new JButton(
            Resources.getString("player.launchButton"), launchIcon) {
            public Dimension getPreferredSize() {
                Dimension size = super.getPreferredSize();
                int maxW = getParent().getWidth();
                if (size.width > maxW) { size.width = maxW; };
                return size;
            }
	};
        _launchButton.setHorizontalTextPosition(SwingConstants.LEADING);
	_launchButton.setToolTipText(
	    Resources.getString("player.launchTooltipText"));
	
	
        _launchButton.addActionListener(new ActionListener() {
		    public void actionPerformed(ActionEvent e) {
			launch();
		    }
		});
	
        JPanel moreOrLessPanel = new JPanel(new BorderLayout());
        moreOrLessPanel.add(_detailButton, BorderLayout.WEST);
/*
        moreOrLessPanel.add(_miniButton, BorderLayout.EAST);
*/
	JPanel launchPanel = new JPanel(new BorderLayout());
	launchPanel.add(_launchButton, BorderLayout.EAST);
        parent.add(moreOrLessPanel);
        parent.add(launchPanel);
	
        parent.setBorder(new EmptyBorder(2, 8, 2, 8));
	
        return parent;
    }
    
    private void setCompactMode() {
        if (_moreDroppedDown) {
	    _detailButton.setText(
	        Resources.getString("player.lessDetailButton"));
	    _detailButton.setIcon(_lessIcon);
	    _detailButton.setMnemonic(
	        Resources.getVKCode("player.lessDetailMnemonic"));
	    _infoPanel.setVisible(true);
        } else {
	    _detailButton.setText(
	        Resources.getString("player.moreDetailButton"));
	    _detailButton.setIcon(_moreIcon);
	    _detailButton.setMnemonic(
	        Resources.getVKCode("player.moreDetailMnemonic"));
            _infoPanel.setVisible(false);
        }
    }

    ArrayList getBookmarkEntryList() {
	return _bookmarkEntryList;
    }

    int getBookmarkIndex(String bookmarkName) {
	for (int i = 0; i < _bookmarkEntryList.size(); i++) {
	    
	    BookmarkEntry e = (BookmarkEntry)_bookmarkEntryList.get(i);

	    if (e.getName().equals(bookmarkName)) {
		return i;
	    }
	}
	return -1;
    }


    // this must be called after the entry is in the entry table
    private void addViewMenuBookmarkEntry(String name) {
	JMenuItem mi = _viewMenu.add(getMenuItemString(name));
	mi.setActionCommand(name);

	mi.addActionListener(new ActionListener() {
	    public void actionPerformed(ActionEvent ae) {
		if (!(_currentManager instanceof HTMLEntryManager)) {
	
		    setSelectedManagerIndex(0);
		    
		}		
	
		String targetName = ae.getActionCommand();	
		int index = getBookmarkIndex(targetName);
		BookmarkEntry targetEntry = 
			(BookmarkEntry)_bookmarkEntryList.get(index);
		String targetURL = targetEntry.getUrl();
		String [] remoteURLs = _remote.getRemoteURLs();
	
		// the target page is the current page
		if (remoteURLs[0].equals(targetURL)) return;

		int len = ConfigProperties.getMaxUrlsLength();

		String [] newRemoteURLs = new String[len];

		// regenerate the remoteURLs array
		// shift up all entries and replace the first entry
		// with targetURL - so it will be loaded by the player
		for (int i = 0; i < len - 1; i++) {
		    if (remoteURLs[i]==null) break;
		   
		    if (remoteURLs[i].equals(targetURL)) {
			newRemoteURLs[0]=targetURL;
		    } else {
		    remoteURLs[i] = newRemoteURLs[i+1];		
		    }
		}

		newRemoteURLs[0] = targetURL;
	
	
		ConfigProperties.getInstance().setRemoteURLs(newRemoteURLs);
		Player.getPlayer().resetHTMLEntryManager();
		// need to update the combo box too
		_remote.setSelectedItem(targetURL);
		
	
	    }
	});
    }

    private String getURLTitle(URL url) {
	InputStream is = null;
	String title = null;
	try {
	    HttpRequest httpreq = JavawsFactory.getHttpRequestImpl(); 
	    HttpResponse response = httpreq.doGetRequest(url);
	    is = response.getInputStream(); 
            InputStreamReader isr = new InputStreamReader(is);
	    BufferedReader br = new BufferedReader (isr);
	    String string = null;
	    while ((string = br.readLine()) != null){
	
		int start = string.toUpperCase().indexOf("<TITLE>");
		int end = string.toUpperCase().indexOf("</TITLE>");
		if (start != -1 && end != -1) {
		    // 7 is length of "<TITLE>"
		    title = string.substring(start + 7, end);
		    break;
		}
	    }
	} catch (IOException ioe) { 
	    Debug.ignoredException(ioe);
	}  
	return title;
    }

    private void addBookmarkEntry() {

	URL currURL = ((HTMLEntryManager)_currentManager).getURL();
	 

	String urlTitle = getURLTitle(currURL);

	if (urlTitle == null) urlTitle = currURL.toString();
	String defaultTitle = urlTitle;

	boolean check = true;

	while (check) {
	   
	    // pop up dialog to ask user for entry name
	    // defaults to <TITLE> of the html page, if that does not exist
	    // than default to the URL of the html page
	    urlTitle = (String)GeneralUtilities.showInputDialog(_frame, 
			Resources.getString("player.addHtmlEntryMsg"), 
			Resources.getString("player.addHtmlEntry"), 
			urlTitle);
	   
	    if (urlTitle == null) {
		// user hit canel
		return;
	    } else if (urlTitle.trim().equals("")) {
		// If the user put in a empty string or
		// a string with no character at all, use default value and 
		// ask for input again
		showInvalidInputDialog(_frame, Resources.getString("player.addHtmlEntryMissingName"), Resources.getString("player.addHtmlEntry"));		
		urlTitle = defaultTitle;
	    } else {
		check = false;
	    }

	}



	boolean exist = addBookmarkEntryToTable(urlTitle, currURL.toString());

	if (exist) return;

	addViewMenuBookmarkEntry(urlTitle);

    }

    private void loadViewMenuBookmarkEntries() {
	for (int i = 0; i < _bookmarkEntryList.size(); i++) {
	    BookmarkEntry e = (BookmarkEntry)_bookmarkEntryList.get(i);
	    addViewMenuBookmarkEntry(e.getName());
	}
    }


    private void editBookmarkEntry() {

	BookmarkEntryEditor hee = new BookmarkEntryEditor(_frame);

	// reload view menu entries
	while (_viewMenu.getItemCount() > DEFAULT_VIEWMENU_ITEM_COUNT) {
	    _viewMenu.remove(DEFAULT_VIEWMENU_ITEM_COUNT);
	}

	loadViewMenuBookmarkEntries();

    }


    // returns true if entry already exist in the table
    // will overwrite it if user wants to
    private boolean addBookmarkEntryToTable(String name, String url) {

	BookmarkEntry e = new BookmarkEntry(name, url);
	int index = getBookmarkIndex(name);
	if (index != -1) {
	    if (Globals.TracePlayer) {
		Debug.println("bookmark entry already exist");
	    }
	    int result = GeneralUtilities.showOptionDialog(_frame, 
		Resources.getString("player.addHtmlEntryExist"), 
		Resources.getString("player.addHtmlEntry"), 
		JOptionPane.YES_NO_OPTION, JOptionPane.QUESTION_MESSAGE);

	    if(result == JOptionPane.YES_OPTION) {
		// overwrite the entry
		if (Globals.TracePlayer) {
		    Debug.println("overwriting entry:");
		    Debug.println("name: " + name + " url: " + url);
		}	
		_bookmarkEntryList.set(index, e);	
	    } else if(result == JOptionPane.NO_OPTION) {
		// do not overwrite	
	    }
	    return true;
	}
	_bookmarkEntryList.add(e);
	return false;
    }

    static void showInvalidInputDialog(Component parent, String msg, 
				       String title) {
	GeneralUtilities.showOptionDialog(parent, msg, title, 
		   JOptionPane.DEFAULT_OPTION, JOptionPane.WARNING_MESSAGE);
    }

    private JComponent createBottomPanel() {
        JPanel parent = new JPanel(new BorderLayout());
        JPanel info = new JPanel(new GridBagLayout());
        _infoPanel = new JPanel(new BorderLayout());

        _applicationTitle = new PlayerLabel(" ");
	_defaultFont = _applicationTitle.getFont();
	_appTitleFont = new Font(_defaultFont.getFontName(), _defaultFont.getStyle(), 
		(125 * _defaultFont.getSize())/100);  // 25% larger

        _appIndicatorPanel = new IndicatorsPanel();
        _appIndicatorPanel.setBorder(BorderFactory.createEtchedBorder());
	
        _madeByLabel = new PlayerLabel(
	    Resources.getString("player.madeByLabel"));
        _madeByInfoLabel = new PlayerLabel();
        JPanel madeByInfoPanel = new JPanel(new BorderLayout());
	
        madeByInfoPanel.add(_madeByInfoLabel, BorderLayout.WEST);
	
        _homePageLabel = new PlayerLabel(
	    Resources.getString("player.homePageLabel"));
	_homePageLabel.setDisplayedMnemonic(
	    Resources.getVKCode("player.homePageLabelMnemonic"));
	_homePageLabel.setBorder(BorderFactory.createEmptyBorder(1,0,1,0));
	
        _homePageButton = new LinkButton();
	_homePageButton.addActionListener(new ActionListener() {
		    public void actionPerformed(ActionEvent ae) {
			JNLEntry entry = getCurrentManager().getSelectedEntry();
			if (entry != null) {
			    entry.showHome();
			}
		    }
		});
        _homePageButton.setToolTipText(
	    Resources.getString("player.homeTooltipText"));
        _homePageLabel.setLabelFor(_homePageButton);
	
	
        _descriptionLabel = new PlayerLabel(
	    Resources.getString("player.descriptionLabel"));
        _descriptionLabel.setVerticalAlignment(SwingConstants.TOP);
	
        _descriptionTextArea = new JTextArea(2, 80) {
	    public Dimension getMinimumSize() {
		Dimension ret = super.getMinimumSize();
		ret.height = getPreferredSize().height;
		return ret;
	    }
	    public Dimension getPreferredSize() {
		Dimension ret = super.getPreferredSize();
		ret.width = 10;  // don't let this component determine our width
		Dimension labelSize = _descriptionLabel.getPreferredSize();
		int max = labelSize.height * 2;
		if (ret.height > max) {
		    ret.height = max;
		}
		return ret;
	    }
	    public boolean isFocusTraversable() {
		return false;
	    }
	};
	_descriptionTextArea.setEditable(false);
	_descriptionTextArea.setOpaque(false);
	_descriptionTextArea.setLineWrap(true);
	_descriptionTextArea.setWrapStyleWord(true);
	
        GridBagConstraints cons = new GridBagConstraints();
	
        cons.insets.left = 4;
        cons.weighty = 0;
        
        cons.anchor = GridBagConstraints.CENTER;
        cons.gridx = 0;
        cons.weightx = 0;
        cons.gridy = 0;
        cons.fill = GridBagConstraints.NONE;
        cons.gridwidth = 2;
        cons.gridheight = 1;
        info.add(_applicationTitle, cons);


        cons.anchor = GridBagConstraints.WEST;
        cons.gridx = 0;
        cons.weightx = 0;
        cons.gridy = 1;
        cons.fill = GridBagConstraints.NONE;
        cons.gridwidth = 1;
        cons.gridheight = 1;
        info.add(_madeByLabel, cons);
	
        cons.anchor = GridBagConstraints.WEST;
        cons.gridx = 1;
        cons.weightx = 1;
        cons.gridy = 1;
        cons.fill = GridBagConstraints.HORIZONTAL;
        cons.gridwidth = GridBagConstraints.REMAINDER;
        cons.gridheight = 1;
        info.add(madeByInfoPanel, cons);
	
        cons.anchor = GridBagConstraints.WEST;
        cons.gridx = 0;
        cons.weightx = 0;
        cons.gridy = 2;
        cons.fill = GridBagConstraints.NONE;
        cons.gridwidth = 1;
        cons.gridheight = 1;
        info.add(_homePageLabel, cons);
	
        cons.anchor = GridBagConstraints.WEST;
        cons.gridx = 1;
        cons.weightx = 1;
        cons.gridy = 2;
        cons.fill = GridBagConstraints.NONE;
        cons.gridwidth = 1;
        cons.gridheight = 1;
        info.add(_homePageButton, cons);
	
        cons.anchor = GridBagConstraints.NORTHWEST;
        cons.gridx = 0;
        cons.weightx = 0;
        cons.gridy = 3;
        cons.fill = GridBagConstraints.NONE;
        cons.gridwidth = 1;
        cons.gridheight = 1;
        info.add(_descriptionLabel, cons);
	
        cons.anchor = GridBagConstraints.NORTHWEST;
        cons.gridx = 1;
        cons.weightx = 1;
        cons.gridy = 3;
        cons.gridwidth = GridBagConstraints.REMAINDER;
        cons.gridheight = 2;
        cons.fill = GridBagConstraints.BOTH;
        info.add(_descriptionTextArea, cons);
	
        cons.anchor = GridBagConstraints.NORTHWEST;
        cons.gridx = 1;
        cons.weightx = 1;
        cons.gridy = 5;
        cons.gridwidth = GridBagConstraints.REMAINDER;
        cons.gridheight = 1;
        cons.fill = GridBagConstraints.BOTH;
        info.add(_appIndicatorPanel, cons);
	
        _infoPanel.setBorder(BorderFactory.createCompoundBorder(
				 BorderFactory.createEmptyBorder(2,8,2,8),
				 BorderFactory.createEtchedBorder()));
	
        _infoPanel.add(info);
	
        _statusPanel = new JPanel(new BorderLayout());
        _statusLabel = new PlayerLabel();
        _statusLabel.setBorder(BorderFactory.createEmptyBorder(0,6,0,6));
        _statusPanel.add(_statusLabel, BorderLayout.EAST);
        _statusPanel.add(_systemIndicatorPanel, BorderLayout.WEST);
	
        _statusPanel.setBorder(BorderFactory.createCompoundBorder(
				   BorderFactory.createEmptyBorder(2,8,2,8),
				   BorderFactory.createEtchedBorder()));
	
        parent.add(_infoPanel, BorderLayout.NORTH);
        parent.add(_statusPanel, BorderLayout.SOUTH);
        parent.setVisible(true);
        return parent;
    }
    
    private class IconList extends JList implements KeyListener {
        int _rows;
        int _columns;
        int _entries;
        int _dummyEntries;
        int _cellHeight;
        int _visibleRows;
        int _visibleColumns;
        int _visibleEntries;
        int _selectedIndex;
	boolean _hasFocus;
        Vector _cells;
        Border _normalBorder, _selectedBorder;
        Color _normalBackground, _selectedBackground;
        Color _normalForeground, _selectedForeground;
        private static final int _heightBorder = 4;
        private static final int _heightPadding = 1;
        private static final int _widthBorder = 6;
        private static final int _widthPadding = 1;

	public void setFocus(boolean focus) {
	    _hasFocus = focus;
	}
	
        public IconList() {
	    _hasFocus = false;
	    _cells = null;
	    _rows = 1;
	    _columns = 1;
	    _entries = 0;
	    _dummyEntries = 0;
	    setFixedCellHeight(IMAGE_HEIGHT + (2 * _heightBorder) +
				   _heightPadding);
	    _visibleRows = 3;
	    _visibleColumns = 2;
	    _visibleEntries = 0;
	    addMouseListener(new MouseAdapter() {
			public void mousePressed(MouseEvent ev) {
			    int x = ev.getX();
			    int y = ev.getY();
			    int row = 0;
			    int col = 0;
			    int index = 0;
			    if (_entries > 0) {
				Component c = getComponent(0);
				if (c != null) {
				    int width = c.getWidth();
				    int height = c.getHeight();
				    if (width != 0) {
					col = x / width;
				    }
				    if (height != 0) {
					row = y / height;
				    }
				}
			    }
			    index = (row * _columns) + col;
			    
			    if (index >= 0) {
				IconList.this.requestFocus();
				getCurrentManager().setSelectedIndex(index);
			    }
			    setSelectedIndex(index);
			}
			
			public void mouseClicked(MouseEvent ev) {
			    int count = ev.getClickCount();
			    if ((count == 2) && (_launchButton != null) &&
				    (_launchButton.isEnabled()) ) {
				_launchButton.doClick();
			    }
			}
		    });
	    addKeyListener(this);
	    
	    _normalBorder = BorderFactory.createEmptyBorder(_heightBorder,
							    _widthBorder, _heightBorder, _widthBorder);
	    
	    _selectedBorder = BorderFactory.createCompoundBorder(
		UIManager.getBorder("List.focusCellHighlightBorder"),
		BorderFactory.createEmptyBorder(_heightBorder - 1,
						_widthBorder - 1, _heightBorder - 1, _widthBorder - 1));
	    
	    _normalBackground = UIManager.getColor("window");
	    _normalForeground = UIManager.getColor("textColor");
	    setBackground(_normalBackground);
	    
	    _selectedBackground = UIManager.getColor("textHighlight");
	    _selectedForeground = UIManager.getColor("textHighlightText");
        }

	// Fix for 4474783
	public int getScrollableUnitIncrement(Rectangle visibleRect, int orientation, int direction) {
	    return getFixedCellHeight();
	}
	
        public void keyTyped(KeyEvent e) {
        }
	
        public void keyReleased(KeyEvent e) {
        }
	
        public void keyPressed(KeyEvent e) {
	    int index = getCurrentManager().getSelectedIndex();
	    int row = 0;
	    int column = 0;
	    if (index >= 0) {
		row = index / _columns;
		column = index % _columns;
	    }
	    switch (e.getKeyCode()) {
		case KeyEvent.VK_RIGHT:
		    if (index  < _entries) {
			index++;
		    }
		    break;
		case KeyEvent.VK_LEFT:
		    if (index > 0) {
			index--;
		    }
		    break;
		case KeyEvent.VK_UP:
		    if (index >= _columns) {
			index -= _columns;
		    } else if (index > 0) {
			index--;
		    }
		    break;
		case KeyEvent.VK_DOWN:
		    if (index + _columns < _entries) {
			index += _columns;
		    } else if (index < _entries) {
			index++;
		    }
		    break;
		default:
		    break;
	    }
	    if (index != _selectedIndex &&
		index >= 0 &&
		index < _entries) {
		getCurrentManager().setSelectedIndex(index);
		setSelectedIndex(index);
	    }
        }
	
        public void setFixedCellHeight(int height) {
	    _cellHeight = height;
        }
        public int getFixedCellHeight() {
	    return _cellHeight;
        }
	
        public void setVisibleColumns(int columns) {
	    _visibleColumns = columns;
	    setList(_cells, _selectedIndex);
        }
        public int getVisibleColumns() {
	    return _visibleColumns;
        }
	
        public void setVisibleRowCount(int count) {
	    if (_entries > 0) {
		int newVisibleEntries = _visibleColumns * count;
		int oldVisibleEntries = _visibleEntries;
		int newDummyEntries = Math.max(0,
						   (newVisibleEntries - _entries));
		
		while (_dummyEntries < newDummyEntries) {
		    JPanel dummy = new JPanel();
		    dummy.setBackground(_normalBackground);
		    add(dummy);
		    _dummyEntries++;
		}
		while (_dummyEntries > newDummyEntries) {
		    int lastEntry = _entries + --_dummyEntries;
		    this.remove(lastEntry);
		}
		_visibleEntries = newVisibleEntries;
	    }
	    _visibleRows = count;
	    invalidate();
        }
        public int getVisibleRowCount() {
	    return _visibleRows;
        }
	
        public Dimension getPreferredSize() {
	    Dimension ret = super.getPreferredSize();
	    int rowsNeeded = _visibleRows - (_dummyEntries / _columns);
	    ret.height = Math.max(rowsNeeded, _rows) * _cellHeight;
	    return ret;
        }
	
        public Dimension getMinimumSize() {
	    Dimension ret = super.getMinimumSize();
	    ret.height = _cellHeight;
	    return ret;
        }
	
        public void setSelectedIndex(int index) {
	    if (index != _selectedIndex) {
		Component comp;
		if ((_selectedIndex >= 0) && (_selectedIndex < _entries)) {
		    comp = getComponent(_selectedIndex);
		    if (comp != null && comp instanceof JLabel) {
			JLabel oldSel = (JLabel) comp;
			oldSel.setBorder(_normalBorder);
			oldSel.setBackground(_normalBackground);
		    }
		}
		if ((index >=0) && (index <_entries)) {
		    comp = getComponent(index);
		    if (comp != null && comp instanceof JLabel) {
			
			// note: there is one remaining problem scrolling
			// the selected icon into view: on the Metal L&F only,
			// hitting up and down arrow keys moves the scrollbar
			// by the block increment.  After we have moved it to
			// the correct position (Dosen't happen on Windows L&F)
			Rectangle r = comp.getBounds();
			if (_iconViewport != null) {
			    r.width = 1;
			    r.x = 0;
			    r.y -= _iconViewport.getViewRect().getY();
			    _iconViewport.scrollRectToVisible(r);
			}
			
			JLabel newSel = (JLabel) comp;		
			if (_hasFocus) {
			    newSel.setBorder(_selectedBorder);
			} else {
			    newSel.setBorder(_normalBorder);
			}
		        newSel.setBackground(_selectedBackground);
		    }
		    _selectedIndex = index;
		} else {
		    _selectedIndex = -1;	// nothing selected
		}
	    }
	}

	public void setIconBorder(int index, boolean selected) {
	    // fix for 4674953
	    if (getComponent(index) instanceof JLabel) {
		JLabel newSel = (JLabel) getComponent(index);	 
		if (selected && newSel != null) {
		    newSel.setBorder(_selectedBorder);
		} else {
		    newSel.setBorder(_normalBorder);
		}	   
	    }
	}
	
        public void setList(Vector list, int index) {
	    _cells = list;
	    _selectedIndex = index;
	    removeAll();
	    _entries = _cells.size();
	    _dummyEntries = 0;
	    if (_entries > 0) {
		_columns = _visibleColumns;
		_rows = (_entries + (_columns - 1)) / _columns;
	    } else {
		_rows = _columns = 1;
	    }
	    setLayout(new IconGridLayout(0, _columns,getFixedCellHeight()));
	    int i;
	    for (i=0; i<_entries; i++) {
		IconCell cell = (IconCell) _cells.get(i);
		JLabel label = new JLabel(cell.getName(), cell.getIcon(),
					  SwingConstants.LEFT){
		    public Dimension getPreferredSize() {
			Dimension ret = super.getPreferredSize();
			if (ret.width > 100) {
			    ret.width = 100;
			}
			return ret;
		    }
		};
		label.setOpaque(true);
		label.setVerticalTextPosition(SwingConstants.CENTER);
		label.setBorder(_normalBorder);
		if (i == index) {	
		    label.setBackground(_selectedBackground);
		    label.setForeground(_selectedForeground);
		} else {		    
		    label.setBackground(_normalBackground);
		    label.setForeground(_normalForeground);
		}
		label.setHorizontalTextPosition(SwingConstants.TRAILING);
		label.setHorizontalAlignment(SwingConstants.LEADING);
		add(label);
		label.addKeyListener(this);
	    }
	    _visibleEntries = _visibleRows * _visibleColumns;
	    for (i=_entries; i<_visibleEntries; i++) {
		JPanel dummy = new JPanel();
		dummy.setBackground(_normalBackground);
		add(dummy);
		_dummyEntries++;
	    }
	    invalidate();
	    _frame.validate();
	}
	
	public boolean isFocusTraversable() {
	    return true;
	}
    }
    
    private class IconGridLayout extends GridLayout {
        public int _cellHeight;
	
        public IconGridLayout(int rows, int cols, int cellHeight) {
	    super(rows, cols);
	    _cellHeight = cellHeight;
        }
        public void setCellHeight( int cellHeight ) {
	    _cellHeight = cellHeight;
        }
        
        public void layoutContainer(Container parent) {
	    Insets insets = parent.getInsets();
	    int ncomponents = parent.getComponentCount();
	    int nrows = getRows();
	    int ncols = getColumns();
	    int hgap = getHgap();
	    int vgap = getVgap();
	    
	    if (ncomponents == 0) {
		return;
	    }
	    if (nrows > 0) {
		ncols = (ncomponents + nrows - 1) / nrows;
	    } else {
		nrows = (ncomponents + ncols - 1) / ncols;
	    }
	    int w = parent.getWidth() - (insets.left + insets.right);
	    int h = parent.getHeight() - (insets.top + insets.bottom);
	    w = (w - (ncols - 1) * hgap) / ncols;
	    h = _cellHeight;
	    
	    for (int c = 0, x = insets.left ; c < ncols ; c++, x += w + hgap) {
		for (int r = 0, y = insets.top; r < nrows; r++, y += h + vgap) {
		    int i = r * ncols + c;
		    if (i < ncomponents) {
			parent.getComponent(i).setBounds(x, y, w, h);
		    }
		}
	    }
        }
    }
    
    private class IconCell {
        Icon _icon;
        String _name;
        
        public IconCell() {
        }
        public IconCell(String name, Icon icon) {
	    _name = name;
	    _icon = icon;
        }
        public void setName(String name) {
	    _name = name;
        }
        public void setIcon(Icon icon) {
	    _icon = icon;
        }
        public String getName() {
	    return _name;
        }
        public Icon getIcon() {
	    return _icon;
        }
    }
    
    /**
     * Listener installed on the current EntryManager, will invoke
     * the appropriate method as the EntryManager changes.
     */
    private class EntryHandler implements ChangeListener,
        PropertyChangeListener {
        public void stateChanged(ChangeEvent e) {
	    entriesChanged();
        }
        public void propertyChange(PropertyChangeEvent e) {
	    if (e.getSource() != getCurrentManager()) {
		return;
	    }
	    String name = e.getPropertyName();
	    if ("selectedIndex".equals(name)) {
		selectionChanged();
	    }
	    else if ("state".equals(name)) {
		updateStatusLabel();
	    }
        }
    }
    
    
    /**
     * InstallRunnable is responsible for handling install/uninstall of
     * an application. The install is done on a separate thread.
     */
    private class InstallRunnable implements Runnable {
        private ArrayList _entries;
        /** This is true while the other Thread is running. */
        private boolean _running;
	
        public InstallRunnable() {
	    _entries = new ArrayList();
        }
	
        public void installOrUninstall(JNLEntry entry) {
	    LocalInstallHandler lih;
	    LocalApplicationProperties lap;
	    LaunchDesc ld;
	    
	    if (entry != null &&
		    (ld = entry.getLaunchDescriptor()) != null &&
		    (lap = entry.getLocalApplicationProperties()) != null &&
		    (lih = LocalInstallHandler.getInstance()) != null) {
		boolean create = false;
		synchronized(InstallRunnable.this) {
		    _entries.add(ld);
		    _entries.add(lap);
		    if (!_running) {
			_running = true;
			create = true;
		    }
		}
		if (create) {
		    new Thread(InstallRunnable.this).start();
		}
	    }
        }
	
        public boolean isInstalling() {
	    boolean installing = false;
	    synchronized(InstallRunnable.this) {
		installing = _running;
	    }
	    return installing;
        }
	
        public void run() {
	    boolean done = false;
	    LocalInstallHandler lih = LocalInstallHandler.getInstance();
	    
	    while (!done) {
		LocalApplicationProperties lap = null;
		LaunchDesc ld = null;
		
		synchronized(InstallRunnable.this) {
		    if (_entries.size() == 0) {
			done = true;
		    }
		    else {
			ld = (LaunchDesc)_entries.remove(0);
			lap = (LocalApplicationProperties)_entries.remove(0);
		    }
		}
		
		if (!done) {
		    lap.refreshIfNecessary();
		    if (lap.isLocallyInstalled()) {
			lih.uninstall(ld, lap);
			lap.setAskedForInstall(false);
		    }
		    else {
			lih.install(_frame, ld, lap);
			lap.setAskedForInstall(true);
		    }
		    try {
			lap.store();
		    } catch (IOException ioe) {
			Debug.ignoredException(ioe);
		    }
		    InstallRunnable.this.installCompleted();
		}
	    }
	    synchronized(InstallRunnable.this) {
		if (_entries.size() == 0) {
		    _running = false;
		}
	    }
	    // also needs to be run AFTER _running is turned off.
	    InstallRunnable.this.installCompleted();
        }
	
        private void installCompleted() {
	    SwingUtilities.invokeLater(new Runnable() {
			public void run() {
			    Player.this.installCompleted();
			}
		    });
        }
    }
    private class PlayerLabel extends JLabel {
        public PlayerLabel() {
	    super();
	    setForeground(UIManager.getColor("textText"));
        }
        public PlayerLabel(String text) {
	    super(text);
	    setForeground(UIManager.getColor("textText"));
        }
    }
    
    private class ArrowIcon implements Icon {
        final static int SIZE = 10;
        int _direction;
	
        public ArrowIcon(int direction) {
	    _direction = direction;
        }
	
        public int getIconHeight() { return SIZE; };
        public int getIconWidth() { return SIZE; };
	
        public void paintIcon(Component c, Graphics g, int x, int y) {
	    g.translate(x, y);
	    if (c.isEnabled()) {
		g.setColor(UIManager.getColor("textText"));
	    } else {
		g.setColor(UIManager.getColor("textInactiveText"));
	    }
	    switch (_direction) {
		
		case SwingConstants.EAST:
		    g.drawLine(2, 0, 2, 0 + (SIZE-1));
		    g.drawLine(3, 1, 3, 1 + (SIZE-3));
		    g.drawLine(4, 2, 4, 2 + (SIZE-5));
		    g.drawLine(5, 3, 5, 3 + (SIZE-7));
		    g.drawLine(6, 4, 6, 4 + (SIZE-9));
		    break;
		    
		case SwingConstants.WEST:
		    g.drawLine(7, 0, 7, 0 + (SIZE-1));
		    g.drawLine(6, 1, 6, 1 + (SIZE-3));
		    g.drawLine(5, 2, 5, 2 + (SIZE-5));
		    g.drawLine(4, 3, 4, 3 + (SIZE-7));
		    g.drawLine(3, 4, 3, 4 + (SIZE-9));
		    break;
		    
		case SwingConstants.NORTH:
		    g.drawLine(0, 7, 0 + (SIZE-1), 7);
		    g.drawLine(1, 6, 1 + (SIZE-3), 6);
		    g.drawLine(2, 5, 2 + (SIZE-5), 5);
		    g.drawLine(3, 4, 3 + (SIZE-7), 4);
		    g.drawLine(4, 3, 4 + (SIZE-9), 3);
		    break;
		    
		case SwingConstants.SOUTH:
		default:
		    g.drawLine(0, 2, 0 + (SIZE-1), 2);
		    g.drawLine(1, 3, 1 + (SIZE-3), 3);
		    g.drawLine(2, 4, 2 + (SIZE-5), 4);
		    g.drawLine(3, 5, 3 + (SIZE-7), 5);
		    g.drawLine(4, 6, 4 + (SIZE-9), 6);
		    break;
	    }
	    g.translate(-x, -y);
        }
    }
    
    private class IndicatorsPanel extends JComponent implements Accessible {
        private Icon _icons[];
        private String[] _toolTips;
        private String[] _unknownToolTips;
        private String[] _yesToolTips;
        private String[] _noToolTips;
        private LazyUpdateCheck _lazyUpdateCheck = null;
	private int _TOTALTOOLTIPWIDTH = 80;
	
        public IndicatorsPanel() {
	    _icons = new Icon[4];
	    _updateAvailable = loadIcon("resources/updateAvailable.gif");
	    _updateUnavailable = loadIcon("resources/updateUnavailable.gif");
	    _online = loadIcon("resources/online.gif");
	    _offline = loadIcon("resources/offline.gif");
	    _inCache = loadIcon("resources/inCache.gif");
	    _notInCache = loadIcon("resources/notInCache.gif");
	    _signed = loadIcon("resources/signed.gif");
	    _unsigned = loadIcon("resources/unsigned.gif");
	    _netYes = loadIcon("resources/netYes.gif");
	    _netNo = loadIcon("resources/netNo.gif");
	    
	    _toolTips = new String [4];
	    
	    _unknownToolTips = new String [4];
	    _unknownToolTips[0] =
		Resources.getString("player.updateIndicatorToolTipText");
	    _unknownToolTips[1] =
		Resources.getString("player.netNeededIndicatorToolTipText");
	    _unknownToolTips[2] =
		Resources.getString("player.cacheIndicatorToolTipText");
	    _unknownToolTips[3] =
		Resources.getString("player.securityIndicatorToolTipText");
	    
	    _yesToolTips = new String [4];
	    _yesToolTips[0] =
		Resources.getString("player.updateAvailableToolTipText");
	    _yesToolTips[1] =
		Resources.getString("player.netNeededToolTipText");
	    _yesToolTips[2] =
		Resources.getString("player.inCacheToolTipText");
	    _yesToolTips[3] =
		Resources.getString("player.signedToolTipText");
	    
	    _noToolTips = new String [4];
	    _noToolTips[0] =
		Resources.getString("player.updateNotAvailablerToolTipText");
	    _noToolTips[1] =
		Resources.getString("player.netNotNeededToolTipText");
	    _noToolTips[2] =
		Resources.getString("player.notInCacheToolTipText");
	    _noToolTips[3] =
		Resources.getString("player.notSignedToolTipText");
	    this.setToolTipText("");    // needed for some reason ?
        }
	
        public boolean isFocusTraversable() {
	    return false;
        }
	
	public Icon loadIcon(String path) {
	    return new ImageIcon(Toolkit.getDefaultToolkit().getImage(
				     Player.class.getResource(path)));
	}
	
        public void setIcon(int index, Icon icon) {
	    if (index >= 0 && index < 4) {
		_icons[index] = icon;
		if (icon == null) {
		    _toolTips[index] = _unknownToolTips[index];
		}
	    }
        }
	
        public void launched() {
	    // We might want to update the icons after the java just launched
	    // has had a chance to run, update the cache, and make
	    // updateAvailable no longer true:
	    (new IndicatorUpdateThread()).start();
	    // However, we don't know how long to wait, and we get no signal
	    // from the other VM that it's done loading, so we mearly change
	    // the state to: "don't know":
	    
	    // if there is an update available:
	    if (_icons[0] != null) {
		setIcon(0, null);       // we no longer know if updateAvailable
	    }
	    
	    // if we wern't allready in cache:
	    if (_icons[2] != _inCache) {
		setIcon(2, null);       // we no longer know if is in cache.
	    }
	    repaint();
        }
	
        public void paint(Graphics g) {
	    Dimension size = getSize();
	    int width = size.width/4;
	    int height = size.height;
	    int x,y;
	    int w,h;
	    int start = 0;
	    if (size.width > _TOTALTOOLTIPWIDTH) {
		width = 20;
                start = size.width - _TOTALTOOLTIPWIDTH;
	    }
	    g.setColor(UIManager.getColor("textText"));
	    for (int i=0; i<4; i++) {
		if (_icons[i] != null) {
		    x = start + 2 + (i * width);
		    y = 0;
		    if ((w = _icons[i].getIconWidth()) < width) {
			x += (width - w)/2;
		    }
		    if ((h = _icons[i].getIconHeight()) < height) {
			y += (height - h)/2;
		    }
		    _icons[i].paintIcon(this, g, x, y);
		}
	    }
        }
	
        public Dimension getMinimumSize() {
	    return getPreferredSize();
        }
	
        public String getToolTipText(MouseEvent e) {
	    int x = e.getX();
	    int w = getWidth();
	    int start = 0;
	    if (w > _TOTALTOOLTIPWIDTH) {
		x -= (w - _TOTALTOOLTIPWIDTH);
		w = _TOTALTOOLTIPWIDTH;
	    }
	    if ((w/4 != 0) && (x >= 0)) {
		int index = x / (w/4);
		if (index >= 0 && index < 4) {
		    return _toolTips[index];
		}
	    }
	    return null;
        }
	
        public Dimension getPreferredSize() {
	    Dimension size = new Dimension(10,10);
	    for (int i=0; i<4; i++) {
		if (_icons[i] != null) {
		    int height = _icons[i].getIconHeight();
		    int width = _icons[i].getIconWidth();
		    size.height = Math.max(size.height, height);
		    size.width = Math.max(size.width, width);
		}
	    }
	    if (_updateAvailable != null) {
		size.height = Math.max(size.height,
				       _updateAvailable.getIconHeight());
	    }
	    size.width = 4 * (size.width + 2);
	    size.height += 4;
	    size.width += 2;
	    return size;
        }
	
        private void setIcons(JNLEntry e) {
	    Icon icon;
	    LaunchDesc ld = null;
	    boolean inCache = false;
	    URL jnlpUrl = null;
	    LocalApplicationProperties lap = null;
	    
	    if (e == null) {
		for (int i=0; i<4; setIcon(i++, null));
	    } else {
	        ld = e.getLaunchDescriptor();
	        if (ld != null) {
		    inCache = LaunchDownload.isInCache(ld);
		    jnlpUrl = ld.getCanonicalHome();
		    if (jnlpUrl != null) {
		        lap = InstallCache.getCache().
			    getLocalApplicationProperties(jnlpUrl, ld);
		    }
	        }
	        
	        // 1.) determine if update available
	        boolean update = false;
	        if (ld != null) {
		    // don't check for updates over https (4775373)
		    if (!_isOffline && !ld.isHttps()) {
		        if (_lazyUpdateCheck == null) {
			    _lazyUpdateCheck = new LazyUpdateCheck();
		        }
		        _lazyUpdateCheck.doUpdateCheck(ld);
		    }
	        }
	        if (update) {
		    icon = _updateAvailable;
		    _toolTips[0] = _yesToolTips[0];
	        } else {
		    icon = null;
	        }
	        setIcon(0,icon);
	        
	        // 2.) can app run offline ?
	        if (e.canRunOffline()) {
		    icon = _netNo;
		    _toolTips[1] = _noToolTips[1];
	        } else {
		    icon = _netYes;
		    _toolTips[1] = _yesToolTips[1];
	        }
	        
	        setIcon(1, icon);
	        
	        // 3.) is in cache
	        if (inCache) {
		    icon = _inCache;
		    _toolTips[2] = _yesToolTips[2];
	        } else {
		    icon = _notInCache;
		    _toolTips[2] = _noToolTips[2];
	        }
	        setIcon(2, icon);
	        
	        // 4.) trusted vs untrusted app
	        icon = null;                // we cant tell if it's signed
	        if (inCache) {
		    JARDesc jarDesc = ld.getResources().getMainJar(true);
		    if (jarDesc != null) {
		        DiskCacheEntry dce = DownloadProtocol.getCachedVersion(
			    jarDesc.getLocation(), jarDesc.getVersion(),
			    DownloadProtocol.NATIVE_DOWNLOAD);
		        if (dce != null) {
			    if (dce.getCertificateChain() != null) {
			        icon = _signed;
			        _toolTips[3] = _yesToolTips[3];
			    } else {
			        icon = _unsigned;
			        _toolTips[3] = _noToolTips[3];
			    }
		        } else {
		        }
		    } else {
		    }
	        }
	        setIcon(3,icon);
            }
	    
	    invalidate();
	    repaint();
        }
	
        private class IndicatorUpdateThread extends Thread {
	    public void run() {
		try {
		    synchronized(this) {
			// try waiting 30 seconds ...
			this.wait(30000);
		    }
		} catch (Exception e) {
		    // Just ignore
		    Debug.ignoredException(e);
		};
		SwingUtilities.invokeLater(new Runnable() {
			    public void run() {
				Player.this.installCompleted();
			    }
			});
	    }
        }
	
        class LazyUpdateCheck extends Thread {
	    private LaunchDesc _next_ld;
	    private boolean _started = false;
	    private Object _signalObject = null;
	    
	    public LazyUpdateCheck() {
		_next_ld = null;
		_signalObject = new Object();
	    }
	    
	    public void doUpdateCheck(LaunchDesc ld) {
		boolean doStart = false;
		synchronized (LazyUpdateCheck.this) {
		    _next_ld = ld;
		    if (!_started) {
			doStart = true;
			_started = true;
		    }
		}
		if (doStart) {
		    start();
		} else {
		    synchronized (_signalObject) {
			_signalObject.notify();
		    }
		}
	    }
	    
	    
	    public void run() {
		LaunchDesc ld;
		for(;;) {
		    try {
			synchronized (LazyUpdateCheck.this) {
			    ld = _next_ld;
			}
			boolean available= LaunchDownload.isUpdateAvailable(ld);
			Icon icon;
			String tip;
			if (available) {
			    icon = _updateAvailable;
			    tip = _yesToolTips[0];
			} else {
			    icon = null;
			    tip = _noToolTips[0];
			}
			setIcon(0,icon);
			_toolTips[0] = tip;
			invalidate();
			repaint();
		    } catch (JNLPException je) {
			// Just ignore
			Debug.ignoredException(je);
		    }
		    try {
			synchronized (_signalObject) {
			    _signalObject.wait();
			}
		    } catch (InterruptedException ie) {
			// Just ignore
			Debug.ignoredException(ie);
		    }
		}
	    }
        }
    }
}
