/*
 * @(#)IntegerType.java	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jdi;

/**
 * The type of all primitive <code>int</code> values 
 * accessed in the target VM. Calls to {@link Value#type} will return an 
 * implementor of this interface.
 *
 * @see IntegerValue
 *
 * @author James McIlree
 * @since  1.3
 */
public interface IntegerType extends PrimitiveType
{
}

