/*
 * @(#)SaslImpl.java	1.16 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.security.sasl.util; 

import com.sun.security.sasl.preview.*;
import java.io.*;
import java.util.Map;
import java.util.StringTokenizer;
import java.security.AccessController;
import java.security.PrivilegedAction;

import sun.misc.HexDumpEncoder;

/**
 * The base class used by client and server implementations of SASL
 * mechanisms to process properties passed in the props argument
 * and strings with the same format (e.g., used in digest-md5).
 *
 * Also contains utilities for doing int to network-byte-order
 * transformations.
 *
 * @author Rosanna Lee
 */
public abstract class SaslImpl {
    protected static final boolean debug = false;
    protected static final boolean verbose = false;

    /*
     * Code for dynamically settable debugging
     * Don't do this for now for backward compat with LDAP 1.2.3 and performance.

    // Property for turning on/off debugging
    private static final String DEBUG_PROPERTY = "com.sun.security.sasl.debug";
    
    protected static boolean debug = false;
    protected static boolean verbose = false;

    static {
	String str = (String)
	    AccessController.doPrivileged(
		new PrivilegedAction() {
		public Object run() {
		    try {
			return System.getProperty(DEBUG_PROPERTY);
		    } catch (SecurityException e) {
		    }
		    return null;
	        }
	    }
	);
	if (str != null) {
	    debug = true;
	    verbose = str.equals("verbose");
	}
    }
    */

    protected boolean completed = false;
    protected boolean privacy = false;
    protected boolean integrity = false;
    protected byte[] qop;	    // ordered list of qops
    protected byte allQop;          // a mask indicating which QOPs are requested
    protected byte[] strength;      // ordered list of cipher strengths
    protected int sendMaxBufSize = 0;     // specified by peer but can override
    protected int recvMaxBufSize = 65536; // optionally specified by client
    protected int rawSendSize;            // derived from sendMaxBufSize

    protected SaslImpl(Map props) throws SaslException {
	// Parse properties  to set desired context options
	if (props != null) {

	    // "auth", "auth-int", "auth-conf"
	    qop = parseQop((String)props.get(Sasl.QOP));
	    allQop = combineMasks(qop);

	    if (debug) {
		System.err.print("client protections: ");
		for (int i = 0; i < qop.length; i++) {
		    System.err.print("" + qop[i]);
		}
		System.err.println();
	    }

	    // "low", "medium", "high"
	    strength = parseStrength((String)props.get(Sasl.STRENGTH));
	    if (debug) {
		System.err.print("cipher strengths: ");
		for (int i = 0; i < strength.length; i++) {
		    System.err.print("" + strength[i]);
		}
		System.err.println();
	    }

	    // Max receive buffer size
	    String prop = (String)props.get(Sasl.MAX_BUFFER);
	    if (prop != null) {
		try {
		    recvMaxBufSize = Integer.parseInt(prop);
		} catch (NumberFormatException e) {
		    throw new SaslException(
		"Property must be string representation of integer: " + 
			Sasl.MAX_BUFFER);
		}
	    }

	    // Max send buffer size
	    prop = (String)props.get(MAX_SEND_BUF);
	    if (prop != null) {
		try {
		    sendMaxBufSize = Integer.parseInt(prop);
		} catch (NumberFormatException e) {
		    throw new SaslException(
		"Property must be string representation of integer: " + 
			MAX_SEND_BUF);
		}
	    }
	} else {
	    qop = DEFAULT_QOP;
	    strength = STRENGTH_MASKS;
	}
    }

    protected static byte combineMasks(byte[] in) {
	byte answer = 0;
	for (int i = 0; i < in.length; i++) {
	    answer |= in[i];
	}
	return answer;
    }

    protected static byte findPreferredMask(byte pref, byte[] in) {
	for (int i = 0; i < in.length; i++) {
	    if ((in[i]&pref) != 0) {
		return in[i];
	    }
	}
	return (byte)0;
    }

    protected byte[] parseQop(String qop) throws SaslException {
	return parseQop(qop, null, false);
    }

    protected byte[] parseQop(String qop, String[] saveTokens, boolean ignore) 
	throws SaslException {
	if (qop == null) {
	    return DEFAULT_QOP;   // default
	}

	return parseProp(Sasl.QOP, qop, QOP_TOKENS, QOP_MASKS, saveTokens, ignore);
    }

    protected byte[] parseStrength(String strength) throws SaslException {
	if (strength == null) {
	    return DEFAULT_STRENGTH;   // default
	}

	return parseProp(Sasl.STRENGTH, strength, STRENGTH_TOKENS, 
	    STRENGTH_MASKS, null, false);
    }

    protected byte[] parseProp(String propName, String propVal, 
	String[] vals, byte[] masks, String[] tokens, boolean ignore) 
	throws SaslException {

	StringTokenizer parser = new StringTokenizer(propVal, ", \t\n");
	String token;
	byte[] answer = new byte[vals.length];
	int i = 0;
	boolean found;

	while (parser.hasMoreTokens() && i < answer.length) {
	    token = parser.nextToken();
	    found = false;
	    for (int j = 0; !found && j < vals.length; j++) {
		if (token.equalsIgnoreCase(vals[j])) {
		    found = true;
		    answer[i++] = masks[j];
		    if (tokens != null) {
			tokens[j] = token;    // save what was parsed
		    }
		} 
	    }
	    if (!found && !ignore) {
		throw new SaslException(
		    "Invalid token in " + propName + ": " + propVal);
	    }
	}
	// Initialize rest of array with 0
	for (int j = i; j < answer.length; j++) {
	    answer[j] = 0;
	}
	return answer;
    }

    /**
     * Determines whether this mechanism has completed.
     *
     * @return true if has completed; false otherwise;
     */
    public boolean isComplete() {
	return completed;
    }

    /**
     * Retrieves the negotiated property.
     * @exception SaslException if this authentication exchange has not completed
     */
    public String getNegotiatedProperty(String propName) throws SaslException {
	if (!completed) {
	    throw new SaslException("Not completed");
	}

	if (propName.equals(Sasl.QOP)) {
	    if (privacy) {
		return "auth-conf";
	    } else if (integrity) {
		return "auth-int";
	    } else {
		return "auth";
	    }
	} else if (propName.equals(Sasl.MAX_BUFFER)) {
	    return Integer.toString(recvMaxBufSize);
	} else if (propName.equals(Sasl.RAW_SEND_SIZE)) {
	    return Integer.toString(rawSendSize);
	} else if (propName.equals(MAX_SEND_BUF)) {
	    return Integer.toString(sendMaxBufSize);
	} else {
	    return null;
	}
    }

    /**
     * Outputs a byte array and converts
     */
    public final static void traceOutput(String traceTag, byte[] output) {
	traceOutput(traceTag, output, 0, output.length);
    }

    public final static void traceOutput(String traceTag, byte[] output, 
	int offset, int len) {
	try {
	    if (debug) {
		System.err.println();
		System.err.println(traceTag + " (" + len + ")");

		if (!verbose) {
		    len = Math.min(16, len);
		}
	    
		new HexDumpEncoder().encodeBuffer(
		    new ByteArrayInputStream(output, offset, len), System.err);
	    }
	} catch (Exception e) {}
    }


    // ---------------- Constants  -----------------
    protected static final String MAX_SEND_BUF = "javax.security.sasl.sendmaxbuffer";

    // default 0 (no protection); 1 (integrity only)
    protected static final byte NO_PROTECTION = (byte)1;
    protected static final byte INTEGRITY_ONLY_PROTECTION = (byte)2;
    protected static final byte PRIVACY_PROTECTION = (byte)4;

    protected static final byte LOW_STRENGTH = (byte)1;
    protected static final byte MEDIUM_STRENGTH = (byte)2;
    protected static final byte HIGH_STRENGTH = (byte)4;

    private static final byte[] DEFAULT_QOP = new byte[]{NO_PROTECTION};
    private final static String[] QOP_TOKENS = {"auth-conf", 
				       "auth-int", 
				       "auth"};
    private final static byte[] QOP_MASKS = {PRIVACY_PROTECTION,
				     INTEGRITY_ONLY_PROTECTION,
				     NO_PROTECTION};

    private static final byte[] DEFAULT_STRENGTH = new byte[]{
	HIGH_STRENGTH, MEDIUM_STRENGTH, LOW_STRENGTH};
    private final static String[] STRENGTH_TOKENS = {"low", 
						     "medium", 
						     "high"};
    private final static byte[] STRENGTH_MASKS = {LOW_STRENGTH, 
						  MEDIUM_STRENGTH,
						  HIGH_STRENGTH};


    /**
     * Returns the integer represented by  4 bytes in network byte order.
     */ 
    public static int networkByteOrderToInt(byte[] buf, int start, int count) {
	if (count > 4) {
	    throw new IllegalArgumentException("Cannot handle more than 4 bytes");
	}

	int answer = 0;

        for (int i = 0; i < count; i++) {
	    answer <<= 8;
	    answer |= ((int)buf[start+i] & 0xff);
	}
	return answer;
    }

    /**
     * Encodes an integer into 4 bytes in network byte order in the buffer
     * supplied.
     */ 
    public static void intToNetworkByteOrder(int num, byte[] buf, int start, 
	int count) {
	if (count > 4) {
	    throw new IllegalArgumentException("Cannot handle more than 4 bytes");
	}

	for (int i = count-1; i >= 0; i--) {
	    buf[start+i] = (byte)(num & 0xff);
	    num >>>= 8;
	}
    }
}
