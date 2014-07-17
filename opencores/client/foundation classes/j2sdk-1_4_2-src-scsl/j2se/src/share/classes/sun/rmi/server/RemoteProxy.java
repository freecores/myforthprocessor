/*
 * @(#)RemoteProxy.java	1.26 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.rmi.server;

import java.lang.reflect.*;
import java.rmi.Remote;
import java.rmi.RemoteException;
import java.rmi.StubNotFoundException;
import java.rmi.server.LogStream;
import java.rmi.server.ObjID;
import java.rmi.server.RemoteRef;
import java.rmi.server.RemoteStub;
import java.rmi.server.Skeleton;
import java.rmi.server.SkeletonNotFoundException;
import java.rmi.server.RMIClientSocketFactory;
import sun.rmi.transport.Endpoint;
import sun.rmi.transport.LiveRef;
import sun.rmi.transport.tcp.TCPEndpoint;

/**
 * The RemoteProxy class has general methods to obtain stubs and
 * skeletons for remote objects.
 */
public final class RemoteProxy extends java.rmi.server.RemoteStub {
    
    /** "server" package log level */
    static int logLevel = LogStream.parseLevel(getLogLevel());

    private static String getLogLevel() {
	return (String) java.security.AccessController.doPrivileged(
            new sun.security.action.GetPropertyAction(
		"sun.rmi.server.logLevel"));
    }

    /** parameter types for stub constructor */
    private static Class[] stubConsParamTypes = { RemoteRef.class };
    
    private RemoteProxy() {
    }
    
    /**
     * Locate a RemoteStub object corresponding to the supplied
     * Remote object.
     */
    public static RemoteStub getStub(Remote object, RemoteRef ref)
	throws StubNotFoundException
    {
	Class cl = null;
	try {
	    cl = getRemoteClass(object.getClass());
	    return getStub(cl.getName(), cl, ref);
	} catch (ClassNotFoundException ex ) {
	    throw new StubNotFoundException("Object does not implement an interface that extends java.rmi.Remote: " + object.getClass().getName());
	}
    }

    /**
     * Create a RemoteStub for the specified implementation class,
     * initialized to the specified RemoteRef.  The stub class must
     * be available locally.
     */
    public static RemoteStub getStub(String classname, RemoteRef ref)
	throws StubNotFoundException
    {
	return getStub(classname, null, ref);
    }

    /**
     * Create a RemoteStub for the specified implementation class,
     * initialized to the specified RemoteRef.
     * The classname is extended with the suffix for stub classes.
     * If the stub class is not available locally, it is loaded
     * relative to fromClass (if not null).
     * The class is found, an instance created, and the RemoteRef is set.
     */
    public static RemoteStub getStub(String classname, Class fromClass,
				     RemoteRef ref)
	throws StubNotFoundException
    {
	String stubname = classname + "_Stub";
	RemoteStub stub = null;

	/* Make sure to use the local stub loader for the stub classes.
	 * When loaded by the local loader the load path can be
	 * propagated to remote clients, by the MarshalOutputStream/InStream
	 * pickle methods
	 */
	try {
	    Class stubcl = loadClassFromClass(stubname, fromClass);
	    Constructor cons = stubcl.getConstructor(stubConsParamTypes);
	    stub = (RemoteStub) cons.newInstance(new Object[] { ref });

	} catch (ClassNotFoundException e) {
	    throw new StubNotFoundException(
		"Stub class not found: " + stubname, e);
	} catch (NoSuchMethodException e) {
	    throw new StubNotFoundException(
		"Stub class missing constructor: " + stubname, e);
	} catch (InstantiationException e) {
	    throw new StubNotFoundException(
		"Can't create instance of stub class: " + stubname, e);
	} catch (IllegalAccessException e) {
	    throw new StubNotFoundException(
		"Stub class constructor not public: " + stubname, e);
	} catch (InvocationTargetException e) {
	    throw new StubNotFoundException(
		"Exception creating instance of stub class: " + stubname, e);
	} catch (java.lang.ClassCastException e) {
	    throw new StubNotFoundException(
		"Stub class not instance of RemoteStub: " + stubname, e);
	}   
	return stub;
    }

    public static RemoteStub getStub(String stubname, int id,
				     String host, int port)
	throws RemoteException
    {
	// Create the needed remote reference 
	// Then find and initialize the stub
	ObjID objID = new ObjID(id);
	Endpoint ep = new TCPEndpoint(host, port);
	LiveRef ref = new LiveRef(objID, ep, false);
	return getStub(stubname, new UnicastRef(ref));
    }

    public static RemoteStub getStub(String stubname, int id,
				     String host, int port,
				     RMIClientSocketFactory csf)
	throws RemoteException
    {
	// Create the needed remote reference 
	// Then find and initialize the stub
	ObjID objID = new ObjID(id);
	Endpoint ep = new TCPEndpoint(host, port, csf, null);
	LiveRef ref = new LiveRef(objID, ep, false);
	return getStub(stubname, new UnicastRef2(ref));
    }
    
    /**
     * Locate and return the Skeleton for the specified remote object
     */
    public static Skeleton getSkeleton(Remote object)
	throws SkeletonNotFoundException
    {
	Class cl;
	try {
	    cl = getRemoteClass(object.getClass());
	} catch (ClassNotFoundException ex ) {
	    throw new SkeletonNotFoundException("Object does not implement an interface that extends java.rmi.Remote: " + object.getClass().getName());
	}

	// now try to load the skeleton based ont he name of the class
	String skelname = cl.getName() + "_Skel";
	try {
	    Class skelcl = loadClassFromClass(skelname, cl);

	    return (Skeleton)skelcl.newInstance();
	} catch (ClassNotFoundException ex) {
	    throw new SkeletonNotFoundException("Skeleton class not found: " +
						skelname, ex);
	} catch (InstantiationException ex) {
	    throw new SkeletonNotFoundException("Can't create skeleton: " +
						skelname, ex);
	} catch (IllegalAccessException ex) {
	    throw new SkeletonNotFoundException("No public constructor: " +
						skelname, ex);
	} catch (java.lang.ClassCastException ex) {
	    throw new SkeletonNotFoundException("Skeleton not of correct class: " + skelname, ex);
	}   
    }

    /**
     * Load a class from the same class loader that loaded another class.
     * If the other class has no class loader, or if it isn't supplied,
     * then it simply loads the class using the system loader (through
     * CLASSPATH, typically).
     */
    private static Class loadClassFromClass(String classname, Class fromClass)
	throws ClassNotFoundException
    {
	ClassLoader loader = null;
	if (fromClass != null)
	    loader = fromClass.getClassLoader();

	if (loader != null)
	    return loader.loadClass(classname);
	else
	    return Class.forName(classname);
    }
    
    /*
     * Find the class/superclass that implements the remote interface.
     * @exception ClassNotFoundException if no class is found to have a
     * remote interface
     */
    static Class getRemoteClass(Class cl)
	throws ClassNotFoundException
    {
	while (cl != null) {
	    Class[] interfaces = cl.getInterfaces();
	    for (int i = interfaces.length -1; i >= 0; i--) {
		if (Remote.class.isAssignableFrom(interfaces[i]))
		    return cl;		// this class implements remote object
	    }
	    cl = cl.getSuperclass();
	}
	throw new ClassNotFoundException(
		"class does not implement java.rmi.Remote");
    }
}
