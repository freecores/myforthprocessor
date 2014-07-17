/*
 * @(#)BaseLineInfo.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdi;

import com.sun.jdi.*;

class BaseLineInfo implements LineInfo {
    private final int lineNumber;
    private final ReferenceTypeImpl declaringType;

    BaseLineInfo(int lineNumber,
                 ReferenceTypeImpl declaringType) {
        this.lineNumber = lineNumber;
        this.declaringType = declaringType;
    }

    public String liStratum() {
        return SDE.BASE_STRATUM_NAME;
    }

    public int liLineNumber() {
        return lineNumber;
    }

    public String liSourceName()
                            throws AbsentInformationException {
        return declaringType.baseSourceName();
    }

    public String liSourcePath()
                            throws AbsentInformationException {
        return declaringType.baseSourcePath();
    }
}

