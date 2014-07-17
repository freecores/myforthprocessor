/*
 * @(#)SAPIDAttachingConnector.java	1.3 03/01/23
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

public class SAPIDAttachingConnector extends ConnectorImpl implements AttachingConnector {

    static final String ARG_PID = "pid";

    public SAPIDAttachingConnector(com.sun.tools.jdi.VirtualMachineManagerService manager) {
         super(manager);
         // fixme jjh:  create resources for the these strings,
        addStringArgument(
                ARG_PID,
                "PID",                     //getString("sa.pid.label"),
                "PID of a Java process",   //getString("sa.pid.description");
                "",
                true);
    }

    public VirtualMachine attach(Map arguments) throws IOException,
                                      IllegalConnectorArgumentsException {
        int pid = Integer.parseInt(argument(ARG_PID, arguments).value());
        VirtualMachine myVM;
        try {
            myVM = VirtualMachineImpl.createVirtualMachineForPID(
                                                                manager(),
                                                                pid, 0);
        } catch (Exception ee) {
            System.out.println("VirtualMachineImpl() got an exception:");
            ee.printStackTrace();
            System.out.println("pid = " + pid);
            return null;
        }
        return myVM;
    }

    public String name() {
        return "sun.jvm.hotspot.jdi.SAPIDAttachingConnector";
    }

    public String description() {
        return getString("This connector allows you to attach to a Java process using the Serviceability Agent");
    }

    public Transport transport() {
        return null;
    }
}

