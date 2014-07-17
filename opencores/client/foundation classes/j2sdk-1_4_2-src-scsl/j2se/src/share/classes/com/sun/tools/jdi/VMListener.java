/*
 * @(#)VMListener.java	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package com.sun.tools.jdi;

import com.sun.jdi.*;
import java.util.EventListener;

interface VMListener extends EventListener {
    boolean vmSuspended(VMAction action);
    boolean vmNotSuspended(VMAction action);
}


