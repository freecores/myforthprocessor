/*
 * @(#)CommentedMethodFinder.java	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets;  

import com.sun.javadoc.*;
import java.lang.*;
import java.util.*;
import java.io.*;

/**
 * Find a commented method.
 */
public class CommentedMethodFinder extends MethodFinder {
    public boolean isCorrectMethod(MethodDoc method) {
        return method.inlineTags().length > 0;
    }
}
 
