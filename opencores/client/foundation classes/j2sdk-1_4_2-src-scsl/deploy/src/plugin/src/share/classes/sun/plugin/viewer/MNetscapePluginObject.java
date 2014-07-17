/*
 * @(#)MNetscapePluginObject.java	1.24 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.plugin.viewer;

import java.applet.Applet;
import java.applet.AppletContext;
import java.awt.Component;
import java.awt.Panel;
import java.awt.Frame;
import java.awt.Toolkit;
import java.awt.EventQueue;
import java.awt.Dimension;
import java.awt.event.WindowEvent;
import java.io.PrintStream;
import java.net.URL;
import java.util.Iterator;
import java.util.Set;
import sun.awt.SunToolkit;
import sun.awt.AppContext;
import sun.applet.AppletListener;
import sun.applet.AppletEvent;
import sun.applet.AppletPanel;
import sun.plugin.BeansApplet;
import sun.plugin.AppletViewer;
import sun.plugin.BeansViewer;
import sun.plugin.navig.motif.Plugin;
import sun.plugin.navig.motif.Worker;
import sun.plugin.util.PluginAWTUtil;
import sun.plugin.util.Trace;
import sun.plugin.viewer.frame.MNetscapeEmbeddedFrame;
import sun.plugin.viewer.context.PluginAppletContext;
import sun.plugin.viewer.context.PluginBeansContext;
import sun.plugin.services.BrowserService;
import sun.plugin.services.ServiceProvider;
import sun.print.PSPrinterJob;

/*
 * Panel for each applet running in the Java Plugin VM.  This class
 * encapsulates the viewing of the Applet. It extends AppletViewer,
 * which extends AppletPanel, which provides the basic state machine
 * of an applet.  There are 5 threads associated with this class, in
 * addition to any associated with the applet:
 *
 *   The event processing thread (of type MotifAppletViewer)
 *   The class loading thread (also of type MotifAppletViewer,
 *         responsible for creating the class loader etc.)
 *   The init thread (Initer thread) , used for calling init on the applet
 *   The destroy thread (DestroyerThread), used for calling destroy
 *   The Plugin's main thread which runs the applet viewer by calling
 *                setWindow etc.
 *
 * Most of the hard work is done in our baseclasses, sun.plugin.AppletViewer
 * and sun.plugin.AppletPanel.
 *
 * The life-cycle for a MotifAppletpanel is slightly strange and is
 * driven by the Netscape plugin API.
 *
 *    (1) First, netscape creates a new plugin instance using CreateApplet

 *    (2) Typically, the next thing that happens is that Netscape
 *	  sets the X11 window information using JAVA_PLUGIN_WINDOW
 *	  At this point we can create our AWT EmbeddedFrame using
 *	  the parent X11 window ID privided by netscape.
 *
 *    (3) Typically, the next thing that happens will be that the
 *	  plugin figures out our DOCBASE and sends it to us.
 *	  This step may sometimes ocur before (2). JAVA_PLUGIN_DOCBASE
 *	  This allows us to set the DOCBASE in our parent.
 *
 *    (4) When both (2) and (3) have occurred we have both a DOCBASE
 *	  value and an EmbeddedFrame, so we can call init() to 
 *	  actually do all the hard work of starting the applet.
 *
 *    (5) We live a fruitful and industrious life, with our applet
 *	  happily running and painting.
 *
 *    (6) When netscape moves to a different page (or whatever) the
 *	  plugin JAVA_PLUGIN_DESTROY method will be called.
 *
 *	  At this point we are called to destroy the applet.
 * */

/**
 * <p> MNetscapePluginObject is a class that encapsulates an applet or bean
 * running in the Java Plug-in inside Netscape Navigator. It contains all
 * functions that are required to create, load, stop and destroy the applet or
 * bean.
 * </p>
 */
public class MNetscapePluginObject implements AppletListener
{
    /** 
     * <p> Embedded Frame object for the plugin.
     * </p>
     */
    private MNetscapeEmbeddedFrame frame = null;

    /** 
     * <p> Panel object for hosting the applet or bean.
     * </p>
     */
    protected AppletViewer panel = null;

    /** 
     * <p> ID of an applet or bean in Navigator.
     * </p>
     */
    protected int id = -1;

    /** 
      * <p> Parent window id of the embedded frame.
      * </p>
      */
    private int winID = 0;

    /**
      * <p> Width of the embedded frame. This is used by relative dimension support.
      * </p>
      */      
    private int width = 0;

    /**
      * <p> Height of the embedded frame. This is used by relative dimension support.
      * </p>
      */      
    private int height = 0;

    /**
     * <p> Identifier of the applet instance.
     * </p>
     */
    private String identifier = null;

    /**
     * <p> Create a MNetscapePluginObject.
     * </p>
     *
     * @param id Identifier of the Plugin instance.
     */
    MNetscapePluginObject(int id, boolean isBeans, String identifier)
    {
        this.id = id;
	this.identifier = identifier;

	// Obtain panel from LifeCycleManager
	this.panel = LifeCycleManager.getAppletPanel(identifier);

	if (isBeans == false)
	{
	    // Applet case
	    if (this.panel == null) {
	        this.panel = new AppletViewer();
		this.panel.addAppletListener(this);
	    }

	    // Obtain the applet context and reset the handle
	    PluginAppletContext pac = (PluginAppletContext) this.panel.getAppletContext();
	    
	    if (pac == null)
	    {
		BrowserService bs = ServiceProvider.getService();
		pac = bs.getAppletContext();
	    }

	    pac.setAppletContextHandle(id);
	    this.panel.setAppletContext(pac);
	}
	else
	{
	    // Beans case
	    if (this.panel == null)
		this.panel = new BeansViewer();

	    // Obtain the applet context and reset the handle
	    PluginBeansContext pbc = (PluginBeansContext) this.panel.getAppletContext();

	    if (pbc == null)
	    {
		BrowserService bs = ServiceProvider.getService();
		pbc = bs.getBeansContext();
	    }

	    pbc.setAppletContextHandle(id);
	    this.panel.setAppletContext(pbc);
	}
    }

    /**
     * <p> Create the embedded frame for an applet or beans depends on the 
     * runtime type.
     * </p>
     */
    private MNetscapeEmbeddedFrame createFrame(int handle)  
    {
	MNetscapeEmbeddedFrame frame = new MNetscapeEmbeddedFrame(handle);

	// Fixed #4754451: Applet can have methods running on main
	// thread event queue. 
	// 
	// The cause of this bug is that the frame of the applet 
	// is created in main thread group. Thus, when certain 
	// AWT/Swing events are generated, the events will be
	// dispatched through the wrong event dispatch thread.
	//
	// To fix this, we rearrange the AppContext with the frame,
	// so the proper event queue will be looked up.
	//
	// Swing also maintains a Frame list for the AppContext,
	// so we will have to rearrange it as well.
	//
	Applet a = panel.getApplet();

	if (a != null)
	{
	    AppletPanel.changeFrameAppContext(frame, SunToolkit.targetToAppContext(a));
	}

	return frame;
    }

    /**
     * Our X11 window information has changed.
     * Newid is the new X11 window ID of our plugin window in Navigator.
     * Width, height, x, and y provide more info on that window.
     * Set window is only called by the plugin, and there is only one thread
     * that can do this, so this need not be synchronized
     */
    public void setWindow(int newID, final int width, 
			  final int height, int x, int y) {

	// setWindow() is called whenever the plug-in is started,
	// stoped, and resized. When the plug-in first staryts
	// up before Applet.start() is called, setWindow()
	// will be called with a valid plug-in window handle.
	// When the plug-in is destroyed, Applet.stop() will
	// be called, and followed by a setWindow() with a 
	// NULL plug-in window handle.


        // Frame has been created
        if (winID == newID)
            return;

        // A different handle is passed. Remove existing frame.
	if (frame != null)
	{
	    // Hide the frame first. This is very important
	    // to hide the frame before calling dispose().
	    // Otherwise, some race condition with repaint()
	    // may be resulted.
	    //
	    frame.setVisible(false);

	    // Close and remove frame.
	    frame.setEnabled(false);

	    // Remove all
	    frame.removeAll();

	    // Set the native waiting object to be notified
	    // when the frame is actually disposed.
	    //	    frame.setWaitingEvent(waitHandle);
		    
	    // Post a WINDOW_CLOSING event to the frame event
	    // queue, and the frame will be disposed.
	    //
	    WindowEvent evt = new WindowEvent(frame, WindowEvent.WINDOW_CLOSING);
	    PluginAWTUtil.postEvent(frame, evt);

	    panel.removeNotify();
	}

        frame = null;
        winID = newID;

	if (winID != 0)
	{
	    // Create a Frame for the applet.
	    // We do this by taking the X window handle provided by Netscape 
	    // and creating an Xt widget inside of it.  Then we create a Motif
	    // EmbeddedFrame inside that widget.
	    int widget = MNetscapePluginContext.getWidget(winID, width, height, x, y);
	    frame = createFrame(widget);

	    if (frame == null) {
		System.err.println("Creation of frame failed");
		return;
	    }

	    // We have to work around a JDK 1.1.4 v 1.1.5 inconsistency by
	    // explicitly mapping the Widget at this point.
	    MNetscapePluginContext.mapWidget(widget, width, height, x, y);

	    //	    frame.setLayout(null);
	    // Needed for accessibility stuff. There is some optimization
	    // in the accessibility mechanism so that it only pays attention
	    // to frames with window listeners. Ref: Peter Korn for details.
	    // frame.addWindowListener(this);
	    // frame.setVisible(true);
	    

	    Object o = getJavaObject();
	    Applet applet;
	    // If the applet has not been set, attempt to set its dimensions.
	    // Try to read the parameters as specified in the html tags
	    // and stored in width and height. If those are not integers,
	    // then just use the width, height provided by the browser
	    // in this setwindow call.
	    // The applet is normally set in the createApplet call
	    if (o == null) {
		Trace.println("setWindow: call before applet exists:" + winID);
		try {
		    int wd = Integer.parseInt(getParameter("width"));
		    int ht = Integer.parseInt(getParameter("height"));
		    this.width = wd;
		    this.height = ht;
		} catch (NumberFormatException e) {
		    // Try and maintain consistency between the parameters and
		    // our width/height. If the parameters are not properly
		    // set to be integers, then reset them to our w, h
		    setParameter("width", new Integer(width));
		    setParameter("height", new Integer(height));
		    this.width = width;
		    this.height = height;
		}
	    } else {
		// The applet exists, so resize it and its frame, and
		// have the width/height parameters reflect these values
		setParameter("width", new Integer(width));
		setParameter("height", new Integer(height));
		this.width = width;
		this.height = height;

		panel.setSize(width, height);
		if (o instanceof Applet) {
		    applet = (Applet) o;
		} else {
		    Trace.println("setWindow: Viewing a bean component");
		    Component c = (Component) o;
		    c.setSize(width, height);
		    applet = (Applet) c.getParent();
		}
		applet.resize(width, height);
		applet.setVisible(true);
	    }

	    /* Note: 2/3/99 - Ben
	       Be very careful with the order in which the window events are called.
	       frame.setbounds changes the size of the window, resulting in
	       xevents being generated that trigger the innerCanvasEH in awt_Frame.c
	       If the XtWidget has not yet been realized, this may result in the event
	       being dropped, and the widget does not get sized properly.
	       The XtWidgets are only realized when the embedded frame is shown.
	       It does not help to have the setbounds be done twice - if the bounds are the
	       same in both calls, the AWT widget will not do anything.
	       
	       The Xt Widget hierarchy for the trivial applet that does nothing is:
	       
	       widget=VendorShell level=5 name=AWTapp wd=100 ht=100
	       widget=XmForm level=4 name=main wd=100 ht=100
	       widget=XmDrawingArea level=3 name=frame_wrap wd=100 ht=100
	       widget=XmDrawingArea level=2 name=frame_canvas wd=100 ht=100
	       widget=XmDrawingArea level=1 name=canvas wd=100 ht=100
	       widget=XmDrawingArea level=0 name=canvas wd=100 ht=100
	       
	       The width's and heights are, of course, from a particular run, but the point
	       is that they are all the same.

	       Something else that might be causing problems is the fact that the
	       EmbeddedFrame is reparented to itself (!) in awt_Frame_MEmbeddedFrame_pShow
	       which overrides the standard pShow (both args to the XReparent are 
	       the same widget).  
	    */

	    panel.setBounds(0, 0, width, height);
	    // Add this appletpanel (self) to the frame we just created
	    // so that it will be viewed in the new frame
	    //frame.add(this);
	    /* Realizes the top level widget, which automatically creates the windows of
	       all its children. */
	    frame.setVisible(true);
	    frame.setBounds(0, 0, width, height);
	    
	    // If the docbase has already been set, we can init the applet.
	    //	maybeInit();
	    initPlugin();
	    
	    // To trigger the accessibility mechanisms we initiate the event
	    // queue with a synthesized event
	    //	    WindowEvent actev = new WindowEvent(frame, 
	    //				WindowEvent.WINDOW_ACTIVATED);
	    //Toolkit tk = Toolkit.getDefaultToolkit();
	    //EventQueue eq = tk.getSystemEventQueue();
	    //eq.postEvent(actev);
	}
    }

    /**
     * Nested class to call 'init' on our applet or bean.
     */
    private class Initer extends Thread {
	AppletViewer that;
	MNetscapePluginObject obj;

	Initer(AppletViewer that, MNetscapePluginObject obj) {
	    this.that = that;
	    this.obj = obj;
  	}
	/**
         * Main body of thread.  Call init on our PluginPanel.
	 */
	public void run() {

	    // This is important to use sync because "initPanel" must be 
	    // completed before the plugin can continue.
	    //
	    LifeCycleManager.initAppletPanel(that);

	    synchronized(obj)
	    {
		obj.initialized = true;

		if (obj.startCount > 0)
		{
		    obj.startCount = 0;
		    obj.startPlugin();
		}
		else if (obj.startCount == 0)
		{
		    obj.startPlugin();
		    obj.stopPlugin();
		}
	    }
	}
    }

    /**
     * <p> Add a new panel to the embedded frame and start to download the 
     * applet/bean.
     * </p>
     */
    private synchronized void initPlugin()
    {
	assert (panel != null);

	// If panel is available, insert it into the frame and
	// start the applet/bean
	if (frame != null)
	{
	    // Fixed #4754451: Applet can have methods running on main
	    // thread event queue. 
	    // 
	    // The cause of this bug is that the frame of the applet 
	    // is created in main thread group. Thus, when certain 
	    // AWT/Swing events are generated, the events will be
	    // dispatched through the wrong event dispatch thread.
	    //
	    // To fix this, we rearrange the AppContext with the frame,
	    // so the proper event queue will be looked up.
	    //
	    // Swing also maintains a Frame list for the AppContext,
	    // so we will have to rearrange it as well.
	    //
	    Applet a = panel.getApplet();

	    if (a != null)
	    {
		AppletPanel.changeFrameAppContext(frame, SunToolkit.targetToAppContext(a));
	    }

	    // Insert the applet/bean into the embedded frame.
	    //
	    frame.add(panel);

	    // This is very important to make it visible as well.
	    //
	    frame.setVisible(true);

	    // Start an applet or bean
	    if (initialized == false && setDocBase == true)
	    {
		initialized = true;

		// life cycle check. Just for consistence, it is not required for
		// UNIX and LINUX
		LifeCycleManager.checkLifeCycle(panel);

		// This is very important to call panel.initPanel() from a
		// different thread, because it may call back into the plugin
		// for auto config proxy during this call. Thus, it must be 
		// called from a secondary thread. This hack is mainly for JDK 1.2.
		//

		new Initer(panel, this).start();
	    }
	}
    }


    /**
     * <p> This count is for remaining the number of pending request for starting 
     * the applet. It is used when the applet is not completely loaded but a 
     * start request is made.
     * </p>
     */
    private int startCount = -1;

    private boolean initialized = false;

    private boolean setDocBase = false;

    /** 
     * <p> Start an applet or bean.
     * </p>
     */
    public synchronized void startPlugin()
    {
	assert (panel != null);

	if (initialized == true)   
	    LifeCycleManager.startAppletPanel(panel);
	else
	{
	    if (startCount < 0)
		startCount = 0;

	    startCount++;
	}
    }



    /** 
     * <p> Stop an applet or bean.
     * </p>
     */
    public synchronized void stopPlugin()
    {
	assert (panel != null);

	if (initialized == true)
	    LifeCycleManager.stopAppletPanel(panel);
	else
	    startCount--;
    }

    /*
     * <p> Remove the panel from the embedded frame and stop the applet/bean.
     * </p>
     */
    public synchronized void destroyPlugin() 
    {
	assert (panel != null);

	if (frame != null)
	{
	    // Hide the frame first. This is very important
	    // to hide the frame before calling dispose().
	    // Otherwise, some race condition with repaint()
	    // may be resulted.
	    //
	    frame.setVisible(false);
			
	    // Close and remove frame.
	    frame.setEnabled(false);

	    // Remove all
	    frame.removeAll();

	    // Send an asynchronous message for frame disposal
	    WindowEvent we = new WindowEvent(frame, WindowEvent.WINDOW_CLOSING);
	    PluginAWTUtil.postEvent(frame, we);

	    panel.removeNotify();
	}
			
	PluginAppletContext pac = (PluginAppletContext) this.panel.getAppletContext();

	LifeCycleManager.destroyAppletPanel(identifier, panel);
		    
	// When applet is stopped/destroyed, it is VERY important to
	// reset the applet context, so applet won't be able to
	// call methods in applet context to interface with the
	// browser if the document has gone away.
	//

	if (pac != null)
	    pac.setAppletContextHandle(-1);

	panel = null;
	frame = null;
    }

    /* 
     * <p> Set the document base of the applet or bean.
     * We want to run the init() method of AppletPanel,
     * which starts the handler thread, when we have *both*:
     * - the window handle, and
     * - the codebase
     *
     * The browser can notify us of these in either order.
     * So the last one we get (window/stream) will also
     * fire the init method.  Otherwise, we can deadlock
     * with the browser if the other isn't ready yet.
     * </p>
     *
     * @param url Document base URL of the applet or bean.
     */
     public synchronized void setDocumentURL(String url)
     {
	 assert (panel != null);

         try 
	 {
             // Notify LiveConnect thread for scripting.
             notifyAll();

	     ((sun.plugin.AppletViewer)panel).setDocumentBase(url);

	     setDocBase = true;

             // Initialize an applet or bean
             initPlugin();

         } catch (Throwable e) {
	     Trace.printException(e);
         }
    }

    /** 
     * <p> Returns the embedded frame of the plugin object.
     * </p>
     *
     *  @returns Embedded frame of the plugin object.
     */
    private Frame getFrame()  
    {
	// This is used for printing
        return frame;
    }


    /**
     * <p> Set the focus on the embedded frame. This is a hack to make Swing
     * focus mechanism works inside Java Plug-in.
     * </p>
     */
    void setFocus()  {
	if (frame != null)
	    frame.requestFocus();
    }

    /** 
     * <p> Set the embedded frame size. This method is called only when the WIDTH
     * and HEIGHT in the EMBED tag are specified using relative dimension (%).
     * </p>
     *
     * @param width Width of the frame.
     * @param height Height of the frame.
     */
    void setFrameSize(int width, int height)  {

	// Synchronization is important here to make sure the
	// width and the height are set are the same time.
	//
	synchronized(this)  {
	    
	    // Reset the WIDTH and HEIGHT in the AppletContext.
	    //
	    setParameter("width", new Integer(width));
	    setParameter("height", new Integer(height));

	    // Resize the embedded frame.
	    if (frame != null)   {
		frame.setSize(width, height);
	    }

	    // Resize the panel.
	    if (panel != null)  {
		Panel p = panel;

		if (p != null)
		{
		    p.setSize(width, height);	    

		    panel.setParameter("width", new Integer(width));
		    panel.setParameter("height", new Integer(height));
		}

		Object obj = panel.getViewedObject();
	
		if (obj != null)
		{	    
		    Applet applet;

		    if (obj instanceof Applet) {
			applet = (Applet) obj;
		    }
		    else {
			// This must be a bean.
			Component c = (Component) obj;
			c.setSize(width, height);
			applet = (Applet) c.getParent();
    		    }

		    if (applet != null)
			applet.resize(width, height);
		}
	    }
	}
    }

    /**
     * <p> Obtain the reference of the applet or bean.
     * </p>
     *
     * @return Java object.
     */
    public Object getJavaObject()   
    {
	Object obj = null;

	if (panel != null)
	    obj = panel.getViewedObject();

	if (obj instanceof BeansApplet)
	{
	    BeansApplet applet = (BeansApplet) obj;
	    obj = applet.getBean();
	}

	return obj;
    }


    /** 
     * <p> Obtain the loading status of the applet or bean.
     * </p>
     *
     * @return loading status.
     */
    int getLoadingStatus()
    {
	if (panel != null)
	    return panel.getLoadingStatus();
	else
	    return AppletPanel.APPLET_ERROR;
    }


    /**
     * <p> Returns the value of a param from the param list.
     * </p>
     *
     * @param k Parameter key name.
     * @returns The value of the parameter.
     */
    public String getParameter(String k)
    {
	assert (panel != null);

	return panel.getParameter(k);
    }


    /**
     * <p> Set the parameter.
     * </p>
     *
     * @param name Parameter key name.
     * @param value Parameter value.
     */
    public void setParameter(String name, Object value)  
    {
	assert (panel != null);

	panel.setParameter(name, value);
    }

    /*
     * Set background/foreground and progress bar color for the applet viewer.
     */
    public void setBoxColors()
    {
        panel.setColorAndText();
    }
    
        

    /* Hack to get around problem with Netscape printing - print gets
     * called 3 times. We ignore the first two calls */
    private int printCounter = 0;

    /* Print the applet to the output stream, 'printOut'. Note - this
     * only works in Nav 4.x. doPrint is called 3 times. We only
     * use the last call. If either of the prior calls are used then
     * the EPS is emitted before the main Postscript stream.
     * Unfortunately this does happen for applets that span page boundaries,
     * as the "3" times rule breaks down. This isn't detectable until too
     * late.
     */
    public void doPrint(int x, int y, int w, int h, PrintStream printOut) {

	if (++printCounter == 3) {
	     printCounter = 0;
	     /* PluginPrinter emits Navigator 4.x specific Postscript code.
	      * For a different browser you may need to make a copy of
	      * the class which behaves as needed by that browser, or
	      * move the class here and make it work for all supported
	      * browsers.
	      */

	     if (panel != null) {
		 PSPrinterJob.PluginPrinter pp = new
		     PSPrinterJob.PluginPrinter(panel, printOut, x, y, w, h);
		 try {
		     pp.printAll();
		 } catch (Throwable t) {
		     /* don't let applet errors hang/crash plugin */
		     Trace.println("doPrint: printAll failed");
		     Trace.printException(t);
		 }
	     }
	}
    }

    private boolean evaluatingExp = false;
    private boolean returnedExp = false;
    private String returnJS = null;

    private Object syncObject = new Object();

    public String evalString(int instance, String jsexp) 
    {
	synchronized(syncObject)
	{
	    try {
		while(evaluatingExp)
		    syncObject.wait();
		evaluatingExp = true;
		returnedExp = false;
		Worker.sendJSRequest(instance, jsexp);
		while (!returnedExp)
		    syncObject.wait();
		evaluatingExp = false;
		String ret = returnJS;
		returnJS = null;
		syncObject.notifyAll();
		return ret;
	    } catch (Exception e) {
		evaluatingExp = false;
		returnJS =  null;
		syncObject.notifyAll();
		return null;
	    }
	}
    }
    
    public void setJSReply(String ret) {
	synchronized(syncObject)
	{
	    if (returnJS == null)
		returnJS= ret;
	    else 
		returnJS += ret;
	}
    }

    public void finishJSReply() {
	synchronized(syncObject)
	{
	    returnedExp = true;
	    syncObject.notifyAll();
	}
    }


    public void appletStateChanged(AppletEvent evt) 
    {
	AppletPanel src = (AppletPanel)evt.getSource();

	switch (evt.getID()) 
	{
	    case AppletPanel.APPLET_LOADING_COMPLETED: {

		// Fixed #4754451: Applet can have methods running on main
		// thread event queue. 
		// 
		// The cause of this bug is that the frame of the applet 
		// is created in main thread group. Thus, when certain 
		// AWT/Swing events are generated, the events will be
		// dispatched through the wrong event dispatch thread.
		//
		// To fix this, we rearrange the AppContext with the frame,
		// so the proper event queue will be looked up.
		//
		// Swing also maintains a Frame list for the AppContext,
		// so we will have to rearrange it as well.
		//
		if (frame != null)
		{
		    Applet a = src.getApplet(); // sun.applet.AppletPanel

		    if (a != null)
			AppletPanel.changeFrameAppContext(frame, SunToolkit.targetToAppContext(a));
		    else
			AppletPanel.changeFrameAppContext(frame, AppContext.getAppContext());
    		}

		break;
	    }
	}
    }
}
