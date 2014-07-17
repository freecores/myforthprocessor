/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.net.ssl.internal.ssl;

import java.util.*;

/**
 * A list of ProtocolVersions. Also maintains the list of supported protocols.
 * Instances of this class are immutable. Some member variables are final
 * and can be accessed directly without method accessors.
 *
 * @version 1.2, 06/24/03
 */
final class ProtocolList {
    
    private static final ProtocolList SUPPORTED;
    
    private final Collection protocols;
    private String[] protocolNames;
    
    // the minimum and maximum ProtocolVersions in this list
    final ProtocolVersion min, max;
    
    // the format for the hello version to use
    final ProtocolVersion helloVersion;
    
    ProtocolList(String[] names) {
	if (names == null) {
	    throw new IllegalArgumentException("Protocols may not be null");
	}
	protocols = new ArrayList(3);
	for (int i = 0; i < names.length; i++ ) {
	    ProtocolVersion version = ProtocolVersion.valueOf(names[i]);
	    if (protocols.contains(version) == false) {
		protocols.add(version);
	    }
	}
	if ((protocols.size() == 1) 
		&& protocols.contains(ProtocolVersion.SSL20Hello)) {
	    throw new IllegalArgumentException("SSLv2Hello" +
		  "cannot be enabled unless TLSv1 or SSLv3 is also enabled");
	}
	min = contains(ProtocolVersion.SSL30) ? ProtocolVersion.SSL30
					      : ProtocolVersion.TLS10;
	max = contains(ProtocolVersion.TLS10) ? ProtocolVersion.TLS10 
					      : ProtocolVersion.SSL30;
	if (protocols.contains(ProtocolVersion.SSL20Hello)) { 
	    helloVersion = ProtocolVersion.SSL20Hello;
	} else {
	    helloVersion = min;
	}
    }
    
    /**
     * Return whether this list contains the specified protocol version.
     * SSLv2Hello is not a real protocol version we support, we always
     * return false for it.
     */
    boolean contains(ProtocolVersion protocolVersion) {
	if (protocolVersion == ProtocolVersion.SSL20Hello) {
	    return false;
	}
	return protocols.contains(protocolVersion);
    }

    /**
     * Return an array with the names of the ProtocolVersions in this list.
     */
    synchronized String[] toStringArray() {
	if (protocolNames == null) {
	    protocolNames = new String[protocols.size()];
	    int i = 0;
	    for (Iterator t = protocols.iterator(); t.hasNext(); i++) {
		ProtocolVersion version = (ProtocolVersion)t.next();
		protocolNames[i] = version.name;
	    }
	}
	return (String[])protocolNames.clone();
    }
    
    public String toString() {
	return protocols.toString();
    }
    
    /**
     * Return the list of default enabled protocols. Currently, this
     * is identical to the supported protocols.
     */
    static ProtocolList getDefault() {
	return SUPPORTED;
    }
    
    /**
     * Return the list of supported protocols.
     */
    static ProtocolList getSupported() {
	return SUPPORTED;
    }
    
    static {
	SUPPORTED = new ProtocolList(new String[] {
	    ProtocolVersion.SSL20Hello.name,
	    ProtocolVersion.SSL30.name,
	    ProtocolVersion.TLS10.name,
	});
    }

}
