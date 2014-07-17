/*
 * @(#)OriginNotAllowedException.java	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.liveconnect;

/**
 * @version 	1.1 
 * @author	Stanley Man-Kit Ho
 */

public class OriginNotAllowedException extends Exception 
{
    OriginNotAllowedException()
    {
	super();
    }

    OriginNotAllowedException(String msg)
    {
	super(msg);
    }
}



