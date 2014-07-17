/*
 * @(#)Activation.java	1.57 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.rmi.server;

import java.lang.Process;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;

import java.io.*;

import java.rmi.*;
import java.rmi.activation.*;
import java.rmi.registry.Registry;
import java.rmi.server.*;
import java.security.*;
import java.text.MessageFormat;
import java.util.*;

import sun.rmi.log.ReliableLog;
import sun.rmi.log.LogHandler;
import sun.rmi.registry.RegistryImpl;
import sun.rmi.runtime.NewThreadAction;
import sun.rmi.server.UnicastServerRef;
import sun.rmi.transport.LiveRef;
import sun.rmi.transport.ObjectTable;
import sun.rmi.transport.Target;
import sun.security.action.GetBooleanAction;
import sun.security.action.GetIntegerAction;
import sun.security.action.GetPropertyAction;

import com.sun.rmi.rmid.*;

/**
 * The Activator facilitates remote object activation. A "faulting"
 * remote reference calls the activator's <code>activate</code> method
 * to obtain a "live" reference to a activatable remote object. Upon
 * receiving a request for activation, the activator looks up the
 * activation descriptor for the activation identifier, id, determines
 * the group in which the object should be activated and invokes the
 * activate method on the object's activation group (described by the
 * remote interface <code>ActivationInstantiator</code>). The
 * activator initiates the execution of activation groups as
 * necessary. For example, if an activation group for a specific group
 * identifier is not already executing, the activator will spawn a
 * child process for the activation group. <p>
 *
 * The activator is responsible for monitoring and detecting when
 * activation groups fail so that it can remove stale remote references
 * from its internal tables. <p>
 *
 * @version	1.57, 01/23/03
 * @author 	Ann Wollrath
 * @since 	JDK1.2
 */
public class Activation implements Serializable, Runnable {
    /** indicate compatibility with JDK 1.2 version of class */
    private static final long serialVersionUID = 2921265612698155191L;
    
    private static Registry registry;

    private static final byte MAJOR_VERSION = 1;
    private static final byte MINOR_VERSION = 0;
   
    /** 
     * used by GroupEntry to count number of restarted groups. inner
     * classes can not have static fields.  
     */
    private static int nextRestart = 0;

    /** exec policy object */
    private static Object execPolicy;
    private static Method execPolicyMethod;
    private final static String execPolicyMethodName = "checkExecCommand";
    private final static Class[] execPolicyMethodParamTypes =
	new Class[] { ActivationGroupDesc.class, String[].class };
    private static boolean debugExec;
    
    /** maps activation id to its respective group id */
    private Hashtable idTable = new Hashtable(101);
    /** maps group id to its GroupEntry groups */
    private Hashtable groupTable = new Hashtable(53);
    
    private byte majorVersion = MAJOR_VERSION;
    private byte minorVersion = MINOR_VERSION;

    /** reliable log to hold descriptor table */
    private transient ReliableLog log;
    /** number of updates since last snapshot */
    private transient int numUpdates = 0;
    
    /** the java command */
    // accessed by GroupEntry
    private transient String[] command;

    /** take snapshot after this many updates */
    private final static int snapshotInterval = 
	getInt("sun.rmi.activation.snapshotInterval", 200);
    
    /** timeout on wait for child process to be created */
    private final static long execTimeout =
	getInt("sun.rmi.activation.execTimeout", 30000);

    // this should be a *private* method since it is privileged
    private static int getInt(String name, int def) {
	return ((Integer)java.security.AccessController.doPrivileged(
            new sun.security.action.GetIntegerAction(name, def))).
	    intValue();
    }

    private transient Activator activator;
    private transient Activator activatorStub;
    private transient ActivationSystem system;
    private transient ActivationSystem systemStub;
    private transient ActivationMonitor monitor;
    private transient volatile boolean shuttingDown = false;
    private transient Object startupLock;
    private transient Thread shutdownHook;

    private static ResourceBundle resources = null;

    /**
     * Create an uninitialized instance of Activation that can be
     * populated with log data.  This is only called when the initial
     * snapshot is taken during the first incarnation of rmid.
     */
    private Activation() {}

    /**
     * Recover activation state from the reliable log and initialize
     * activation services.  
     */
    private static void startActivation(int port, String logName, 
					      String[] childArgs)
	throws Exception 
    {
	ReliableLog log = new ReliableLog(logName, new ActLogHandler());
	Activation state = (Activation)log.recover();
	state.init(port, log, childArgs);
	state.restartServices();
    }

    /** 
     * Initialize the Activation instantiation; start activation
     * services.
     */
    private void init(int port, ReliableLog log, String[] childArgs) 
	throws Exception 
    {
	// initialize
	this.log = log;
	numUpdates = 0;
	shutdownHook = 	(Thread) java.security.AccessController.doPrivileged(
	    new NewThreadAction(new ShutdownHook(), "RMID Shutdown Hook",
				false));
	Runtime.getRuntime().addShutdownHook(shutdownHook);
	synchronized (startupLock = new Object()) {
	    // all the remote methods briefly synchronize on startupLock
	    // (via checkShutdown) to make sure they don't happen in the
	    // middle of this block.  This block must not cause any such
	    // incoming remote calls to happen, or deadlock would result!
	    activator = new ActivatorImpl(port);
	    activatorStub = (Activator)RemoteObject.toStub(activator);
	    system = new ActivationSystemImpl(port);
	    systemStub = (ActivationSystem)RemoteObject.toStub(system);
	    monitor = new ActivationMonitorImpl(port);
	    initCommand(childArgs);
	
	    Naming.rebind("//:" + port +
			  "/java.rmi.activation.ActivationSystem",
			  system);
	}
	startupLock = null;
    }

    private void restartServices() {
	Enumeration enum = groupTable.keys();
	while (enum.hasMoreElements()) {
	    ActivationGroupID id = (ActivationGroupID) enum.nextElement();
	    try {
		GroupEntry entry = (GroupEntry) getGroupEntry(id);
		entry.restartServices();
	    } catch (UnknownGroupException e) {
		System.err.println("\nrmid: unable to restart services");
		System.err.println("rmid: exception occurred:");
		e.printStackTrace(System.err);
	    }
	}
    }

    class ActivatorImpl extends RemoteServer implements Activator {
	// Because ActivatorImpl has a fixed ObjID, it can be
	// called by clients holding stale remote references.  Each of
	// its remote methods, then, must check startupLock (calling
	// checkShutdown() is easiest).
	/**
	 * Construct a new Activator on a specified port.
	 */
	ActivatorImpl(int port) throws RemoteException {
	    /* Server ref must be created and assigned before remote object 
	     * 'this' can be exported.
	     */
	    LiveRef lref = new LiveRef(new ObjID(ObjID.ACTIVATOR_ID), port);
	    UnicastServerRef uref = new UnicastServerRef(lref);
	    ref = uref;
	    uref.exportObject(this, null);
	}
    
	/**
	 * Activate the object associated with the activation identifier,
	 * <code>id</code>. If the activator knows the object to be active
	 * already, and <code>force</code> is false , the stub with a
	 * "live" reference is returned immediately to the caller;
	 * otherwise, if the activator does not know that corresponding
	 * the remote object is active, the activator uses the activation
	 * descriptor information (previously registered) to determine the
	 * group (VM) in which the object should be activated. If an
	 * <code>ActivationInstantiator</code> corresponding to the
	 * object's group descriptor already exists, the activator invokes
	 * the activation group's <code>newInstance</code> method passing
	 * it the object's id and descriptor. <p>
	 *
	 * If the activation group for the object's group descriptor
	 * does not yet exist, the activator starts an
	 * <code>ActivationInstantiator</code> executing (by spawning
	 * a child process, for example). When the activator receives
	 * the activation group's call back (via the
	 * <code>ActivationSystem</code>'s <code>activeGroup</code>
	 * method) specifying the activation group's reference, the
	 * activator can then invoke that activation group's activate
	 * method to forward the pending activation request to the
	 * activation group and return the result (a serialized remote
	 * object reference, a stub) to the caller.<p>
	 *
	 * Note that the activator receives a "marshalled" object instead of a
	 * Remote object so that the activator does not need to load the
	 * code for that object, or participate in distributed garbage
	 * collection for that object. If the activator kept a strong
	 * reference to the remote object, the activator would then
	 * prevent the object from being garbage collected under the
	 * normal distributed garbage collection mechanism. <p>
	 *
	 * @param id the activation identifier for the object being activated
	 * @param force if true, the activator contacts the group to obtain
	 * the remote object's reference; if false, returning the cached value
	 * is allowed.
	 * @return the remote object (a stub) in a marshalled form
	 * @exception ActivationException if object activation fails
	 * @exception UnknownObjectException if object is unknown (not
	 * registered)
	 * @exception RemoteException if remote call fails
	 */
	public MarshalledObject activate(ActivationID id, boolean force)
    	    throws ActivationException, UnknownObjectException, RemoteException
	{
	    checkShutdown();
	    
	    return getGroupEntry(id).activate(id, force);
	}
    }

    class ActivationMonitorImpl extends UnicastRemoteObject
        implements ActivationMonitor
    {
	ActivationMonitorImpl(int port) throws RemoteException 
	{
	    super(port);
	}
	
	/**
	 * An activation group calls its monitor's
	 * <code>inactiveObject</code> method when an object in its group
	 * becomes inactive (deactivates).  An activation group discovers
	 * that an object (that it participated in activating) in its VM
	 * is no longer active, via calls to the activation group's
	 * <code>inactiveObject</code> method. <p>
	 *
	 * The <code>inactiveObject</code> call informs the
	 * <code>ActivationMonitor</code> that the remote object reference
	 * it holds for the object with the activation identifier,
	 * <code>id</code>, is no longer valid. The monitor considers the
	 * reference associated with <code>id</code> as a stale reference.
	 * Since the reference is considered stale, a subsequent
	 * <code>activate</code> call for the same activation identifier
	 * results in re-activating the remote object.<p>
	 *
	 * @param id the object's activation identifier
	 * @exception UnknownObjectException if object is unknown
	 * @exception RemoteException if remote call fails
	 */
	public void inactiveObject(ActivationID id)
	    throws UnknownObjectException, RemoteException
	{
	    try {
		checkShutdown();
	    } catch (ActivationException e) {
		return;
	    }
	    
	    RegistryImpl.checkAccess("Activator.inactiveObject");
	
	    try {
		getGroupEntry(id).inactiveObject(id);
	    } catch (UnknownGroupException e) {
		throw new UnknownObjectException("object's group removed");
	    }
	}
    
	/**
	 * Informs that an object is now active. An
	 * <code>ActivationGroup</code> informs its monitor if an
	 * object in its group becomes active by other means than
	 * being activated directly (i.e., the object is registered
	 * and "activated" itself).
	 *
	 * @param id the active object's id
	 * @param obj the marshalled form of the object's stub
	 * @exception UnknownObjectException if object is unknown
	 * @exception RemoteException if remote call fails
	 */
	public void activeObject(ActivationID id, MarshalledObject mobj)
    	    throws UnknownObjectException, RemoteException
	{
	    try {
		checkShutdown();
	    } catch (ActivationException e) {
		return;
	    }
	    
	    RegistryImpl.checkAccess("ActivationSystem.activeObject");

	    try {
		getGroupEntry(id).activeObject(id, mobj);
	    } catch (UnknownGroupException e) {
		throw new UnknownObjectException("object's group removed");
	    }
	}
	
	/**
	 * Informs that the group is now inactive. The group will be
	 * recreated upon a subsequent request to activate an object
	 * within the group. A group becomes inactive when all objects
	 * in the group report that they are inactive.
	 *
	 * @param id the group's id
	 * @param incarnation the group's incarnation number	 
	 * @exception UnknownObjectException if group is unknown, or
	 * the incarnation number is earlier than the current.
	 * @exception RemoteException if remote call fails
	 */
	public void inactiveGroup(ActivationGroupID id,
				  long incarnation)
	    throws UnknownGroupException, RemoteException
	{
	    try {
		checkShutdown();
	    } catch (ActivationException e) {
		return;
	    }

	    RegistryImpl.checkAccess("ActivationMonitor.inactiveGroup");

	    GroupEntry entry = getGroupEntry(id);
	    entry.inactiveGroup(incarnation);
	}
    }
    

    class ActivationSystemImpl
	extends RemoteServer 
	implements ActivationSystem
    {
	// Because ActivationSystemImpl has a fixed ObjID, it can be
	// called by clients holding stale remote references.  Each of
	// its remote methods, then, must check startupLock (calling
	// checkShutdown() is easiest).
	ActivationSystemImpl(int port) throws RemoteException 
	{
	    /* Server ref must be created and assigned before remote object 
	     * 'this' can be exported.
	     */
	    LiveRef lref = new LiveRef(new ObjID(4), port);
	    UnicastServerRef uref = new UnicastServerRef(lref);
	    ref = uref;
	    uref.exportObject(this, null);
	}
	
	/**
	 * The <code>registerObject</code> method is used to register
	 * an activation descriptor, <code>desc</code>, and obtain an
	 * activation identifier for a activatable remote object. The
	 * <code>ActivationSystem</code> creates an
	 * <code>ActivationID</code> (a activation identifier) for the
	 * object specified by the descriptor, <code>desc</code>, and
	 * records, in stable storage, the activation descriptor and
	 * its associated identifier for later use. When the
	 * <code>Activator</code> receives an <code>activate</code>
	 * request for a specific identifier, it looks up the
	 * activation descriptor (registered previously) for the
	 * specified identifier and uses that information to activate
	 * the object. <p>
	 *
	 * @param desc the object's activation descriptor
	 * @return the activation id that can be used to activate the object
	 * @exception ActivationException if registration fails (e.g., database
	 * update failure, etc).
	 * @exception UnknownGroupException if group referred to in
	 * <code>desc</code> is not registered with this system
	 * @exception RemoteException if remote call fails
	 */
	public ActivationID registerObject(ActivationDesc desc)
	    throws ActivationException, UnknownGroupException, RemoteException
	{
	    checkShutdown();
	    RegistryImpl.checkAccess("ActivationSystem.registerObject");

	    ActivationGroupID groupID = desc.getGroupID();
	    ActivationID id = new ActivationID(activatorStub);
	    getGroupEntry(groupID).registerObject(id, desc, true);
	    return id;
	}

	/**
	 * Remove the activation id and associated descriptor previously
	 * registered with the <code>ActivationSystem</code>; the object
	 * can no longer be activated via the object's activation id.
	 *
	 * @param id the object's activation id (from previous registration)
	 * @exception ActivationException if unregister fails (e.g., database
	 * update failure, etc).
	 * @exception UnknownObjectException if object is unknown (not
	 * registered)
	 * @exception RemoteException if remote call fails
	 */
	public void unregisterObject(ActivationID id)
	    throws ActivationException, UnknownObjectException, RemoteException
	{
	    checkShutdown();
	    RegistryImpl.checkAccess("ActivationSystem.unregisterObject");

	    /*
	     * Make sure object and group is still registered.
	     */
	    ActivationGroupID groupID = (ActivationGroupID) idTable.get(id);
	    
	    if (groupID != null) {
		getGroupEntry(groupID).unregisterObject(id, true);
	    } else {
		throw new UnknownObjectException("object not registered");
	    }
	}
	
	/**
	 * Register the activation group. An activation group must be
	 * registered with the <code>ActivationSystem</code> before objects
	 * can be registered within that group.
	 *
	 * @param desc the group's descriptor
	 * @return an identifier for the group
	 * @exception ActivationException if group registration fails
	 * @exception RemoteException if remote call fails
	 */
	public ActivationGroupID registerGroup(ActivationGroupDesc desc)
	    throws ActivationException, RemoteException
	{
	    checkShutdown();
	    RegistryImpl.checkAccess("ActivationSystem.registerGroup");
	    checkArgs(desc, null);
	    
	    ActivationGroupID id = new ActivationGroupID(systemStub);

	    GroupEntry entry = new GroupEntry(id, desc);

	    // table insertion must take place before log update
	    groupTable.put(id, entry);
	    addLogRecord(new LogRegisterGroup(id, desc));
	    return id;
	}
	
	/**
	 * Callback to inform activation system that group is now
	 * active. This call is made internally by the
	 * <code>ActivationGroup.createGroup</code> method to inform
	 * the <code>ActivationSystem</code> that the group is now
	 * active.
	 *
	 * @param id the activation group's identifier
	 * @param group the group's instantiator
	 * @param incarnation the group's incarnation number
	 *
	 * @exception UnknownGroupException if group is not registered
	 * @exception ActivationException if group is already active
	 * @exception RemoteException if remote call fails
	 */
	public ActivationMonitor activeGroup(ActivationGroupID id,
					     ActivationInstantiator group,
					     long incarnation)
	    throws ActivationException, UnknownGroupException, RemoteException
	{
	    checkShutdown();
	    RegistryImpl.checkAccess("ActivationSystem.activeGroup");
	    
	    getGroupEntry(id).activeGroup(group, incarnation);
	    return monitor;
	}
	
	/**
	 * Remove the activation group. An activation group makes this
	 * call back to inform the activator that the group should be
	 * removed (destroyed).  If this call completes successfully,
	 * objects can no longer be registered or activated within the
	 * group. All information of the group and its associated
	 * objects is removed from the system.
	 *
	 * @param id the activation group's identifier
	 * @exception ActivationException if unregister fails (e.g., database
	 * update failure, etc).
	 * @exception UnknownGroupException if group is not registered
	 * @exception RemoteException if remote call fails
	 */
	public void unregisterGroup(ActivationGroupID id)
	    throws ActivationException, UnknownGroupException, RemoteException
	{
	    checkShutdown();
	    RegistryImpl.checkAccess("ActivationSystem.unregisterGroup");
	
	    // remove entry
	    GroupEntry entry = (GroupEntry) groupTable.remove(id);
	    if (entry == null) {
		throw new UnknownGroupException("unknown group: " + id);
	    }

	    entry.unregisterGroup(true);
	}

	/**
	 * Set the activation descriptor, <code>desc</code> for the object with
	 * the activation identifier, <code>id</code>. The change will take
	 * effect upon subsequent activation of the object.
	 *
	 * @param id the activation identifier for the activatable object
	 * @param desc the activation descriptor for the activatable object
	 * @exception UnknownGroupException the group associated with
	 * <code>desc</code> is not a registered group
	 * @exception UnknownObjectException the activation <code>id</code>
	 * is not registered
	 * @exception ActivationException for general failure (e.g., unable
	 * to update log)
	 * @exception RemoteException if remote call fails
	 * @return the previous value of the activation descriptor
	 */
	public ActivationDesc setActivationDesc(ActivationID id,
						ActivationDesc desc)
	    throws ActivationException, UnknownGroupException,
		   UnknownObjectException, RemoteException
	{
	    checkShutdown();
	    if (!getGroupID(id).equals(desc.getGroupID()))
		throw new ActivationException("ActivationDesc contains " +
					      "wrong group");
	    
	    RegistryImpl.checkAccess("ActivationSystem.setActivationDesc");

	    return getGroupEntry(id).setActivationDesc(id, desc, true);
	}

	/**
	 * Set the activation group descriptor, <code>desc</code> for the
	 * object with the activation group identifier, <code>id</code>. The
	 * change will take effect upon subsequent activation of the group.
	 * 
	 * @param id the activation group identifier for the activation group
	 * @param desc the activation group descriptor for the activation group
	 * @exception UnknownGroupException the group associated with
	 * <code>id</code> is not a registered group
	 * @exception ActivationException for general failure (e.g., unable
	 * to update log)
	 * @exception RemoteException if remote call fails
	 * @return the previous value of the activation group descriptor 
	 */
	public ActivationGroupDesc setActivationGroupDesc(ActivationGroupID id,
							  ActivationGroupDesc desc)
	    throws ActivationException, UnknownGroupException, RemoteException
	{
	    checkShutdown();
	    RegistryImpl.checkAccess
		("ActivationSystem.setActivationGroupDesc");
	    checkArgs(desc, null);

	    return getGroupEntry(id).setActivationGroupDesc(id, desc, true);
	}

	/**
	 * Returns the activation descriptor, for the object with the
	 * activation identifier, <code>id</code>.
	 * 
	 * @param id the activation identifier for the activatable object
	 * @exception UnknownObjectException if <code>id</code> is not
	 * registered
	 * @exception ActivationException for general failure
	 * @exception RemoteException if remote call fails
	 * @return the activation descriptor */
	public ActivationDesc getActivationDesc(ActivationID id)
	    throws ActivationException, UnknownObjectException, RemoteException
	{
	    checkShutdown();
	    RegistryImpl.checkAccess("ActivationSystem.getActivationDesc");

	    return getGroupEntry(id).getActivationDesc(id);
	}
	      
	/**
	 * Returns the activation group descriptor, for the group
	 * with the activation group identifier, <code>id</code>. 
	 * 
	 * @param id the activation group identifier for the group
	 * @exception UnknownGroupException if <code>id</code> is not
	 * registered
	 * @exception ActivationException for general failure
	 * @exception RemoteException if remote call fails
	 * @return the activation group descriptor
	 */
	public ActivationGroupDesc getActivationGroupDesc(ActivationGroupID id)
	    throws ActivationException, UnknownGroupException, RemoteException
	{
	    checkShutdown();
	    RegistryImpl.checkAccess
		("ActivationSystem.getActivationGroupDesc");

	    return getGroupEntry(id).desc;
	}
	
	/**
	 * Shutdown the activation system. Destroys all groups spawned by
	 * the activation daemon and exits the activation daemon.
	 */
	public void shutdown() throws AccessException {
	    RegistryImpl.checkAccess("ActivationSystem.shutdown");
	    
	    Object lock = startupLock;
	    if (lock != null) {
		synchronized (lock) {
		    // nothing
		}
	    }
	    shutdownAll();
	    
	}
    }
    
    private void checkShutdown() throws ActivationException {
	// if the startup critical section is running, wait until it
	// completes/fails before continuing with the remote call.
	Object lock = startupLock;
	if (lock != null) {
	    synchronized (lock) {
		// nothing
	    }
	}

	if (shuttingDown == true) {
	    throw new ActivationException("activation system is " +
					  "shutting down");
	}
	
    }

    private synchronized void shutdownAll() {
	Thread shutdownThread = null;

	if (shuttingDown == true) {
	    return;
	}
	
	shuttingDown = true;
	
	/* start an RMI thread to shut down RMID */
	shutdownThread = (Thread) java.security.AccessController.doPrivileged(
	    new NewThreadAction(Activation.this, "RMID Shutdown", false));
	shutdownThread.start();
    }

    private static void unexport(Remote obj) {
	for (;;) {
	    try {
		if (UnicastRemoteObject.unexportObject(obj, false) == true) {
		    break;
		} else {
		    Thread.sleep(100);
		}
	    } catch (Exception e) {
		continue;
	    }
	}
    }
	
    /**
     * Thread to shutdown rmid.
     */
    public void run() {
	try {
	    /*
	     * Unexport activation system services
	     */
	    unexport(activator);
	    unexport(system);

	    // destroy all child processes (groups)
	    Enumeration enum = groupTable.elements();
	    while (enum.hasMoreElements()) {
		GroupEntry entry = (GroupEntry)enum.nextElement();
		entry.shutdown();
	    }
	    Runtime.getRuntime().removeShutdownHook(shutdownHook);

	    /*
	     * Unexport monitor safely since all processes are destroyed.
	     */
	    unexport(monitor);

	    /*
	     * Close log file, fix for 4243264: rmid shutdown thread
	     * interferes with remote calls in progress.  Make sure
	     * the log file is only closed when it is impossible for
	     * its closure to interfere with any pending remote calls.
	     * We close the log when all objects in the rmid VM are
	     * unexported.
	     */
	    try {
		log.close();
	    } catch (IOException e) {
	    }

	} finally {
	    /*
	     * Now exit... A System.exit should only be done if
	     * the RMI activation system daemon was started up
	     * by the main method below (in which should always
	     * be the case since the Activation contructor is private).
	     */
	    System.err.println(getTextResource("rmid.daemon.shutdown"));
	    System.exit(0);
	}
    }
    
    /** Thread to destroy children in the event of abnormal termination. */
    private class ShutdownHook implements Runnable {
	ShutdownHook() {
	}

	public void run() {
	    shuttingDown = true;
	    // destroy all child processes (groups) quickly
	    synchronized (groupTable) {
		Enumeration enum = groupTable.elements();
		while (enum.hasMoreElements()) {
		    GroupEntry entry = (GroupEntry)enum.nextElement();
		    entry.shutdownFast();
		}
	    }
	}
    }

    /**
     * Returns the groupID for a given id of an object in the group.
     * Throws UnknownObjectException if the object is not registered.
     */
    private ActivationGroupID getGroupID(ActivationID id)
	throws UnknownObjectException
    {
	
	ActivationGroupID groupID = (ActivationGroupID) idTable.get(id);
	if (groupID == null)
	    throw new UnknownObjectException("unknown object: " + id);
	return groupID;
    }
    
    /**
     * Returns the group entry for the group id. Throws
     * UnknownGroupException if the group is not registered.
     */
    private GroupEntry getGroupEntry(ActivationGroupID id)
	throws UnknownGroupException
    {
	GroupEntry entry = (GroupEntry)groupTable.get(id);
	if (entry == null)
	    throw new UnknownGroupException("unknown group: " + id);
	return entry;
    }

    /**
     * Returns the group entry for the object's id. Throws
     * UnknownGroupException if the group is not registered
     * or UnknownObjectException if the object is not registered.
     */
    private GroupEntry getGroupEntry(ActivationID id)
	throws UnknownGroupException, UnknownObjectException
    {
	return getGroupEntry(getGroupID(id));
    }

    /**
     * Container for group information: group's descriptor, group's
     * instantiator, flag to indicate pending group creation, and
     * table of the group's actived objects.
     *
     * WARNING: GroupEntry objects should not be written into log file
     * updates.  GroupEntrys are inner classes of Activation and they
     * can not be serialized independent of this class.  If the
     * complete Activation system is written out as a log update, the
     * point of having updates is nullified.  
     */
    private class GroupEntry implements Serializable {
	
	/** indicate compatibility with JDK 1.2 version of class */
	private static final long serialVersionUID = 7222464070032993304L;
	static final int MAX_TRIES = 2;
	
	ActivationGroupDesc desc = null;
	ActivationGroupID groupID = null;
	long incarnation = 0;
	HashMap objects = null;
	HashSet restartSet = null;
	
	transient ActivationInstantiator group = null;
	transient boolean pendingCreate = false;
	transient Process child = null;
	transient boolean removed = false;
	transient RestartThread restartThread = null;
	
	GroupEntry(ActivationGroupID groupID, ActivationGroupDesc desc) {
	    this.groupID = groupID;
	    this.desc = desc;
	    this.objects = new HashMap(11);
	    this.restartSet = new HashSet();
	}

	void restartServices() {
	    Iterator iter = null;
	    
	    synchronized (this) {
		if (restartSet.size() == 0)
		    return;

		/*
		 * Clone the restartSet so the set does not have to be locked
		 * during iteration. Locking the restartSet could cause
		 * deadlock if an object we are restarting caused another
		 * object in this group to be activated.
		 */
		iter = ((HashSet) restartSet.clone()).iterator();
	    }
	    
	    while (iter.hasNext()) {
		ActivationID id = (ActivationID) iter.next();
		try {
		    activate(id, true);
		} catch (Exception e) {
		    System.err.println("\nrmid: unable to restart service");
		    System.err.println("rmid: exception occurred: " +
				       e.getMessage());
		    e.printStackTrace(System.err);
		}
	    }
	}
	
	synchronized void activeGroup(ActivationInstantiator inst,
				      long instIncarnation)
	    throws ActivationException, UnknownGroupException
	{
	    if (child != null && !pendingCreate) {
		throw new ActivationException("group not being created");
	    }
	    
	    if (incarnation != instIncarnation) {
		throw new ActivationException("invalid incarnation");
	    }
		
	    if (group != null) {
		throw new ActivationException("group already active");
	    }
	    
	    group = inst;
	    pendingCreate = false;
	    notifyAll();
	}

	private void checkRemoved() throws UnknownGroupException {
	    if (removed == true)
		throw new UnknownGroupException("Group removed");
	}

	synchronized void registerObject(ActivationID id, 
					 ActivationDesc desc,
					 boolean addRecord)
    	    throws UnknownGroupException, ActivationException
	{
	    checkRemoved();
	    objects.put(id, new ObjectEntry(desc));
	    if (desc.getRestartMode() == true) {
		restartSet.add(id);
		ensureRestart();
	    }
	    
	    // table insertion must take place before log update
	    idTable.put(id, groupID);

	    if (addRecord)
		addLogRecord(new LogRegisterObject(id, desc));
	}

	synchronized void unregisterObject(ActivationID id, boolean addRecord)
    	    throws UnknownGroupException, ActivationException
	{
	    checkRemoved();
	    ObjectEntry objEntry = (ObjectEntry) objects.remove(id);
	    objEntry.removed = true;
	    if (objEntry.desc.getRestartMode() == true) {
		restartSet.remove(id);
		if (restartSet.size() == 0)
		    disposeRestartThread();
	    }

	    // table insertion must take place before log update
	    idTable.remove(id);
	    if (addRecord)
		addLogRecord(new LogUnregisterObject(id));
	}
	
	synchronized void unregisterGroup(boolean addRecord)
    	   throws UnknownGroupException, ActivationException
	{
	    checkRemoved();
	    removed = true;
	    Iterator iter = objects.keySet().iterator();
	    while (iter.hasNext()) {
		ActivationID id = (ActivationID) iter.next();
		idTable.remove(id);

		ObjectEntry objEntry = (ObjectEntry) objects.get(id);
		objEntry.removed = true;
	    }
	    objects.clear();
	    restartSet.clear();
	    disposeRestartThread();

	    // removal should be recorded before log update
	    if (addRecord)
		addLogRecord(new LogUnregisterGroup(groupID));

	    // signal waiters for group creation
	    if (pendingCreate) {
		notifyAll();
	    }
	}

	synchronized ActivationDesc setActivationDesc(ActivationID id,
						      ActivationDesc desc,
						      boolean addRecord)
	    throws UnknownObjectException, UnknownGroupException,
	           ActivationException
	{
	    checkRemoved();
	    ObjectEntry objEntry = (ObjectEntry) objects.get(id);
	    if (objEntry == null)
		throw new UnknownObjectException("object unknown");
	    ActivationDesc oldDesc = objEntry.desc;
	    objEntry.desc = desc;
	    if (desc.getRestartMode() == true) {
		restartSet.add(id);
		ensureRestart();
	    } else {
		restartSet.remove(id);
		if (restartSet.size() == 0) {
		    disposeRestartThread();
		}
	    }
	    // restart information should be recorded before log update
	    if (addRecord)
		addLogRecord(new LogUpdateDesc(id, desc));

	    return oldDesc;
	}

	synchronized ActivationDesc getActivationDesc(ActivationID id)
	    throws UnknownObjectException, UnknownGroupException
	{
	    checkRemoved();
	    ObjectEntry objEntry = (ObjectEntry) objects.get(id);
	    if (objEntry == null)
		throw new UnknownObjectException("object unknown");
	    return objEntry.desc;
	}

	synchronized ActivationGroupDesc setActivationGroupDesc(
		ActivationGroupID id,
		ActivationGroupDesc desc,
		boolean addRecord)
    	    throws UnknownGroupException, ActivationException
	{
	    checkRemoved();

	    ActivationGroupDesc oldDesc = this.desc;
	    this.desc = desc;
	    // state update should occur before log update
	    if (addRecord)
		addLogRecord(new LogUpdateGroupDesc(id, desc));

	    return oldDesc;
	}


	synchronized void inactiveGroup(long incarnation)
	    throws UnknownGroupException
	{
	    checkRemoved();
	    if (this.incarnation != incarnation) {
		throw new UnknownGroupException("invalid incarnation");
	    }

	    disposeRestartThread();

	    if (!pendingCreate) {
		reset();
	    }
	}

	synchronized void activeObject(ActivationID id, MarshalledObject mobj)
    		throws UnknownObjectException, UnknownGroupException
	{
	    checkRemoved();
	    ObjectEntry objEntry = (ObjectEntry) objects.get(id);
	    if (objEntry == null)
		throw new UnknownObjectException("object unknown");
	    objEntry.stub = mobj;
	}

	synchronized void inactiveObject(ActivationID id)
    	    throws UnknownObjectException, UnknownGroupException
	{
	    checkRemoved();
	    ObjectEntry objEntry = (ObjectEntry) objects.get(id);
	    if (objEntry == null)
		throw new UnknownObjectException("object unknown");
	    objEntry.reset();
	}

	private synchronized void reset() {
	    group = null;
	    Iterator iter = objects.values().iterator();
	    while (iter.hasNext()) {
		((ObjectEntry) iter.next()).reset();
	    }
	}
	
	private synchronized void resetAll() {
	    reset();
	    if (child != null) {
		child.destroy();
		child = null;
	    }
	}
	
	// no synchronization to avoid delay wrt getInstantiator
	void shutdownFast() {
	    if (child != null) {
		child.destroy();
	    }
	}

	synchronized void shutdown() {
	    resetAll();
	    disposeRestartThread();
	}

	private synchronized void disposeRestartThread() {
	    if (restartThread != null) {
		restartThread.dispose();
		restartThread = null;
	    }
	}
	
	MarshalledObject activate(ActivationID id, boolean force)
	    throws ActivationException
	{
	    /*
	     * Look up object to activate
	     */
	    ObjectEntry objEntry;
	    synchronized (this) {
		objEntry = (ObjectEntry) objects.get(id);
	    }
	    
	    if (objEntry == null)
		throw new UnknownObjectException("object unknown " + id);

	    /*
	     * If not forcing activation, then return cached stub as
	     * an optimization.
	     */
	    if (force != true) {
		synchronized (this) {
		    if (objEntry.stub != null)
			return objEntry.stub;
		}
	    }
	    
	    Exception detail = null;
	    ActivationDesc desc = null;

	    /*
	     * Attempt to activate object and reattempt (several times)
	     * if activation fails due to communication problems.
	     */
	    for (int tries = MAX_TRIES; tries > 0; tries--) {
		ActivationInstantiator inst;
		long currentIncarnation;
		boolean groupInactive = false;

		// get group (may activate group)
		synchronized (this) {
		    inst = getInstantiator(groupID);
		    currentIncarnation = incarnation;
		}
		
		// activate object
		try {
		    return objEntry.activate(id, force, inst);
		} catch (NoSuchObjectException e) {
		    groupInactive = true;
		    if (detail == null) {
			detail = e;
		    }
		} catch (ConnectException e) {
		    groupInactive = true;
		    if (detail == null) {
			detail = e;
		    }
		} catch (ConnectIOException e) {
		    groupInactive = true;
		    if (detail == null) {
			detail = e;
		    }
		} catch (RemoteException e) {
		    // REMIND: do I want to wait some here before continuing?

		    if (detail == null) {
			detail = e;
		    }
		}
		
		if (groupInactive == true) {
		    // group has failed; mark inactive
		    try {
			getGroupEntry(groupID).
			    inactiveGroup(currentIncarnation);
		    } catch (Exception ex) {
			// REMIND: what do I do here?
		    }
		    continue;
		}
	    }

	    /** 
	     * signal that group activation failed, nested exception
	     * specifies what exception occurred when the group did not
	     * activate 
	     */
	    throw new ActivationException("object activation failed after " +
					  MAX_TRIES + " tries", detail);
	}

	/**
	 * Returns the instantiator for the group specified by id and
	 * entry. If the group is currently inactive, exec some
	 * bootstrap code to create the group.
	 */
	private	synchronized
	ActivationInstantiator getInstantiator(ActivationGroupID id)
	    throws UnknownGroupException, ActivationException
	{
	    checkRemoved();
	    boolean acquired = false;
	    boolean mustwait = false;

	    if (group != null)
		return group;
	    try {
		if (!pendingCreate) {
		    // argv is the literal command to exec
		    String[] argv = activationArgs(desc);
		    checkArgs(desc, argv);
		    
		    resetAll();
		    pendingCreate = true;
		    // group does not exist; exec a VM for the group
		    try {
			try {
			    Pstartgroup(); acquired = true;
			} catch (java.lang.InterruptedException e) {
			}

			if (debugExec) {
			    StringBuffer sb = new StringBuffer(argv[0]);
			    int j;
			    for (j = 1; j < argv.length; j++) {
				sb.append(' ');
				sb.append(argv[j]);
			    }
			    System.err.println("rmid: debugExec: running \"" +
				    sb.toString() + "\"");
			}

			child = Runtime.getRuntime().exec(argv);
			++incarnation;
			ensureRestart();


			OutputStream cout = child.getOutputStream();
			MarshalOutputStream out =
			    new MarshalOutputStream(cout);
			out.writeObject(id);
			out.writeObject(desc);
			out.writeLong(incarnation);
			out.flush();
			out.close();

			// retrieve outputs with annotations.
  			PipeWriter.plugTogetherPair
  			    (child.getInputStream(), System.out,
  			     child.getErrorStream(), System.err);

			// group state should be updated before log update
			addLogRecord
			    (new LogGroupIncarnation(id, incarnation));

		    } catch (IOException e) {
			pendingCreate = false;
			resetAll();
			throw new ActivationException("unable to create " +
						      "activation group", e);
		    }
		}
		
		try {
		    long now = System.currentTimeMillis();
		    long deadline = now + execTimeout;
		    do {
			wait(deadline - now);
			if (group != null || removed)
			    break;
			now = System.currentTimeMillis();
		    } while (now < deadline);
		} catch (java.lang.InterruptedException e) {
		}

		if (group == null) {
		    pendingCreate = false;
		    resetAll();
		    throw new ActivationException(
			(removed ?
			 "activation group unregistered" :
			 "timeout creating child process"));
		}
	    } finally {
		if (acquired)
		    Vstartgroup();
		acquired = false;
	    }
	
	    return group;
	}

	/**
	 * Create the thread to activate restartable objects if their group
	 * crashes.  If a restartable object's process was not created by
	 * rmid (i.e., there is no "child"), then the process is not monitored.
	 */
	private synchronized void ensureRestart() {

	    Thread thread = null;
	    
	    if (restartSet.size() > 0 &&
		restartThread == null &&
		child != null)
	    {
		restartThread = new RestartThread();
		restartThread.start();
	    }
	}

	private class RestartThread extends Thread {
	    private final Process groupProcess = child;
	    private final long groupIncarnation = incarnation;
	    private boolean canInterrupt = true;
	    private boolean shouldQuit = false;

	    RestartThread() {
		super("RMID RestartObjects-" + (++nextRestart));
		setDaemon(true);
	    }

	    /**
	     * The "ensureRestart" method creates this thread to restart
	     * services.  The thread is created after the group process is
	     * created. This thread exits if either of the following
	     * situations occur:
	     *    - if no more restartable objects in the group
	     *    - if the group is unregistered
	     *    - if group becomes inactive gracefully
	     *    - if rmid is shutdown
	     *    - after restarting objects when it detects the
	     *      group crashes
	     */
	    public void run() {

		if (shouldQuit)
		    return;

		/*
		 * Wait for the group to crash or exit.
		 */
		try {
		    groupProcess.waitFor();
		    /*
		     * Since the group crashed, we should
		     * reset the entry before activating objects
		     */
		    synchronized (GroupEntry.this) {
			/*
			 * Clear the interrupted state.
			 */
			Thread.interrupted();
			canInterrupt = false;
			if (shouldQuit)
			    return;

			if (groupIncarnation == incarnation)
			    reset();
		    }
		} catch (InterruptedException exit) {
		    /*
		     * This thread is disposed of if the group
		     * becomes inactive gracefully, is unregistered or
		     * shut down, or no more restartable objects are
		     * in the group. In this situation, we should
		     * simply exit the thread.
		     */
		    return;
		}
		
		/*
		 * Activate those objects that require restarting
		 * after a crash.
		 */
		restartServices();
	    }

	    /** 
	     * Marks this thread as one that is no longer needed.
	     * If the thread is in a state in which it can be interrupted,
	     * then the thread is interruptted.
	     */
	    void dispose() {
		/*
		 * This thread is disposed of if the group
		 * becomes inactive gracefully, is unregistered or
		 * shut down, or no more restartable objects are
		 * in the group. In this situation, we should
		 * simply exit the thread.
		 */
		shouldQuit = true;
		if (canInterrupt)
		    interrupt();
	    }
	}
    }
	
    private String[] activationArgs(ActivationGroupDesc desc) {
	ActivationGroupDesc.CommandEnvironment cmdenv;
	cmdenv = desc.getCommandEnvironment();

	// argv is the literal command to exec
	List argv = new ArrayList();

	// Command name/path
	argv.add(
		(cmdenv != null && cmdenv.getCommandPath() != null)
		    ? cmdenv.getCommandPath()
		    : command[0]);

	// Group-specific command options
	if (cmdenv != null && cmdenv.getCommandOptions() != null) {
	    argv.addAll(Arrays.asList(cmdenv.getCommandOptions()));
	}

	// Properties become -D parameters
	Properties props = desc.getPropertyOverrides();
	if (props != null) {
	    Enumeration p = props.propertyNames();
	    while (p.hasMoreElements()) {
		String name = (String)p.nextElement();
		argv.add("-D" + name + "=" +
			 props.getProperty(name));
		// Note on quoting: it would be wrong
		// here, since argv will be passed to
		// Runtime.exec, which should not parse
		// arguments or split on whitespace.
	    }
	}

	/* Finally, rmid-global command options (e.g. -C options)
	 * and the classname 
	 */
	int i;
	for (i = 1; i < command.length; i++) {
	    argv.add(command[i]);
	}

	String[] realArgv = new String[argv.size()];
	System.arraycopy(argv.toArray(), 0, realArgv, 0,
			 realArgv.length);

	return (realArgv);
    }

    private void checkArgs(ActivationGroupDesc desc, String[] cmd)
	throws SecurityException, ActivationException
    {
	/*
	 * Check exec command using execPolicy object
	 */
	if (execPolicyMethod != null) {
	    if (cmd == null) {
		cmd = activationArgs(desc);
	    }
	    Object[] args = new Object[] { desc, cmd };
	    try {
		execPolicyMethod.invoke(execPolicy, args);
	    } catch (InvocationTargetException e) {
		Throwable targetException = e.getTargetException();
		if (targetException instanceof SecurityException) {
		    throw (SecurityException) targetException;
		} else {
		    throw new ActivationException(
			execPolicyMethodName + ": unexpected exception", e);
		}
	    } catch (Exception e) {
		throw new ActivationException(
		    execPolicyMethodName + ": unexpected exception", e);
	    }
	}
    }

    private static class ObjectEntry implements Serializable {
	/** descriptor for object */
	ActivationDesc desc;
	/** the stub (if active) */
	transient MarshalledObject stub = null;
	transient boolean removed = false;

	ObjectEntry(ActivationDesc desc) {
	    this.desc = desc;
	}

	synchronized MarshalledObject activate(ActivationID id,
					       boolean force,
					       ActivationInstantiator inst)
    	    throws RemoteException, ActivationException
	{
	    if (removed == true)
		throw new UnknownObjectException("object removed");

	    if (force != true && stub != null)
		return stub;
		
	    MarshalledObject newstub = inst.newInstance(id, desc);
	    stub = newstub;
	    /*
	     * stub could be set to null by a group reset, so return
	     * the newstub here to prevent returning null.
	     */
	    return newstub;
	}
	
	void reset() {
	    stub = null;
	}
    }

    /**
     * Add a record to the activation log. If the number of updates
     * passes a predetermined threshold, record a snapshot.
     */
    private synchronized void addLogRecord(LogRecord rec)
	throws ActivationException
    {
	Exception e = null;
	
	try {
	    log.update(rec, true);
	} catch (Exception ex) {
	    e = ex;
	}
	
	try {
	    if ((++numUpdates >= snapshotInterval) || (e != null)) {
		log.snapshot(this);
		numUpdates = 0;
	    }

 	    if (e != null) {
		/*
		 * Fix 4283101: rmid should print a warning message if
		 * a log update fails and snapshot succeeds
		 */
		System.err.println("\nrmid: warning, could not apply log " +
				   "record update but snapshot succeeded.");
		System.err.println("\nrmid: state is consistent. update " +
				   "exception was: ");
		e.printStackTrace();
	    }
	} catch (Exception ex) {
	    /*
	     * Fix for 4183169: Minor problem with the way
	     * ReliableLog handles IOExceptions.  Gracefully
	     * shutdown the activation system when it cannot
	     * snapshot its state.
	     */
	    System.err.println("\nrmid: could not update update " +
			       "or snapshot log file");
	    System.err.println("rmid: the activation system is " +
			       "shutting down");
	    
	    if (e != null) {
		System.err.println("rmid: exception occurred during " +
				   "log update:");
		e.printStackTrace();
	    }
	    System.err.println("rmid: exception occurred during log snapshot:");	    
	    ex.printStackTrace();

	    try {
		((ActivationSystemImpl) system).shutdown();
	    } catch (AccessException ignore) {
		// can't happen
	    }
	    
	    // warn the client of the original update problem
	    throw new ActivationException("rmid could not update its " +
					  "log file", e);
	}
    }

    /**
     * Handler for the log that knows how to take the initial snapshot
     * and apply an update (a LogRecord) to the current state.
     */
    private static class ActLogHandler extends LogHandler {
	
        ActLogHandler() {
	}
	
	public Object initialSnapshot() 
	{
	    /**
	     * Return an empty Activation object.  Log will update
	     * this object with recovered state.
	     */
	    return new Activation();
	}

	public Object applyUpdate(Object update, Object state)
    	    throws Exception
	{
	    return ((LogRecord)update).apply(state);
	}
	
    }

    /**
     * Abstract class for all log records. The subclass contains
     * specific update information and implements the apply method
     * that applys the update information contained in the record
     * to the current state.
     */
    private static abstract class LogRecord implements Serializable {
	/** indicate compatibility with JDK 1.2 version of class */
	private static final long serialVersionUID = 8395140512322687529L;
	abstract Object apply(Object state) throws Exception;
    }

    /**
     * Log record for registering an object.
     */
    private static class LogRegisterObject extends LogRecord {
	/** indicate compatibility with JDK 1.2 version of class */
	private static final long serialVersionUID = -6280336276146085143L;
	private ActivationID id;
	private ActivationDesc desc;

	LogRegisterObject(ActivationID id, ActivationDesc desc) {
	    this.id = id;
	    this.desc = desc;
	}
	
	Object apply(Object state) {
	    try {
		((Activation) state).getGroupEntry(desc.getGroupID()).
		    registerObject(id, desc, false);
	    } catch (Exception ignore) {
		System.err.println("LogRegisterObject: skipping...");
		ignore.printStackTrace();
	    }
	    return state;
	}
    }

    /**
     * Log record for unregistering an object.
     */
    private static class LogUnregisterObject extends LogRecord {
	/** indicate compatibility with JDK 1.2 version of class */
	private static final long serialVersionUID = 6269824097396935501L;
	private ActivationID id;

	LogUnregisterObject(ActivationID id) {
	    this.id = id;
	}
	
	Object apply(Object state) {
	    try {
		((Activation)state).getGroupEntry(id).
		    unregisterObject(id, false);
	    } catch (Exception ignore) {
		System.err.println("LogUnregisterObject: skipping...");
		ignore.printStackTrace();
	    }
	    return state;
	}
    }

    /**
     * Log record for registering a group.
     */
    private static class LogRegisterGroup extends LogRecord {
	/** indicate compatibility with JDK 1.2 version of class */
	private static final long serialVersionUID = -1966827458515403625L;
	private ActivationGroupID id;
	private ActivationGroupDesc desc;

	LogRegisterGroup(ActivationGroupID id, ActivationGroupDesc desc) {
	    this.id = id;
	    this.desc = desc;
	}

	Object apply(Object state) {
	    // modify state directly; cant ask a nonexistent GroupEntry
	    // to register itself.
	    ((Activation)state).groupTable.put(id, ((Activation) state).new
					       GroupEntry(id, desc));
	    return state;
	}
    }

    /**
     * Log record for udpating an activation desc
     */
    private static class LogUpdateDesc extends LogRecord {
	/** indicate compatibility with JDK 1.2 version of class */
	private static final long serialVersionUID = 545511539051179885L;

	private ActivationID id;
	private ActivationDesc desc;

	LogUpdateDesc(ActivationID id, ActivationDesc desc) {
	    this.id = id;
	    this.desc = desc;
	}
	
	Object apply(Object state) {
	    try {
		((Activation) state).getGroupEntry(id).
		    setActivationDesc(id, desc, false);
	    } catch (Exception ignore) {
		System.err.println("LogUpdateDesc: skipping...");
		ignore.printStackTrace();
	    }
	    return state;
	}
    }
    
    /**
     * Log record for unregistering a group.
     */
    private static class LogUpdateGroupDesc extends LogRecord {
	/** indicate compatibility with JDK 1.2 version of class */
	private static final long serialVersionUID = -1271300989218424337L;
	private ActivationGroupID id;
	private ActivationGroupDesc desc;

	LogUpdateGroupDesc(ActivationGroupID id, ActivationGroupDesc desc) {
	    this.id = id;
	    this.desc = desc;
	}
	
	Object apply(Object state) {
	    try {
		((Activation) state).getGroupEntry(id).
		    setActivationGroupDesc(id, desc, false);
	    } catch (Exception ignore) {
		System.err.println("LogUpdateGroupDesc: skipping...");
		ignore.printStackTrace();
	    }
	    return state;
	}
    }
    
    /**
     * Log record for unregistering a group.
     */
    private static class LogUnregisterGroup extends LogRecord {
	/** indicate compatibility with JDK 1.2 version of class */
	private static final long serialVersionUID = -3356306586522147344L;
	private ActivationGroupID id;

	LogUnregisterGroup(ActivationGroupID id) {
	    this.id = id;
	}
	
	Object apply(Object state) {
	    GroupEntry entry = (GroupEntry)
		((Activation)state).groupTable.remove(id);
	    try {
	    	entry.unregisterGroup(false);
	    } catch (Exception ignore) {	
		System.err.println("LogUnregisterGroup: skipping...");
		ignore.printStackTrace();
	    }
	    return state;
	}
    }

    /**
     * Log record for an active group incarnation
     */
    private static class LogGroupIncarnation extends LogRecord {
	/** indicate compatibility with JDK 1.2 version of class */
	private static final long serialVersionUID = 4146872747377631897L;
	private ActivationGroupID id;
	private long inc;

	LogGroupIncarnation(ActivationGroupID id, long inc) {
	    this.id = id;
	    this.inc = inc;
	}

	Object apply(Object state) {
	    try {
		GroupEntry entry = ((Activation)state).getGroupEntry(id);
		entry.incarnation = inc;
	    } catch (Exception ignore) {
		System.err.println("LogUnregisterGroup: skipping...");
		ignore.printStackTrace();
	    }
	    return state;
	}
    }
    
    /**
     * Initialize command to exec a default group.
     */
    private void initCommand(String[] childArgs) {
	command = new String[childArgs.length + 2];
	java.security.AccessController.doPrivileged(new PrivilegedAction() {
	    public Object run() {
		try {
		    command[0] = System.getProperty("java.home") +
			File.separator + "bin" + File.separator + "java";
		} catch (Exception e) {
		    System.err.println(getTextResource("rmid.unfound.java.home."
						+ "property"));
		    command[0] = "java";
		}
		return null;
	    }
	});
	System.arraycopy(childArgs, 0, command, 1, childArgs.length);
	command[command.length-1] = "sun.rmi.server.ActivationGroupInit";
    }

    private static void bomb(String error) {
	System.err.println("rmid: " + error); // $NON-NLS$
 	System.err.println(MessageFormat.format(getTextResource("rmid.usage"),
		    new String[] {"rmid"}));
	System.exit(1);
    }

    /**
     * The default policy for checking a command before it is executed
     * makes sure the appropriate com.sun.rmi.rmid.ExecPermission and
     * set of com.sun.rmi.rmid.ExecOptionPermissions have been granted.
     */
    public static class DefaultExecPolicy {

	public void checkExecCommand(ActivationGroupDesc desc, String[] cmd)
    	    throws SecurityException
	{
	    PermissionCollection perms = getExecPermissions();
	    
	    /*
	     * Check properties overrides.
	     */
	    Properties props = desc.getPropertyOverrides();
	    if (props != null) {
		Enumeration p = props.propertyNames();
		while (p.hasMoreElements()) {
		    String name = (String) p.nextElement();
		    String option =
			"-D" + name + "=" + props.getProperty(name);
		    checkPermission(perms, new ExecOptionPermission(option));
		}
	    }

	    /*
	     * Check group class name (allow nothing but the default),
	     * code location (must be null), and data (must be null).
	     */
	    String groupClassName = desc.getClassName();
	    if ((groupClassName != null &&
		 !groupClassName.equals(
		    ActivationGroupImpl.class.getName())) ||
		(desc.getLocation() != null) ||
		(desc.getData() != null))
	    {
		throw new AccessControlException(
		    "access denied (custom group implementation not allowed)");
	    }
	    
	    /*
	     * If group descriptor has a command environment, check
	     * command and options.
	     */
	    ActivationGroupDesc.CommandEnvironment cmdenv;
	    cmdenv = desc.getCommandEnvironment();
	    if (cmdenv != null) {
		String path = cmdenv.getCommandPath();
		if (path != null) {
		    checkPermission(perms, new ExecPermission(path));
		}

		String[] options = cmdenv.getCommandOptions();
		if (options != null) {
		    for (int i = 0; i < options.length; i++) {
			checkPermission(perms,
					new ExecOptionPermission(options[i]));
		    }
		}
	    }
	}
	
	/**
	 * Prints warning message if installed Policy is the default Policy
	 * implementation and globally granted permissions do not include
	 * AllPermission or any ExecPermissions/ExecOptionPermissions.
	 */
	static void checkConfiguration() {
	    Policy policy = (Policy) 
		java.security.AccessController.doPrivileged(
		    new java.security.PrivilegedAction() {
			public Object run() {
			    return java.security.Policy.getPolicy(); 
			}
		    });
	    if (!(policy instanceof sun.security.provider.PolicyFile)) {
		return;
	    }
	    PermissionCollection perms = getExecPermissions();
	    for (Enumeration e = perms.elements(); e.hasMoreElements();) {
		Permission p = (Permission) e.nextElement();
		if (p instanceof AllPermission ||
		    p instanceof ExecPermission ||
		    p instanceof ExecOptionPermission)
		{
		    return;
		}
	    }
	    System.err.println(getTextResource("rmid.exec.perms.inadequate"));
	}

	private static PermissionCollection getExecPermissions() {
	    /*
	     * The approach used here is taken from the similar method
	     * getLoaderAccessControlContext() in the class
	     * sun.rmi.server.LoaderHandler.
	     */

	    // obtain permissions granted to all code in current policy
	    PermissionCollection perms = (PermissionCollection)
		java.security.AccessController.doPrivileged(
		    new java.security.PrivilegedAction() {
		    public Object run() {
			CodeSource codesource = new CodeSource(null, null);
			Policy p = java.security.Policy.getPolicy();
			if (p != null) {
			    return p.getPermissions(codesource);
			} else {
			    return new Permissions();
			}
		    }
		});

	    return perms;
	}
	
	private static void checkPermission(PermissionCollection perms,
					    Permission p)
	    throws AccessControlException
	{
	    if (!perms.implies(p)) {
		throw new AccessControlException(
		   "access denied " + p.toString());
	    }
	}
    }

    /**
     * Main program to start the activation system. <br>
     * The usage is as follows: rmid [-port num] [-log dir].
     */
    public static void main(String[] args) {
	boolean stop = false;

	// Create and install the security manager if one is not installed
	// already.
	if (System.getSecurityManager() == null) {
	    System.setSecurityManager(new RMISecurityManager());
	}
	
	try {
	    int port = ActivationSystem.SYSTEM_PORT;
	    String log = "log";
	    ArrayList childArgs = new ArrayList();

	    /*
	     * Parse arguments
	     */
	    for (int i = 0; i< args.length ; i ++) {
		if (args[i].equals("-port")) {
		    if ((i + 1) < args.length) {
			try {
			    port = Integer.parseInt(args[++i]);
			} catch (NumberFormatException nfe) {
			    bomb(getTextResource("rmid.syntax.port.badnumber"));
			}
		    } else {
			bomb(getTextResource("rmid.syntax.port.missing"));
		    }
		    
		} else if (args[i].equals("-log")) {
		    if ((i + 1) < args.length) {
			log = args[++i];
		    } else {
			bomb(getTextResource("rmid.syntax.log.missing"));
		    }
		    
		} else if (args[i].equals("-stop")) {
		    stop = true;
		
		} else if (args[i].startsWith("-C")) {
		    childArgs.add(args[i].substring(2));
		    
		} else {
		    Object[] bad = new Object[1];
		    bad[0] = args[i];
		    bomb (MessageFormat.format(
		               getTextResource("rmid.syntax.illegal.option"), bad));
		}
	    }

	    Boolean tmp;
	    tmp = (Boolean) java.security.AccessController.doPrivileged(
		new GetBooleanAction("sun.rmi.server.activation.debugExec"));
	    debugExec = tmp.booleanValue();

	    /**
	     * Determine class name for activation exec policy (if any).
	     */
	    String execPolicyClassName = (String)
		java.security.AccessController.doPrivileged(
		    new GetPropertyAction("sun.rmi.activation.execPolicy",
					  null));
	    if (execPolicyClassName == null) {
		if (!stop) {
		    DefaultExecPolicy.checkConfiguration();
		}
		execPolicyClassName = "default";
	    }
				   
	    /**
	     * Initialize method for activation exec policy.
	     */
	    if (!execPolicyClassName.equals("none")) {
		if (execPolicyClassName.equals("") ||
		    execPolicyClassName.equals("default"))
		{
		    execPolicyClassName = DefaultExecPolicy.class.getName();
		}
		
		try {
		    Class execPolicyClass =
			RMIClassLoader.loadClass(execPolicyClassName);
		    execPolicy = execPolicyClass.newInstance();
		    execPolicyMethod =
			execPolicyClass.getMethod(execPolicyMethodName,
						  execPolicyMethodParamTypes);
		} catch (Exception e) {
		    if (debugExec) {
			System.err.println(
			    "rmid: exception obtaining exec policy");
			e.printStackTrace();
		    }
		    bomb(getTextResource("rmid.exec.policy.invalid"));
		}
	    }

	    if (stop == true) {
		final int finalPort = port;
		java.security.AccessController.doPrivileged(
		    new PrivilegedAction() {
		    public Object run() {
			System.setProperty("java.rmi.activation.port",
					   Integer.toString(finalPort));
			return null;
		    }
		});
		ActivationSystem system = ActivationGroup.getSystem();
		system.shutdown();
		System.exit(0);
	    }

	    /*
	     * Start internal registry and the whole activation system.
	     */
	    registry = java.rmi.registry.LocateRegistry.createRegistry(port);

	    /*
	     * Fix for 4173960: Create and initialize activation using
	     * a static method, startActivation, which will build the
	     * Activation state in two ways: if when rmid is run, no
	     * log file is found, the ActLogHandler.recover(...)
	     * method will create a new Activation instance.
	     * Alternatively, if a logfile is available, a serialized
	     * instance of activation will be read from the log's
	     * snapshot file.  Log updates will be applied to this
	     * Activation object until rmid's state has been fully
	     * recovered.  In either case, only one instance of
	     * Activation is created.  
	     */
	    startActivation(port, log,
	        (String[])childArgs.toArray(new String[childArgs.size()]));

	    // prevent activator from exiting
	    while (true) {
		try {
		    Thread.sleep(Long.MAX_VALUE);
		} catch (InterruptedException e) {
		}
	    }
	} catch (Exception e) {
	    Object[] bad = new Object[1];
	    bad[0] = e.getMessage();
	    System.err.println(MessageFormat.format(
	                            getTextResource("rmid.unexpected.exception"), bad));
	    e.printStackTrace();
	}
	System.exit(1);
    }

    /**
     * Retrieves text resources from the locale-specific properties file.
     */
    private static String getTextResource(String key) {
	if (Activation.resources == null) {
	    try {
		Activation.resources = ResourceBundle.getBundle(
		    "sun.rmi.rmid.resources.rmid");
	    } catch (MissingResourceException mre) {
	    }
	    if (Activation.resources == null) {
		// throwing an Error is a bit extreme, methinks
		return ("[missing resource file: " + key + "]");
	    }
	}

	String val = null;
	try {
	    val = Activation.resources.getString (key);
	} catch (MissingResourceException mre) {
	}
	
	if (val == null)
	    return ("[missing resource: " + key + "]");
	else
	    return (val);
    }

    /*
     * Dijkstra semaphore operations to limit the number of subprocesses
     * rmid attempts to make at once.
     */

    /**
     * Acquire the startgroup semaphore.  Each Pstartgroup must be followed
     * by a Vstartgroup.  The calling thread will wait until there are
     * fewer than <code>N</code> other threads holding the startgroup
     * semaphore.  The calling thread will then acquire the semaphore and
     * return. <p>
     * <code>N</code> is determined by the value of the system property
     * <code>"sun.rmi.rmid.maxstartgroup"</code> (default 3).
     */
    private static void Pstartgroup() throws InterruptedException {
	synchronized (sem_startgroup) {
	    // Wait until positive, then decrement.
	    while (sem_startgroup[0] <= 0)
		sem_startgroup.wait();
	    sem_startgroup[0]--;
	}
    }

    /**
     * Release the startgroup semaphore.  Every P operation must be
     * followed by a V operation.  This may cause another thread to
     * wake up and return from its P operation.
     */
    private static void Vstartgroup() {
	synchronized (sem_startgroup) {
	    // Increment and notify a waiter (not necessarily FIFO).
	    sem_startgroup[0]++;
	    sem_startgroup.notifyAll();
	}
    }

    private static int[] sem_startgroup;

    static {
	Integer n = (Integer) java.security.AccessController.doPrivileged(
	    new GetIntegerAction("sun.rmi.rmid.maxstartgroup", 3));
	int[] na = new int[1];
	na[0] = n.intValue();
	sem_startgroup = na;
    }
}

/**
 * PipeWriter plugs together two pairs of input and output streams by
 * providing readers for input streams and writing through to
 * appropriate output streams.  Both output streams are annotated on a
 * per-line basis.
 *
 * @author Laird Dornin, much code borrowed from Peter Jones, Ken
 *         Arnold and Ann Wollrath.  
 */
class PipeWriter implements Runnable {

    /** stream used for buffering lines */
    private ByteArrayOutputStream bufOut;

    /** count since last separator */
    private int cLast;  
    
    /** current chunk of input being compared to lineSeparator.*/
    private byte[] currSep;
    
    private PrintWriter out;
    private InputStream in;
    
    private String pipeString;
    private String execString;
    
    private static String lineSeparator;
    private static int lineSeparatorLength;

    private static int numExecs = 0;

    static {
	lineSeparator =	(String) java.security.AccessController.doPrivileged(
                new sun.security.action.GetPropertyAction("line.separator"));
	lineSeparatorLength = lineSeparator.length();
    }

    /**
     * Create a new PipeWriter object. All methods of PipeWriter,
     * except plugTogetherPair, are only accesible to PipeWriter
     * itself.  Synchronization is unnecessary on functions that will
     * only be used internally in PipeWriter.
     *
     * @param in input stream from which pipe input flows
     * @param out output stream to which log messages will be sent
     * @param dest String which tags output stream as 'out' or 'err'
     * @param nExecs number of execed processes, Activation groups.
     */
    private PipeWriter
	(InputStream in, OutputStream out, String tag, int nExecs) {

	this.in = in;
	this.out = new PrintWriter(out);
	
	bufOut = new ByteArrayOutputStream();
	currSep = new byte[lineSeparatorLength];
	
	/* set unique pipe/pair annotations */
	execString = ":ExecGroup-" +
	    Integer.toString(nExecs) + ':' + tag + ':';
    }
    
    /**
     * Create a thread to listen and read from input stream, in.  buffer
     * the data that is read until a marker which equals lineSeparator
     * is read.  Once such a string has been discovered; write out an
     * annotation string followed by the buffered data and a line
     * separator.  
     */
    public void run() {
	byte[] buf = new byte[256];
	int count;
	
	try {
	    /* read bytes till there are no more. */
	    while ((count = in.read(buf)) != -1) {
		write(buf, 0, count);
	    }

	    /*  flush internal buffer... may not have ended on a line
             *  separator, we also need a last annotation if 
             *  something was left.
	     */
	    String lastInBuffer = bufOut.toString();
	    bufOut.reset();
	    if (lastInBuffer.length() > 0) {
		out.println (createAnnotation() + lastInBuffer);    
		out.flush();                    // add a line separator
                                                // to make output nicer
	    }

	} catch (IOException e) {
	}
    }
    
    /**
     * Write a subarray of bytes.  Pass each through write byte method.
     */
    private void write(byte b[], int off, int len) throws IOException {

	if (len < 0) {
	    throw new ArrayIndexOutOfBoundsException(len);
	}
	for (int i = 0; i < len; ++ i) {
	    write(b[off + i]);
	}
    }

    /**
     * Write a byte of data to the stream.  If we have not matched a
     * line separator string, then the byte is appended to the internal
     * buffer.  If we have matched a line separator, then the currently
     * buffered line is sent to the output writer with a prepended 
     * annotation string.
     */
    private void write(byte b) throws IOException {
	int i = 0;
	
	/* shift current to the left */
	for (i = 1 ; i < (currSep.length); i ++) {
	    currSep[i-1] = currSep[i];
	}
	currSep[i-1] = b;
	bufOut.write(b);
	
	/* enough characters for a separator? */
	if ( (cLast >= (lineSeparatorLength - 1)) &&
	     (lineSeparator.equals(new String(currSep))) ) {
	    
	    cLast = 0;
	    
	    /* write prefix through to underlying byte stream */
	    out.print(createAnnotation() + bufOut.toString());
	    out.flush();
	    bufOut.reset();
	    
	    if (out.checkError()) {
		throw new IOException
		    ("PipeWriter: IO Exception when"+
		     " writing to output stream.");
	    }
	    
	} else {
	    cLast++;
	}
    }
    
    /** 
     * Create an annotation string to be printed out after
     * a new line and end of stream.
     */
    private String createAnnotation() {
	
	/* construct prefix for log messages:
	 * date/time stamp...
	 */
	return ((new Date()).toString()  +
		 /* ... print pair # ... */
		 (execString));
    }

    /**
     * Allow plugging together two pipes at a time, to associate
     * output from an execed process.  This is the only publicly
     * accessible method of this object; this helps ensure that
     * synchronization will not be an issue in the annotation
     * process.
     *
     * @param in input stream from which pipe input comes
     * @param out output stream to which log messages will be sent
     * @param in1 input stream from which pipe input comes
     * @param out1 output stream to which log messages will be sent
     */
    static void plugTogetherPair(InputStream in,
				 OutputStream out,
				 InputStream in1,
				 OutputStream out1) {
	Thread inThread = null;
	Thread outThread = null;

	int nExecs = getNumExec();

	/* start RMI threads to read output from child process */
	inThread = (Thread) java.security.AccessController.doPrivileged(
	    new NewThreadAction(new PipeWriter(in, out, "out", nExecs),
				"out", true));
	outThread = (Thread) java.security.AccessController.doPrivileged(
	    new NewThreadAction(new PipeWriter(in1, out1, "err", nExecs),
				"err", true));
	inThread.start();
	outThread.start();
    }

    private static synchronized int getNumExec() {
	return numExecs++;
    }
}
