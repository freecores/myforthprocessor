/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)CacheTable.java	1.7 03/06/24
 *
 * Portions Copyright 2002 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 * 
 * ===========================================================================
 *  IBM Confidential
 *  OCO Source Materials
 *  Licensed Materials - Property of IBM
 * 
 *  (C) Copyright IBM Corp. 1999 All Rights Reserved.
 * 
 *  The source code for this program is not published or otherwise divested of
 *  its trade secrets, irrespective of what has been deposited with the U.S.
 *  Copyright Office.
 * 
 *  Copyright 1997 The Open Group Research Institute.  All rights reserved.
 * ===========================================================================
 * 
 */

package sun.security.krb5.internal.rcache;

import java.util.Hashtable;
/**
 * This class implements Hashtable to store the replay caches.
 *
 * @author Yanni Zhang
 * @version 1.00 10 Jul 00
 */
public class CacheTable extends Hashtable {
    private boolean DEBUG = sun.security.krb5.internal.Krb5.DEBUG; 
    public CacheTable () {
    }

    /**
     * Puts the client timestamp in replay cache.
     * @params principal the client's principal name.
     * @params time authenticator timestamp.
     */
    public synchronized void put(String principal, AuthTime time) {
        ReplayCache rc = (ReplayCache)super.get(principal);
        if (rc == null) {
            if (DEBUG) {
                System.out.println("replay cache for " + principal + " is null.");
            }
            rc = new ReplayCache(principal, this); 
            rc.put(time);
            super.put(principal, rc);
        }
        else {
            rc.put(time);
            if (DEBUG) {
                System.out.println("replay cache found.");
            }
        }
            
    }

    /**
     * This method tests if replay cache keeps a record of the authenticator's time stamp.
     * If there is a record (replay attack detected), the server should reject the client request.
     * @params principal the client's principal name.
     * @params time authenticator timestamp.
     * @return null if no record found, else return an <code>AuthTime</code> object.
     */
    public Object get(AuthTime time, String principal) {
        ReplayCache rc = (ReplayCache)super.get(principal);
        if ((rc != null) && (rc.contains(time))) {
            return time;
        }
        return null;
    }
}
