/*
 * @(#)InvalidRequestStateException.java	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jdi.request;

/**
 * Thrown to indicate that the requested event cannot be modified
 * because it is enabled. Filters can be added only to disabled 
 * event requests.
 * Also thrown if an operation is attempted on a deleted request.
 * See {@link EventRequestManager#deleteEventRequest(EventRequest)}
 *
 * @author Robert Field
 * @since  1.3
 */
public class InvalidRequestStateException extends RuntimeException {
    public InvalidRequestStateException()
    {
	super();
    }

    public InvalidRequestStateException(String s)
    {
	super(s);
    }
}
