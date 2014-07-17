/*
 * @(#)NameValueNode.java	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;
import java.io.*;

class NameValueNode extends NameNode {
    
    String val;

    NameValueNode(String name, String val) {
        super(name);
        this.val = val;
    }

    NameValueNode(String name, int ival) {
        super(name);
        this.val = Integer.toString(ival);
    }

    String value() {
        return val;
    }

    public String toString() {
        return name + "=" + val;
    }
}
