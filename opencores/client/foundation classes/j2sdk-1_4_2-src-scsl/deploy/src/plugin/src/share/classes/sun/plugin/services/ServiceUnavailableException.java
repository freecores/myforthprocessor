/*
 * @(#)ServiceUnavailableException.java	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.services;

/**
 * ServiceUnavailableException is thrown when a particular
 * browser service is unavailable for the time being -
 * especially when legacy-lifecycle model is used.
 */
public class ServiceUnavailableException extends Exception
{
    /**
     * Create a service unavailable exception object
     */
    public ServiceUnavailableException()
    {
    }    

    /**
     * Create a service unavailable exception object
     */
    public ServiceUnavailableException(String msg)
    {
	super(msg);
    }    

    /**
     * Create a service unavailable exception object
     */
    public ServiceUnavailableException(String msg, Throwable e)
    {
	super(msg, e);
    }    
}
