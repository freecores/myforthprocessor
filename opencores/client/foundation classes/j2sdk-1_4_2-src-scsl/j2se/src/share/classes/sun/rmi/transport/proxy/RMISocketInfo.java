/*
 * @(#)RMISocketInfo.java	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.rmi.transport.proxy;

/**
 * RMISocketInfo is an interface that extensions of the java.net.Socket
 * class may use to provide more information on its capabilities.
 */
public interface RMISocketInfo {

    /**
     * Return true if this socket can be used for more than one
     * RMI call.  If a socket does not implement this interface, then
     * it is assumed to be reusable.
     */
    public boolean isReusable();
}
