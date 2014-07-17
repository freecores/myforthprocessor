/*
 * @(#)SocketAttachingConnector.java	1.12 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdi;

import com.sun.jdi.connect.*;
import com.sun.jdi.VirtualMachine;
import java.util.Map;
import java.util.HashMap;
import java.io.IOException;
import java.net.InetAddress;
import java.net.UnknownHostException;

class SocketAttachingConnector extends ConnectorImpl 
                               implements AttachingConnector {

    static final String ARG_PORT = "port";
    static final String ARG_HOST = "hostname";

    TransportService transport;

    SocketAttachingConnector(VirtualMachineManagerService manager) {

        super(manager);

        String defaultHostName;
        try {
            defaultHostName = InetAddress.getLocalHost().getHostName();
        } catch (UnknownHostException e) {
            defaultHostName = "";
        }

        addStringArgument(
                ARG_HOST,
                getString("socket_attaching.host.label"),
                getString("socket_attaching.host"),
                defaultHostName,
                false);

        addIntegerArgument(
                ARG_PORT,
                getString("socket_attaching.port.label"),
                getString("socket_attaching.port"),
                "",
                true,
                0, Integer.MAX_VALUE);

        transport = new SocketTransport();
    }

    public VirtualMachine attach(Map arguments) throws IOException,
                                      IllegalConnectorArgumentsException {
        String portString = argument(ARG_PORT, arguments).value();
        String hostString = argument(ARG_HOST, arguments).value();

        if (hostString.length() == 0) {
            hostString = "localhost";
        }
        hostString = hostString + ":";

        ConnectionService connection = transport.attach(hostString + portString);
        return manager().createVirtualMachine(connection);
    }

    public String name() {
        return "com.sun.jdi.SocketAttach";
    }

    public String description() {
        return getString("socket_attaching.description");
    }

    public Transport transport() {
        return transport;
    }
}

