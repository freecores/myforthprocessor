/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */


package com.sun.net.ssl.internal.ssl;

import java.io.*;
import java.net.*;
import java.util.Date;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.NoSuchElementException;
import java.util.Vector;

import javax.net.ssl.SSLSession;
import javax.net.ssl.SSLSessionContext;
import javax.net.ssl.SSLSessionBindingListener;
import javax.net.ssl.SSLSessionBindingEvent;
import javax.net.ssl.SSLPeerUnverifiedException;
import javax.net.ssl.SSLSession;

import sun.misc.Cache;


final class SSLSessionContextImpl implements SSLSessionContext
{
    private Cache	sessionCache = new Cache();
    private Cache	sessionHostPortCache = new Cache();
    private int		cacheLimit;
    private long	timeoutMillis;
    private static final Debug debug = Debug.getInstance("ssl");

    // file private
    SSLSessionContextImpl()
    {
	cacheLimit = getCacheLimit();
	timeoutMillis = 86400000; // default, 24 hours
    }

    /**
     * Returns the SSL session object associated with the
     * specific session ID passed.
     */
    public SSLSession	getSession(byte[] id)
    {
	SSLSession sess = (SSLSession) sessionCache.get(
                		new SessionId(id));
        return checkTimeValidity(sess);
    }

    /**
     * Returns an enumeration of the active SSL sessions.
     */
    public Enumeration	getIds() {
	Vector v = new Vector(sessionCache.size());
	SessionId sessId;

	for (Enumeration e = sessionCache.keys(); e.hasMoreElements(); ) {
	    sessId = (SessionId) e.nextElement();
	    if (!isTimedout((SSLSession)sessionCache.get(sessId)))
		v.addElement(sessId.getId());
	}
	return v.elements();
    }

    public void setSessionTimeout(int seconds)
		 throws IllegalArgumentException {
	if (seconds < 0)
	    throw new IllegalArgumentException();
        timeoutMillis = seconds * 1000;
    }

    public int getSessionTimeout() {
	return (int) (timeoutMillis / 1000);
    }

    public void setSessionCacheSize(int size)
		 throws IllegalArgumentException {
	if (size < 0)
	    throw new IllegalArgumentException();
	cacheLimit = size;

	/**
         * If cache size limit is reduced, when the cache is full to its
         * previous limit, trim the cache before its contents 
	 * are used.
         */
        if ((cacheLimit != 0) && (sessionCache.size() > cacheLimit))
            adjustCacheSizeTo(cacheLimit);
    }

    public int getSessionCacheSize() {
	return cacheLimit;
    }

    SSLSessionImpl get(byte[] id) {
	return (SSLSessionImpl) getSession(id);
    }

    /**
     * Returns the SSL session object associated with the
     * specific host name and port number passed.
     */
    SSLSessionImpl get(String hostname, int port) {
        SSLSession sess =  (SSLSessionImpl) sessionHostPortCache
				.get(getKey(hostname, port));
        return (SSLSessionImpl) checkTimeValidity(sess);
    }

    private String getKey(String hostname, int port) {
	return (hostname + ":" + String.valueOf(port))
			.toLowerCase();
    }

    void put(SSLSessionImpl s) {

	// make space for the new session to be added
	if ((cacheLimit != 0) && (sessionCache.size() >= cacheLimit))
            adjustCacheSizeTo(cacheLimit - 1);
	sessionCache.put(s.getSessionId(), s);
        sessionHostPortCache.put(getKey(s.getPeerHost(), s.getPeerPort()), s);
	s.setContext(this);
    }

    private void adjustCacheSizeTo(int targetSize) {

	int cacheSize = sessionCache.size();
	
	if (targetSize < 0)
	   return;

        while (cacheSize > targetSize) {
            SSLSessionImpl lru = null;
            SSLSessionImpl s = null;
            Enumeration e;

            if (debug != null && Debug.isOn("sessioncache")) {
                System.out.println("exceeded cache limit of " + cacheLimit);
            }

	    /*
	     * Count the number of elements in the cache. The size() method
	     * does not reflect the cache entries that are no longer available,
	     * i.e entries that are garbage collected (the cache entries are
	     * held using soft references and are garbage collected when not
	     * in use).
	     */
	    int count;
            for (count = 0, e = sessionCache.elements();
			 e.hasMoreElements(); count++) {
                try {
                    s = (SSLSessionImpl)e.nextElement();
                } catch (NoSuchElementException nsee) {
                    break;
                }
                if (isTimedout(s)) {
                    lru = s;
                    break;
                } else if ((lru == null) || (s.getLastAccessedTime()
                         < lru.getLastAccessedTime())) {
                    lru = s;
                }
            }
            if ((lru != null) && (count > targetSize)) {
                if (debug != null && Debug.isOn("sessioncache")) {
                    System.out.println("uncaching " + lru);
                }
                lru.invalidate();
		count--; // element removed from the cache
            }
	    cacheSize = count;
        }
    }

    // file private
    void remove(SessionId key)
    {
	SSLSessionImpl s = (SSLSessionImpl) sessionCache.get(key);
	sessionCache.remove(key);
	sessionHostPortCache.remove(getKey(s.getPeerHost(),
					 s.getPeerPort()));
    }

    private int getCacheLimit() {
	int cacheLimit = 0;
	try {
        String s =
            (String) java.security.AccessController.doPrivileged(
                         new java.security.PrivilegedAction() {
                         public Object run() {
                             return System.getProperty(
				"javax.net.ssl.sessionCacheSize");
                         }
            });
            cacheLimit = (s != null) ? Integer.valueOf(s).intValue() : 0;
	} catch (Exception e) {
	}

	return (cacheLimit > 0) ? cacheLimit : 0;
    }

    SSLSession checkTimeValidity(SSLSession sess) {
        if (isTimedout(sess)) {
            sess.invalidate();
            return null;
        } else
            return sess;
    }

    boolean isTimedout(SSLSession sess) {
	if (timeoutMillis == 0)
	    return false;
        if ((sess != null) &&
            ((sess.getCreationTime() + timeoutMillis)
		<= (System.currentTimeMillis())))
            return true;
        return false;
    }
}
