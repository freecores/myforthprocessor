/*
 * @(#)SACoreAttachingConnector.java	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All Rights Reserved.
 * 
 * This software is the proprietary information of Sun Microsystems, Inc.  
 * Use is subject to license terms.
 * 
 */

package sun.jvm.hotspot.jdi;

import com.sun.jdi.connect.*;
import com.sun.jdi.VirtualMachine;
import java.util.Map;
import java.util.HashMap;
import java.io.IOException;
import java.net.InetAddress;
import java.net.UnknownHostException;

public class SACoreAttachingConnector extends ConnectorImpl implements AttachingConnector {

    static final String ARG_COREFILE = "core";
    static final String ARG_JAVA_EXECUTABLE = "javaExecutable";

    public SACoreAttachingConnector(com.sun.tools.jdi.VirtualMachineManagerService manager) {
         super(manager);
         //fixme jjh  Must create resources for these strings
        addStringArgument(
                ARG_JAVA_EXECUTABLE,
                "Java Executable",              //getString("sa.javaExecutable.label"),
                "Pathname of Java Executable",  //getString("sa.javaExecutable.description");
                "",
                true);

        addStringArgument(
                ARG_COREFILE,
                "Corefile",                                    // getString("sa.CoreFile.label"),
                "Pathname of a corefile from a Java Process",  //getString("sa.CoreFile.description"),
                "core",
                false);
    }

    public VirtualMachine attach(Map arguments) throws IOException,
                                      IllegalConnectorArgumentsException {
        String javaExec = argument(ARG_JAVA_EXECUTABLE, arguments).value();
        String corefile = argument(ARG_COREFILE, arguments).value();
        VirtualMachine myVM;
        try {
            myVM = VirtualMachineImpl.createVirtualMachineForCorefile(
                                                             manager(),
                                                             javaExec, 
                                                             corefile, 0);
        } catch (Exception ee) {
            System.out.println("VirtualMachineImpl() got an exception:");
            ee.printStackTrace();
            System.out.println("coreFile = " + corefile + ", javaExec = " + javaExec);
            return null;
        }
        return myVM;
    }

    public String name() {
        return "sun.jvm.hotspot.jdi.SACoreAttachingConnector";
    }

    public String description() {
        return getString("This connector allows you to attach to a core file using the Serviceability Agent");
    }

    public Transport transport() {
        return null;
    }
}

