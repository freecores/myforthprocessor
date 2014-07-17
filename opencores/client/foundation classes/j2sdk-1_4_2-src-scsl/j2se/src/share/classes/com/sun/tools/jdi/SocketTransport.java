/*
 * @(#)SocketTransport.java	1.17 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdi;

import com.sun.jdi.*;
import java.net.*;
import java.io.*;
import java.util.Map;
import java.util.HashMap;

class SocketTransport extends Object implements TransportService
{
    Map listenMap = new HashMap();

    SocketTransport() {
    }

    public String name() {
        return "dt_socket";
    }

    public ConnectionService attach(String address) throws IOException {
        if (address == null) {
            throw new NullPointerException("address is null");
        }

        int splitIndex = address.indexOf(':');
        if (splitIndex == -1) {
            throw new IllegalArgumentException("address string is malformed");
        }

        String hostname = address.substring(0, splitIndex);
        int portNumber;
        try {
            portNumber = Integer.decode(address.substring(splitIndex+1)).intValue();
        } catch (NumberFormatException e) {
            throw new IllegalArgumentException(
                "unable to parse port number in address");
        }

        Socket socket = new Socket(hostname, portNumber);
        return new SocketConnection(socket);
    }

    public boolean supportsMultipleConnections() {
        return true;
    }

    public String startListening(String address) throws IOException {
        if (address == null) {
            throw new NullPointerException("address is null");
        }

        int splitIndex = address.indexOf(':');
        String portString;
        if (splitIndex != -1) {
            // If a machine name is specified, make sure it's this one
            String name = address.substring(0, splitIndex);
            InetAddress inetAddress = InetAddress.getByName(name);
            if (!inetAddress.equals(InetAddress.getLocalHost())) {
                throw new IllegalArgumentException(
                    "Cannot listen on remote host: " + name);
            }
            portString = address.substring(splitIndex + 1);
        } else {
            // Implicit localhost
            portString = address;
        }

        int portNumber;
        try {
            portNumber = Integer.decode(portString).intValue();
        } catch (NumberFormatException e) {
            throw new IllegalArgumentException(
                "unable to parse port number in address");
        }
        return startListening(portNumber);
    }

    public String startListening() throws IOException {
        return startListening(0);
    }

    String startListening(int portNumber) throws IOException {
        ServerSocket socket = new ServerSocket(portNumber);
        String address = InetAddress.getLocalHost().getHostName() + ":" +
                         socket.getLocalPort();
        listenMap.put(address, socket);
        return address;
    }

    public void stopListening(String address) throws IOException {
        ServerSocket socket = (ServerSocket)listenMap.remove(address);
        if (socket == null) {
            throw new IllegalArgumentException(
                "Unknown listen address: " + address);
        }

        socket.close();
    }

    public ConnectionService accept(String address) throws IOException {
        ServerSocket server = (ServerSocket)listenMap.get(address);
        if (server == null) {
            throw new IllegalArgumentException(
                "Unknown listen address: " + address);
        }

        Socket socket = server.accept();
        return new SocketConnection(socket);
    }
    public String toString() {
       return name();
    }
}

