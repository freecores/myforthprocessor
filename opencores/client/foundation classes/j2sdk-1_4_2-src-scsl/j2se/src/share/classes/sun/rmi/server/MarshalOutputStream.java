/*
 * @(#)MarshalOutputStream.java	1.27 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.rmi.server;

import java.io.*;
import java.rmi.Remote;
import java.rmi.server.RemoteStub;
import sun.rmi.transport.ObjectTable;
import sun.rmi.transport.Target;

/**
 * A MarshalOutputStream extends ObjectOutputStream to add functions
 * specific to marshaling of remote object references. If it is
 * necessary to serialize remote objects or objects that contain
 * references to remote objects a MarshalOutputStream must be used
 * instead of ObjectOutputStream. <p>
 *
 * A new MarshalOutputStream is constructed to serialize remote
 * objects or graphs containing remote objects. Objects are written to
 * the stream using the ObjectOutputStream.writeObject method. <p>
 *
 * MarshalOutputStream maps remote objects to the corresponding remote
 * stub and embeds the location from which to load the stub
 * classes. The location may be ignored by the client but is supplied.
 */
public class MarshalOutputStream extends ObjectOutputStream
{
    /**
     * Creates a marshal output stream with protocol version 1.
     */
    public MarshalOutputStream(OutputStream out) throws IOException {
	this(out, ObjectStreamConstants.PROTOCOL_VERSION_1);
    }

    /**
     * Creates a marshal output stream with the given protocol version.
     */
    public MarshalOutputStream(OutputStream out, int protocolVersion)
	throws IOException
    {
	super(out);
	this.useProtocolVersion(protocolVersion);
	java.security.AccessController.doPrivileged(
				    new java.security.PrivilegedAction() {
	    public Object run() {
		enableReplaceObject(true);
		return null;
	    }
	});
    }

    /**
     * replaceObject is extended to check for instances of Remote
     * that need to be serialized as proxy objects.  RemoteProxy.getProxy
     * is called to check for and find the stub.
     */
    protected final Object replaceObject(Object obj) throws IOException {
	if ((obj instanceof Remote) && !(obj instanceof RemoteStub)) {
	    Target target = ObjectTable.getTarget((Remote) obj);
	    if (target != null) {
		return target.getStub();
	    }
	}
	return obj;
    }

    /**
     * annotateClass is extended to serialize a location from which
     * to load the the specified class.
     */
    protected void annotateClass(Class cl) throws IOException {
	// write the specified location (may be null).
	writeLocation(java.rmi.server.RMIClassLoader.getClassAnnotation(cl));
    }

    /**
     * annotateProxyClass is extended to have the same behavior
     * as annotateClass.
     */
    protected void annotateProxyClass(Class cl) throws IOException {
	annotateClass(cl);
    }

    /**
     * Write the location for the class into the stream.  This method can
     * be overridden by subclasses that store this annotation somewhere
     * else than as the next object in the stream, as is done by this class.
     */
    protected void writeLocation(String location) throws IOException {
	writeObject(location);
    }
}
