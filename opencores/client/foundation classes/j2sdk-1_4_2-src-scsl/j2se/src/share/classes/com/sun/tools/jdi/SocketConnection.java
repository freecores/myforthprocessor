/*
 * @(#)SocketConnection.java	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdi;

import java.net.*;
import java.io.*;

class SocketConnection extends Object implements ConnectionService {
    Socket socket;
    BufferedOutputStream socketOutput;
    BufferedInputStream socketInput;
    Object receiveLock = new Object();
    Object sendLock = new Object();

    SocketConnection(Socket socket) throws IOException {
        this.socket = socket;
        socket.setTcpNoDelay(true);
        socketInput = new BufferedInputStream(socket.getInputStream());
        socketOutput = new BufferedOutputStream(socket.getOutputStream());
    }

    public void close() {
        try {
            socketOutput.flush();
            socketOutput.close();
            socketInput.close();
            socket.close();
        } catch (Exception e) {
            ;
        }
    }

    public byte receiveByte() throws IOException {
        synchronized (receiveLock) {
            int b = socketInput.read();
            return (byte)b;
        }
    }

    public void sendByte(byte b) throws IOException {
        synchronized (sendLock) {
            socketOutput.write(b);
            socketOutput.flush();
        }
    }

    public Packet receivePacket() throws IOException {
        synchronized (receiveLock) {
            Packet p = new Packet();
            int b1,b2,b3,b4;
    
            // length
            b1 = socketInput.read();
            b2 = socketInput.read();
            b3 = socketInput.read();
            b4 = socketInput.read();
    
            if (b1<0 || b2<0 || b3<0 || b4<0)
                throw new EOFException();
    
            int length = ((b1 << 24) + (b2 << 16) + (b3 << 8) + (b4 << 0));
    
            // id
            b1 = socketInput.read();
            b2 = socketInput.read();
            b3 = socketInput.read();
            b4 = socketInput.read();
    
            if (b1<0 || b2<0 || b3<0 || b4<0)
                throw new EOFException();
    
            p.id = ((b1 << 24) + (b2 << 16) + (b3 << 8) + (b4 << 0));
    
            p.flags = (short)socketInput.read();
            if (p.flags < 0) {
                throw new EOFException();
            }
            if ((p.flags & Packet.Reply) == 0) {
                p.cmdSet = (short)socketInput.read();
                p.cmd = (short)socketInput.read();
                if (p.cmdSet < 0 || p.cmd < 0){
                    throw new EOFException();
                }
            } else {
                b1 = socketInput.read();
                b2 = socketInput.read();
                if (b1 < 0 || b2 < 0){
                    throw new EOFException();
                }
                p.errorCode = (short)((b1 << 8) + (b2 << 0));
            }
    
            length -= 11; // subtract the header
    
            if (length < 0) {
                // WHoa! this shouldn't be happening!
                System.err.println("length is " + length);
                System.err.println("Read is " + socketInput.read());
            }
            p.data = new byte[length];
    
            int n = 0;
            while (n < p.data.length) {
                int count = socketInput.read(p.data, n, p.data.length - n);
                if (count < 0) {
                    throw new EOFException();
                }
                n += count;
            }
    
            return p;
        }
    }

    public void sendPacket(Packet p) throws IOException {
        synchronized (sendLock) {
            int length = p.data.length + 11;
    
            // Length
            socketOutput.write((length >>> 24) & 0xFF);
            socketOutput.write((length >>> 16) & 0xFF);
            socketOutput.write((length >>>  8) & 0xFF);
            socketOutput.write((length >>>  0) & 0xFF);
    
            // id
            socketOutput.write((p.id >>> 24) & 0xFF);
            socketOutput.write((p.id >>> 16) & 0xFF);
            socketOutput.write((p.id >>>  8) & 0xFF);
            socketOutput.write((p.id >>>  0) & 0xFF);
    
            socketOutput.write(p.flags);
    
            if ((p.flags & Packet.Reply) == 0) {
                socketOutput.write(p.cmdSet);
                socketOutput.write(p.cmd);
            } else {
                socketOutput.write((p.errorCode >>>  8) & 0xFF);
                socketOutput.write((p.errorCode >>>  0) & 0xFF);
            }
            socketOutput.write(p.data);
    
            socketOutput.flush();
        }
    }
}

