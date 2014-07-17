/*
 * @(#)AbstractCommandNode.java	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;
import java.io.*;

class AbstractCommandNode extends AbstractNamedNode {

    void document(PrintWriter writer) {
        writer.println("<h5><a name=\"" + context.whereC + "\">" + name + 
                       " Command</a> (" + nameNode.value() + ")</h5>");
        writer.println(comment());
        writer.println("<dl>");
        for (Iterator it = components.iterator(); it.hasNext();) {
            ((Node)it.next()).document(writer);
        }
        writer.println("</dl>");
    }

    void documentIndex(PrintWriter writer) {
        writer.print("        <li><a href=\"#" + context.whereC + "\">");
        writer.println(name() + "</a> (" + nameNode.value() + ")");
    }
}
