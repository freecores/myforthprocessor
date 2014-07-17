/*
 * @(#)DispatchImpl.java	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.com;

import sun.plugin.util.Trace;
import sun.plugin.javascript.ocx.JSObject;
import java.applet.Applet;
import sun.plugin.viewer.context.IExplorerAppletContext;

/**
 *  DispatchImpl encapsulates a Java Object and provides Dispatch interface
 *  It is responsible for maintaining the identity and type of one instance of
 *  a Java object. Objects of this type are used to invoke methods on the java
 *  object that they represent.
 */
public class DispatchImpl implements Dispatch
{
    JavaClass targetClass = null;
    Object targetObj = null;

    /*
     * Constructor
     * @param obj the object to be wrapped
     */
    public DispatchImpl(Object obj)
    {
	targetObj = obj;
    }

    /**
     * Invoke a method according to the method index.
     *
     * @param flag Invoke flag
     * @param index Method index
     * @param params Arguments.
     * @return Java object.
     */
    public Object invoke(int flag, int index, Object []params)
        throws  Exception
    {
	Object retObj = null;
	Dispatcher disp = null;
	try {
	    convertParams(params);
	    disp = targetClass.getDispatcher(flag, index, params);
	    return disp.invoke(targetObj, params);
        }
        catch (Throwable e)
        {
	    //e.printStackTrace();
	    Throwable cause = e.getCause();
	    if(cause == null) {
		cause = e;
	    }

       	    Trace.liveConnectPrintException(cause);
	    throw new Exception(cause.toString());
        }
    }

    /**
     * Return the Java object wrapped by this proxy
     */
    public Object getWrappedObject()
    {
        return targetObj;
    }

    /**
     * Return the class object of the java object wrapped by this proxy
     */
    public JavaClass getTargetClass() {
	if(targetClass == null && targetObj != null) {
	    targetClass = new JavaClass(targetObj.getClass());
	}

	return targetClass;
    }


    /*
     *
     */
    public int getIdForName(String name) throws Exception{
	int id = -1;

	if(targetClass == null && targetObj != null) {
	    targetClass = new JavaClass(targetObj.getClass());
	}
	
	if(targetClass != null) {
	    id = targetClass.getIdForName(name);
	}

	//Trace.liveConnectPrintln("Object: " + targetObj + " Name: " + name + " Id: " + id);
		    
	return id;
    }

    /*
     *
     */
    private void convertParams(Object []params) {
	for(int i=0;i<params.length;i++) {
	    if(params[i] != null && params[i] instanceof DispatchImpl) {
		params[i] = ((DispatchImpl)params[i]).getWrappedObject();
	    } else if(params[i] != null && params[i] instanceof DispatchClient){
		JSObject jsObj = new JSObject((DispatchClient)params[i]);
		jsObj.setIExplorerAppletContext((IExplorerAppletContext)
				((Applet)targetObj).getAppletContext());
		params[i] = jsObj;
	    }
	}
    }

    public String toString() {
	if(targetObj != null) {
	    return targetObj.toString();
	}
	return null;
    }
}




