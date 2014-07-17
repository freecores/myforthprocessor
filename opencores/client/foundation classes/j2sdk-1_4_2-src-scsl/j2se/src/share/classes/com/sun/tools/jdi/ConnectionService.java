/*
 * @(#)ConnectionService.java	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdi;

import java.io.IOException;

/**
 * Connection SPI
 */
public interface ConnectionService {
    void close();
    byte receiveByte() throws IOException;
    void sendByte(byte b) throws IOException;

    // TO DO: Hacks for now; we will eventually have PacketService interfaces
    com.sun.tools.jdi.Packet receivePacket() throws IOException;
    void sendPacket(com.sun.tools.jdi.Packet p) throws IOException;
}

