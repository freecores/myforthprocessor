/**
 * @(#)Retro.java	1.19 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8;
import java.io.*;

import com.sun.tools.javac.v8.util.*;

import com.sun.tools.javac.v8.code.*;

import com.sun.tools.javac.v8.tree.*;

import com.sun.tools.javac.v8.comp.*;

import com.sun.tools.javac.v8.code.Symbol.*;

import com.sun.tools.javac.v8.code.Type.*;


/**
 * A class for retrofitting plain Java class files with generic signatures.
 */
public class Retro implements TypeTags, Kinds, Flags {

    public Retro() {
        super();
    }
}
