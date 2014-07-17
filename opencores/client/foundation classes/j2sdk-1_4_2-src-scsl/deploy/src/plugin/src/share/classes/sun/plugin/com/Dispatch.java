/*
 * @(#)Dispatch.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.com;

/**
 *  Dispatch is the Java side companion to the IDispatch COM interface.
 */
public interface Dispatch
{
    /**
     * Invoke a method according to the method index.
     *
     * @param flag invoke context
     * @param methodIndex Method index
     * @param args Arguments.
     * @return Java object.
     */
    public Object invoke(int flag, int methodIndex, Object [] args)
        throws  Exception;

    /**
     * Return the Java object wrapped by this proxy/
     */
    public int getIdForName(String methodName) throws Exception;

    /**
     * Return the wrapped Java object 
     */
    public Object getWrappedObject();

    //constants
    public final static int METHOD = 0x1;
    public final static int PROPERTYGET = 0x2;
    public final static int PROPERTYPUT = 0x4;
    public final static int PROPERTYPUTREF = 0x8;

    public final static int propertyBase = 0x1000;
    public final static int eventBase = 0x4000;
    public final static int methodBase = 0x8000;
}

