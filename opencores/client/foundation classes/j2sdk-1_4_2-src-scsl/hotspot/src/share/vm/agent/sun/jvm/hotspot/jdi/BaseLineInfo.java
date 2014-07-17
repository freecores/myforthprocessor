/*
 * @(#)BaseLineInfo.java	1.1 02/03/04
 *
 * Copyright 2003 Sun Microsystems, Inc. All Rights Reserved.
 * 
 * This software is the proprietary information of Sun Microsystems, Inc.  
 * Use is subject to license terms.
 * 
 */

package sun.jvm.hotspot.jdi;

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

