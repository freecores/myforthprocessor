/*
 * @(#)CommandSender.java	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package com.sun.tools.jdi;

import com.sun.jdi.*;
import java.util.EventListener;

interface CommandSender {
    PacketStream send();
}


