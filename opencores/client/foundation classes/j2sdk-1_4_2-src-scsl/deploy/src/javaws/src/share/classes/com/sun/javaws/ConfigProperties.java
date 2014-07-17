/*
 * @(#)ConfigProperties.java	1.108 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws;

import java.util.Properties;
import java.util.Enumeration;
import java.util.StringTokenizer;
import java.util.Vector;
import java.net.URL;
import java.security.Security;
import java.awt.Rectangle;
import java.net.MalformedURLException;
import java.io.*;
import java.util.*;
import com.sun.javaws.debug.*;
import com.sun.javaws.util.VersionID;
import com.sun.javaws.util.VersionString;
import com.sun.javaws.util.URLUtil;
import com.sun.javaws.ui.player.BookmarkEntry;
import com.sun.javaws.net.*;

//
//  Interface to manipulate the deployment properties file
//
public class ConfigProperties {
    
    // Reference to canonical instance
    private static ConfigProperties _instance;
    
    // Internally the config properties are stored in a standard JDK property
    // file. We do part-object, instead of inheritance to enforce type access
    // to the properties
    private static Properties _props = null;
    // Properties that are default
    private static Properties _defaultProps;
    
    // The JREInformation is contained in a separate list
    private ArrayList _jreInfoList = null;
    
    // Time that we either last read the config file, or last saved it.
    private long _lastChanged;
    // If true, the properties have changed.
    private boolean _dirty;
    
    private static final String PROPERTIES_FILE = "deployment.properties";
    private static final String CONFIG_FILE     = "deployment.config";
    private static final String JAVAWS          = "javaws";
    private static final String CACHE           = "cache";
    private static final String MUFFINS         = "muffins";
    private static final String SPLASHES        = "splashes";
    private static final String SPLASH_INDEX    = "splash.xml";
   
 
    // The default size of Console.
    private static final String DEFAULT_CONSOLE_SIZE = "10000";
    
    // The default browser under unix.
    private static final String DEFAULT_BROWSER_PATH = "netscape";
    
    // Constants for the types of proxies that could be set.
    public static final String PROXY_AUTO       = "AUTOMATIC";
    public static final String PROXY_MANUAL     = "MANUAL";
    public static final String PROXY_NONE       = "NONE";
    
    
    // Default location to do installs from.
    private static final String DEFAULT_JRE_INSTALL_URL
        =  "http://java.sun.com/products/autodl/j2se";
    
    private static final String DEFAULT_JRE_LOCATION
        = DEFAULT_JRE_INSTALL_URL;
    
    // Default auto updater location
    private static final String DEFAULT_AUTO_UPDATER_URL =
        "http://java.sun.com/products/javawebstart/auto-update/javaws-1_0.conf";
    
    // Default URL for remote apps
    private static final String REMOTE_APPS_URL
        = "http://java.sun.com/products/javawebstart/apps/remoteApps.html";
    
    // this is not a key for our property, but rather for a system property
    public static final String JAUTHENTICATOR_SYSTEM_KEY = 
					"javaws.cfg.jauthenticator";
    //
    // Partial keys:
    //

    private static final String DEPLOYMENT 	     = "deployment.";
    private static final String BASE		     = DEPLOYMENT + "javaws.";
    private static final String JRE_PLATFORM_ID      = ".platform";
    private static final String JRE_PRODUCT_ID       = ".product";
    private static final String JRE_PRODUCT_LOCATION = ".location";
    private static final String JRE_PATH             = ".path";
    private static final String JRE_OS_NAME          = ".osname";
    private static final String JRE_OS_ARCH          = ".osarch";
    private static final String JRE_ISENABLED        = ".enabled";
    private static final String JRE_ISREGISTERED     = ".registered";
    private static final String SERVICES	     = BASE + "services.";
    private static final String PLAYER		     = BASE + "player.";

    
    //
    // Recognized properties
    //
    
    public static final String JRE_KEY             = BASE + "jre.";
    public static final String SPLASH_CACHE_KEY    = BASE + "splash.cache";
    public static final String MUFFIN_KEY          = SERVICES + "persistence";
    public static final String MUFFIN_LIMIT_KEY    = MUFFIN_KEY + ".limit";
    public static final String CACHE_KEY           = BASE + "cache";
    public static final String CACHE_DIR_KEY       = CACHE_KEY + ".dir";
    public static final String CACHE_SIZE_KEY      = CACHE_KEY + ".size";
    public static final String PROXY_KEY           = BASE + "proxy.";
    public static final String AUTO_UPDATER_URL_KEY= BASE + "autoUpdateURL";
    public static final String JAUTHENTICATOR_KEY  = BASE + "jauthenticator";
    public static final String PROXY_TYPE_KEY      = PROXY_KEY + "setting";
    public static final String HTTP_PROXY_KEY      = PROXY_KEY + "http";
    public static final String HTTP_PROXY_PORT_KEY = HTTP_PROXY_KEY + "port";
    public static final String OVERRIDE_KEY = HTTP_PROXY_KEY+"proxyoverride";
    public static final String SHOW_CONSOLE_KEY    = BASE + "showConsole";
    public static final String CONSOLE_SIZE_KEY    = BASE + "consoleBufferSize";
    public static final String LOG_TO_FILE_KEY     = BASE + "logToFile";
    public static final String LOG_DIR_KEY         = BASE + "logdir";
    public static final String LOG_FILENAME_KEY    = BASE + "logFileName";
    public static final String BROWSER_PATH_KEY    = BASE + "browserPath";
    public static final String NETSCAPE_BROWSER_KEY= BASE + "netscapeBrowser";
    public static final String JRE_INSTALL_URL_KEY = BASE + "installURL";
    public static final String FAVORITE_SIZE_KEY   = BASE + "favorites.size";
    public static final String FAVORITES_KEY       = BASE + "favorites";
    public static final String REMOTE_APPS_KEY     = PLAYER + "remoteURL";
    public static final String PLAYER_BOUNDS_KEY   = PLAYER + "bounds";
    public static final String PLAYER_MANAGER_KEY  = PLAYER + "manager";
    public static final String PLAYER_MODE_KEY     = PLAYER + "mode";
    public static final String BOOKMARK_NAME_KEY   = PLAYER + "bookmark.name";
    public static final String BOOKMARK_URL_KEY    = PLAYER + "bookmark.url";
    public static final String FORCE_UPDATE_CHECK  = BASE + "forceUpdate";
    public static final String UPDATE_TIMEOUT_KEY  = BASE + "updateTimeout";
    public static final String TRUSTED_PLAYER_URL  = BASE + "trustedPlayerURL";
    public static final String VERSION_UPDATED_KEY = BASE + "version";
    public static final String SECURE_PROPS_KEY    = BASE + "secure.properties";
    public static final String WHEN_INSTALL_KEY    = BASE + "whenInstall";
    public static final String WHAT_INSTALL_KEY    = BASE + "whatInstall";

    //
    //  These two are keys into deployment.comfig property file:
    //
    private static final String SYSTEM_POLICY_KEY = 
	"deployment.system.security.policy";
    private static final String USER_POLICY_KEY = 
	"deployment.user.security.policy";

    // for local install handler
    public static final int INSTALL_NEVER =   0;
    public static final int INSTALL_ALWAYS =  1;
    public static final int INSTALL_ASK =     2;
    public static final int INSTALL_ASK_OLD = 3;   // no longer used

    // for JAuthenticator
    public final static int JA_ALL = 0;
    public final static int JA_PROXY = 1;
    public final static int JA_DOWNLOAD = 2;
    public final static int JA_BOTH = 3;
    public final static int JA_NONE = 4;


    private static final String DEFAULT_UPDATE_TIMEOUT = "1500";
    private static final long   DEFAULT_MUFFIN_LIMIT = 262144;  // 256 * 1024
    private static final int MAX_URLS_LEN = 16;
    private static int   _nextAvailableJREIndex = 0;

    // note - should be same list as in native: launchFile.c
    private static final String DefaultSecureProperties [] = {
			"sun.java2d.noddraw",
			"javax.swing.defaultlf",
    			"javaws.cfg.jauthenticator" };
    
    
    public boolean isVersionCurrent() {
	String s = getProperty(VERSION_UPDATED_KEY);
	if (s == null) return false;
	return s.equals(Globals.getComponentName());
    }

    public void setVersionUpdated() {
	setProperty(VERSION_UPDATED_KEY, Globals.getComponentName());
    }

    public static synchronized int getNextAvailableUserJREIndex() {
	return _nextAvailableJREIndex++;
    }
	
    static private String getSystemProperty(final String key) {
        return System.getProperty(key);
    }
    
    //
    // Returns the path to the config file.
    //
    static private String getDeploymentUserPropertiesFilePath() {
        String home = JnlpxArgs.getDeploymentUserHome();
        if (home == null || home.length() == 0) {
	    Debug.fatal("DeploymentUserHome not set");
        }
        return home + File.separator +  PROPERTIES_FILE;
    }

    static private String getDeploymentSystemPropertiesFilePath() {
        String home = JnlpxArgs.getDeploymentSystemHome();
        if (home == null || home.length() == 0) {
	    Debug.fatal("DeploymentSystemHome not set");
        }
        return home + File.separator + PROPERTIES_FILE;
    }
    
    public static String getApplicationHome() {
        String home = JnlpxArgs.getHome();
        if (home == null || home.length() == 0) {
	    Debug.fatal("AppHome not set");
        }
	try {
	    File f = new File(home);
	    home = f.getCanonicalPath();
	} catch (IOException ioe) {
	    Debug.ignoredException(ioe);
	}
        return home;
    }
    
    //
    // Returns the command used to start up a new version of Java Web Start.
    //
    public static String getStartCommand() {
        String path = ConfigProperties.getApplicationHome();
        
        if (!path.endsWith(File.separator)) {
	    path = path + File.separator;
        }
        if (Globals.isWindowsPlatform()) {
	    path += "javaws.exe";
        }
        else {
	    path += "javaws";
        }
        return path;
    }
    
    static String getJavaWSJarFilePath() {
        String homePath = ConfigProperties.getApplicationHome();
        
        if (!homePath.endsWith(File.separator)) {
	    homePath = homePath + File.separator;
        }

        String path = homePath + "javaws.jar" + File.pathSeparator + homePath
	    + "javaws-l10n.jar";
	
        return path;
    }
    
    //
    // Returns the command used uninstall Java Web Start
    //
    public static String getUninstallCommand(String path) {
        String iPath = ConfigProperties.getStartCommand();
        
        return iPath + " -uninstall @\"" + path + "\"";
    }

    public static void setPlayerBounds(Rectangle bounds) {
	getInstance().setProperty(PLAYER_BOUNDS_KEY,
		""+bounds.x+ ","+bounds.y+
		","+bounds.width+ ","+bounds.height);
    }

    public static Rectangle getPlayerBounds() {
	String p = getInstance().getProperty(PLAYER_BOUNDS_KEY);
	if (p != null) {
	    StringTokenizer st = new StringTokenizer(p,",");
	    int xywh[] = new int[4];
	    for (int i=0; i<4; i++) {
		if (st.hasMoreTokens()) {
		    String str = st.nextToken();
		    try {
			xywh[i]=Integer.parseInt(str);
			continue;
		    } catch (NumberFormatException e) { };
		}
		return null; // no more tokens or exception
	    }
	    return new Rectangle(xywh[0], xywh[1], xywh[2], xywh[3]);
	}
	return null;
    }

    public static void setPlayerManager(int index) {
        getInstance().setProperty(PLAYER_MANAGER_KEY, ""+index);
    }

    public static int getPlayerManager() {
	String p = getInstance().getProperty(PLAYER_MANAGER_KEY);
	if (p != null) {
	    try {
		return Integer.parseInt(p);
	    } catch (NumberFormatException e) { };
	}
	return -1;
    }

    public static void setPlayerMode(int index) {
        getInstance().setProperty(PLAYER_MODE_KEY, ""+index);
    }

    public static int getPlayerMode() {
	String p = getInstance().getProperty(PLAYER_MODE_KEY);
	if (p != null) {
	    try {
		return Integer.parseInt(p);
	    } catch (NumberFormatException e) { };
	}
	return -1;
    }
    
    void setSplashCache(String indexFile) {
	setProperty(SPLASH_CACHE_KEY, indexFile);
    }

    //
    // Construct a new ConfigProperty object with default values. 
    // It is private since an instance should always be refered to through 
    // the ConfigProperties.getInstance method
    //
    private ConfigProperties() {
        super();
        // Setup the default properties, and systems deployment.properties
        setupApplicationDefaultProperties();
        // Load users deployment.properties
        refresh();
	// may need to create dirs for Deployment user home
	new File(JnlpxArgs.getDeploymentUserHome()).mkdirs();
	// and the cache dir
	new File(getCacheDir()).mkdirs();
	// and setup from users deployment.config
	setupDeploymentConfig();
    }
    
    // Returns a potential cached version of the property object
    static public synchronized ConfigProperties getInstance() {
        if (_instance == null) {
	    _instance = new ConfigProperties();
        }
        return _instance;
    }
    
    //
    // Query methods
    //
    
    public long        getMuffinSizeLimit()   { 
        return getLongProperty(MUFFIN_LIMIT_KEY, getDefaultMuffinSizeLimit()); 
    }
    public int         getCacheSize()         { 
        return getIntegerProperty(CACHE_SIZE_KEY); 
    }
    public String      getCacheDir()          { 
        return getProperty(CACHE_DIR_KEY, getDefaultCacheDir());
    }
    public void setCacheDir(String dir) {
	setProperty(CACHE_DIR_KEY, dir);
    }
    public String      getHTTPProxy()         { 
        return getProperty(HTTP_PROXY_KEY); 
    }
    public String      getHTTPProxyPort()     { 
        return getProperty(HTTP_PROXY_PORT_KEY); 
    }
    boolean     getForceUpdateCheck()  { 
        return getBooleanProperty(FORCE_UPDATE_CHECK); 
    }
    int	       getRapidUpdateTimeout() {
        return getIntegerProperty(UPDATE_TIMEOUT_KEY); 
    }
    // Console Loging methods
    public boolean     getshowConsole()     { 
        return getBooleanProperty(SHOW_CONSOLE_KEY); 
    }
    public int         getConsoleSize()     { 
        return getIntegerProperty(CONSOLE_SIZE_KEY); 
    }
    public boolean     isLogToFileEnabled() { 
        return getBooleanProperty(LOG_TO_FILE_KEY); 
    }
    public String getDefaultLogDir() {
        return JnlpxArgs.getDeploymentUserHome() + File.separator + "log" +
                File.separator + JAVAWS ;
    }
    public String      getLogFileDir()     { 
        return getProperty(LOG_DIR_KEY, getDefaultLogDir()); 
    }
    public String      getLogFilename()     { 
	String filename = getLogFileDir() + File.separator + "TEMP";
        return getProperty(LOG_FILENAME_KEY, filename); 
    }
    // For launching browser on Solaris/Unix
    boolean     isBrowserNetscape()      { 
        return getBooleanProperty(NETSCAPE_BROWSER_KEY); 
    }
    String      getBrowserPath()         { 
        return getProperty(BROWSER_PATH_KEY); 
    }


    public void setBookmarks(ArrayList list) {
	BookmarkEntry e = null;

	for (int i = 0; i < list.size(); i++) {
	    e = (BookmarkEntry)list.get(i);	 
	   
	    if (Globals.TracePlayer) {
		Debug.println("setBookmarks: "+ e.getName() + " " + e.getUrl());
	    }
	    setProperty(BOOKMARK_NAME_KEY+"."+i, e.getName());	   
		
	    setProperty(BOOKMARK_URL_KEY+"."+i, e.getUrl());	    
	}
    }
  

    public ArrayList getBookmarks() {
	String name = null;
	String url = null;
	boolean loading = true;
	int index = 0;
	ArrayList list = new ArrayList();
	while (loading) {
	    name = getProperty(BOOKMARK_NAME_KEY+"."+index);
	    url = getProperty(BOOKMARK_URL_KEY+"."+index);

	    // remove the properties, we will save it again later
	    removeProperty(BOOKMARK_NAME_KEY+"."+index);
	    removeProperty(BOOKMARK_URL_KEY+"."+index);
	  
	    if (name == "" && url == "") {
		loading = false;
	    }
	    // only load entries that are well defined
	    if (name != null && url != null && 
			!name.equals("") && !url.equals("")) {
		if (Globals.TracePlayer) {
		     Debug.println("loaded bookmark: " + name + " " + url);
		}       
		BookmarkEntry entry = new BookmarkEntry(name, url);
		list.add(entry);
	    }
	   
	    index++;
	}
	return list;
    }
    
    public String getRemoteURL() {
        return getProperty(REMOTE_APPS_KEY);
    }

    public void setRemoteURLs(String[] urls) {
	setProperty(REMOTE_APPS_KEY, urls[0]);
	for (int i=1; i<urls.length; i++) {
	    setProperty(REMOTE_APPS_KEY+"."+i, urls[i]);
	}
    }

    public String[] getRemoteURLs() {
	String[] urls = new String[MAX_URLS_LEN];
	urls[0] = getProperty(REMOTE_APPS_KEY);
	for (int i=1; i<MAX_URLS_LEN; i++) {
	    urls[i] = getProperty(REMOTE_APPS_KEY+"."+i);
	}
	return urls;
    }

    public static int getMaxUrlsLength() {
	return MAX_URLS_LEN;
    }
    
    public String getAutoUpdateURL() {
        return getProperty(AUTO_UPDATER_URL_KEY, DEFAULT_AUTO_UPDATER_URL);
    }
    
    //  JRE information
    //
    //  Currently all installed JREs are stored in configuration file. 
    //  JREs can be looked up based on either the product version given by
    //  a (URL, version string), or by a platform-version (version string). 
    //  The product version is found from the java.version property, and the 
    //  platform version from the java.specification.version property.
    //
    //

    // Read all JRE information from properties file
    private void readJREInfoProperties(String key, Properties props, int src) {
	// Get all availabled indices from the list.  We have to use this 
	// getJREIndex because there might be missing indices
	Vector indices = new Vector();
	Enumeration e = props.keys();
	while (e.hasMoreElements()) {
	    String s = (String)e.nextElement();
	    if (s.startsWith(key)) {
		Integer i = new Integer(getJREIndex(key, s, src));
		if (i.intValue() >= 0 && !indices.contains(i)) {
		    indices.add(i);
		}
	    }
	}
	Collections.sort(indices);
	
	// Go through each index and add the JRE associated with it to the list
        JREInformation je = null;
	e = indices.elements();
	while (e.hasMoreElements()) {
	    Integer i = (Integer)e.nextElement();
	    je = JREInformation.fromProperty(key, props, i.intValue(), src);
	    if (je != null && !_jreInfoList.contains(je)) {
		_jreInfoList.add(je);
		if (Globals.TraceConfig) {
		    Debug.println("Adding jre " + je.getIndex() +
				" with path: " + je.getInstalledPath());
		}
	    }
	}
    }
    
    // Get the index in the specified JRE Key
    private int getJREIndex(String key, String s, int src) {
	int i = -1;
	try {
	    int end = s.indexOf('.', key.length());
	    if (end != -1) {
		String sub = s.substring(key.length(), end);
		i = Integer.parseInt(sub);
	    }
	} catch (NumberFormatException nfe) {
	    Debug.ignoredException(nfe);
	}
	return i;
    }
    
    private void updateJREEnabledProperties(Properties props) {
	for(int i = 0; i < _jreInfoList.size(); i++) {
	    JREInformation je = (JREInformation)_jreInfoList.get(i);
	    je.setEnabledProperty(props);
	}
    }
    
    // Create property object with information about all JREs
    private void updateJREInfoProperties(Properties props, int src) {
        ArrayList addedList = new ArrayList();

        for(int i = 0; i < _jreInfoList.size(); i++) {
	    JREInformation je = (JREInformation)_jreInfoList.get(i);
	    // don't write out duplicate JREs when doing -updateVersions
	    if (!addedList.contains(je) && src == je.getWhereSpecified()) {
		je.toProperty(props); 
	    	addedList.add(je);
	    }
        }

    }
    
    // Returns a set of JREs from a given URL as JREInformation objects
    public ArrayList getJREInformationList() { 
	return _jreInfoList; 
    }
    
    // Return a specific version
    public JREInformation getJRE(URL location, String versionId) {
        VersionString vs = new VersionString(versionId);
        for(int i = 0; i < _jreInfoList.size(); i++) {
	    JREInformation je = (JREInformation)_jreInfoList.get(i);
	    if (je.isProductMatch(location, vs)) return je;
        }
        return null;
    }
    
    // Returns list of instanceoflled platforms
    public String getKnownPlatforms() {
        StringBuffer knownPlatforms = new StringBuffer();
        for(int i = 0; i < _jreInfoList.size(); i++) {
	    JREInformation je = (JREInformation)_jreInfoList.get(i);
	    knownPlatforms.append(je.getPlatformVersionId());
	    knownPlatforms.append(" ");
        }
        return knownPlatforms.toString();
    }

    int getJAuthenticator() {
	int retval = JA_ALL;
	String prop = getSystemProperty(JAUTHENTICATOR_SYSTEM_KEY);
	if (prop == null) {
	    prop = getProperty(JAUTHENTICATOR_KEY);
	}
	if (prop != null) {
	    prop=prop.toLowerCase();
	    if (prop.equals("none")) { retval = JA_NONE; }
	    else if (prop.equals("all")) { retval = JA_ALL; }
	    else if (prop.equals("both")) { retval = JA_BOTH; }
	    else {
		if (prop.indexOf("proxy") >= 0) { retval |= JA_PROXY; }
		if (prop.indexOf("download") >= 0) { retval |= JA_DOWNLOAD; }
	    }
	}
	return retval;
    }
    
    //
    //  Helper class to store version information about a JRE
    //
    static public class JREInformation {
	public static final int SRC_USER = 0;
	public static final int SRC_INSTALL = 1;
	private String _platformVersionId;  // Platform version ID
        private String _productVersionId;   // Product Version ID
        private URL    _productLocation;    // Product URL
	private String _osName;             // JRE os version
	private String _osArch;             // JRE os arch
        private String _installedPath;      // Path to Java command
        private boolean _isFCSVersion;       // Is FCS or non-FCS version
	private int _whereSpecified;
	private boolean _isEnabled;
	private boolean _isRegistered;
	private int _index;
        // Package private
	
        JREInformation() {
	    _osName = null;
	    _osArch = null;
	    _platformVersionId = null;
	    _productVersionId = null;
	    _productLocation = null;
	    _installedPath = null;
	    _isFCSVersion = true;
	    _whereSpecified = SRC_USER;
	    _isEnabled = true;
	    _isRegistered = false;
	    _index = _nextAvailableJREIndex;	    
        }
        
        public JREInformation(	String platformVersionId, 
				String productVersionId, 
				URL productLocation, 
				String installedPath,
			      	int whereSpecified, 
				boolean isEnabled, 
				int index, 
				String osName, 
				String osArch) {	  
	    this(platformVersionId, productVersionId, productLocation, 
		installedPath, whereSpecified, isEnabled, 
		index, osName, osArch, false);
	}

        public JREInformation(	String platformVersionId, 
				String productVersionId, 
				URL productLocation, 
				String installedPath,
				int whereSpecified, 
				boolean isEnabled, 
				int index, 
				String osName, 
				String osArch, 
				boolean isRegistered) {	  
	    setOsInfo(osName, osArch); 
	    _platformVersionId = platformVersionId;
	    _productVersionId = productVersionId;
	    _productLocation = productLocation;
	    _installedPath = installedPath;
	    _whereSpecified = whereSpecified;
	    _isEnabled = isEnabled;
	    _isRegistered = isRegistered;
	    _index = index;
	    if (_productLocation == null) {
		try {
		    _productLocation = new URL(DEFAULT_JRE_LOCATION);
		} catch(MalformedURLException mue) {
		    Debug.ignoredException(mue);
		}
	    }
	    
	    // A platform match is only allowed for FCS versions of JREs.
	    // By convention, if the product-id contains a '-' then
	    // it is non-FCS version. This convention is used for 1.3.0 and
	    // higher
	    if (_productVersionId != null &&
		    (_productVersionId.indexOf('-') != -1) &&
		    (!_productVersionId.startsWith("1.2")) &&
		    // now also allowing the jre this javaws came in on
		    (!isInstallJRE()) ) {
		// Product version contains '-' and is not a 1.2 version 
		_isFCSVersion = false;
	    } else {
		// Otherwise fine 
		_isFCSVersion = true;
	    }
	}
	
	public int    getIndex() { return _index; }
	public int    getWhereSpecified()    { return _whereSpecified; }
	public String getPlatformVersionId() { return _platformVersionId; }
	public String getProductVersionId()  { return _productVersionId; }
	public URL    getProductLocation() { return _productLocation; }
	public String getInstalledPath()   { return _installedPath; }
	public boolean isEnabled() { return _isEnabled; }
	public String getOsName() { return _osName; }
	public String getOsArch() { return _osArch; }

	public boolean isRegistered() { return _isRegistered; }
	
	public void setEnabled(boolean isEnabled) {
	    _isEnabled = isEnabled;
	}
	
	public void setVersions(String platformId, String productId) {
	    _platformVersionId = platformId;
	    _productVersionId = productId;
	    // Insert default location is neccesary
	    if (_productLocation == null) {
		try {
		    _productLocation = new URL(DEFAULT_JRE_LOCATION);
		} catch(MalformedURLException mue) {
		    Debug.ignoredException(mue);
		}
	    }
	}

	public boolean isOsInfoMatch(String osName, String osArch) {
	    // assume it is for the current platform if osName and
	    // osArch is not specified
	    if (_osName == null || _osArch == null) return true;

	    return (_osName.equals(osName) && _osArch.equals(osArch));
	}

	public void setOsInfo(String osName, String osArch) {   
	    _osName = osName;
	    _osArch = osArch;
	    // All windows OS uses the same JRE binary
	    if (_osName != null && _osName.startsWith("Windows")) {  
		_osName = "Windows";
	    }    
	}
	
	
	boolean isProductMatch(URL location, VersionString version) {
	    // Make sure to compare URL Strings. Doing an equals on a URL
	    // might require network lookup in order to resolve IP. Thus, it
	    // will fail in offline mode.
	    return URLUtil.equals(location, _productLocation) && 
				version.contains(_productVersionId);
	}
	
	boolean isPlatformMatch(VersionString platformVersion) {
	    // Make sure to compare URL Strings. Doing an equals on a URL
	    // might require network lookup in order to resolve IP. Thus, it
	    // will fail in offline mode.
	    return _isFCSVersion && 
			platformVersion.contains(_platformVersionId);
	}
	
	static private JREInformation fromProperty(String jrekey, 
					Properties props, int n, int src) {
	    String platformId = props.getProperty(jrekey + n + JRE_PLATFORM_ID);
	    String productId =  props.getProperty(jrekey + n + JRE_PRODUCT_ID);
	    String location = props.getProperty(jrekey + n + 
						JRE_PRODUCT_LOCATION);
	    String path = props.getProperty(jrekey + n + JRE_PATH);   

	    String osName = props.getProperty(jrekey + n + JRE_OS_NAME);
	    String osArch = props.getProperty(jrekey + n + JRE_OS_ARCH);

    
	    boolean isEnabled = (new Boolean(_props.getProperty(
			jrekey + n + JRE_ISENABLED, "true"))).booleanValue();
	    boolean isRegistered = (new Boolean(_props.getProperty(
			jrekey + n + JRE_ISREGISTERED, "true"))).booleanValue();

	    try {
		if (platformId != null && path != null) {
		    URL loc = (location == null || location.length() ==0) ? 
				null : new URL(location);
		    if (src == SRC_USER) n = _nextAvailableJREIndex++;
		    else if (src == SRC_INSTALL && n >= _nextAvailableJREIndex)
			_nextAvailableJREIndex = n + 1;
		    JREInformation je = new JREInformation(platformId, 
			productId, loc, path, src, isEnabled, n, 
			osName, osArch, isRegistered);
		    return je;
		}
	    } catch(MalformedURLException mue) {
		Debug.ignoredException(mue);
	    }
	    return null;
	}
			
	// Creates a copy
	public JREInformation copy() {
	    return new JREInformation(_platformVersionId, _productVersionId, 
			_productLocation, _installedPath, _whereSpecified, 
			_isEnabled, _index, _osName, _osArch, _isRegistered);
	}
	
	private void setEnabledProperty(Properties props) {
	    String jrekey = JRE_KEY;
	    if (_platformVersionId != null) {
		props.setProperty(jrekey + _index + JRE_ISENABLED, 
		    (new Boolean(_isEnabled)).toString());
	    }
	}
	
	private void toProperty(Properties props) {
	    if (_platformVersionId != null) {
		setJREKey(props, _index, JRE_PLATFORM_ID, _platformVersionId);
		setJREKey(props, _index, JRE_PRODUCT_ID, _productVersionId);
		setJREKey(props, _index, JRE_PRODUCT_LOCATION, 
				_productLocation.toString());
		setJREKey(props, _index, JRE_PATH, _installedPath);
		setJREKey(props, _index, JRE_OS_NAME, _osName);
		setJREKey(props, _index, JRE_OS_ARCH, _osArch);
		setJREKey(props, _index, JRE_ISREGISTERED, 
				(new Boolean(_isRegistered)).toString());
	    }
	}
	private boolean isInstallJRE() {
	    File installDir = (new File(getApplicationHome())).getParentFile();
	    File binDir = (new File(_installedPath)).getParentFile();
	    if (installDir != null && binDir != null) {
		if (installDir.equals(binDir.getParentFile())) {
		    return true;
		}
	    }	
	    return false;
	}
	
	private void setJREKey( Properties props, int n, 
				String kind, String value) {
	    if (value != null) {
		props.setProperty(JRE_KEY + n + kind, value);
	    }
	}
	
	// Returns true if this is the current running JRE 
	boolean isCurrentRunningJRE() {
	    File current = JnlpxArgs.getJVMCommand();
	    // Compare commands
	    return current.equals(new File(_installedPath));
	}
	
	public final boolean equals(Object o) {
	    if (o == null || !(o instanceof JREInformation)) return false;
	    JREInformation jreinfo = (JREInformation)o;
	    
	    return 
		equalsHelper( _platformVersionId, jreinfo._platformVersionId) &&
		equalsHelper(_productVersionId, jreinfo._productVersionId) &&
		URLUtil.equals(_productLocation, jreinfo._productLocation) &&
		equalsHelper(_installedPath, jreinfo._installedPath) &&
		equalsHelper(_osName, jreinfo._osName) && 
		equalsHelper(_osArch, jreinfo._osArch); 

	}
	
	public final int hashCode() {
	    return (_platformVersionId == null) ? 
		0 : _platformVersionId.hashCode();
	}
	
	static private boolean equalsHelper(Object o1, Object o2) {
	    if (o1 == null || o2 == null) return (o1 == o2);
	    return o1.equals(o2);
	}
	
	public String toString() {
	    return "JREInformation [prod=" + getProductVersionId() + 
		",plat=" + getPlatformVersionId() + "," + getInstalledPath() + 
		"," + getProductLocation() + ",cur=" + isCurrentRunningJRE() + 
		",src=" + getWhereSpecified() + ",osName=" + _osName + 
		",osArch=" + _osArch + "]";
	}
    }
    
    
    public void setWhenInstall(int value) {
	setProperty(WHEN_INSTALL_KEY, Integer.toString(value));
    }
    
    public int getWhenInstall() {
	return getIntegerProperty(WHEN_INSTALL_KEY);
    }
    
    public void setWhatInstall(int value) {
	setProperty(WHAT_INSTALL_KEY, Integer.toString(value));
    }
    
    public int getWhatInstall() {
	return getIntegerProperty(WHAT_INSTALL_KEY);
    }
    
    //
    // Returns the location to get JNL files for install from.
    //
    URL getJREInstallURL() {
	try {
	    URL url = new URL(getProperty(JRE_INSTALL_URL_KEY));
	    return url;
	} catch (MalformedURLException murle) {
	    // Should never happen...
	    return null;
	}
    }
    
    private long getDefaultMuffinSizeLimit() {
	return DEFAULT_MUFFIN_LIMIT;
    }
    
    // Returns the splash cache dir 
    String getSplashDir() {
	return getCacheDir() + File.separator + SPLASHES;
    }

    // Returns the splash cache index filename 
    String getSplashCache() {
	return getSplashDir() + File.separator + SPLASH_INDEX;
    }

    // Returns the muffin cache dir
    public String getMuffinDir() {
        return getCacheDir() + File.separator + MUFFINS;
    }

    //
    // Returns the default cache dir: $DEPLOYMENT_HOME/javaws/cache
    //
    private String getDefaultCacheDir() {
	return JnlpxArgs.getDeploymentUserHome() + File.separator + 
		JAVAWS + File.separator + CACHE;
    }
    
    //
    // Returns the integer value for the property with
    // <code>propertyName</code>. This assumes the value is a String,
    // if no value exists for <code>propertyName</code>, or it is not a valid
    // integer, -1 is returned.
    //
    private int getIntegerProperty(String propertyName) {
	String value = get(propertyName);
	if (value == null) {
	    return -1;
	}
	try {
	    return Integer.parseInt(value);
	}
	catch (NumberFormatException nfe) {
	    return -1;
	}
    }
    
    private long getLongProperty(String key, long defaultValue) {
	String value = get(key);
	if (value == null) {
	    return defaultValue;
	}
	try {
	    return Long.parseLong(value);
	} catch (NumberFormatException nfe) {
	    return defaultValue;
	}
    }
    
    //
    // Returns the boolean value for the property with
    // <code>propertyName</code>. This assumes the value is a String,
    // if no value exists for <code>propertyName</code>, or it is not a valid
    // integer, false is returned.
    //
    private boolean getBooleanProperty(String propertyName) {
	String value = get(propertyName);
	if (value == null) {
	    return false;
	}
	return Boolean.valueOf(value).booleanValue();
    }
    public int getProxyType() {
	String type = getProperty(PROXY_TYPE_KEY);
	if (type.compareToIgnoreCase(PROXY_NONE) == 0) {
	    return com.sun.javaws.proxy.InternetProxyInfo.NONE;
	} else if (type.compareToIgnoreCase(PROXY_MANUAL) == 0) {
	    return com.sun.javaws.proxy.InternetProxyInfo.MANUAL;
	} else {
	    return com.sun.javaws.proxy.InternetProxyInfo.AUTO;
	}
    }
    
    //
    // This is an all purpose proxy setter method
    // depending on the proxyType params will have the
    // strings the ex: if proxyType is null then params will be null
    // if proxyType is Manual then params will contain the nodename
    // and port.
    //
    public void setProxy(int proxyType, String httpHost, 
			int httpPort, String overrides) {
	if (proxyType == com.sun.javaws.proxy.InternetProxyInfo.NONE) {
	    setProperty(PROXY_TYPE_KEY, PROXY_NONE);
	} else if (proxyType == com.sun.javaws.proxy.InternetProxyInfo.MANUAL){
	    setProperty(PROXY_TYPE_KEY,PROXY_MANUAL);
	} else {
	    setProperty(PROXY_TYPE_KEY, PROXY_AUTO);
	}
	// Normalize unreasonable port numbers
	if (httpHost != null && httpHost.length() > 0) {
	    setProperty(HTTP_PROXY_KEY, httpHost);
	}
	if (httpPort < -1 || httpPort > 65565) {
	    httpPort = -1;
	}
	if (httpPort != -1) {
	    setProperty(HTTP_PROXY_PORT_KEY, String.valueOf(httpPort));
	}
	if (overrides != null) {
	    setProperty(OVERRIDE_KEY, overrides.trim());
	}
    }
    
    //
    // Returns a String of comma separated hostnames which could
    // be used to set TextFields etc.
    //
    public String getProxyOverrideString() {
	return getProperty(OVERRIDE_KEY);
    }
    
    //
    // This method will get the hostnames which do not need
    // proxies, normally used in the get call in Main, so we
    // return a List
    //
    public List getProxyOverrideList() {
	String delim = " \t\n\r\f,";
	ArrayList hnamesArray = new ArrayList();
	StringTokenizer st = 
		new StringTokenizer(getProxyOverrideString(),delim);
	while (st.hasMoreTokens()) {
	    hnamesArray.add((st.nextToken()).trim());
	}
	return hnamesArray;
    }
    
    // Load Config file from file/url
    public void setFavorites(Iterator favorites) {
	synchronized(this) {
	    int count = getIntegerProperty(FAVORITE_SIZE_KEY);
	    
	    if (count > 0) {
		for (int counter = 0; counter < count; counter++) {
		    removeProperty(FAVORITES_KEY + Integer.toString(counter));
		}
	    }
	    count = 0;
	    if (favorites != null) {
		while (favorites.hasNext()) {
		    String fav = (String)favorites.next();
		    setProperty(FAVORITES_KEY + Integer.toString(count++),fav);
		}
	    }
	    setProperty(FAVORITE_SIZE_KEY, Integer.toString(count));
	}
    }
    
    public Iterator getFavorites() {
	synchronized(this) {
	    int count = getIntegerProperty(FAVORITE_SIZE_KEY);
	    
	    if (count <= 0) {
		return new ArrayList(0).iterator();
	    }
	    ArrayList favs = new ArrayList(count);
	    for (int counter = 0; counter < count; counter++) {
		String value = getProperty(FAVORITES_KEY +
					       Integer.toString(counter));
		if (value != null && value.length() > 0) {
		    favs.add(value);
		}
	    }
	    return favs.iterator();
	}
    }

    public Iterator getSecurePropertyKeys() {
	ArrayList list = new ArrayList(4);
	for (int i=0; i<DefaultSecureProperties.length; 
		list.add(DefaultSecureProperties[i++]));
	String ConfigKeys = get(SECURE_PROPS_KEY);
	if (ConfigKeys != null) {
	    StringTokenizer st = new StringTokenizer(ConfigKeys, ",");
	    while (st.hasMoreTokens()) {
		list.add(st.nextToken());
	    }
	}
	return list.iterator();
    }
    
    public void put(String key, String value) {
	setProperty(key, value);
    }
    
    public String get(String key) {
	return getProperty(key);
    }
    
    //
    // Helper methods
    //
    private String getProperty(String key) {
	String value;
	synchronized(this) {
	    value = _props.getProperty(key, "");
	}
	return value.trim();
    }    
    
    private String getProperty(String key, String defaultValue) {
	String value;
	synchronized(this) {
	    value = _props.getProperty(key, defaultValue);
	}
	if (value != null) {
	    return value.trim();
	} else {
	    return value;
	}
    }

    private void setProperty(String key, String value) {
	if (value == null) {
	    removeProperty(key);
	} else {
	    synchronized(this) {
		_props.put(key, value);
		_dirty = true;
	    }
	}
    }
    
    private void removeProperty(String key) {
	synchronized(this) {
	    _props.remove(key);
	    _dirty = true;
	}
    }
    
    //
    // Returns true if the properties have changed.
    //
    private boolean isDirty() {
	return _dirty;
    }
    
    //
    // Returns true if a new version exists on the file system.
    //
    public synchronized boolean doesNewVersionExist() {
	File f = new File(getDeploymentUserPropertiesFilePath());
	if (f.exists() && f.lastModified() > _lastChanged) {
	    return true;
	}
	return false;
    }
    
    //
    // Reloads the config properties, throwing away any changes.
    //
    public synchronized void refreshIfNecessary() {
	if (!isDirty() && doesNewVersionExist()) {
	    refresh();
	}
    }

    private void loadProperties(Properties props, String urlfile) {
	InputStream is = null;
	try {
	    HttpRequest httpreq = JavawsFactory.getHttpRequestImpl();
	    HttpResponse response = httpreq.doGetRequest(new URL(urlfile)); 
	    is = response.getInputStream();
	} catch(MalformedURLException e) {
	    // Try to open as file
	    try {
		is = new FileInputStream(urlfile);
	    } catch (FileNotFoundException fnfe) {}
	} catch (IOException ioe) {}
	if (is != null) {
	    try {
		props.load(is);
		is.close();
	    } catch (IOException ioe) {
		Debug.ignoredException(ioe);
	    }
	}
    }
    
    public synchronized void refresh() {
	String urlfile = getDeploymentUserPropertiesFilePath();
	Properties userProps = new Properties();
	
	_dirty = false;
	
	// Read values from user-specific configuration file
	loadProperties(userProps, urlfile);

	_props = new Properties();
	mergeProperties(_props, _defaultProps);
	mergeProperties(_props, userProps);
	
	// Extract all JRE information
        _jreInfoList = new ArrayList();
	readJREInfoProperties(JRE_KEY, _defaultProps, 
					JREInformation.SRC_INSTALL);
	readJREInfoProperties(JRE_KEY, userProps, JREInformation.SRC_USER);
	
	// Filter out the JRE properties so we don't get stall entries
	// (store() automatically adds them back in)
	Properties props = new Properties();
	Enumeration en = _props.keys();
	while(en.hasMoreElements()) {
	    String key = (String)en.nextElement();
	    String value = _props.getProperty(key);
	    if (!key.startsWith(JRE_KEY) && value != null) {
		props.setProperty(key, value);
	    }
	}
	_props = props;
	
	// Update timestamp
	File f = new File(urlfile);
	_lastChanged = f.exists() ? 
	    f.lastModified() : System.currentTimeMillis();
    }
            
    // Store user-specific properties
    public synchronized void store() {
	storeHelper(getDeploymentUserPropertiesFilePath());
    }
    
    private synchronized void storeHelper(String path) {
	if (_props != null) {
	    // Create copy to keep JRE keys sepearate
	    Properties props = new Properties();

	    // Sift out all default values
	    mergeProperties(props, _props);
	    props = diffProperties(props, _defaultProps);

	    // Don't store the application ones in the user-local file
	    // remove all JRE props before RE-adding them
	    updateJREInfoProperties(props, JREInformation.SRC_USER);
	    updateJREEnabledProperties(props);
	    
	    File propFile = new File(path);
	    propFile.getParentFile().mkdirs();
	    try {
		FileOutputStream fos = new FileOutputStream(propFile);
		try {		 
		    props.store(fos, "");
		} catch (IOException ioe) {
		    Debug.ignoredException(ioe);
		}
		fos.flush();
		fos.close();
	    } catch (IOException ioe) {}
	}
	_dirty = false;
	
	File f = new File(getDeploymentUserPropertiesFilePath());
	_lastChanged = f.exists() ? 
	    f.lastModified() : System.currentTimeMillis();
    }
    
    // Creates the default hardcoded values 
    private Properties getPreSetProperties() {
	Properties props = new Properties();
	props.setProperty(CACHE_DIR_KEY,   getDefaultCacheDir());
	props.setProperty(CACHE_SIZE_KEY,  "10");
	props.setProperty(PROXY_TYPE_KEY,  PROXY_AUTO);
	props.setProperty(SHOW_CONSOLE_KEY,  "false");
	props.setProperty(CONSOLE_SIZE_KEY,  DEFAULT_CONSOLE_SIZE);
	props.setProperty(BROWSER_PATH_KEY, DEFAULT_BROWSER_PATH);
	props.setProperty(NETSCAPE_BROWSER_KEY, "true");
	props.setProperty(JRE_INSTALL_URL_KEY, DEFAULT_JRE_INSTALL_URL);
	props.setProperty(REMOTE_APPS_KEY, REMOTE_APPS_URL);
	props.setProperty(AUTO_UPDATER_URL_KEY, DEFAULT_AUTO_UPDATER_URL);
	props.setProperty(UPDATE_TIMEOUT_KEY, DEFAULT_UPDATE_TIMEOUT);
	return props;
    }
    
    //
    // Setup the preconfigured properties for the application. 
    // This includes the ones defined in the properties file in the 
    // deployment.system.home, as well as deployment.user.home
    //
    private void setupApplicationDefaultProperties() {
	// Setup default values
	Properties sysProps = new Properties();
        loadProperties(sysProps, getDeploymentSystemPropertiesFilePath());	

	_defaultProps = getPreSetProperties();
	mergeProperties(_defaultProps, sysProps);
    }

    private void setupDeploymentConfig() {
        // Setup props from deployment.config
        Properties configProps = new Properties();
        loadProperties(configProps, JnlpxArgs.getDeploymentUserHome() +
                                    File.separator + CONFIG_FILE);
        String systemPolicyURL = configProps.getProperty(SYSTEM_POLICY_KEY);
        String userPolicyURL = configProps.getProperty(USER_POLICY_KEY);
	if (systemPolicyURL != null || userPolicyURL != null) {
            // Add system/user security policy in Deployment infrastructure.
            int numPolicy;
            String policyURL = null;
	    final String key = "policy.url.";

            for (numPolicy = 1; Security.getProperty(key + numPolicy) != null;
                 numPolicy++);

            // Set system policy
	    if (systemPolicyURL != null) {
                Security.setProperty(key + numPolicy++, systemPolicyURL);
	    }
            // Set user policy
	    if (userPolicyURL != null) {
                Security.setProperty(key + numPolicy++, userPolicyURL);
	   }
	}
    }


    //
    // Add the 'src' properties to dest. Properties in 'src' will overwrite
    // existing properties in 'dest'
    //
    private void mergeProperties(Properties dest, Properties src) {
	if (dest == null || src == null) return;
	Enumeration en = src.keys();
	while(en.hasMoreElements()) {
	    String key = (String)en.nextElement();
	    String value= src.getProperty(key);
	    dest.setProperty(key, value);
	}
    }
    
    // Return the set of properties that are defined in propA
    // but not in propB
    //
    private Properties diffProperties(Properties propA, Properties propB) {
	if (propB == null) return propA;
	Properties res = new Properties();
	Enumeration en = propA.keys();
	while(en.hasMoreElements()) {
	    String key = (String)en.nextElement();
	    String value= propA.getProperty(key);
	    // Add to set if not in B
	    if (value != null && !value.equals(propB.getProperty(key))) {
		res.setProperty(key, value);
	    }
	}
	return res;
    }
    
    public String toString() {
	return super.toString() + "[" + " : " + _jreInfoList + 
					" : " + _props + "]";
    }


    //
    // updateConfiguration()
    //     This function is called by Main whenever javaws is invoked with 
    //     javaws -UpdateVersions, or the above isVersionCurrent() is false.
    //
    public void updateConfiguration() {
        String oldVersion = getProperty(VERSION_UPDATED_KEY);
        String newVersion =  Globals.getComponentName();

	if (Globals.TraceConfig) {
	    Debug.println("in updateConfiguration, oldVersion = " +
		oldVersion + ", newVersion = " + newVersion);
	}

        if (oldVersion == null || oldVersion.length() == 0) {  
            // Upgrading from 1.0.1 or 1.2
	    String baseDir_12 = getSystemProperty("user.home") +
                    File.separator + ".javaws";
            String cfgFile_12 = baseDir_12 + File.separator + "javaws.cfg";
	    String cacheDefault_12 = baseDir_12 + File.separator + "cache";
	    String jreKey_12 = "javaws.cfg.jre.";
            Properties props_12 = new Properties();

            loadProperties(props_12, cfgFile_12);

            Enumeration en = props_12.keys();

	    // only need old default cache location if there was old cfg file.
            boolean needCacheDir = (new File(cfgFile_12)).exists();
            boolean needJREs = false;
            while(en.hasMoreElements()) {
		String newKey = null;
                String key = (String)en.nextElement();
                if (key.startsWith(jreKey_12)) {
		    needJREs = true;
                } else if (key.equals("javaws.cfg.cache.dir")) {
                    needCacheDir = false;
                    newKey = CACHE_DIR_KEY;
                } else if (key.startsWith("javaws.cfg.")) {
                    newKey = key.replaceFirst("javaws.cfg.", BASE);
                } else if (key.startsWith("javaws.")) {
                    newKey = key.replaceFirst("javaws.", BASE);
                } else {
                    newKey = jreKey_12;
                }
		if (newKey != null) {
		    String value = props_12.getProperty(key);
		    setProperty(newKey, value);
	            if (Globals.TraceConfig) {
                        Debug.println("translated key:"+key+"\n"+
				      "    to new key:"+newKey);
		    }
		}
            }
	    if (needJREs) {
         	readJREInfoProperties(jreKey_12, props_12, 
				      JREInformation.SRC_USER);
	    }
	    if (needCacheDir) {
		setProperty(CACHE_DIR_KEY, cacheDefault_12);
		if (Globals.TraceConfig) {
		    Debug.println("adding cache key:"+CACHE_DIR_KEY+
				  ", value:"+cacheDefault_12);
		}
	    }

        } else if (oldVersion.startsWith("javaws-1.4.2")) {
            // upgrading from 1.4.2
            // thats us - nothing to do
        }
        setVersionUpdated();
        store();
    }
}


