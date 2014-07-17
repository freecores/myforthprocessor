/*
 * @(#)ActivatableServerRef.java	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.rmi.server;

import java.io.ObjectInput;
import java.rmi.*;
import java.rmi.server.*;
import java.rmi.activation.ActivationID;
import sun.rmi.transport.LiveRef;

/**
 * Server-side ref for a persistent remote impl.
 *
 * @author Ann Wollrath
 */
public class ActivatableServerRef extends UnicastServerRef2
{
    private ActivationID id;

    /**
     * Construct a Unicast server remote reference to be exported
     * on the specified port.
     */
    public ActivatableServerRef(ActivationID id, int port)
    {
	this(id, port, null, null);
    }

    /**
     * Construct a Unicast server remote reference to be exported
     * on the specified port.
     */
    public ActivatableServerRef(ActivationID id, int port,
				RMIClientSocketFactory csf,
				RMIServerSocketFactory ssf)
    {
	super(new LiveRef(port, csf, ssf));
	this.id = id;
    }

    /**
     * Returns the class of the ref type to be serialized
     */
    public String getRefClass(java.io.ObjectOutput out)
    {
	if (out instanceof MarshalOutputStream) {
	    throw new InternalError("ActivatableServerRef.writeExternal: " +
		"server reference written to MarshalOutputStream");
	}
	return "ActivatableServerRef";
    }

    /**
     * Return the client remote reference for this remoteRef.
     * In the case of a client RemoteRef "this" is the answer.
     * For  a server remote reference, a client side one will have to
     * found or created.
     */
    protected RemoteRef getClientRef() {
	return new ActivatableRef(id, new UnicastRef2(ref));
    }

}
