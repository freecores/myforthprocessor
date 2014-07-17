/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.net.ssl.internal.ssl;

/**
 * Type safe enum for an SSL/TLS protocol version. Instances are obtained
 * using the static factory methods or by referencing the static members
 * in this class. Member variables are final and can be accessed without
 * accessor methods.
 *
 * There is only ever one instance per supported protocol version, this
 * means == can be used for comparision instead of equals() if desired.
 *
 * Checks for a particular version number should generally take this form:
 *
 * if (protocolVersion.v >= ProtocolVersion.TLS10) {
 *   // TLS 1.0 code goes here
 * } else {
 *   // SSL 3.0 code here
 * }
 *
 * @version 1.3, 06/24/03
 */
final class ProtocolVersion {
    
    // dummy protocol version value for invalid SSLSession
    final static ProtocolVersion NONE = new ProtocolVersion(-1, "NONE");
    
    // If enabled, send/ accept SSLv2 hello messages
    final static ProtocolVersion SSL20Hello = new ProtocolVersion(0x0002, 
    								"SSLv2Hello");
    
    // SSL 3.0
    final static ProtocolVersion SSL30 = new ProtocolVersion(0x0300, "SSLv3");
    
    // TLS 1.0
    final static ProtocolVersion TLS10 = new ProtocolVersion(0x0301, "TLSv1");
    
    // minimum version we implement (SSL 3.0)
    final static ProtocolVersion MIN = SSL30;
    
    // maximum version we implement (TLS 1.0)
    final static ProtocolVersion MAX = TLS10;
    
    // ProtocolVersion to use by default (TLS 1.0)
    final static ProtocolVersion DEFAULT = TLS10;
    
    // Default version for hello messages (SSLv2Hello)
    final static ProtocolVersion DEFAULT_HELLO = SSL20Hello;
    
    // version in 16 bit MSB format as it appears in records and
    // messages, i.e. 0x0301 for TLS 1.0
    final int v;
    
    // major and minor version
    final byte major, minor;
    
    // name used in JSSE (e.g. TLSv1 for TLS 1.0)
    final String name;
    
    // private
    private ProtocolVersion(int v, String name) {
	this.v = v;
	this.name = name;
	major = (byte)(v >>> 8);
	minor = (byte)(v & 0xff);
    }
    
    // private
    private static ProtocolVersion valueOf(int v) {
	if (v == SSL30.v) {
	    return SSL30;
	} else if (v == TLS10.v) {
	    return TLS10;
	} else if (v == SSL20Hello.v) {
	    return SSL20Hello;
	} else {
	    int major = (v >>> 8) & 0xff;
	    int minor = v & 0xff;
	    return new ProtocolVersion(v, "Unknown " + major + "." + minor);
	}
    }
    
    /**
     * Return a ProtocolVersion with the specified major and minor version
     * numbers. Never throws exceptions.
     */
    static ProtocolVersion valueOf(int major, int minor) {
	major &= 0xff;
	minor &= 0xff;
	int v = (major << 8) | minor;
	return valueOf(v);
    }
    
    /**
     * Return a ProtocolVersion for the given name.
     *
     * @exception IllegalArgumentException if name is null or does not
     * identify a supported protocol
     */
    static ProtocolVersion valueOf(String name) {
	if (name == null) {
	    throw new IllegalArgumentException("Protocol cannot be null");
	} else if (name.equals(SSL30.name)) {
	    return SSL30;
	} else if (name.equals(TLS10.name)) {
	    return TLS10;
	} else if (name.equals(SSL20Hello.name)) {
	    return SSL20Hello;
	} else {
	    throw new IllegalArgumentException(name);
	}
    }
    
    public String toString() {
	return name;
    }
    
}
