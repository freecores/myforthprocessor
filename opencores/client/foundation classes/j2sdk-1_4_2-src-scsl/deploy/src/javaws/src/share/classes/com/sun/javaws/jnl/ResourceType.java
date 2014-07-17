/*
 * @(#)ResourceType.java	1.7 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.jnl;
import com.sun.javaws.xml.XMLable;

/*
 * Public super class for all resource entries
 */
interface ResourceType extends XMLable {
    /** Visit this specific type */
    void visit(ResourceVisitor visitor);
}
