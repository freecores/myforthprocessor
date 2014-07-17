/*
 * @(#)InterfaceTypeNode.java	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;

class InterfaceTypeNode extends ReferenceTypeNode {

    String docType() {
        return "interfaceID";
    }

    String javaType() {
        return "InterfaceTypeImpl";
    }

    String javaRead() {
        return "vm.interfaceType(ps.readClassRef())";
    }
}
