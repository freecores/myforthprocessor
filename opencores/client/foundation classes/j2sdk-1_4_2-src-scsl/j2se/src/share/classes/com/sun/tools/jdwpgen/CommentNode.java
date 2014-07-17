/*
 * @(#)CommentNode.java	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;
import java.io.*;

class CommentNode extends AbstractSimpleNode {
    
    String text;

    CommentNode(String text) {
        this.text = text;
    }

    String text() {
        return text;
    }
}
