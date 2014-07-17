/*
 * @(#)JarCacheVersionException.java	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.cache;

public class JarCacheVersionException extends Exception
{
    JarCacheVersionException()
    {
    }    

    JarCacheVersionException(String msg)
    {
	super(msg);
    }    
}
