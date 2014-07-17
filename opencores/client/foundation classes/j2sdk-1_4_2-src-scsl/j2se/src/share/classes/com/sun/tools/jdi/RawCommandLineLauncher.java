/*
 * @(#)RawCommandLineLauncher.java	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdi;

import com.sun.jdi.connect.*;
import com.sun.jdi.VirtualMachine;
import java.util.Map;
import java.io.IOException;

class RawCommandLineLauncher extends AbstractLauncher implements LaunchingConnector {

    static private final String ARG_COMMAND = "command";
    static private final String ARG_ADDRESS = "address";
    static private final String ARG_QUOTE   = "quote";

    RawCommandLineLauncher(VirtualMachineManagerService manager, 
                           TransportService transport) {
        super(manager, transport);

        addStringArgument(
                ARG_COMMAND,
                getString("raw.command.label"),
                getString("raw.command"),
                "",
                true);
        addStringArgument(
                ARG_ADDRESS,
                getString("raw.address.label"),
                getString("raw.address"),
                "",
                true);
        addStringArgument(
                ARG_QUOTE,
                getString("raw.quote.label"),
                getString("raw.quote"),
                "\"",
                true);
    }


    public VirtualMachine launch(Map arguments) 
                                 throws IOException, 
                                        IllegalConnectorArgumentsException,
                                        VMStartException {
        String command = argument(ARG_COMMAND, arguments).value();
        String address = argument(ARG_ADDRESS, arguments).value();
        String quote = argument(ARG_QUOTE, arguments).value();

        if (quote.length() > 1) {
            throw new IllegalConnectorArgumentsException("Invalid length", 
                                                         ARG_QUOTE);
        }

        address = transportService().startListening(address);

        try {
            return launch(tokenizeCommand(command, quote.charAt(0)), 
                          address);
        } finally {
            transportService().stopListening(address);
        }
    }

    public String name() {
        return "com.sun.jdi.RawCommandLineLaunch";
    }

    public String description() {
        return getString("raw.description");
    }
}

