/*
 * @(#)DuplicateRequestException.java	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jdi.request;

/**
 * Thrown to indicate a duplicate event request.
 *
 * @author Robert Field
 * @since  1.3
 */
public class DuplicateRequestException extends RuntimeException
{
    public DuplicateRequestException()
    {
	super();
    }

    public DuplicateRequestException(String s)
    {
	super(s);
    }
}
