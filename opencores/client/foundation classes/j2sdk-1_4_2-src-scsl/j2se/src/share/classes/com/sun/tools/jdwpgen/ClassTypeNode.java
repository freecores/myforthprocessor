/*
 * @(#)ClassTypeNode.java	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;

class ClassTypeNode extends ReferenceTypeNode {

    String docType() {
        return "classID";
    }

    String javaType() {
        return "ClassTypeImpl";
    }

    String javaRead() {
        return "vm.classType(ps.readClassRef())";
    }
}
