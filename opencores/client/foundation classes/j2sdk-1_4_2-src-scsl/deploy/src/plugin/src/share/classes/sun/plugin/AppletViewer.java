/*
 * @(#)AppletViewer.java	1.166 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin;

/*
 * A recipient to show a Java Applet or a JavaBeans component in.
 * This class implements all platform independent behaviour of showing
 * an applet in one version of the Java Activator.
 *
 * @version 	1.3
 *
 */


import java.applet.Applet;
import java.applet.AppletContext;
import java.awt.Frame;
import java.awt.Color;
import java.awt.Component;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Image;
import java.awt.PopupMenu;
import java.awt.MenuItem;
import java.awt.RenderingHints;
import java.awt.Point;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.io.BufferedInputStream;
import java.io.File;
import java.io.FilenameFilter;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.ObjectInputStream;
import java.lang.reflect.Method;
import java.net.URL;
import java.net.MalformedURLException;
import java.text.MessageFormat;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.Iterator;
import java.util.Properties;
import java.util.StringTokenizer;
import java.util.HashMap;
import java.security.AccessController;
import java.security.Security;
import netscape.javascript.JSObject;
import netscape.javascript.JSException;
import sun.applet.AppletEvent;
import sun.applet.AppletListener;
import sun.applet.AppletPanel;
import sun.applet.AppletClassLoader;
import sun.plugin.viewer.context.PluginAppletContext;
import sun.plugin.javascript.JSContext;
import sun.plugin.cache.JarCacheUtil;
import sun.plugin.net.proxy.PluginProxyManager;
import sun.plugin.net.proxy.PluginProxyServiceProvider;
import sun.plugin.resources.ResourceHandler;
import sun.plugin.security.PluginClassLoader;
import sun.plugin.services.BrowserService;
import sun.plugin.util.ProgressTracker;
import sun.plugin.util.ProgressListener;
import sun.plugin.util.Trace;
import sun.plugin.util.UserProfile;
import sun.plugin.util.ThreadGroupLocal;
import sun.plugin.util.URLUtil;
import sun.plugin.util.DialogFactory;
import sun.plugin.util.Steamer;
import sun.plugin.util.DeploymentConfig;
import com.sun.java.browser.net.ProxyService;
import com.sun.java.browser.net.ProxyServiceProvider;
import sun.net.www.protocol.jar.URLJarFile;


public class AppletViewer extends AppletPanel
    implements ProgressListener, WindowListener
{
    private final ProgressInfo appletProgress = new ProgressInfo();
        
    private boolean progress_bar_local = false;
    
    private Steamer steam_the_cup = null;
    private GrayBoxListener grayBoxListener = null;
    
    private boolean loading_first_time=true;
    private boolean preloading = false;
    private volatile boolean stopped = false;

	private static Frame dummyFrame = new Frame();
    
    /*
     * Background/foreground color for the applet's gray box, color for the
     * progress bar.
     * Optional.
     */
    private Color boxBGColor = Color.lightGray;
    private Color boxFGColor = Color.black;
    private Color progressColor = new Color(153, 153, 204);
    private String userMsg = null;
    private boolean colorSet=false;
    
    /*
     * are we initialized
     */
    private static boolean initialized = false;
    
    /**
     * Some constants...
     */
    public static String theVersion = "1.1";
    
    /**
     * Look here for the properties file
     */
    public static File theUserPropertiesFile;
    
    private URL documentURL = null;
    private URL baseURL = null;
    private static AppletMessageHandler amh = new AppletMessageHandler("appletpanel");
    
    // Parameters handler
    protected java.util.HashMap atts = new java.util.HashMap();
    
    private ClassLoaderInfo cli = null;
    
    // Exception dialogboxes enable/disable status
    private static boolean fShowException = false;
    
    public static void loadPropertiesFiles() 
    {        
	try
	{
	    theUserPropertiesFile = new File(UserProfile.getPropertyFile());
	    // ensure the props folder can be made
	    new File(theUserPropertiesFile.getParent()).mkdirs();
	}
	catch (Throwable e)
	{
	    Trace.printException(e);
	}
    }

    /**
     *	activex control load time is recorded for 
     *	performance analysis
     */	
    public static void setStartTime(long time) {
	String perfProp = System.getProperty("sun.perflog");
	//set the startup time if the performance logger is enabled
	if(perfProp != null ) {
	    try {
		Class perfLogger = Class.forName("sun.misc.PerformanceLogger");
		if (perfLogger != null) {
		    Class [] params = new Class[2];
		    params[0] = String.class;
		    params[1] = Long.TYPE;
		    Method setMethod = perfLogger.getMethod("setStartTime", params);
		    if(setMethod != null) {
			Object [] args = new Object[2];
			args[0] = "Java Plug-in load time";
			args[1] = new Long(time);
			setMethod.invoke(null, args);
		    }
		}
	    } catch(Exception exc) {
		//exc.printStackTrace();
		//Ignore exceptions
	    }
	}
    }

    /**
     * Prepare the enviroment for executing applets.
     */
    public static void initEnvironment(int browserType, long startTime){
	if (initialized)
            return;

	setStartTime(startTime);
	initEnvironment(browserType);
    }
    
    /**
     * Prepare the enviroment for executing applets.
     */
    public static void initEnvironment(int browserType)
    {
        // if we are already initialized, just return
        if (initialized)
            return;
        
        initialized=true;
        
	// to make sure plugin thread group is child of main thread group
	JavaRunTime.getPluginThreadGroup();

        // Initial proxy manager provider
        try {
            ProxyService.setProvider(new PluginProxyServiceProvider());
        }
        catch(Exception e) {
            // should not happen
        }
        
        sun.plugin.services.ServiceProvider.setService(browserType);

	// This is a hack to fix the memory leak problem in Swing.
	// Apparently, when these classes are loaded, it will create
	// Component objects that are stored in static variables.
	// However, each Component object bound to the caller's
	// AppContext. Thus, if these classes are first used by
	// any applet, the applet's AppContext will never be freed.
	// We fix this by loading these classes in the main
	// AppContext, so we won't leak anything.
	//
	try
	{
	    Class a = javax.swing.ImageIcon.class;
	    Class b = javax.swing.plaf.basic.BasicComboBoxUI.class;
	}
	catch (Throwable e)
	{
    
	}

        
        try
        {
            // Initialize the ClassFileTransformer for JDK 1.1
            // class file.
            sun.plugin.security.JDK11ClassFileTransformer.init();
        }
        catch (Throwable e)
        {
            e.printStackTrace();
        }
        
        Properties props = new Properties(System.getProperties());
        
        // Define a number of standard properties
        props.put("acl.read", "+");
        props.put("acl.read.default", "");
        props.put("acl.write", "+");
        props.put("acl.write.default", "");
        
        // Standard browser properties
        props.put("browser.version", theVersion);
        props.put("browser.vendor", "Sun Microsystems, Inc.");
        
        // Set HTTP User-Agent
        props.put("http.agent", "Mozilla/4.0 (" + System.getProperty("os.name") + " " + System.getProperty("os.version") + ")");
        
        // Define which packages can NOT be accessed by applets
        props.put("package.restrict.access.sun", "true");
        //
        // This is important to set the netscape package access to "false".
        // Some applets running in IE and NS will access
        // netscape.javascript.JSObject sometimes. If we set this
        // restriction to "true", these applets will not run at all.
        // However, if we set it to "false", the applet may continue
        // to run by catching an exception.
        props.put("package.restrict.access.netscape", "false");
        
        // Define which packages can NOT be extended by applets
        props.put("package.restrict.definition.java", "true");
        props.put("package.restrict.definition.sun", "true");
        props.put("package.restrict.definition.netscape", "true");
        
        // Define which properties can be read by applets.
        // A property named by "key" can be read only when its twin
        // property "key.applet" is true.  The following ten properties
        // are open by default.  Any other property can be explicitly
        // opened up by the browser user setting key.applet=true in
        // ~/.java/properties.   Or vice versa, any of the following can
        // be overridden by the user's properties.
        props.put("java.version.applet", "true");
        props.put("java.vendor.applet", "true");
        props.put("java.vendor.url.applet", "true");
        props.put("java.class.version.applet", "true");
        props.put("os.name.applet", "true");
        props.put("os.version.applet", "true");
        props.put("os.arch.applet", "true");
        props.put("file.separator.applet", "true");
        props.put("path.separator.applet", "true");
        props.put("line.separator.applet", "true");
        
        // Install new protocol handler
        String pkgs = (String) props.getProperty("java.protocol.handler.pkgs");
        if (pkgs != null)
            props.put("java.protocol.handler.pkgs", pkgs + "|sun.plugin.net.protocol");
        else
            props.put("java.protocol.handler.pkgs", "sun.plugin.net.protocol");
        
        
        // Set allow default user interaction in HTTP/HTTPS
        java.net.URLConnection.setDefaultAllowUserInteraction(true);
        
        // Set default SSL handshaking protocols to SSLv3 and SSLv2Hello
        // because some servers may not be able to handle TLS. #46268654.
        //
        // Set only if users hasn't set it manually.
        //
        if (props.get("https.protocols") == null)
            props.put("https.protocols", "SSLv3,SSLv2Hello");
        
	// Add new system property for proxy authentication
	props.put("http.auth.serializeRequests", "true");
	
	// Load deployment configuration properties
	//
	Properties configProps = UserProfile.getConfigProperties();

	if (configProps != null)
	{
	    try
	    {
		for (Enumeration e = configProps.propertyNames(); e.hasMoreElements();)
		{
		    String key = (String) e.nextElement();
		    props.put(key, configProps.getProperty(key));
    		}	

		// Add system/user security policy in Deployment infrastructure.
		int numPolicy = 1;
		String policyURL = null;

		while (true)
		{
		    policyURL = Security.getProperty("policy.url." + numPolicy);

		    if (policyURL == null)
			break;
		    
//		    props.put("policy.url." + numPolicy, policyURL);

		    numPolicy++;
		}

		String systemPolicyURL = configProps.getProperty("deployment.system.security.policy");
		String userPolicyURL = configProps.getProperty("deployment.user.security.policy");

		// Set system policy 
		if (systemPolicyURL != null)
		{
		    Security.setProperty("policy.url." + numPolicy, systemPolicyURL);
//		    props.put("policy.url." + numPolicy, systemPolicyURL);
		    numPolicy++;
		}

		// Set user policy
		if (userPolicyURL != null)
		{
		    Security.setProperty("policy.url." + numPolicy, userPolicyURL);
//		    props.put("policy.url." + numPolicy, userPolicyURL);
		    numPolicy++;
		}
	    }
	    catch (Exception e)
	    {
		e.printStackTrace();
	    }
	}
	

      
        // Try loading the saved user properties file to override some
        // of the above defaults.
        try {
            FileInputStream fis = new FileInputStream(theUserPropertiesFile);
	    BufferedInputStream bis = new BufferedInputStream(fis);

            props.load(bis);

	    bis.close();
            fis.close();

        } catch (Exception e) {
            // We silently ignore a missing property file.
        }


        // Look in the user choices if we should open the Java Console
	String consoleStartup = props.getProperty("javaplugin.console", "hide");

        if("show".equalsIgnoreCase(consoleStartup))
        {
            //This is when we want full size Java Console.
            sun.plugin.JavaRunTime.showJavaConsole(true);
        }
        else if("nothing".equalsIgnoreCase(consoleStartup))
        {
            // We do not want the Console at all, so don't start it!

            sun.plugin.JavaRunTime.initTraceEnvironment();

//	    sun.plugin.JavaRunTime.isJavaConsoleVisible();
        }
        else
        {
//	    sun.plugin.JavaRunTime.initTraceEnvironment();

	    // Hide Java Console
	    BrowserService service = sun.plugin.services.ServiceProvider.getService();

	    // Start console in hidden state if console cannot be
	    // accessible through browser menu. Otherwise, delay 
	    // console loading until users access the menu
	    //
	    if (service.isConsoleIconifiedOnClose())
	    {
		// Start console in hidden/iconified state
		sun.plugin.JavaRunTime.showJavaConsole(false);
	    }
	    else
	    {
		// Delay console to start
		sun.plugin.JavaRunTime.initTraceEnvironment();
	    }

//	    sun.plugin.JavaRunTime.showJavaConsole(false);
        }
        
        //Check whether the exception dialogboxes are allowed
        if ("true".equalsIgnoreCase(props.getProperty("javaplugin.exception", "false"))) {
            fShowException = true;
        }
        
        // Set the default connection timeout value
        // to 120 seconds (2 minutes)
        String timeout = (String) props.getProperty("sun.net.client.defaultConnectTimeout", "120000");
            
        // Connection timeout not already set, so set it
        props.put("sun.net.client.defaultConnectTimeout", timeout);

        
        try {
            Class c = Class.forName("sun.misc.ExtensionDependency");
            if (c != null) {
                Class[] parms = new Class[1];
                parms[0] = Class.forName("sun.misc.ExtensionInstallationProvider");
                java.lang.reflect.Method m = c.getMethod("addExtensionInstallationProvider", parms);
                if (m != null) {
                    Object[] args = new Object[1];
                    args[0] = new sun.plugin.extension.ExtensionInstallationImpl();
                    m.invoke(null, args);
                } else {
                    Trace.msgPrintln("optpkg.install.error.nomethod");
                }
            } else {
                Trace.msgPrintln("optpkg.install.error.noclass");
            }
        } catch(Throwable e) {
            Trace.printException(e);
        }
               
        // Remove Proxy Host & Port
        props.remove("proxyHost");
        props.remove("proxyPort");
        props.remove("http.proxyHost");
        props.remove("http.proxyPort");
        props.remove("https.proxyHost");
        props.remove("https.proxyPort");
        props.remove("ftpProxyHost");
        props.remove("ftpProxyPort");
        props.remove("ftpProxySet");
        props.remove("gopherProxyHost");
        props.remove("gopherProxyPort");
        props.remove("gopherProxySet");
        props.remove("socksProxyHost");
        props.remove("socksProxyPort");
        
        // Enable proxy/web server authentication
        if ("true".equalsIgnoreCase(
        props.getProperty("javaplugin.proxy.authentication", "true"))) {
            
            java.net.Authenticator.setDefault(new sun.plugin.security.PluginAuthenticator());
        }
        
        
        // Install a property list.
        System.setProperties(props);
        
        System.out.println("");
        
        // Reset plugin proxy manager
        sun.plugin.net.proxy.PluginProxyManager.reset();
        
        System.out.println("");
        
        // Initialize the JAR cache.  This will output the JAR cache
        // settings to the console.
        sun.plugin.cache.JarCache.init();
        
        // Dump properties - if in trace mode
        if (Trace.isEnabled())
        {
            sun.plugin.JavaRunTime.showSystemProperties();
        }
        
        System.out.println("");
        
        /* we reference this class to get it loaded
         * from a class w/o a classloader.  If not,
         * it first gets loaded from a thread w/ an
         * applet on the stack, and then it can't read
         * it's own content-types.properties file.
         */
        try {
            sun.net.www.MimeTable mt =
            sun.net.www.MimeTable.getDefaultTable();
        } catch (Throwable e) {
            Trace.printException(e);
        }
        

        // Install security manager, so everyone feels
        // warm and fuzzy.  Hold me, the way I need to be held.
        
        sun.plugin.security.ActivatorSecurityManager security = new sun.plugin.security.ActivatorSecurityManager();
        System.setSecurityManager(security);
        
        // REMIND: Create and install a socket factory!
        
        // All initialization is done, show help information in Java Console.
        sun.plugin.JavaRunTime.showJavaConsoleHelp();
        
        // Set the interface to call back to plugin.
        URLJarFile.setCallBack(new PluginURLJarFileCallBack());       
    }
    
    /**
     * Construct a new applet viewer.
     * Restricted to subclasses for security reasons,
     */
    public AppletViewer()
    {
        // Don't set it to light gray!  User could specify color, then gray will blink in
        //the beginning - not pretty.
        //setBackground(Color.lightGray);
    }

    private static class AppletEventListener implements AppletListener
    {
        public void appletStateChanged(AppletEvent evt) {
            switch (evt.getID()) {
                case AppletPanel.APPLET_RESIZE: {
                    AppletViewer src = (AppletViewer) evt.getSource();
                    if(src != null)
                    {
                        Object obj = src.getViewedObject();
                        
                        if (obj instanceof Component)
                        {
                            src.setSize(src.getSize());
                            ((Component)obj).setSize(src.getSize());
                            src.validate();
                        }
                    }
                }
            }
        }
    }
    
    private AppletEventListener appletEventListener = new AppletEventListener();
    
    private Object syncInit = new Object();
    private boolean bInit = false;
    
    /**
     * Init the applet, called once to start the download of the applet
     */
    public void appletInit()
    {
        // Add applet event listener
        addAppletListener(appletEventListener);
        
        //Use the cache_version to mark the cached jar files as
        //uptodate or not.
        String jpi_archive = getParameter("cache_archive");
        String jpi_version = getParameter("cache_version");
        String jpi_archive_ex = getParameter("cache_archive_ex");
        
        try{
            //Obtain the map of jar files and their version
            HashMap jarVersionMap = JarCacheUtil.getJarsWithVersion(jpi_archive, jpi_version,
            jpi_archive_ex);
            if(!jarVersionMap.isEmpty())
                JarCacheUtil.verifyJarVersions(getCodeBase(), getClassLoaderCacheKey(), jarVersionMap);
            
            
        } catch(Exception ex) {
            Trace.printException(ex, ResourceHandler.getMessage("cache.error.text"),
            ResourceHandler.getMessage("cache.error.caption"));
        }
        
        
        // AddRef to classloader
        cli = ClassLoaderInfo.find(getCodeBase(), getClassLoaderCacheKey());
        cli.addReference();
        
        // Add applet into AppletContext
        appletContext.addAppletPanelInContext(this);
        
        // We must synchronized the class because
        // super.init() will create classloader -
        // and we need the creation to be synchronized
        //
        synchronized(AppletViewer.class)
        {
            super.init();
        }
        
        // After super.init() is called, applet handler thread is
        // created. Added applet thread group into tracing.
        //
        Thread t = getAppletHandlerThread();
//        Trace.addTraceListener(t.getThreadGroup(), this);        

	// Setup animation in graybox through steamer                
        steam_the_cup = new Steamer(this);

        // See if user specified special image to be shown while applet is loading.
        String user_image = getParameter("image");
        if (user_image != null)
            steam_the_cup.setCustomImage(getCodeBase(), user_image);
	                    
	steam_the_cup.init();


        // Add ourselves to progress tracker and
        // create timer object for dispatching gray box
        // messages and progress bar if user asked for progress
        if (progress_bar_local)
	{
	    ThreadGroupLocal.put(t.getThreadGroup(), ThreadGroupLocal.PROGRESS_TRACKING, Boolean.TRUE);

            ProgressTracker.addProgressListener(t.getThreadGroup(), this);
             
            grayBoxPainter = new GrayBoxPainter(t.getThreadGroup(), this);
            grayBoxPainter.start();
            
            // Initialize progress info
            appletProgress.init(this);
        }
        
	// Add mouse listener                
	grayBoxListener = new GrayBoxListener(this, userMsg);
        addMouseListener(grayBoxListener);
        
        Trace.msgPrintln("applet.progress.load");
        sendEvent(sun.applet.AppletPanel.APPLET_LOAD);
        Trace.msgPrintln("applet.progress.init");
        sendEvent(sun.applet.AppletPanel.APPLET_INIT);
        
        // Mark viewer as initialized
        synchronized(syncInit)
        {
            bInit = true;
        }
    }
    
    /**
     * Start the applet.
     */
    public void appletStart()
    {
        // Do nothing if viewer hasn't been initialized
        synchronized(syncInit)
        {
            if (bInit == false)
                return;
        }
        
	stopped = false;

        Trace.msgPrintln("applet.progress.start");
        sendEvent(sun.applet.AppletPanel.APPLET_START);
    }
    
    /**
     * Stop the applet.
     */
    public void appletStop()
    {
        // Do nothing if viewer hasn't been initialized
        synchronized(syncInit)
        {
            if (bInit == false)
                return;
        }
        
        if (status==APPLET_LOAD)
        {
            Trace.msgPrintln("applet.progress.stoploading");
            stopLoading();
        }
        
	stopped = true;

        Trace.msgPrintln("applet.progress.stop");
        sendEvent(sun.applet.AppletPanel.APPLET_STOP);
    }
    
    /**
     * Notification that the applet is being closed
	 *
	 * @since 1.4.1
	 */
    public void appletDestroy() {
	appletDestroy(1000);
    }
	     
    /**
     * Notification that the applet is being closed
     *
     * @param timeOut max time we are waiting for the applet to die
     *		in milliseconds.
     */
    public void appletDestroy(long timeOut)
    {
        // Do nothing if viewer hasn't been initialized
        synchronized(syncInit)
        {
            if (bInit == false)
                return;
        }

	// Destroy the steamer       
	if (steam_the_cup != null)
	{
	    steam_the_cup.destroy();
	    steam_the_cup = null;
	}

        // Destroy gray box painter
        if (grayBoxPainter != null)
        {
            // Interrupt
            grayBoxPainter.interrupt();
            grayBoxPainter = null;
        }

	// Remove graybox listener
	if (grayBoxListener != null)
	{
    	    removeMouseListener(grayBoxListener);
	    grayBoxListener = null;
	}
        
        // Remove applet event listener
        removeAppletListener(appletEventListener);
        appletEventListener = null;
        
        // Remove applet from the context first, so it won't be called
        appletContext.removeAppletPanelFromContext(this);
        
        // Remove applet's thread group from tracing. Applet may
        // still be running, so obtain applet handler thread
        // is legal.
        Thread t = getAppletHandlerThread();
        
        if (t != null)
        {
            if (progress_bar_local){
                ProgressTracker.removeProgressListener(t.getThreadGroup(), this);
		ThreadGroupLocal.destroy(t.getThreadGroup());
            }
//            Trace.removeTraceListener(t.getThreadGroup(), this);
        }
        
        // Just make sure we are destroying the thread from a utility
        // thread to not block the main thread for that.
        
        Runnable work = new Runnable() {
            public void run() {
                onPrivateClose(5000);
            }
        };
        Thread closingThread = new Thread(work);
        closingThread.start();
        
        
        // Let's wait for one second before page switch
        // to allow applets to be killed properly
        //
        try {
            // Let's wait no more than 1 secs that the handler thread
            // die of natural causes.
            Trace.msgPrintln("applet.progress.joining");
            
            if (t != null)
                t.join(timeOut);
            
            Trace.msgPrintln("applet.progress.joined");
        } catch (InterruptedException e) {
            // Ignore
        }
    }
    
    
    /**
     * Notification that the applet is being closed
     *
     * @param timeOut max time we are waiting for the applet to die
     *		in milliseconds.
     */
    protected void onPrivateClose(int timeOut)
    {
        Trace.msgPrintln("applet.progress.destroy");
        sendEvent(APPLET_DESTROY);
        Trace.msgPrintln("applet.progress.dispose");
        sendEvent(APPLET_DISPOSE);
        Trace.msgPrintln("applet.progress.quit");
        sendEvent(APPLET_QUIT);
        
        // Release classloader
        Trace.msgPrintln("applet.progress.findinfo.0");
        cli.removeReference();
        cli = null;
        Trace.msgPrintln("applet.progress.findinfo.1");
        
        // Reset applet context
        this.appletContext = null;
    }
    

    /**
     * Pre-refresh the applet
     */
    public void preRefresh()
    {
	// Mark the classloader as non-cachable
        if (cli != null)    
	{
	    cli.markNotCachable(getCodeBase(), getClassLoaderCacheKey());
	}
    }
    
    /**
     * Get an applet parameter.
     */
    public String getParameter(String name) {
        name = name.toLowerCase(java.util.Locale.ENGLISH);
        
        synchronized(atts)
        {
            String value = (String) atts.get(name);
            
            if (value != null)
                value = trimWhiteSpaces(value);
            
            return value;
        }
    }
    
    /**
     * Set an applet parameter.
     */
    public void setParameter(String name, Object value) {
        name = name.toLowerCase(java.util.Locale.ENGLISH);
        
        synchronized(atts)
        {
            atts.put(name, trimWhiteSpaces(value.toString()));
        }
    }
    
    /**
     * Trim whitespaces
     */
    private String trimWhiteSpaces(String str)
    {
        if (str == null)
            return str;
        
        StringBuffer buffer = new StringBuffer();
        
        for (int i=0; i < str.length(); i++)
        {
            char c = str.charAt(i);
            
            // Skip over whitespaces
            if (c == '\n' || c == '\f' || c == '\r' || c == '\t')
                continue;
            else
                buffer.append(c);
        }
        
        // Trim whitespaces on both ends of the strings
        return buffer.toString().trim();
    }
    
    private boolean docbaseInit = false;
    private Object docBaseSyncObj = new Object();
    
    /**
     * Get the document url.
     */
    public void setDocumentBase(String url)
    {
        if (docbaseInit == false)
        {
	    // Canonicalize URL if necessary
            String urlString = URLUtil.canonicalize(url);
                      
            try
            {
                documentURL = new URL(canonicalizeDocumentURL(urlString));
            }
            catch (Throwable e)
            {
                e.printStackTrace();
            }
            
            docbaseInit = true;
            
            // Notify all the pending getDocumentBase() calls
            synchronized(docBaseSyncObj)
            {
                docBaseSyncObj.notifyAll();
            }
        }
    }
    
    /**
     * Canonicalize URL.
     */
    public String canonicalizeDocumentURL(String url) {
        
        int fromIndex=-1,lastIndex;
        
        // Strip off "#" and "?" from URL
        int fragmentIndex = url.indexOf('#');
        int queryIndex = url.indexOf('?');
        
        if(queryIndex != -1 && fragmentIndex != -1)
        {
            fromIndex = Math.min(fragmentIndex, queryIndex);
        }
        else if(fragmentIndex != -1)
        {
            fromIndex = fragmentIndex;
        }
        else if(queryIndex != -1)
        {
            fromIndex = queryIndex;
        }
        
        // Strip off the end of the URL
        String strippedURL;
        
        if (fromIndex == -1)
            strippedURL = url;
        else
            strippedURL = url.substring(0, fromIndex);
        
        
        // Replace "|" character with ":" 
        StringBuffer urlBuffer = new StringBuffer(strippedURL);
        int index = urlBuffer.toString().indexOf("|");
        if (index >= 0)
        {
            urlBuffer.setCharAt(index, ':'); 
        }
        
        if (fromIndex != -1)
	    urlBuffer.append(url.substring(fromIndex));
       
	return urlBuffer.toString();
			     
    }
    
    
    /**
     * Get the document url.
     */
    public URL getDocumentBase()
    {
        Object syncObject = new Object();
        
        synchronized(syncObject)
        {
            if (docbaseInit == false)
            {
                BrowserService service = sun.plugin.services.ServiceProvider.getService();
                
                if (service.isNetscape() && service.getBrowserVersion() >= 5.0f)
                {
                    // If we are running in Netscape 6.x, we MUST wait for setDocumentBase()
                    // to get the document base.
                    try
                    {
                        synchronized(docBaseSyncObj)
                        {
                            while (docbaseInit == false)
                                docBaseSyncObj.wait(0);
                        }
                    }catch (InterruptedException e)
                    {
                        e.printStackTrace();
                    }
                }
                else
                {
                    // Otherwise, obtain the document base directly through JSObject
                    //
                    JSContext jsCtx = (JSContext) getAppletContext();
                    
                    try
                    {
                        JSObject win = (JSObject) jsCtx.getJSObject();
                        if (win == null)
                            throw new JSException("Unable to obtain Window object");
                        
                        JSObject doc = (JSObject) win.getMember("document");
                        if (doc == null)
                            throw new JSException("Unable to obtain Document object");
                        
                        String url = (String) doc.getMember("URL");
                        
			// Canonicalize URL in case the URL is in some
			// weird form only recognized by the browsers
			//
                        String docBase = URLUtil.canonicalize(url);
                                            
                        documentURL = new URL(canonicalizeDocumentURL(docBase));
                    }
                    catch (Throwable e)
                    {
                        e.printStackTrace();
                    }

                    docbaseInit = true;
                }
            }
        }
        
        return documentURL;
    }
    
    private boolean codeBaseInit = false;
    
    
    /**
     * Get the base url.
     */
    public URL getCodeBase()
    {
        Object syncObject = new Object();
        
        synchronized(syncObject)
        {
            if (codeBaseInit == false)
            {
                String att = getParameter("java_codebase");

                if (att == null)
                    att = getParameter("codebase");

                if (att != null) 
		{
                    if (!att.equals(".") && !att.endsWith("/")) 
		    {
                        att += "/";
                    }

		    // Canonicalize URL in case the URL is in some
		    // weird form only recognized by the browser
		    //
		    att = URLUtil.canonicalize(att);

                    try {
                        baseURL = new URL(getDocumentBase(), att);
                    } catch (MalformedURLException e) {
                        // ignore exception
                    }
                }

		// Obtain doc base
		URL docBase = getDocumentBase();

                if (baseURL == null) 
		{
                    String urlString = docBase.toString();
                    int i = urlString.lastIndexOf('/');
                    if (i > -1 && i < urlString.length() - 1) {
                        try 
			{
			    // Canonicalize URL to ensure it is well-formed
                            baseURL = new URL(URLUtil.canonicalize(urlString.substring(0, i + 1)));
                        } 
			catch (MalformedURLException e) 
			{
                            // ignore exception
                        }
                    }

		    // when all is said & done, baseURL shouldn't be null
		    if (baseURL == null)
			baseURL = docBase;
		}
                
                codeBaseInit = true;
            }
        }
        
        return baseURL;
    }
    
    /**
     * Get the width.
     */
    public int getWidth() {
        String w = getParameter("width");
        if (w != null) {
            return Integer.valueOf(w).intValue();
        }
        return 0;
    }
    
    /**
     * Get the height.
     */
    public int getHeight() {
        String h = getParameter("height");
        if (h != null) {
            return Integer.valueOf(h).intValue();
        }
        return 0;
    }
    
    /**
     * Get the code parameter
     */
    public String getCode() {
        
        // Support HTML 4.0 style of OBJECT tag.
        //
        // <OBJECT classid=java:sun.plugin.MyClass .....>
        // <PARAM ....>
        // </OBJECT>
        //
        // In this case, the CODE will be inside the classid
        // attribute.
        //
        String moniker = getParameter("classid");
        String code = null;
        
        if (moniker != null)
        {
            int index = moniker.indexOf("java:");
            
            if (index > -1)
            {
                code = moniker.substring(5 + index);
                
                if (code != null || !code.equals(""))
                    return code;
            }
        }
        
        code = getParameter("java_code");
        if (code==null)
            code=getParameter("code");
        return code;
    }
    
    /**
     * Return true if the applet requires legacy lifecycle
     */
    public boolean isLegacyLifeCycle()
    {
        String lifecycle = getParameter("legacy_lifecycle");
        
        if (lifecycle != null && lifecycle.equalsIgnoreCase("true"))
            return true;
        else
            return false;
    }
    
    
    private String classLoaderCacheKey = null;
    
    /**
     * Return the caching key for classloader
     */
    public String getClassLoaderCacheKey()
    {
        // Fixed #4516442 - Switch for classloader policy.
        //
        // If classic classloader policy is set to true
        //
        String param = getParameter("classloader-policy");
        
        if (param != null && param.equals("classic"))
            return super.getClassLoaderCacheKey();
        
        // This method is responsible for determining
        // the classloader sharing policy.
        //
        // Classloader is shared between applets if
        // and only if
        //
        // 1. codebase's values are the same
        // 2. list of jar files are the same
        //
        if (classLoaderCacheKey == null)
        {
            StringBuffer buffer = new StringBuffer();
            
            buffer.append(getCodeBase());
            
            // Append cache_archive
            String value = getParameter("cache_archive");
            if (value != null)
            {
                buffer.append(",");
                buffer.append(value);
            }
            
            // Append java_archive
            value = getParameter("java_archive");
            if (value != null)
            {
                buffer.append(",");
                buffer.append(value);
            }
            
            // Append archive
            value = getParameter("archive");
            if (value != null)
            {
                buffer.append(",");
                buffer.append(value);
            }
            
            classLoaderCacheKey = buffer.toString();
        }
        
        return classLoaderCacheKey;
    }
    
    
    /**
     * Return the list of jar files if specified.
     * Otherwise return null.
     */
    public String getJarFiles()
    {
        // Figure out the list of all required JARs.
        String archive = getParameter("archive");
        String java_archive = getParameter("java_archive");
        String jpi_archive = getParameter("cache_archive");
        String jpi_archive_ex = getParameter("cache_archive_ex");
        String archive_ex = null;
        
        if(jpi_archive_ex != null) {
            int index = jpi_archive_ex.indexOf(";");
            if(index != -1) {
                archive_ex = JarCacheUtil.getJarsInCacheArchiveEx(jpi_archive_ex);
            } else {
                archive_ex = jpi_archive_ex;
            }
        }
        
        return addJarFileToPath( archive_ex,
        addJarFileToPath(jpi_archive,
        addJarFileToPath(java_archive, archive))
        );
    }
    
    private String addJarFileToPath(String original, String jar)
    {
        if(original == null && jar == null)
        {
            return null;
        }
        
        if(original == null && jar != null)
        {
            return jar;
        }
        
        if(original != null && jar == null)
        {
            return original;
        }
        
        return original + "," + jar;
    }
    
   /*
    *  Allow pre-loading of local .jar files in plug-in lib/app directory
    *  These .jar files are loaded with the PluginClassLoader so they
    *  run in the applet's sandbox thereby saving developers the trouble
    *  of writing trusted support classes.
    *  The ClassLoaderInfo cli should be locked.
    */
    private void loadLocalJarFiles(PluginClassLoader loader)
    {
        String fSep = File.separator;
        String basePath = System.getProperty("java.home") + fSep + "lib" + fSep + "applet";

        File dir = new File(basePath);

        if (dir.exists())  
	{
            String[] jarList = dir.list(new FilenameFilter()  
	    {
                public boolean accept(File f, String s)  {
                    return(s.endsWith(".jar"));
                }
            });
            
            for (int i = 0; i < jarList.length; i++)  
	    {
		try 
		{
		    URL localJarUrl = (new File(basePath + fSep + jarList[i])).toURL(); 

		    loader.addLocalJar(localJarUrl);
		}
		catch (MalformedURLException e)
		{
		    e.printStackTrace();
		}
            }
        }
    }  
    
    
    /*
     * We overload our parent loadJarFiles so tht we can avoid
     * reloading JAR files that are already loaded.
     *						KGH Mar 98
     */
    protected void loadJarFiles(AppletClassLoader loader)
    throws IOException, InterruptedException
    {
        if (loading_first_time)
        {
            loading_first_time=false;
            String jpi_archive_ex = getParameter("cache_archive_ex");
            try{
                //pre-load jar files are loaded first
                if(jpi_archive_ex != null){
                    preloading = true;
                    JarCacheUtil.preload(getCodeBase(), jpi_archive_ex);
                    preloading = false;
                }
            } catch(Exception ex) {
                Trace.printException(ex, ResourceHandler.getMessage("cache.error.text"),
                ResourceHandler.getMessage("cache.error.caption"));
            }
        }
        
        // Get a list of jar files
        String archive = getJarFiles();
        
        try
        {
            // Prevent two applets trying to load JARS from the same
            // classloader at the same time.
            cli.lock();
            
            if (!cli.getLocalJarsLoaded() &&
            (loader instanceof PluginClassLoader) )
            {
		loadLocalJarFiles((PluginClassLoader) loader);
                cli.setLocalJarsLoaded(true);
            }
            
            // If there are no JARs, this is easy.
            if (archive == null)
            {
                return;
            }
            
            // Figure out which JAR files still need to be loaded.
            StringTokenizer st = new StringTokenizer(archive, ",", false);
            while(st.hasMoreTokens())
            {
                String tok = st.nextToken().trim();
                if (cli.hasJar(tok))
                {
                    continue;
                }
                
                cli.addJar(tok);
            }
            
            // Now call into our superlcass to do the actual JAR loading.
            // It will call back to our getJarFiles method to find which
            // JARs need to be loaded, and we will give it the cooked list.
            super.loadJarFiles(loader);
        }
        finally
        {
            // Other people can load JARs now.
            cli.unlock();
        }
    }
    
    
  /**
   * Return the value of the object param
   */
    public String getSerializedObject() {
        String object = getParameter("java_object");
        if (object==null)
            object=getParameter("object");// another name?
        return object;
    }
    
    public Applet getApplet() {
        Applet applet = super.getApplet();
        if (applet!=null) {
            if (applet instanceof BeansApplet)
                return null;
            else
                return applet;
        }
        return null;
    }
    
    
    /*
     * return the viewed object
     */
    public Object getViewedObject() {
        Applet applet = super.getApplet();
        if (applet instanceof BeansApplet)
            return ((BeansApplet) applet).bean;
        else
            return applet;
    }
    
    
    /**
     * Set the applet context. For now this is
     * also implemented by the AppletPanel class.
     */
    public void setAppletContext(AppletContext appletContext) {
        
        if (appletContext==null)
            throw new IllegalArgumentException("AppletContext");
        
        if (this.appletContext != null)
            this.appletContext.removeAppletPanelFromContext(this);
        
        this.appletContext = (PluginAppletContext) appletContext;
    }
    
    /**
     * Get the applet context. For now this is
     * also implemented by the AppletPanel class.
     */
    public AppletContext getAppletContext() {
        return appletContext;
    }
    

    /**
     * Implement getParent() method, return null when caller doesn't have
     * AllPermission, which prevent caller get to AppletContext level.
     */
    /*public Container getParent()
    {
        Container con = null;
     
        try {
            Permission perm = new AllPermission();
            AccessControlContext acc = AccessController.getContext();
            acc.checkPermission(perm);
            con = super.getParent();
        }
        catch(Throwable e) {
            con = null;
        }
     
        return con;
    }*/
    
    /*
     * See if user specified the background color for the applet's gray box
     * or a foreground color and set these values for the applet.
     */
    public void setColorAndText()
    {
        Color val = null;
        /*
         * See if user specified any colors
         *      BOXBGCOLOR - background color for the gray box
         *      BOXFGCOLOR - foreground color for the gray box
         *      PROGRESSCOLOR - color for the progress bar.
         *      BOXMESSAGE - user-defined message for the applet viewer.
         * If not - use defaults - light gray for background, black for foreground,
         * and purple for the progress bar color.
         */
        String boxBGColorStr = getParameter("boxbgcolor");
        if(boxBGColorStr != null){
            val = createColor("boxbgcolor", boxBGColorStr);
            
            /*
             * If user specified valid color, set background color.
             */
            if (val != null)
                boxBGColor = val;
        }
        setBackground(boxBGColor);
        
        String boxFGColorStr = getParameter("boxfgcolor");
        if(boxFGColorStr != null){
            val = createColor("boxfgcolor", boxFGColorStr);
            
            /*
             * If user specified valid color, set foreground color.
             */
            if (val != null)
                boxFGColor = val;
        }
        
        String progressColorStr = getParameter("progresscolor");
        if(progressColorStr != null){
            val = createColor("progresscolor", progressColorStr);
            
            /*
             * If user specified valid color, set progress color.
             */
            if (val != null)
                progressColor = val;
        }
        
        userMsg = getParameter("boxmessage");

        // See if user wants to see progress bar in the applet viewer area.
        String progressON = getParameter("progressbar");
        if (progressON!=null){
            progress_bar_local = new Boolean(progressON).booleanValue(); 
        }
        
        /*
         * Set or not, we've done it once and not going to do it again!!!
         */
        colorSet=true;
    }
    
    /*
     * Create color
     * @param html_tag - name of the html tag to use in tracing message
     * @param str - string representation of color.  It should consist three digits
     *              separated by commas, or one of standard colors.
     *
     * Returns color or null(if number of components for RGB color was incorrect,
     * or if standard color was spelled incorrectly).
     */
    private Color createColor(String html_tag, String str)
    {
        if (str != null && str.indexOf(",") != -1)
        {
            // This is the RGB format.  Tokenize the string.
            StringTokenizer st = new StringTokenizer(str, ",");
            if (st.countTokens()==3)
            {
                // We've got three components for the color.
                int i=0;
                int red=0, green=0, blue=0;
                while (st.hasMoreTokens())
                {
                    String token = (String)st.nextElement();
                    switch(i){
                        case 0:
                        {
                            if(! token.trim().equals(""))
                                red = new Integer(token.trim()).intValue();
                            break;
                        }
                        case 1:
                        {
                            if (! token.trim().equals(""))
                                green = new Integer(token.trim()).intValue();
                            break;
                        }
                        case 2:
                        {
                            if(! token.trim().equals(""))
                                blue = new Integer(token.trim()).intValue();
                            break;
                        }
                    }
                    i++;
                }
                return new Color(red, green, blue);
            }
            else
            {
                Trace.msgPrintln("applet_viewer.color_tag", new Object[] {html_tag});
                return null;
            }
            
        }
        else if(str != null)
        {
            //Check & decode if the color is in hexadecimal color format (i.e. #808000)
            try {
                return Color.decode(str);
            }
            catch (NumberFormatException e) {
                // ignore exception
            }
            
            //This is a string representation of color
            if(str.equalsIgnoreCase("red"))
                return Color.red;
            if(str.equalsIgnoreCase("yellow"))
                return Color.yellow;
            if(str.equalsIgnoreCase("black"))
                return Color.black;
            if(str.equalsIgnoreCase("blue"))
                return Color.blue;
            if(str.equalsIgnoreCase("cyan")
            || str.equalsIgnoreCase("aqua"))
                return Color.cyan;
            if(str.equalsIgnoreCase("darkGray"))
                return Color.darkGray;
            if(str.equalsIgnoreCase("gray"))
                return Color.gray;
            if(str.equalsIgnoreCase("lightGray")
            || str.equalsIgnoreCase("silver"))
                return Color.lightGray;
            // green is Java std #00ff00 not w3 HTML std.
            // w3 HTML std - lime is #00ff00 & green is #008000
            if(str.equalsIgnoreCase("green")
            || str.equalsIgnoreCase("lime"))
                return Color.green;
            if(str.equalsIgnoreCase("magenta")
            || str.equalsIgnoreCase("fuchsia"))
                return Color.magenta;
            if(str.equalsIgnoreCase("orange"))
                return Color.orange;
            if(str.equalsIgnoreCase("pink"))
                return Color.pink;
            if(str.equalsIgnoreCase("white"))
                return Color.white;
            if(str.equalsIgnoreCase("maroon"))
                return new Color(128, 0, 0);
            if(str.equalsIgnoreCase("purple"))
                return new Color(128, 0, 128);
            if(str.equalsIgnoreCase("navy"))
                return new Color(0, 0, 128);
            if(str.equalsIgnoreCase("teal"))
                return new Color(0, 128, 128);
            if(str.equalsIgnoreCase("olive"))
                return new Color(128, 128, 0);
        }
        
        /*
         * Misspelling?
         */
        return null;
    }
    
    /**
     * Paint this panel while visible and loading an applet to entertain
     * the user.  paint() is called by AWT to repaint this component.
     *
     * @param g the graphics context
     */
    public void paint(Graphics g)
    {
        Dimension d = getSize();
        
        if (d.width > 0 && d.height > 0 &&
        (status == APPLET_LOAD || status == APPLET_INIT || status == APPLET_ERROR))
        {
            paintForegrnd(g);
        }
        else
        {
            super.paint(g);
        }
    }

	// Fix bug #4671775. If the foreground color is not set, set to white
	// to avoid NullPointerException inside Java2D code.
	public Color getForeground() {
		Color c = super.getForeground();
		if(null == c)
			c = Color.BLACK;

		return c;
	}
    
    /**
     * Paint this panel while visible and loading an applet to entertain
     * the user.  This method will be called by update() when repainting by request
     * from GrayBoxPainter and by paint() when repainting by request from AWT.
     *
     * @param g the graphics context
     */
    public void paintForegrnd(Graphics g)
    {

        Dimension d = getSize();
        
        if (d.width > 0 && d.height > 0)
        {
            // Create buffered image for double-buffering
            Image image = createImage(d.width, d.height);
            Graphics2D bg = (Graphics2D) image.getGraphics();
            
            // Use antialiasing
            bg.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
            RenderingHints.VALUE_ANTIALIAS_ON);
            
            // Set foreground and background colors
            bg.setBackground(boxBGColor);
            bg.setColor(boxBGColor);
            
            // Erase background
            bg.fillRect(0, 0, d.width, d.height);
            
            // Set font
            Font font = bg.getFont();
            font = font.deriveFont(Font.PLAIN, 10);
            bg.setFont(font);
            
            FontMetrics fm = bg.getFontMetrics(font);
                      
            // Display progress bar or image on the screen if the applet is not started yet
            // and if width and height of the applet is greater of equals to 36 pixels.
            if (d != null && fm != null && d.width >= 36 && d.height >= 36)
            {
                
                //Draw progress bar if user asked for it.
                if (progress_bar_local){
                    /*
                     * Set the foreground color for the applet viewer.
                     */
                    bg.setColor(boxFGColor);
                    
		    // Default string to show in the loading box and in the status bar.
		    String str = null;
            
		    // Set message string to custom message if it is specified.
		    if (userMsg != null)
			str = userMsg;
		    else
			str = getWaitingMessage();

                    bg.drawString(str, (d.width - fm.stringWidth(str)) / 2,
                    (d.height + fm.getAscent()) / 2);
                    
                    bg.drawRect((int)(d.width * 0.2),
                    (int)(d.height + fm.getAscent()) / 2 + fm.getHeight() - 1, //1 pixel
                    (int)(d.width * 0.6),
                    fm.getHeight() + 1);// 1 pixel
                    
                    /*
                     * Set the color for the progress bar.
                     */
                    bg.setColor(progressColor);
                    
                    // Determine the length of the actual progress bar
                    int length = 0;
                    
                    synchronized(appletProgress)
                    {
                        length = (int) ((d.width * 0.6 - 1) * appletProgress.getProgress() / appletProgress.getProgressMax());
                    }
                    
                    bg.fillRect((int) (d.width * 0.2 + 1) ,
                    (int) ((d.height + fm.getAscent()) / 2 + fm.getHeight()),
                    length,
                    fm.getHeight());
                    
                }//end draw progress bar
                
                else {  //draw image

                    bg.setColor(boxBGColor);
                    
		    if (steam_the_cup != null)
		    {
			Image img = steam_the_cup.getCurrentImage();

			if (img != null)
			{
			    Point p = steam_the_cup.getImgCoordinates();
			    bg.drawImage(img, (int)p.getX(), (int)p.getY(), null);
			}
		    }

                }//end draw image
            }
            g.drawImage(image, 0, 0, null);
	    bg.dispose();
            image.flush();
        }
    }
    
    public String getWaitingMessage() 
    {
	if (status == APPLET_ERROR)
	{
	    return getMessage("failed");
	}
	else
	{
	    MessageFormat mf = new MessageFormat(getMessage("loading"));
	    return mf.format(new Object[] {getHandledType()});
	}
    }
    
    
    /*
     * <p>
     * Load an applet from a serialized stream. This is likely to happen
     * when the user uses the Back/Forward buttons
     * </p>
     *
     * @param is Input stream of the serialized applet
     */
    protected void load(java.io.InputStream is) {
        this.is = is;
    }
    
    protected Applet createApplet(AppletClassLoader loader) throws ClassNotFoundException,
    IllegalAccessException, IOException, InstantiationException, InterruptedException {
        
        // If we are not in a reload...
        if (is==null)
            return super.createApplet(loader);
        
        ObjectInputStream ois =
        new AppletObjectInputStream(is, loader);
        Object serObject = ois.readObject();
        Applet applet = (Applet) serObject;
        doInit = false; // skip over the first init
        if (Thread.interrupted()) {
            try {
                status = APPLET_DISPOSE; // APPLET_ERROR?
                applet = null;
                // REMIND: This may not be exactly the right thing: the
                // status is set by the stop button and not necessarily
                // here.
                showAppletStatus("death");
            } finally {
                Thread.currentThread().interrupt(); // resignal interrupt
            }
            return null;
        }
        is = null;
        return applet;
    }
    /*
     * @return the applet name
     */
    public String getName()  {
        
        String name = getParameter("name");
        if (name!=null)
            return name;
        
        // Remove .class extension
        name = getCode();
        if (name != null){
            int index = name.lastIndexOf(".class");
            if (index != -1)
                name = name.substring(0, index);
        } else{
            // Remove .object extension
            name = getSerializedObject();
            
            if (name != null) {
                int index = name.lastIndexOf(".ser");
                if (index != -1)
                    name = name.substring(0, index);
            }
        }
        
        return name;
    }
    
    public static int getAcceleratorKey(String key) {
        return sun.plugin.resources.ResourceHandler.getAcceleratorKey(key);
    }
    
    /**
     * @return the java component displayed by this viewer class
     */
    protected String getHandledType() {
        return getMessage("java_applet");
    }
    
    private AppletStatusListener statusListener = null;
    
    public void addAppletStatusListener(AppletStatusListener listener)
    {
        statusListener = listener;
    }
    
    public void removeAppletStatusListener(AppletStatusListener listener)
    {
        statusListener = null;
    }
    
    public void setStatus(int status) {
        this.status = status;
    }
    
    public void showAppletLog(String msg) {
        super.showAppletLog(msg);
    }
    
    public boolean isStopped() {
	return stopped;
    }

    public void showAppletStatus(String msg) {
        
        if (msg != null && !msg.equals("") && !msg.equals("\n"))
        {
            try
            {
                String name = getName();
		MessageFormat mf = new MessageFormat(getMessage("status_applet"));
                
                if (name != null && !msg.equals(""))
		    getAppletContext().showStatus(mf.format(new Object[] {name, msg}));
                else
		    getAppletContext().showStatus(mf.format(new Object[] {msg, ""}));
                
                // We no longer need the gray box processer if the applet has started
                if (status == APPLET_START || status == APPLET_ERROR)
                {
                    // Destroy gray box processer
                    if (grayBoxPainter != null)
                    {
                        grayBoxPainter.interrupt();
                        grayBoxPainter = null;
                    }                   
                }

		// Need to stop timer here.  If everything was successfull,
                // then applet loaded and timer still running.                    
		if (steam_the_cup != null)
		{
		    if (status == APPLET_ERROR)
			steam_the_cup.error();
		    else if (status >= APPLET_START) 
			steam_the_cup.stop();                    
		}
				                    
                if (statusListener != null)
                    statusListener.statusChanged(status);
                
            }
            catch(Throwable e)
            {
                e.printStackTrace();
            }
            
        }
    }
    
    
    public void setDoInit(boolean doInit) {
        this.doInit = doInit;
    }
    
    private PluginAppletContext appletContext;
    
    
    /**
     * Method to get an internationalized string from the Activator resource.
     */
    public static String getMessage(String key)
    {
        return sun.plugin.resources.ResourceHandler.getMessage(key);
    }
    
    
    /**
     * Method to get an internationalized string from the Activator resource.
     */
    public static String[] getMessageArray(String key)
    {
        return sun.plugin.resources.ResourceHandler.getMessageArray(key);
    }
    
    private java.io.InputStream is;
    
    /**
     * This method actually creates an AppletClassLoader.
     *
     * It can be override by subclasses (such as the Plug-in)
     * to provide different classloaders. This method should be
     * called only when running inside JDK 1.2.
     */
    protected AppletClassLoader createClassLoader(final URL codebase) {
        return ClassLoaderInfo.find(codebase, getClassLoaderCacheKey()).getLoader();
    }
    
    /**
     * Called by the AppletPanel to provide
     * feedback when an exception has happened.
     */
    protected void showAppletException(Throwable exc)
    {
        super.showAppletException(exc);
        
        Trace.msgPrintln("exception", new Object[]{exc.toString()});        
        
        if (fShowException)
        {
            Trace.printException(exc);
        }

	if (steam_the_cup != null)
	    steam_the_cup.error();
    }
    
    // Use an object to process all the request to the gray box
    private GrayBoxPainter grayBoxPainter = null;
    
    public void showStatusText(String msg)
    {
        // Message should be displayed in the status bar
        //
        getAppletContext().showStatus(msg);
    }
    
    /**
     * Start progress binding.
     *
     * @param identifier Progress identifier
     */
    public void onStartBinding(final Object identifier)
    {
        appletProgress.doOnStart(identifier);
        
        // Post the progress update request to the painter
        if (grayBoxPainter != null)
        {
            grayBoxPainter.repaint();
        }
        
        if (preloading){
            Trace.msgPrintln("progress_bar.preload", new Object[] {
			     identifier.toString().substring(identifier.toString().lastIndexOf('/')+1)});
        }
    }
    
    /**
     * Update progress.
     *
     * @param identifier Progress identifier
     * @param current Current progress.
     * @param max Maximum progress.
     */
    public void onProgressAvailable(final Object identifier, final int current, final int max)
    {
        appletProgress.doOnProgress(identifier, current, max);
        
        // Post the progress update request to the processer
        if (grayBoxPainter != null)
        {
            grayBoxPainter.repaint();
        }
    }
    
    
    /*
     * Update graphics.
     *
     * @param g - the graphics context.
     */
    public void update(Graphics g)
    {
        Dimension d = getSize();
        
        if (d.width > 0 && d.height > 0 &&
        (status == APPLET_LOAD || status == APPLET_INIT || status == APPLET_ERROR))
        {
            paintForegrnd(g);
        }
        else
        {
            super.update(g);
        }
    }
    
    
    /**
     * Stop progress binding.
     *
     * @param identifier Progress identifier
     */
    public void onStopBinding(final Object identifier)
    {
        // Delegate to onProgressComplete
        onProgressComplete(identifier);
    }
    
    /**
     * Paint progress bar.  This would happen if JAR file was loaded from
     * cache or file:/URL.
     */
    public void onProgressComplete(final Object identifier)
    {
        appletProgress.doOnFinish(identifier);
        
        // Trigger graybox to repaint
        if (grayBoxPainter != null)
        {
            grayBoxPainter.repaint();
        }
    }
    
    /**
     * <p> Obtain the loading status of the applet.
     * </p>
     *
     * @return loading status.
     */
    public int getLoadingStatus()
    {
        return status;
    }
    
    
    // To satisfy the WindowEventListener interface for
    // accessibility to work
    public void windowActivated(WindowEvent e) {}
    public void windowClosed(WindowEvent e) {}
    public void windowClosing(WindowEvent e) {}
    public void windowDeactivated(WindowEvent e) {}
    public void windowDeiconified(WindowEvent e) {}
    public void windowIconified(WindowEvent e) {}
    public void windowOpened(WindowEvent e) {}

    static class GrayBoxListener implements MouseListener, ActionListener
    {
	private PopupMenu popup;
	private MenuItem open_console, about_java;
	private String msg = null;
	private AppletViewer av;

	GrayBoxListener(AppletViewer av, String msg)
	{
	    this.msg = msg;
	    this.av = av;
	}

	private PopupMenu getPopupMenu()
	{
	    if (popup == null)
	    {
		Font f = av.getFont();

		// Derive font to make it looks like Swing
		Font menuFont = f.deriveFont(11.0f);

		// Create popup menu
		popup = new PopupMenu();
		open_console = new MenuItem(ResourceHandler.getMessage("usability.menu.open_console")); 
		open_console.setFont(menuFont);
		about_java = new MenuItem(ResourceHandler.getMessage("usability.menu.about_java"));
		about_java.setFont(menuFont);

		open_console.addActionListener(this);
		about_java.addActionListener(this);

		popup.add(open_console);
		popup.add("-");
		popup.add(about_java);

		av.add(popup);          	    
	    }

	    return popup;
	}

	/*
	 * When pointing at the applet - display status message (or user-defined
	 * message) in browser status field.
	 */
	public void mouseEntered(MouseEvent e) {
	    if (msg != null)
		av.showStatusText(msg);
            else
                av.showStatusText(av.getWaitingMessage());
	}
    
	public void mouseExited(MouseEvent e) {}
	public void mousePressed(MouseEvent e) {
	    if (e.isPopupTrigger() && av.getLoadingStatus() == APPLET_ERROR) {
		//Pop up menu with option to display Java Console if an error occured
		//while loading applet.  This will be called on unix.
		getPopupMenu().show(e.getComponent(), e.getX(), e.getY());
            
	    }
	}
	public void mouseReleased(MouseEvent e) {
	     if (e.isPopupTrigger() && av.getLoadingStatus() == APPLET_ERROR) {
		//Pop up menu with option to display Java Console if an error occured
		//while loading applet.  This will be called on windows.
		getPopupMenu().show(e.getComponent(), e.getX(), e.getY());            
	    }
	}   
	public void mouseClicked(MouseEvent e) {}

	/**
	 * Invoked when an action occurs.
	 */
	public void actionPerformed(ActionEvent e) {
	    if (e.getSource() == open_console){ 
		// Popup java console and print exceptions to it from trace file.        
		JavaRunTime.showJavaConsoleLater(true);
	    }
	    else if (e.getSource() == about_java){
		// Show about java dialog
		DialogFactory.showAboutPluginDialog();
	    }
	}
    }        
}


/**
 * Gray box painter is a helper class
 * that helps painting the progress update
 * in the gray box.
 */
class GrayBoxPainter extends Thread
{
    private Object syncObject = new Object();
    
    private Component c = null;
    
    /**
     * Construct gray box painter object.
     *
     * @param tg Thread group
     */
    GrayBoxPainter(ThreadGroup tg, final Component c)
    {
        super(tg, "GrayBoxPainter");
        
        this.c = c;
    }
    
    /**
     * Trigger repaint in gray box.
     */
    void repaint()
    {
        synchronized(syncObject)
        {
            syncObject.notifyAll();
        }
    }
    
    public void run()
    {
        Thread t = Thread.currentThread();
        
        try
        {
            // Check if current thread is interrupted
            while (!t.isInterrupted())
            {
                synchronized(syncObject)
                {
                    // Wait for next request
                    syncObject.wait();
                }
                
                // Force component to update
                Graphics g = c.getGraphics();
                c.update(g);
                g.dispose();
                
                // Sleep for one second to avoid
                // too many repaints
                //
                t.sleep(1000);
            }
        }
        catch (InterruptedException ie)
        {
        }
    }
}
