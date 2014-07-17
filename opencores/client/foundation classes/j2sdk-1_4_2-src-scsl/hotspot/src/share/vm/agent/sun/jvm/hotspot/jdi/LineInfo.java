/*
 * @(#)LineInfo.java	1.1 02/03/04
 *
 * Copyright 2003 Sun Microsystems, Inc. All Rights Reserved.
 * 
 * This software is the proprietary information of Sun Microsystems, Inc.  
 * Use is subject to license terms.
 * 
 */

package sun.jvm.hotspot.jdi;

import com.sun.jdi.*;

interface LineInfo {
    
    String liStratum();

    int liLineNumber();

    String liSourceName() throws AbsentInformationException;

    String liSourcePath() throws AbsentInformationException;
}
