/*
 * @(#)Pool.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jndi.ldap.pool;

import java.util.Map;
import java.util.WeakHashMap;
import java.util.Collection;
import java.util.Iterator;
import java.util.Set;
import java.io.PrintStream;
import javax.naming.NamingException;

/**
 * A map of pool ids to Connections.
 * Key is an object that uniquely identifies a PooledConnection request
 * (typically information needed to create the connection).
 * The definitions of the key's equals() and hashCode() methods are
 * vital to its unique identification in a Pool.
 *
 * Value is a ConnectionsRef, which is a reference to Connections, 
 * a list of equivalent connections.
 *
 * Supports methods that
 * - retrieves (or creates as necessary) a connection from the pool
 * - removes expired connections from the pool
 *
 * @author Rosanna Lee
 */

final public class Pool {
    static final boolean debug = com.sun.jndi.ldap.LdapPoolManager.debug;

    final private int maxSize;    // max num of identical conn per pool
    final private int prefSize;   // preferred num of identical conn per pool
    final private int initSize;   // initial number of identical conn to create
    final private Map map;

    public Pool(int initSize, int prefSize, int maxSize) {
	map = new WeakHashMap();
	this.prefSize = prefSize;
	this.maxSize = maxSize;
	this.initSize = initSize;
    }

    /**
     * Gets a pooled connection for id. The pooled connection might be
     * newly created, as governed by the maxSize and prefSize settings.
     * If a pooled connection is unavailable and cannot be created due
     * to the maxSize constraint, this call blocks until the constraint
     * is removed or until 'timeout' ms has elapsed.
     *
     * @param id identity of the connection to get
     * @param timeout the number of milliseconds to wait before giving up
     * @param factory the factory to use for creating the connection if 
     * 	      creation is necessary
     * @return a pooled connection
     * @throws NamingException the connection could not be created due to 
     *   an error.
     */
    public PooledConnection getPooledConnection(Object id, long timeout, 
	PooledConnectionFactory factory) throws NamingException {

	d("get(): ", id);
	d("size: ", map.size());

	Connections conns;
	synchronized (map) {
	    conns = getConnections(id);
	    if (conns == null) {
		d("get(): creating new connections list for ", id);

		// No connections for this id so create a new list
		conns = new Connections(id, initSize, prefSize, maxSize, 
		    factory);
		map.put(id, new ConnectionsRef(conns));
	    }
	}
	d("get(): size after: ", map.size());

	return conns.get(timeout, factory); // get one connection from list
    }

    private Connections getConnections(Object id) {
	ConnectionsRef ref = (ConnectionsRef) map.get(id);
	return (ref != null) ? ref.getConnections() : null;
    }

    /**
     * Goes through the connections in this Pool and expires ones that
     * have been idle before 'threshold'. An expired connection is closed
     * and then removed from the pool (removePooledConnection() will eventually
     * be called, and the list of pools itself removed if it becomes empty).
     *
     * @param threshold connections idle before 'threshold' should be closed
     *                  and removed.
     */
    public void expire(long threshold) {
	synchronized (map) {
	    Collection coll = map.values();
	    Iterator iter = coll.iterator();
	    Connections conns;
	    while(iter.hasNext()) {
		conns = ((ConnectionsRef) (iter.next())).getConnections();
		if (conns.expire(threshold)) {
		    d("expire(): removing ", conns);
		    iter.remove();
		}
	    }
	}
    }

    public void showStats(PrintStream out) {
	Map.Entry entry;
	Object id;
	Connections conns;

	out.println("===== Pool start ======================");
	out.println("maximum pool size: " + maxSize);
	out.println("preferred pool size: " + prefSize);
	out.println("initial pool size: " + initSize);
	out.println("current pool size: " + map.size());

	Set entries = map.entrySet();
	Iterator iter = entries.iterator();

	while(iter.hasNext()) {
	    entry = (Map.Entry) iter.next();
	    id = entry.getKey();
	    conns = ((ConnectionsRef) entry.getValue()).getConnections();
	    out.println("   " + id + ":" + conns.getStats());
	}

	out.println("====== Pool end =====================");
    }

    public String toString() {
	return super.toString() + map.toString();
    }

    private void d(String msg, int i) {
	if (debug) {
	    System.err.println(this + "." + msg + i);
	}
    }
    private void d(String msg, Object obj) {
	if (debug) {
	    System.err.println(this + "." + msg + obj);
	}
    }
}
