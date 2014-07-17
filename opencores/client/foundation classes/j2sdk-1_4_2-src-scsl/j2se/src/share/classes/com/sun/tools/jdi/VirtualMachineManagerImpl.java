/*
 * @(#)VirtualMachineManagerImpl.java	1.19 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdi;

import com.sun.jdi.*;
import com.sun.jdi.connect.*;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.util.List;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.ResourceBundle;
import java.io.IOException;

/* Public for use by com.sun.jdi.Bootstrap */
public class VirtualMachineManagerImpl implements VirtualMachineManagerService {
    private List connectors = new ArrayList();
    private LaunchingConnector defaultConnector = null;
    private List targets = new ArrayList();
    private List connectionListeners = new ArrayList();
    private final ThreadGroup mainGroupForJDI;
    private ResourceBundle messages = null;
    private int vmSequenceNumber = 0;
    private static final int majorVersion = 1;
    private static final int minorVersion = 4;

    //The Servicability Agent connectors.
    private static final String [] SAConnectorStrings = {
        "sun.jvm.hotspot.jdi.SACoreAttachingConnector",
        "sun.jvm.hotspot.jdi.SADebugServerAttachingConnector",
        "sun.jvm.hotspot.jdi.SAPIDAttachingConnector",
    };
 
    private void addSAConnectors() {
        /*
         * Locate and load the SA core file connectors, if they are
         * available on classpath.  Ignore if not available.
         */
        for (int i = 0; i < SAConnectorStrings.length; i++) {
            try {
                Class c = Class.forName(SAConnectorStrings[i]);
                try {
                    Class[] argumentTypes =
                        {com.sun.tools.jdi.VirtualMachineManagerService.class};
                    Constructor classConstructor =
                        c.getConstructor(argumentTypes); 
                    Object[] constructorArgs = { this };
                    addConnector((com.sun.jdi.connect.Connector)classConstructor.newInstance(constructorArgs));
                } catch (NoSuchMethodException nsm) {
                    System.out.println("NoSuchMethodException while loading: " +                                        SAConnectorStrings[i]);
                    nsm.printStackTrace();
                } catch (InstantiationException iex) {
                    iex.printStackTrace();
                } catch (InvocationTargetException ite) {
                    ite.printStackTrace();
                } catch (IllegalAccessException iae) {
                    iae.printStackTrace();
                }
            } catch (ClassNotFoundException cnf) {
                // Just continue.  This SA connector is not available.
            } catch (Throwable t) {
                t.printStackTrace();
            }
        }
    }

    /* Public for use by com.sun.jdi.Bootstrap */
    public VirtualMachineManagerImpl() {
        /*
         * Create and register the connectors provided as part of this
         * JDI implementation.
         */
        TransportService transport;
        boolean haveLaunchers = false;

	/* Create main ThreadGroup for all JDI threads and ThreadGroups
	 */
	mainGroupForJDI = new ThreadGroup("JDI main");

        // Shared memory
        try {
            transport = new SharedMemoryTransport();
            addConnector(new SunCommandLineLauncher(this, transport));
            addConnector(new RawCommandLineLauncher(this, transport));
            haveLaunchers = true;
            addConnector(new SharedMemoryAttachingConnector(this));
            addConnector(new SharedMemoryListeningConnector(this));
        } catch (UnsatisfiedLinkError e) {
            // Shared memory not supported on all platforms... just continue
        }

        // Sockets
        transport = new SocketTransport();
        if (!haveLaunchers) {
            addConnector(new SunCommandLineLauncher(this, transport));
            addConnector(new RawCommandLineLauncher(this, transport));
        }
        addConnector(new SocketAttachingConnector(this));
        addConnector(new SocketListeningConnector(this));

        addSAConnectors();

        // Set the default connector as the first launcher we were
        // able to create. 
        LaunchingConnector firstLauncher = 
                 (LaunchingConnector)launchingConnectors().get(0);
        setDefaultConnector(firstLauncher);

    }

    public LaunchingConnector defaultConnector() {
        return defaultConnector;
    }

    public void setDefaultConnector(LaunchingConnector connector) {
        defaultConnector = connector;
    }

    public List launchingConnectors() {
        List launchingConnectors = new ArrayList(connectors.size());
        Iterator iter = connectors.iterator();
        while (iter.hasNext()) {
            Object connector = iter.next();
            if (connector instanceof LaunchingConnector) {
                launchingConnectors.add(connector);
            }
        }
        return Collections.unmodifiableList(launchingConnectors);
    }

    public List attachingConnectors() {
        List attachingConnectors = new ArrayList(connectors.size());
        Iterator iter = connectors.iterator();
        while (iter.hasNext()) {
            Object connector = iter.next();
            if (connector instanceof AttachingConnector) {
                attachingConnectors.add(connector);
            }
        }
        return Collections.unmodifiableList(attachingConnectors);
    }

    public List listeningConnectors() {
        List listeningConnectors = new ArrayList(connectors.size());
        Iterator iter = connectors.iterator();
        while (iter.hasNext()) {
            Object connector = iter.next();
            if (connector instanceof ListeningConnector) {
                listeningConnectors.add(connector);
            }
        }
        return Collections.unmodifiableList(listeningConnectors);
    }

    public List allConnectors() {
        return Collections.unmodifiableList(connectors);
    }

    public List connectedVirtualMachines() {
        return Collections.unmodifiableList(targets);
    }

    public void addConnector(Connector connector) {
        connectors.add(connector);
    }

    public void removeConnector(Connector connector) {
        connectors.remove(connector);
    }

    private static final String handshakeString = "JDWP-Handshake";

    void handshake(ConnectionService connection) throws IOException {
        int length = handshakeString.length();

        for (int i = 0; i < length; i++) {
            connection.sendByte((byte)handshakeString.charAt(i));
        }

        for (int i = 0; i < length; i++) {
            byte b = connection.receiveByte();

            if (b != (byte)handshakeString.charAt(i)) {
                throw new IOException("Target failed to handshake.");
            }
        }

        return;
    }

    public synchronized VirtualMachine createVirtualMachine(
                                        ConnectionService connection,
                                        Process process) throws IOException {
        // Make sure we've connected to a JDWP implementor
        handshake(connection);

        VirtualMachine vm = new VirtualMachineImpl(this, connection, process,
                                                   ++vmSequenceNumber);
        targets.add(vm);
        return vm;
    }

    public VirtualMachine createVirtualMachine(ConnectionService connection) 
                                               throws IOException {
        return createVirtualMachine(connection, null);
    }

    public void addVirtualMachine(VirtualMachine vm) {
        targets.add(vm);
    }

    void disposeVirtualMachine(VirtualMachine vm) {
        targets.remove(vm);
    }

    public int majorInterfaceVersion() {
        return majorVersion;
    }

    public int minorInterfaceVersion() {
        return minorVersion;
    }

    ThreadGroup mainGroupForJDI() {
	return mainGroupForJDI;
    }

    String getString(String key) {
        if (messages == null) { 
            messages = ResourceBundle.getBundle("com.sun.tools.jdi.resources.jdi");
        }
        return messages.getString(key);
    }

}
