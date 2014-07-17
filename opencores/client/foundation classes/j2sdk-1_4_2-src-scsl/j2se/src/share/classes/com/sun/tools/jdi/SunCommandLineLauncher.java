/*
 * @(#)SunCommandLineLauncher.java	1.21 03/01/23
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
import java.io.File;

class SunCommandLineLauncher extends AbstractLauncher implements LaunchingConnector {

    static private final String ARG_HOME = "home";
    static private final String ARG_OPTIONS = "options";
    static private final String ARG_MAIN = "main";
    static private final String ARG_INIT_SUSPEND = "suspend";
    static private final String ARG_QUOTE = "quote";
    static private final String ARG_VM_EXEC = "vmexec";

    SunCommandLineLauncher(VirtualMachineManagerService manager, 
                           TransportService transport) {
        super(manager, transport);

        addStringArgument(
                ARG_HOME,
                getString("sun.home.label"),
                getString("sun.home"),
                System.getProperty("java.home"),
                false);
        addStringArgument(
                ARG_OPTIONS,
                getString("sun.options.label"),
                getString("sun.options"),
                "",
                false);
        addStringArgument(
                ARG_MAIN,
                getString("sun.main.label"),
                getString("sun.main"),
                "",
                true);

        addBooleanArgument(
                ARG_INIT_SUSPEND,
                getString("sun.init_suspend.label"),
                getString("sun.init_suspend"),
                true,
                false);

        addStringArgument(
                ARG_QUOTE,
                getString("sun.quote.label"),
                getString("sun.quote"),
                "\"",
                true);
        addStringArgument(
                ARG_VM_EXEC,
                getString("sun.vm_exec.label"),
                getString("sun.vm_exec"),
                "java",
                true);
    }

    static boolean hasWhitespace(String string) {
        int length = string.length();
        for (int i = 0; i < length; i++) {
            if (Character.isWhitespace(string.charAt(i))) {
                return true;
            }
        }
        return false;
    }

    public VirtualMachine launch(Map arguments) 
                                 throws IOException, 
                                        IllegalConnectorArgumentsException,
                                        VMStartException {
        VirtualMachine vm;
        String home = argument(ARG_HOME, arguments).value();
        String options = argument(ARG_OPTIONS, arguments).value();
        String mainClassAndArgs = argument(ARG_MAIN, arguments).value();
        boolean wait = ((BooleanArgumentImpl)argument(ARG_INIT_SUSPEND, 
                                                  arguments)).booleanValue();
        String quote = argument(ARG_QUOTE, arguments).value();
        String exe = argument(ARG_VM_EXEC, arguments).value();
        String exePath = null;

        if (quote.length() > 1) {
            throw new IllegalConnectorArgumentsException("Invalid length", 
                                                         ARG_QUOTE);
        }

        if ((options.indexOf("-Djava.compiler=") != -1) &&
            (options.toLowerCase().indexOf("-djava.compiler=none") == -1)) {
            throw new IllegalConnectorArgumentsException("Cannot debug with a JIT compiler", 
                                                         ARG_OPTIONS);
        }

        String address = transportService().startListening();

        try {
            if (home.length() > 0) {
                /*
                 * A wrinkle in the environment:
                 * 64-bit executables are stored under $JAVA_HOME/bin/sparcv9
                 * 32-bit executables are stored under $JAVA_HOME/bin
                 */
                if ("sparcv9".equals(System.getProperty("os.arch"))) {
                    exePath = home + File.separator + "bin" + File.separator +
                        System.getProperty("os.arch") + File.separator +
                        exe;
                } else {
                    exePath = home + File.separator + "bin" + File.separator + exe;
                }
            } else {
                exePath = exe;
            }
            // Quote only if necessary in case the quote arg value is bogus 
            if (hasWhitespace(exe)) {
                exePath = quote + exePath + quote;
            }

            String xrun = "transport=" + transport().name() + 
                          ",address=" + address +
                          ",suspend=" + (wait? 'y' : 'n');
            // Quote only if necessary in case the quote arg value is bogus 
            if (hasWhitespace(xrun)) {
                xrun = quote + xrun + quote;
            }
                          
            String command = exePath + ' ' + 
                             options + ' ' +
                             "-Xdebug -Xnoagent -Djava.compiler=NONE " +
                             "-Xrunjdwp:" + xrun + ' ' +
                             mainClassAndArgs;

            // System.err.println("Command: \"" + command + '"');
            vm = launch(tokenizeCommand(command, quote.charAt(0)), address);
        } finally {
            transportService().stopListening(address);
        }

        return vm;
    }

    public String name() {
        return "com.sun.jdi.CommandLineLaunch";
    }

    public String description() {
        return getString("sun.description");

    }
}

