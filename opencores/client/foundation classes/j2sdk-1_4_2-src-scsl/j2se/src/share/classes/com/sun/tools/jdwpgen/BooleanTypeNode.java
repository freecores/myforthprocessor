/*
 * @(#)BooleanTypeNode.java	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;
import java.io.*;

class BooleanTypeNode extends AbstractSimpleTypeNode {

    String docType() {
        return "boolean";
    }

    public void genJavaWrite(PrintWriter writer, int depth, 
                             String writeLabel) {
        genJavaDebugWrite(writer, depth, writeLabel);
        indent(writer, depth);
        writer.println("ps.writeBoolean(" + writeLabel + ");");
    }

    String javaRead() {
        return "ps.readBoolean()";
    }
}