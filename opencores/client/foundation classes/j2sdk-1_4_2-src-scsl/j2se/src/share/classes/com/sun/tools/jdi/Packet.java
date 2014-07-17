/*
 * @(#)Packet.java	1.29 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdi;

import com.sun.jdi.*;

public class Packet extends Object {
    public final static short NoFlags = 0x0;
    public final static short Reply = 0x80;
    public final static short ReplyNoError = 0x0;

    static int uID = 1;
    final static byte[] nullData = new byte[0];

    // Note! flags, cmdSet, and cmd are all byte values.
    // We represent them as shorts to make them easier
    // to work with. 
    int id;
    short flags;
    short cmdSet;
    short cmd;
    short errorCode;
    byte[] data;
    volatile boolean replied = false;

    Packet()
    {
        id = uniqID();
        flags = NoFlags;
        data = nullData;
    }

    static synchronized private int uniqID()
    {
        /*
         * JDWP spec does not require this id to be sequential and 
         * increasing, but our implementation does. See 
         * VirtualMachine.notifySuspend, for example.
         */
        return uID++;
    }
}
