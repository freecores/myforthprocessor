/*
 * @(#)ForbiddenTargetException.java	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package netscape.security;

/**
 * This exception is thrown when a privilege request is denied. 
 *
 * This class acts as a stub to provide backward compatibility for Netscape 
 * 4.x VM.
 */
public class ForbiddenTargetException extends java.lang.RuntimeException 
{
/**
     * Constructs an ForbiddenTargetException with no detail message.
     */
    public ForbiddenTargetException()
    {
	super();
    }

    /**
     * Constructs an ForbiddenTargetException with the specified detail message. 
     *
     * @param msg string that describes this particular exception
     */
    public ForbiddenTargetException(String msg)
    {
	super(msg);
    }
}
