/*
 * @(#)SADebugServerAttachingConnector.java	1.3 03/01/23
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

public class SADebugServerAttachingConnector extends ConnectorImpl implements AttachingConnector {

    static final String ARG_DEBUG_SERVER_NAME = "debugServerName";

    public SADebugServerAttachingConnector(com.sun.tools.jdi.VirtualMachineManagerService manager) {
         super(manager);
         // fixme jjh  create resources for the these strings,
        addStringArgument(
                ARG_DEBUG_SERVER_NAME,
                "Debug Server",                      //getString("sa.debugServer.label"),
                "Name of a remote SA Debug Server",  //getString("sa.debugServer.description");
                "",
                true);
    }

    public VirtualMachine attach(Map arguments) throws IOException,
                                      IllegalConnectorArgumentsException {
        String debugServerName = argument(ARG_DEBUG_SERVER_NAME, arguments).value();
        VirtualMachine myVM;
        try {
            myVM = VirtualMachineImpl.createVirtualMachineForServer(
                                                             manager(), 
                                                             debugServerName, 0);
        } catch (Exception ee) {
            System.out.println("VirtualMachineImpl() got an exception:");
            ee.printStackTrace();
            System.out.println("debug server name = " + debugServerName);
            return null;
        }
        return myVM;
    }

    public String name() {
        return "sun.jvm.hotspot.jdi.SADebugServerAttachingConnector";
    }

    public String description() {
        return getString("This connector allows you to attach to a Java Process via a debug server with the Serviceability Agent");
    }

    public Transport transport() {
        return null;
    }
}

