/*
 * @(#)AbstractSimpleNode.java	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;
import java.io.*;

abstract class AbstractSimpleNode extends Node {

    AbstractSimpleNode() {
        kind = "-simple-";
        components = new ArrayList();
    }

    void document(PrintWriter writer) {
        writer.print(toString());
    }
}
