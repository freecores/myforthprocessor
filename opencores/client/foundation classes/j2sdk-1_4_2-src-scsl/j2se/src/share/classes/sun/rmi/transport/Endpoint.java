/*
 * @(#)Endpoint.java	1.12 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.rmi.transport;

import java.rmi.RemoteException;
import java.rmi.Remote;
import java.rmi.server.ObjID;
import java.rmi.server.RemoteServer;

public interface Endpoint {
    /**
     * Return a channel that generates connections to the remote
     * endpoint.
     */
    Channel getChannel();

    /**
     * Export the object so that it can accept incoming calls at
     * the endpoint.
     */
    void exportObject(Target target)
	throws RemoteException;

    /**
     * Returns transport for remote endpoint.
     */
    Transport getOutboundTransport();
}
