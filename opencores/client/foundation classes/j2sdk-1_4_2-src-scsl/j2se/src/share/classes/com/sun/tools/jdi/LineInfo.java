/*
 * @(#)LineInfo.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdi;

import com.sun.jdi.*;

interface LineInfo {
    
    String liStratum();

    int liLineNumber();

    String liSourceName() throws AbsentInformationException;

    String liSourcePath() throws AbsentInformationException;
}
