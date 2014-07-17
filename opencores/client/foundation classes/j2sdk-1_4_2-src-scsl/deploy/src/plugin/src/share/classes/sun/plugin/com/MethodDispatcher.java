/*
 * @(#)MethodDispatcher.java	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.com;

import java.lang.reflect.Method;
import sun.plugin.util.Trace;

/**
 * A <code>MethodDispatcher</code> provides information about,
 * and access to, a single method on a class or interface.  The
 * reflected method may be a class method or an instance method
 * (including an abstract method).
 *
 * <p>A <code>MethodDispatcher</code> permits widening conversions
 * to occur when matching the actual parameters to invokewith the
 * underlying method's formal parameters, but it throws an
 * <code>IllegalArgumentException</code> if a narrowing conversion
 * would occur.
 *
 * @see Member
 * @see java.lang.Class
 * @see java.lang.Class#getMethods()
 * @see java.lang.Class#getMethod(String, Class[])
 * @see java.lang.Class#getDeclaredMethods()
 * @see java.lang.Class#getDeclaredMethod(String, Class[])
 */
public class MethodDispatcher implements Dispatcher
{
    private Method method = null;

    /**
     * Constructor
     */
    public MethodDispatcher(Method inMethod)
    {
	method = inMethod;
    }

    /*
     */
    public Object invoke(Object obj, Object[] args) throws Exception {
	Object retObj = null;
	if(method != null && obj != null) {
	    Trace.msgLiveConnectPrintln("com.method.invoke", new Object[] {method});
	    Object[] params = TypeConverter.convertObjectArray(
				    method.getParameterTypes(), args);
	    retObj = Utils.convertReturn( method.getReturnType(), 
					  method.invoke(obj, params ));
	}
	return retObj;
    }
}
