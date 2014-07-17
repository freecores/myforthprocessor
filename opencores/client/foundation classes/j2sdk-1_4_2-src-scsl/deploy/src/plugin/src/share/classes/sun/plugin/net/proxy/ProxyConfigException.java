/*
 * @(#)ProxyConfigException.java	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.net.proxy;

public class ProxyConfigException extends Exception
{
    /**
     * Create a proxy config exception object
     */
    ProxyConfigException()
    {
    }    

    /**
     * Create a proxy config exception object
     */
    ProxyConfigException(String msg)
    {
	super(msg);
    }    

    /**
     * Create a proxy config exception object
     */
    ProxyConfigException(String msg, Throwable e)
    {
	super(msg, e);
    }    
}
