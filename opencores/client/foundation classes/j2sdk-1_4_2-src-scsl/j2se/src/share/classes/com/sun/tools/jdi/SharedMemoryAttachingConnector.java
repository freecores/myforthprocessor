/*
 * @(#)SharedMemoryAttachingConnector.java	1.11 03/01/23
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

class SharedMemoryAttachingConnector extends ConnectorImpl 
                               implements AttachingConnector {

    static final String ARG_NAME = "name";

    TransportService transport;

    SharedMemoryAttachingConnector(VirtualMachineManagerService manager) {

        super(manager);

        addStringArgument(
                ARG_NAME,
                getString("memory_attaching.name.label"),
                getString("memory_attaching.name"),
                "",
                true);

        transport = new SharedMemoryTransport();
    }

    public VirtualMachine attach(Map arguments) throws IOException,
                                      IllegalConnectorArgumentsException {
        String address = argument(ARG_NAME, arguments).value();

        ConnectionService connection = transport.attach(address);
        return manager().createVirtualMachine(connection);
    }

    public String name() {
        return "com.sun.jdi.SharedMemoryAttach";
    }

    public String description() {
        return getString("memory_attaching.description");
    }

    public Transport transport() {
        return transport;
    }
}

