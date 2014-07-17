/*
 * @(#)TelnetProtocolException.java	1.17 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.net;

import java.io.*;

/**
 * An unexpected result was received by the client when talking to the
 * telnet server.
 * 
 * @version     1.17,01/23/03
 * @author      Jonathan Payne 
 */

public class TelnetProtocolException extends IOException {
    public TelnetProtocolException(String s) {
	super(s);
    }
}
