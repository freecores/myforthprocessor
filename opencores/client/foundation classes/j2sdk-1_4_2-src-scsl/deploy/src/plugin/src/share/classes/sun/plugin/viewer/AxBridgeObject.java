/*
 * @(#)AxBridgeObject.java	1.5 03/02/12
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
import java.net.URL;
import java.util.Iterator;
import java.util.HashMap;
import java.util.Set;
import sun.applet.AppletPanel;
import sun.plugin.BeansApplet;
import sun.plugin.AppletViewer;
import sun.plugin.BeansViewer;
import sun.plugin.util.Trace;
import sun.plugin.viewer.frame.IExplorerEmbeddedFrame;
import sun.plugin.viewer.context.IExplorerAppletContext;
import sun.plugin.viewer.context.AxBridgeAppletContext;
import sun.plugin.viewer.context.PluginBeansContext;
import sun.plugin.services.BrowserService;
import sun.plugin.services.ServiceProvider;
import sun.plugin.com.BeanDispatchImpl;
import sun.plugin.com.DispatchImpl;
import sun.plugin.com.event.ListenerProxy;
import sun.plugin.com.BeanClass;
import sun.plugin.com.BeanCustomizer;
import java.io.ByteArrayOutputStream;
import java.beans.XMLEncoder;
import java.beans.BeanInfo;

/**
 * <p> AxBridgeObject is a class that encapsulates a bean running inside
 * of any ActiveX containers. It contains all functions that are 
 * required to create, load, stop and destroy the applet or bean.
 * </p>
 */
public class AxBridgeObject extends IExplorerPluginObject implements sun.plugin.AppletStatusListener
{
    private BeanDispatchImpl dispImpl = null;
    private ListenerProxy listener = null;

    AxBridgeObject(int id, boolean isBeans, String identifier) {
	super(id, isBeans, identifier);

	//Create applet context and associate it with the panel
	AxBridgeAppletContext axContext = new AxBridgeAppletContext();
	this.panel.setAppletContext(axContext);
    }

    public void mayInit() {
	if(bFrameReady == true && bInit == false) {
	    bInit = true;
	    initPlugin();	    
	}
    }

    /**
     * <p> Add a new panel to the embedded frame and start to download the 
     * applet/bean.
     * </p>
     */
    private synchronized void initPlugin() {
	assert (panel != null);
	this.panel.addAppletStatusListener(this);
	this.panel.appletInit();
	this.panel.appletStart();
    }

    /**
     * <p>
     * The JavaBeans component or applet is being closed
     * </p>
     */
    public void destroyPlugin() {
	assert (panel != null);
	panel.removeAppletStatusListener(this);
	panel.appletStop();
	panel.appletDestroy();
    }

    public Object getDispatchObject() {
	Object obj = getJavaObject();
	if(dispImpl == null && obj != null)
	    dispImpl = new BeanDispatchImpl(obj);

	return dispImpl;
    }

    public Object getCustomizer() {
	BeanInfo bInfo = ((BeanClass)dispImpl.getTargetClass()).getBeanInfo();
	return new BeanCustomizer(bInfo);
    }

    /**
     * Notify applet status change
     */
    public void statusChanged(int status) {
	if(status == panel.APPLET_INIT) {
	    Object dispObj = getDispatchObject();
	    if(dispObj != null) {
		listener = new ListenerProxy(id, (DispatchImpl)dispObj);
		listener.register();
	    }
	}

	notifyStatusChange(id, status);
    }


    /*
     * 	Save the bean and return the byte array
     */
    public Object save(){
	byte[] bytes =  null;
	//System.out.println("Saving bean: " + this);
	Object obj = getJavaObject();
	ClassLoader loader = Thread.currentThread().getContextClassLoader();
	Thread.currentThread().setContextClassLoader(obj.getClass().getClassLoader());

	if(getLoadingStatus() > panel.APPLET_INIT)
	    listener.unregister();

	try {
	    ByteArrayOutputStream bos = new ByteArrayOutputStream();
	    XMLEncoder xml = new XMLEncoder(bos);
	    xml.writeObject(obj);
	    xml.close();
	    bytes = bos.toByteArray();
	    bos.close();
	}catch(Exception exc) {
	    exc.printStackTrace();
	}

	if(getLoadingStatus() > panel.APPLET_INIT)
	    listener.register();

	Thread.currentThread().setContextClassLoader(loader);
	return bytes;
    }

    /*
     *	Load the bean using the byte array stream
     */
    public void load(Object obj) {
	//System.out.println("Loading bean: " + this);
	try {
	    byte[] bArray = (byte[])obj;
	    ((BeansViewer)panel).setByteStream(bArray);
	}catch(Exception exc) {
	    exc.printStackTrace();
	}
    }


    // Native code to notify the changes
    private native void notifyStatusChange(int handle, int status);
}

