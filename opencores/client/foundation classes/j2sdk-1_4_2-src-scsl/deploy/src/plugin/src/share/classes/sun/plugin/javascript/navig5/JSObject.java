/*
 * @(#)JSObject.java	1.28 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.javascript.navig5;

import java.applet.Applet;
import java.net.URL;
import java.security.AccessController;
import java.security.AccessControlContext;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import java.security.CodeSource;
import java.security.ProtectionDomain;
import java.security.AccessControlException;
import java.security.AllPermission;
import sun.plugin.javascript.JSContext;
import netscape.javascript.JSException;
import sun.plugin.util.Trace;


/**
 * <p> Emulate netscape.javascript.JSObject so applet can access the JavaScript
 * Document Object Model in the Mozilla.
 *
 * This class provides a wrapper around the latest JSObject definition in
 * Mozilla. In Mozilla, the layout and JS thread are combined into one, and
 * the restriction is that we are allowed to access any LiveConnect interface from
 * the JS thread (main thread) only. Thus, all calls in this class will be
 * forwarded to the main thread for execution.
 *
 * Notice that almost all native methods will take an array of certificates,
 * security context, etc as arguments. This is required for supporting LiveConnect
 * with security enabled. The certificate params are basically the break-down
 * representation of the CodeSource, and this is done to avoid multiple JNI
 * callback for extracting the information inside the CodeSource object.
 * The AccessControlContext is a snap-shot of the current Java stack
 * representation. LiveConnect will use these information to determine if
 * certain operations are allowed or not.
 * </p>
 */
public class JSObject extends netscape.javascript.JSObject {

    private int nativeJSObject = 0;
    private int jsThreadID = 0;

    /**
     * <p> Construct a JSObject. It is declared as protected so the applet can
     * not contains classes derived from it.
     * </p>
     *
     * @param Plug-in instance peer
     */
    public JSObject(int peer)  {
	this(JSGetThreadID(peer), JSGetNativeJSObject(peer));
    }

    /**
     * <p> Construct a JSObject. It is declared as protected so the applet can
     * not contains classes derived from it.
     * </p>
     *
     * @param jsThreadID JavaScript Thread ID
     * @param native JavaScript object
     */
    public JSObject(int jsThreadID, int nativeJSObject)  {
	this.jsThreadID = jsThreadID;
	this.nativeJSObject = nativeJSObject;
    }

    /**
     * <p> Calls a JavaScript method. Equivalent to
     * "this.methodName(args[0], args[1], ...)" in JavaScript.
     * </p>
     *
     * @param methodName The name of the JavaScript method to be invoked.
     * @param args An array of Java object to be passed as arguments to the method.
     * @return Result of the method.
     */
    public Object call(String methodName, Object args[]) throws JSException
    {
	SecurityContext ctx = SecurityContext.getCurrentSecurityContext();

	boolean isAllPermission = false;

	try
	{
	    AccessControlContext acc = ctx.getAccessControlContext();

	    acc.checkPermission(new AllPermission());

	    isAllPermission = true;
	}
	catch (AccessControlException e)
	{
	}

        Trace.msgLiveConnectPrintln("jsobject.call", 
                                    new Object[] {methodName, ctx.getURL(), String.valueOf(isAllPermission)});

	return JSObjectCall(jsThreadID, nativeJSObject, ctx.getURL(),
			    methodName, args, isAllPermission);
    }

    /**
     * <p> Evaluates a JavaScript expression. The expression is a string of
     * JavaScript source code which will be evaluated in the context given by
     * "this".
     * </p>
     *
     * @param s The JavaScript expression.
     * @return Result of the JavaScript evaluation.
     */
    public Object eval(String s) throws JSException
    {
	SecurityContext ctx = SecurityContext.getCurrentSecurityContext();

	boolean isAllPermission = false;

	try
	{
	    AccessControlContext acc = ctx.getAccessControlContext();

	    acc.checkPermission(new AllPermission());

	    isAllPermission = true;
	}
	catch (AccessControlException e)
	{
	}

        Trace.msgLiveConnectPrintln("jsobject.eval.url.permission", 
                                     new Object[] {s, ctx.getURL(), String.valueOf(isAllPermission)});
	return JSObjectEval(jsThreadID, nativeJSObject, ctx.getURL(),
			    s, isAllPermission);
    }

    /**
     * <p> Retrieves a named member of a JavaScript object. Equivalent to
     * "this.name" in JavaScript.
     * </p>
     *
     * @param name The name of the JavaScript property to be accessed.
     * @return The value of the propery.
     */
    public Object getMember(String name) throws JSException
    {
	SecurityContext ctx = SecurityContext.getCurrentSecurityContext();

	boolean isAllPermission = false;

	try
	{
	    AccessControlContext acc = ctx.getAccessControlContext();

	    acc.checkPermission(new AllPermission());

	    isAllPermission = true;
	}
	catch (AccessControlException e)
	{
	}

        Trace.msgLiveConnectPrintln("jsobject.getMember", 
                                    new Object[] {name, ctx.getURL(), String.valueOf(isAllPermission)});
	return JSObjectGetMember(jsThreadID, nativeJSObject, ctx.getURL(),
				 name, isAllPermission);
    }

    /**
     * <p> Sets a named member of a JavaScript object. Equivalent to
     * "this.name = value" in JavaScript.
     * </p>
     *
     * @param name The name of the JavaScript property to be accessed.
     * @param value The value of the propery.
     */
    public void setMember(String name, Object value) throws JSException
    {
	SecurityContext ctx = SecurityContext.getCurrentSecurityContext();

	boolean isAllPermission = false;

	try
	{
	    AccessControlContext acc = ctx.getAccessControlContext();

	    acc.checkPermission(new AllPermission());

	    isAllPermission = true;
	}
	catch (AccessControlException e)
	{
	}

        Trace.msgLiveConnectPrintln("jsobject.setMember", 
                                    new Object[] {name, ctx.getURL(), String.valueOf(isAllPermission)});                                    

	JSObjectSetMember(jsThreadID, nativeJSObject, ctx.getURL(),
			  name, value, isAllPermission);
    }

    /**
     * <p> Removes a named member of a JavaScript object.
     * </p>
     *
     * @param name The name of the JavaScript property to be removed.
     */
    public void removeMember(String name) throws JSException
    {
	SecurityContext ctx = SecurityContext.getCurrentSecurityContext();

	boolean isAllPermission = false;

	try
	{
	    AccessControlContext acc = ctx.getAccessControlContext();

	    acc.checkPermission(new AllPermission());

	    isAllPermission = true;
	}
	catch (AccessControlException e)
	{
	}

        Trace.msgLiveConnectPrintln("jsobject.removeMember",
                                    new Object[] {name, ctx.getURL(), String.valueOf(isAllPermission)});

	JSObjectRemoveMember(jsThreadID, nativeJSObject, ctx.getURL(),
			     name, isAllPermission);
    }

    /**
     * <p> Retrieves an indexed member of a JavaScript object. Equivalent to
     * "this[index]" in JavaScript.
     * </p>
     *
     * @param int The index of the array to be accessed.
     * @return The value of the indexed member.
     */
    public Object getSlot(int index) throws JSException
    {
	SecurityContext ctx = SecurityContext.getCurrentSecurityContext();

	boolean isAllPermission = false;

	try
	{
	    AccessControlContext acc = ctx.getAccessControlContext();

	    acc.checkPermission(new AllPermission());

	    isAllPermission = true;
	}
	catch (AccessControlException e)
	{
	}

        Trace.msgLiveConnectPrintln("jsobject.getSlot", 
                                    new Object[] {String.valueOf(index), ctx.getURL(), String.valueOf(isAllPermission)});

	return JSObjectGetSlot(jsThreadID, nativeJSObject, ctx.getURL(),
			       index, isAllPermission);
    }

    /**
     * <p> Sets an indexed member of a JavaScript object. Equivalent to
     * "this[index] = value" in JavaScript.
     * </p>
     *
     * @param int The index of the array to be accessed.
     */
    public void setSlot(int index, Object value) throws JSException
    {
	SecurityContext ctx = SecurityContext.getCurrentSecurityContext();

	boolean isAllPermission = false;

	try
	{
	    AccessControlContext acc = ctx.getAccessControlContext();

	    acc.checkPermission(new AllPermission());

	    isAllPermission = true;
	}
	catch (AccessControlException e)
	{
	}

        Trace.msgLiveConnectPrintln("jsobject.setSlot", 
                                    new Object[] {String.valueOf(index), ctx.getURL(), String.valueOf(isAllPermission)});

	JSObjectSetSlot(jsThreadID, nativeJSObject, ctx.getURL(),
		        index, value, isAllPermission);
    }

    /**
     * <p> Convert JSObject to a string.
     * </p>
     */
    public String toString()
    {
        return JSObjectToString(jsThreadID, nativeJSObject);
    }

    /**
     * <p> Free up any outstanding resource that was held.
     * </p>
     */
    public void finalize()
	throws Throwable
    {
	JSFinalize(jsThreadID, nativeJSObject);
	super.finalize();
    }

    /**
     * <p> Obtain native JavaScript object for a given plugin instance peer.
     * </p>
     *
     * @param peer nsIPluginInstancePeer
     * @return int native JavaScript object
     */
    private static int JSGetNativeJSObject(int peer)
    {
	SecurityContext ctx = SecurityContext.getCurrentSecurityContext();

	boolean isAllPermission = false;

	try
	{
	    AccessControlContext acc = ctx.getAccessControlContext();

	    acc.checkPermission(new AllPermission());

	    isAllPermission = true;
	}
	catch (AccessControlException e)
	{
	}

	return JSGetNativeJSObject(peer, ctx.getURL(), isAllPermission);
    }


    /**
     * <p> Obtain native JavaScript object for a given plugin instance peer.
     * </p>
     *
     * @param peer nsIPluginInstancePeer interface pointer
     * @param url URL of the caller
     * @param chain Certificate chain array
     * @param certLength Length of certificate chain array
     * @param numOfCerts Number of certificates
     * @param ctx AccessControlContext
     * @return int native JavaScript object
     */
    private static native int JSGetNativeJSObject(int peer, String url,
						  boolean securityContext);

    /**
     * <p> Obtain JavaScript Thread ID associated with a given plugin instance peer.
     * </p>
     *
     * @param peer nsIPluginInstancePeer interface pointer
     * @return int JavaScript thread ID associated with the nsIPluginInstancePeer
     */
    private static native int JSGetThreadID(int peer);

    /**
     * <p> Call JavaScript to free up any resource.
     * </p>
     *
     * @param jsThreadID JavaScript Thread ID
     * @param int The native JavaScript object
     */
    private static native void JSFinalize(int jsThreadID, int nativeJSObject);

    /**
     * <p> Calls a JavaScript method. Equivalent to
     * "this.methodName(args[0], args[1], ...)" in JavaScript.
     * </p>
     *
     * @param jsThreadID JavaScript thread ID
     * @param int The native JavaScript object
     * @param url URL of the caller
     * @param chain Certificate chain array
     * @param certLength Length of certificate chain array
     * @param numOfCerts Number of certificates
     * @param methodName The name of the JavaScript method to be invoked.
     * @param args An array of Java object to be passed as arguments to the method.
     * @param ctx AccessControlContext
     * @return Result of the method.
     */
    private static native Object JSObjectCall(int jsThreadID, int nativeJSObject, String url,
					      String methodName, Object args[], boolean securityContext) throws JSException;

    /**
     * <p> Evaluates a JavaScript expression. The expression is a string of
     * JavaScript source code which will be evaluated in the context given by
     * "this".
     * </p>
     *
     * @param jsThreadID JavaScript thread ID
     * @param int The native JavaScript object
     * @param url URL of the caller
     * @param chain Certificate chain array
     * @param certLength Length of certificate chain array
     * @param numOfCerts Number of certificates
     * @param script The JavaScript expression.
     * @param ctx AccessControlContext
     * @return Result of the JavaScript evaluation.
     */
    private static native Object JSObjectEval(int jsThreadID, int nativeJSObject, String url,
					      String script, boolean securityContext) throws JSException;

    /**
     * <p> Retrieves a named member of a JavaScript object. Equivalent to
     * "this.name" in JavaScript.
     * </p>
     *
     * @param jsThreadID JavaScript thread ID
     * @param int The native JavaScript object
     * @param url URL of the security context
     * @param chain Certificate chains
     * @param certLength Certificate length in chain
     * @param numOfCerts Number of certificate in chain
     * @param name The name of the JavaScript property to be accessed.
     * @param ctx Security context in the current stack
     * @return The value of the propery.
     */
    private static native Object JSObjectGetMember(int jsThreadID, int nativeJSObject, String url,
						   String name, boolean securityContext) throws JSException;

    /**
     * <p> Sets a named member of a JavaScript object. Equivalent to
     * "this.name = value" in JavaScript.
     * </p>
     *
     * @param jsThreadID JavaScript thread ID
     * @param int The native JavaScript object
     * @param url URL of the security context
     * @param chain Certificate chains
     * @param certLength Certificate length in chain
     * @param numOfCerts Number of certificate in chain
     * @param name The name of the JavaScript property to be accessed.
     * @param value The value of the propery.
     * @param ctx Security context in the current stack
     */
    private static native void JSObjectSetMember(int jsThreadID, int nativeJSObject, String url,
						 String name, Object value, boolean securityContext) throws JSException;

    /**
     * <p> Removes a named member of a JavaScript object.
     * </p>
     *
     * @param jsThreadID JavaScript thread ID
     * @param int The native JavaScript object
     * @param url URL of the security context
     * @param chain Certificate chains
     * @param certLength Certificate length in chain
     * @param numOfCerts Number of certificate in chain
     * @param name The name of the JavaScript property to be removed.
     * @param ctx Security context in the current stack
     */
    private static native void JSObjectRemoveMember(int jsThreadID, int nativeJSObject, String url,
						    String name, boolean securityContext) throws JSException;

    /**
     * <p> Retrieves an indexed member of a JavaScript object. Equivalent to
     * "this[index]" in JavaScript.
     * </p>
     *
     * @param jsThreadID JavaScript thread ID
     * @param int The native JavaScript object
     * @param url URL of the security context
     * @param chain Certificate chains
     * @param certLength Certificate length in chain
     * @param numOfCerts Number of certificate in chain
     * @param int The index of the array to be accessed.
     * @param ctx Security context in the current stack
     * @return The value of the indexed member.
     */
    private static native Object JSObjectGetSlot(int jsThreadID, int nativeJSObject, String url,
						 int index, boolean securityContext) throws JSException;

    /**
     * <p> Sets an indexed member of a JavaScript object. Equivalent to
     * "this[index] = value" in JavaScript.
     * </p>
     *
     * @param jsThreadID JavaScript thread ID
     * @param int The native JavaScript object
     * @param url URL of the security context
     * @param chain Certificate chains
     * @param certLength Certificate length in chain
     * @param numOfCerts Number of certificate in chain
     * @param int The index of the array to be accessed.
     * @param value The value of the indexed member to set
     * @param ctx Security context in the current stack
     */
    private static native void JSObjectSetSlot(int jsThreadID, int nativeJSObject, String url,
					       int index, Object value, boolean securityContext) throws JSException;

    /**
     * <p> Obtains string representation of JavaScript object.
     * </p>
     *
     * @param jsThreadID JavaScript thread ID
     * @param int The native JavaScript object
     * @return String representation of the JavaScript object.
     */
    private static native String JSObjectToString(int jsThreadID, int nativeJSObject);
}


/**
 * SecurityContext encapulates all the security context on the current Java stack.
 */
class SecurityContext
{
    private ProtectionDomain domain;
    private AccessControlContext ctx;

    SecurityContext(ProtectionDomain domain, AccessControlContext ctx)
    {
	this.domain = domain;
	this.ctx = ctx;
    }

    String getURL()
    {
	if (domain != null)
	{
	    CodeSource src = domain.getCodeSource();

	    if (src != null)
	    {
		URL u = src.getLocation();
		if (u != null)
		{
		    StringBuffer buffer = new StringBuffer();
		    String protocol = u.getProtocol();
		    String host = u.getHost();
		    int port = u.getPort();

		    buffer.append(protocol);
		    buffer.append("://");
		    buffer.append(host);

		    if (port != -1)
			buffer.append(":" + port);

		    return buffer.toString();
		}
	    }
	}

	return "file://";
    }

    byte[][] getCertChain()
    {
	return null;
    }

    int[] getCertLength()
    {
	return null;
    }

    int getNumOfCert()
    {
	return 0;
    }

    AccessControlContext getAccessControlContext()
    {
	return ctx;
    }

    static class PrivilegedBlockAction implements PrivilegedExceptionAction {
	AccessControlContext ctx;

	PrivilegedBlockAction(AccessControlContext ctx)
	{
	    this.ctx = ctx;
	}

	public Object run() throws Exception {
	    SecurityManager sm = System.getSecurityManager();

	    if (sm != null && sm instanceof sun.plugin.security.ActivatorSecurityManager)
	    {
		sun.plugin.security.ActivatorSecurityManager mgr = (sun.plugin.security.ActivatorSecurityManager) sm;

		Class[] clazz = mgr.getExecutionStackContext();

		// Walk up the stack to find a class loader.
		for (int i=0; i < clazz.length; i++)
		{
		    Class c = clazz[i];
		    ClassLoader loader = c.getClassLoader();
		    if (loader instanceof java.net.URLClassLoader || loader instanceof sun.applet.AppletClassLoader || sun.applet.AppletClassLoader.class.isAssignableFrom(c))
			return new SecurityContext(c.getProtectionDomain(), ctx);
		}
	    }

	    return new SecurityContext(null, ctx);
	}
    }

    static SecurityContext getCurrentSecurityContext()
    {
	// Obtain snap shot of the stack
	AccessControlContext ctx = AccessController.getContext();

	try {
	    return (SecurityContext) AccessController.doPrivileged(new PrivilegedBlockAction(ctx));
	} catch (PrivilegedActionException e) {
	    Trace.securityPrintException(e);
	    return new SecurityContext(null, ctx);
	}
    }
}

