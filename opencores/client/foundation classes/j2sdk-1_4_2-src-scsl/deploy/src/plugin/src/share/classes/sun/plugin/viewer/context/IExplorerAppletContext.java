/*
 * @(#)IExplorerAppletContext.java	1.14 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.viewer.context;

import java.net.URL;
import java.util.ArrayList;
import java.util.Iterator;
import java.lang.ref.SoftReference;
import sun.plugin.javascript.ocx.JSObject;
import sun.applet.AppletPanel;
import sun.plugin.AppletViewer;

/**
 * This class corresponds to an applet's environment: the
 * document containing the applet and the other applets in the same
 * document in Internet Explorer.
 * <p>
 * The methods in this interface can be used by an applet to obtain
 * information about its environment.
 *
 */
public class IExplorerAppletContext extends DefaultPluginAppletContext
{
    /**
     * Create a IExplorerAppletContext object.
     */
    public IExplorerAppletContext()
    {
	this.handle = 0;
    }

    /**
     * Set the underlying handle of the Applet context
     * 
     * @param handle Handle
     */
    public void setAppletContextHandle(int handle)
    {
	this.handle = handle;
    }

    /**
     * Get the underlying handle of the Applet context
     * 
     * @return handle Handle
     */
    public int getAppletContextHandle()
    {
	return handle;
    }

    /*
     * Requests that the browser or applet viewer show the Web page
     * indicated by the url argument. 
     *
     * @param url the address to transfer to
     * @param target One of the value
     *	"_self"  show in the current frame
     *  "_parent"show in the parent frame
     *  "_top"   show in the topmost frame
     *  "_blank" show in a new unnamed top-level windownameshow in a 
     *           new top-level window named name
     */
    public void doShowDocument(URL url, String target)
    {
	// Call native method
	if (handle > 0) {
	    Object [] args = new Object[2];
	    args[0] = url.toString();
	    args[1] = target;
	    nativeInvokeScript(handle, JSObject.DISPATCH_METHOD, "open", args);
	}
    }

    /*
     * Show status.
     * 
     * @param status status message
     */ 
    public void doShowStatus(String status)
    {
	//During applet shutdown, AxControl window goes away before showing 
	//the satus message and releasing the resources. Therefore we avoid
	//status messages during shutdown
	boolean stopStatus = ((AppletViewer)appletPanel).isStopped();
	if( handle > 0 && !stopStatus){
	    Object [] args = new Object[1];
	    args[0] = status;
	    // Call native method
	    nativeInvokeScript(handle, JSObject.DISPATCH_PROPERTYPUT, "status", args);
	}
    }

    // Native methods
    private native void nativeInvokeScript(int handle, int flags, String name, Object [] args);

    /*
     * <p>
     * Notification of being closed, we have to release all JSObject 
     * that this particular applet has opened
     * </p>
     */
    public void onClose() {
	handle=0;

	synchronized(exported)
	{
	    for (java.util.Iterator iter = exported.iterator(); iter.hasNext();) {
		SoftReference ref = (SoftReference) iter.next();

		if (ref != null)
		{
		    JSObject obj = (JSObject) ref.get();
		    
		    if (obj != null)
			obj.cleanup();
		}
	    }

	    exported.clear();
	}
    }

    public synchronized netscape.javascript.JSObject getJSObject()
    {
	JSObject jsObj = getJSObject(handle);
	jsObj.setIExplorerAppletContext(this);
	return jsObj;
    }


    /* 
     * <p>
     * JSContext implementation
     * @return the JSObject implementation for the current browser 
     * <p>
     */
    private native JSObject getJSObject(int handle);

    /*
     * <p>
     * Add a JSObject to the list of exported JSObjects so it gets properly
     * released upon applet destruction
     * </p>
     * @param JSObject instance to add
     */
    public void addJSObjectToExportedList(netscape.javascript.JSObject jsObject) 
    {
	synchronized (exported)
	{
	    exported.add(new SoftReference(jsObject));
	}
    }

    /*
     * <p>
     * Native object reference
     * </p>
     */
    private int handle;

    /*
     * <p>
     * List of exported JSObject to be finalized upon closing the applet
     * </p>
     */
    private java.util.ArrayList exported = new ArrayList();
}


