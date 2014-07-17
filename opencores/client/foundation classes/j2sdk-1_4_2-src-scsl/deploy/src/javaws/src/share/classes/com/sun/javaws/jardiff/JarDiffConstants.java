/*
 * @(#)JarDiffConstants.java	1.7 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.jardiff;

import com.sun.javaws.debug.*;
import java.io.*;
import java.util.*;
import java.util.jar.*;
import java.util.zip.*;

/**
 * Constants used by creating patch and applying patch for JarDiff.
 *
 * @version 1.7, 01/23/03
 */
public interface JarDiffConstants {
    public final String VERSION_HEADER = "version 1.0";
    public final String INDEX_NAME = "META-INF/INDEX.JD";
    public final String REMOVE_COMMAND = "remove";
    public final String MOVE_COMMAND = "move";
}
