/*
 * @(#)SharedMemoryConnection.java	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdi;

import java.io.IOException;
import java.io.EOFException;

class SharedMemoryConnection extends Object implements ConnectionService {
    private long id;
    private Object receiveLock = new Object();
    private Object sendLock = new Object();
    private volatile boolean closed = false;

    SharedMemoryConnection(long id) throws IOException {
        this.id = id;
    }

    private native void close0(long id);
    private native byte receiveByte0(long id) throws IOException;
    private native void sendByte0(long id, byte b) throws IOException;
    private native void receivePacket0(long id, Packet packet) throws IOException;
    private native void sendPacket0(long id, Packet packet) throws IOException;

    public void close() {
        if (!closed) {
            closed = true;
            close0(id);
        }
    }
    
    public byte receiveByte() throws IOException {
        synchronized(receiveLock) {
            if (closed) {
                throw new IOException("Connection closed");
            }
            return receiveByte0(id);
        }
    }
    
    public void sendByte(byte b) throws IOException {
        synchronized(sendLock) {
            if (closed) {
                throw new IOException("Connection closed");
            }
            sendByte0(id, b);
        }
    }

    public Packet receivePacket() throws IOException {
        synchronized(receiveLock) {
            if (closed) {
                throw new IOException("Connection closed");
            }
            Packet packet = new Packet();
            receivePacket0(id, packet);
            return packet;
        }
    }
    
    public void sendPacket(Packet packet) throws IOException {
        synchronized(sendLock) {
            if (closed) {
                throw new IOException("Connection closed");
            }
            sendPacket0(id, packet);
        }
    }
}

