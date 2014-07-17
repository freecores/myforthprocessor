/*
 * @(#)ArrayTypeNode.java	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;

class ArrayTypeNode extends ReferenceTypeNode {

    String docType() {
        return "arrayTypeID";
    }

    String javaType() {
        return "ArrayTypeImpl";
    }

    String javaRead() {
        return "--- should not get generated ---";
    }
}
