/*
 * @(#)SharedMemoryListeningConnector.java	1.11 03/01/23
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
import java.util.ArrayList;

class SharedMemoryListeningConnector extends ConnectorImpl 
                               implements ListeningConnector {

    static final String ARG_NAME = "name";

    Map listenMap;
    TransportService transport;

    SharedMemoryListeningConnector(VirtualMachineManagerService manager) {

        super(manager);

        addStringArgument(
                ARG_NAME,
                getString("memory_listening.name.label"),
                getString("memory_listening.name"),
                "",
                false);

        transport = new SharedMemoryTransport();

        listenMap = new HashMap(10);
    }

    public boolean supportsMultipleConnections() {
        return true;
    }

    public VirtualMachine accept(Map arguments) throws IOException,
                                     IllegalConnectorArgumentsException {
        ConnectionService connection;

        String address = (String)listenMap.get(arguments);
        if (address == null) {
            // It makes no sense to do a one-shot accept at an unspecified
            // address. So in this case, the port is required.
            address = argument(ARG_NAME, arguments).value();

            transport.startListening(address);
            connection = transport.accept(address);
            transport.stopListening(address);
        } else {
            connection = transport.accept(address);
        }
        return manager().createVirtualMachine(connection);
    }

    public String startListening(Map arguments) throws IOException,
                                     IllegalConnectorArgumentsException {
        if (listenMap.get(arguments) != null) {
            throw new IllegalConnectorArgumentsException("Already listening",
                                                new ArrayList(arguments.keySet()));
        }

        String address = argument(ARG_NAME, arguments).value();
        if (address.length() == 0) {
            address = transport.startListening();
        } else {
            address = transport.startListening(address);
        }

        listenMap.put(arguments, address);
        return address;
    }

    public void stopListening(Map arguments) throws IOException,
                                     IllegalConnectorArgumentsException {
        String address = (String)listenMap.remove(arguments);
        if (address == null) {
            throw new IllegalConnectorArgumentsException("Not listening",
                                               new ArrayList(arguments.keySet()));
        }

        transport.stopListening(address);
    }

    public String name() {
        return "com.sun.jdi.SharedMemoryListen";
    }

    public String description() {
        return getString("memory_listening.description");
    }

    public Transport transport() {
        return transport;
    }
}

