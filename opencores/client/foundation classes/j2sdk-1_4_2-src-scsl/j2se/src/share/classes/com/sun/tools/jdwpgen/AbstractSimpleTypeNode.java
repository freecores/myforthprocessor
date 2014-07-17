/*
 * @(#)AbstractSimpleTypeNode.java	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;

abstract class AbstractSimpleTypeNode extends AbstractTypeNode {

    void constrain(Context ctx) {
        context = ctx;
        nameNode.constrain(ctx);
        if (components.size() != 0) {
            error("Extraneous content: " + components.get(0));
        }
    }
}
