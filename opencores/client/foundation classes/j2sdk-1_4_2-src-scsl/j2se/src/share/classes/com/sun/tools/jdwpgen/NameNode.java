/*
 * @(#)NameNode.java	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;
import java.io.*;

class NameNode extends AbstractSimpleNode {
    
    String name;

    NameNode(String name) {
        this.name = name;
    }

    String text() {
        return name;
    }

    String value() {
        error("Valueless Name asked for value");
        return null;
    }

    public String toString() {
        return name;
    }
}
