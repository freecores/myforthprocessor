/*
 * @(#)AbsentInformationException.java	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jdi;

/**
 * Thrown to indicate line number or variable information is not available.
 *
 * @author Gordon Hirsch
 * @since  1.3
 */
public class AbsentInformationException extends Exception
{
    public AbsentInformationException()
    {
	super();
    }

    public AbsentInformationException(String s)
    {
	super(s);
    }
}
