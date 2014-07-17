/*
 * @(#)IExplorerEmbeddedFrame.java	1.78 02/12/17
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.plugin.viewer.frame;

import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.EventQueue;
import java.awt.Toolkit;
import java.awt.event.InvocationEvent;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;
import java.applet.Applet;
import java.beans.Beans;
import java.lang.reflect.InvocationTargetException;
import sun.awt.SunToolkit;
import sun.plugin.util.PluginAWTUtil;
import sun.plugin.util.Trace;
import sun.plugin.viewer.IExplorerPluginObject;
import sun.plugin.services.PlatformService;

/*
 * This is the java window used to inplace edit a JavaBeans component
 * An IExplorerEmbeddedFrame does not have decorations, always live within a 
 * container window (specified by the container application).
 * The lifetime of an IExplorerEmbeddedFrame is decided by the OLE container
 * Each OleEmbeddedWindow will itself embed a unique JavaBeans component
 *
 * @version 1.2.2
 * @date 12/15/98
 * @author Jerome Dochez
 */
public class IExplorerEmbeddedFrame extends sun.awt.windows.WEmbeddedFrame
                              implements WindowListener,
					 sun.awt.windows.ModalityListener
{
    private int waitingEvent = 0;
    private IExplorerPluginObject pluginObj = null;

    /*
     * Build a new in place window and set its parent to the window 
     * handle specified. 
     *
     * @param nativeHandler native window implementation
     * @param hWnd Native Window handle of the parent window
     */
    public IExplorerEmbeddedFrame(int hWnd, IExplorerPluginObject obj) 
    {
        super(hWnd);

	handle = hWnd;
	pluginObj = obj;

	// Use borderlayout, so the panel/applet/beans will always 
	// be the same size as the frame.
	setLayout(new BorderLayout());

        addWindowListener(this);

	sun.awt.windows.WToolkit.getWToolkit().addModalityListener(this);
	Trace.msgPrintln("modality.register");
    }

    private void setWaitEvent(int hWaitEvent) {
	assert(waitingEvent == 0);
	assert(hWaitEvent != 0);
	waitingEvent = hWaitEvent;
    }

    /*
     * Specify which JavaBean component should be painted inside this
     * embedded frame window. 
     * 
     * @param bean the JavaBeans reference
     */
    private void setComponent(Component c) 
    {
	if (c!=null) {
	    // get the component original size
	    Dimension d = c.getSize();

	    add(c);

	    // Set the size to the original value
	    if (d.width!=0 && d.height!=0)
		setSize(d);
    
	    // Now that we have a window, start if this is a applet !
	    if (Beans.isInstanceOf(c, Applet.class)) {
		Applet apl = (Applet) Beans.getInstanceOf(c, Applet.class);
		apl.start();
	    }
	    c.requestFocus();
	}
    }

    /*
     * Set the JavaBeans component as the embedded object.If the JavaBeans is
     * a visible bean, it supports java.awt.Component by direct inheritance or
     * by aggregation. If the bean is an invisible bean, then this method 
     * does nothing.
     * 
     * @param c Component
     */
    public void setBean(final Object component) {
        try 
	{            
	    // Use invoke later to avoid blocking and race condition
	    // with browser window.
	    //
	    PluginAWTUtil.invokeAndWait(this, new Runnable()
	    {
		public void run()
		{
                    try 
		    {
                        c =(java.awt.Component) Beans.getInstanceOf(component, 
							Component.class);
                    } 
		    catch (ClassCastException e) 
		    {
                        return;
                    }
                    
		    if (c != null)
                        setComponent(c);
                 }
            });

        } 
	catch( Exception e ) 
	{
            System.out.println( "Failed to setBean() : " );
            e.printStackTrace();
        }
    }

    /*
     * <p>
     * Called by the native layers, when the container is requesting for 
     * closing the ActiveX component. 
     * We post the WINDOW_CLOSING event on the AWT window thread
     * </p>
     */
    public void destroy() {
	final java.awt.Frame frame = this;

	try {
	    PluginAWTUtil.invokeLater(this, new Runnable() {
		public void run() {
		    // Hide the frame first. This is very important
		    // to hide the frame before calling dispose().
		    // Otherwise, some race condition with repaint()
		    // may be resulted.
		    //
		    frame.setVisible(false);

		    // Close and remove frame.
		    frame.setEnabled(false);

		    WindowEvent we = new WindowEvent(frame, WindowEvent.WINDOW_CLOSING);
		    PluginAWTUtil.postEvent(frame, we);
		}});
	} catch(Exception e) {
	}


	assert(waitingEvent != 0);
	PlatformService.getService().waitEvent(handle, waitingEvent);
    }


    /*
     * Set the size of the embedded window, if the embedded window 
     * currently embed a component, resets its size as well.
     * 
     * @param width of the embedded frame
     * @param height of the embedded frame
     */
    public void setFrameSize(final int width, final int height) 
    {
	// If width != 0 && height !=0, then the appletviewer
	// may begin to run.
	//
	if (width > 0 && height > 0)
	{
	    try
	    {
		// Set parameter in main thread
		synchronized(this)
		{
		    if (c instanceof sun.plugin.AppletViewer) {
			sun.plugin.AppletViewer panel = (sun.plugin.AppletViewer) c;
			panel.setParameter("width", Integer.toString(width));
			panel.setParameter("height", Integer.toString(height));
		    }
		}

		// Use invoke later to avoid blocking and race condition
		// with browser window.
		//
		PluginAWTUtil.invokeLater(this, new Runnable()
		{
		    public void run()
		    {
			// Resize embedded frame
			setSize(width, height);

			// Resize AppletViewer/applet/beans
			if (c!=null) 
			{
//			    c.setSize(width, height);
			    c.setBounds(0, 0, width, height);

			    if (c instanceof sun.plugin.AppletViewer)
			    {
				sun.plugin.AppletViewer panel = (sun.plugin.AppletViewer) c;
				panel.appletResize(width, height);

				Applet applet = panel.getApplet();

				if (applet != null)
				    applet.resize(width, height);
			    }
			}
		    }
		});

		// Notify the pluginObj that the frame is ready 
		// in the main thread
		pluginObj.frameReady();
	    }
	    catch (Throwable e)
	    {
		Trace.printException(e);
	    }
	}
    }


    /*
     * <p>
     * Manually notify the applet to grab the focus
     * </p>
     */
    public void requestFocus() 
    {
	WindowEvent we = new WindowEvent(this, WindowEvent.WINDOW_ACTIVATED);
	PluginAWTUtil.postEvent(this, we);

	// Delegate focus event to applet if necessary
	//
	if (c instanceof sun.plugin.AppletViewer) 
	{
	    final Applet a = ((sun.plugin.AppletViewer) c).getApplet();

	    if (a != null) 
	    {
		// Use invoke later to avoid blocking and race condition
		// with browser window.
		//
		PluginAWTUtil.invokeLater(this, new Runnable()
		{
		    public void run()
		    {
			a.requestFocus();
		    }
		});
	    }
	}
    }


    /**
     * Mimic windows activation sequence. This is called
     * from native code if necessary because the window
     * may not be activated automatically in some cases.
     */
    private void activate()
    {
	// Post WINDOW_ACTIVATED event
	//
	WindowEvent we = new WindowEvent(this, WindowEvent.WINDOW_ACTIVATED);
	PluginAWTUtil.postEvent(this, we);

	// Post WINDOW_GAINED_FOCUS event
	//
	we = new WindowEvent(this, WindowEvent.WINDOW_GAINED_FOCUS);
	PluginAWTUtil.postEvent(this, we);
    }


    /**
     * Mimic windows deactivation sequence. This is called
     * from native code if necessary because the window
     * may not be deactivated automatically in some cases.
     */
    private void deactivate()
    {
	// Post WINDOW_LOST_FOCUS event
	//
	WindowEvent we = new WindowEvent(this, WindowEvent.WINDOW_LOST_FOCUS );
	PluginAWTUtil.postEvent(this, we);

	// Post WINDOW_DEACTIVATED event
	//
	we = new WindowEvent(this, WindowEvent.WINDOW_DEACTIVATED);
	PluginAWTUtil.postEvent(this, we);
    }

    /**
     * Restore focus on most recent focus owner.
     */
    private void restoreMostRecentFocusOwner()
    {
	// Restore most recent focus owner. This is saved when
	// the window was previously deactivated.
	//
	final IExplorerEmbeddedFrame f = this;

	// Use invoke later to avoid blocking and race condition
	// with browser window.
	//
	PluginAWTUtil.invokeLater(this, new Runnable()
	{
	    public void run()
	    {
		if (f.focusOwner != null)
		{
		    f.focusOwner.requestFocusInWindow();
		}
		else
		{
		    f.requestFocus();
		}
	    }
	});
    }


    /*
     * <p>
     * Window Listener interface implementation.
     * Do nothing but register itself for assistive technology
     * </p>
     */
    public void windowActivated(WindowEvent e) { }
    public void windowClosed(WindowEvent e) { }
    public void windowClosing(WindowEvent e) {
	try 
	{
	    removeWindowListener(this);

    	    sun.awt.windows.WToolkit.getWToolkit().removeModalityListener(this);
	    Trace.msgPrintln("modality.unregister");

	    // Remove all components
    	    removeAll();

            dispose();

	    // Release applet viewer
	    c = null;
	    handle = 0;
	    focusOwner = null;
	} 
	catch(Throwable ex) 
	{
	    Trace.printException(ex);
 	} 
	finally 
	{
	    if (waitingEvent != 0)
		PlatformService.getService().signalEvent(waitingEvent);
	}
    }
    public void windowDeactivated(WindowEvent e) 
    { 
	// Before we deactivate, store the most recent focus owner.
	//
	// This is important because we may not be able to get this
	// focus owner again once the window is re-activated.
	//
	focusOwner = getMostRecentFocusOwner();   
    }
    public void windowDeiconified(WindowEvent e) {}
    public void windowIconified(WindowEvent e) {}
    public void windowOpened(WindowEvent e) {}

    /*
     * Called by AWT when it enters a new level of modality
     */
    public void modalityPushed(sun.awt.windows.ModalityEvent evt) {

	Trace.msgPrintln("modality.pushed");
	enableModeless(handle, false);
    }

    /*
     * Called by AWT when it exits a level of modality
     */
    public void modalityPopped(sun.awt.windows.ModalityEvent evt) {

	Trace.msgPrintln("modality.popped");
	enableModeless(handle, true);
    }   


    /*
     * <p>
     * Native methods implementations
     * /p>
     */
    private native void enableModeless(int nativeHandler, boolean fEnable);

    /* 
     * reference to the JavaBeans component
     */
    private Component c;
    private int handle=0;
    private Component focusOwner = null;
}


