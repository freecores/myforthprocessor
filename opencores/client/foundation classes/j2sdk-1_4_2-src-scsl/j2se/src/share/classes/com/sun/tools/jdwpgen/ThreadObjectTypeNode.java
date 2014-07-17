/*
 * @(#)ThreadObjectTypeNode.java	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;

class ThreadObjectTypeNode extends ObjectTypeNode {

    String docType() {
        return "threadID";
    }

    String javaType() {
        return "ThreadReferenceImpl";
    }

    String javaRead() {
        return "ps.readThreadReference()";
    }
}
