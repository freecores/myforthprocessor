/*
 * @(#)SharedMemoryTransport.java	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdi;

import java.io.IOException;
import java.util.Map;
import java.util.HashMap;

class SharedMemoryTransport extends Object implements TransportService
{
    private static final String NAME = "dt_shmem";
    private Map listenMap = new HashMap();    /* keyed by address */

    SharedMemoryTransport() {
        System.loadLibrary(NAME);
        initialize();
    }

    public String name() {
        return NAME;
    }

    public boolean supportsMultipleConnections() {
        return true;
    }

    private native void initialize();
    private native long startListening0(String address) throws IOException;
    private native long attach0(String address) throws IOException;
    private native void stopListening0(long id) throws IOException;
    private native long accept0(long id) throws IOException;
    private native String name(long id) throws IOException;

    public ConnectionService attach(String address) throws IOException {
        if (address == null) {
            throw new NullPointerException("address is null");
        }
        long id = attach0(address);
        return new SharedMemoryConnection(id);
    }

    public String startListening(String address) throws IOException {
        long id = startListening0(address);
        address = name(id);
        listenMap.put(address, new Long(id));
        return address;
    }

    public String startListening() throws IOException {
        return startListening(null);
    }

    public void stopListening(String address) throws IOException {
        Long idObject = (Long)listenMap.remove(address);
        if (idObject == null) {
            throw new IllegalArgumentException("Invalid address");
        }
        stopListening0(idObject.longValue());
    }

    public ConnectionService accept(String address) throws IOException {
        Long idObject = (Long)listenMap.get(address);
        if (idObject == null) {
            throw new IllegalArgumentException("Invalid address");
        }
        long transportId = idObject.longValue();
        long connectId = accept0(transportId);
        return new SharedMemoryConnection(connectId);
    }
}

