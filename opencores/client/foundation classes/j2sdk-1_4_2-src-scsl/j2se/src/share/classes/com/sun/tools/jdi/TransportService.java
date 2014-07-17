/*
 * @(#)TransportService.java	1.13 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdi;

import com.sun.jdi.connect.Transport;
import java.io.IOException;

/**
 * Transport SPI
 */
public interface TransportService extends Transport {
    ConnectionService attach(String address) throws IOException;

    boolean supportsMultipleConnections();
    String startListening(String address) throws IOException;
    String startListening() throws IOException;
    void stopListening(String address) throws IOException;
    ConnectionService accept(String address) throws IOException;
}

