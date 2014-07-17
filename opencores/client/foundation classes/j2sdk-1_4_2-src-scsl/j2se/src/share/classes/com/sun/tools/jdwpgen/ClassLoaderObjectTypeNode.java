/*
 * @(#)ClassLoaderObjectTypeNode.java	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;

class ClassLoaderObjectTypeNode extends ObjectTypeNode {

    String docType() {
        return "classLoaderID";
    }

    String javaType() {
        return "ClassLoaderReferenceImpl";
    }

    String javaRead() {
        return "ps.readClassLoaderReference()";
    }
}
