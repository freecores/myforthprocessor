/*
 * @(#)ConnectionResetException.java	1.2 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.net;

import java.net.SocketException;

/**
 * Thrown to indicate a connection reset.
 *
 * @since   1.4
 */
public
class ConnectionResetException extends SocketException {

    public ConnectionResetException(String msg) {
        super(msg);
    }

    public ConnectionResetException() {
    }
}

