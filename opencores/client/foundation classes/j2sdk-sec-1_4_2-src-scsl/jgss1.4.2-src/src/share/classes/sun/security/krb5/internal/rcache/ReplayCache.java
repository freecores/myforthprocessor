/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)ReplayCache.java	1.8 03/06/24
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

import sun.security.krb5.KrbException;
import sun.security.krb5.Config;
import sun.security.krb5.internal.Krb5;
import java.util.LinkedList;
import java.util.ListIterator;

/**
 * This class provides an efficient caching mechanism to store the timestamp of client authenticators.
 * The cache minimizes the memory usage by doing self-cleanup of expired items in the cache.
 *
 * @author Yanni Zhang
 * @version 1.00 10 Jul 00
 */
public class ReplayCache extends LinkedList {
    private String principal;
    private CacheTable table;
    private int nap = 10 * 60 * 1000; //10 minutes break
    private boolean DEBUG = Krb5.DEBUG; 
    /** 
     * Constructs a ReplayCache for a client principal in specified <code>CacheTable</code>.
     * @param p client principal name.
     * @param ct CacheTable.
     */
    public ReplayCache (String p, CacheTable ct) {
        principal = p;
        table = ct;
    }

    /**
     * Puts the authenticator timestamp into the cache in descending order.
     * @param t <code>AuthTime</code>
     */
    public synchronized void put(AuthTime t) {
        
        if (this.size() == 0) {
            addFirst(t);
        }
        else {
            AuthTime temp = (AuthTime)getFirst();
            if (temp.kerberosTime < t.kerberosTime) { 
		// in most cases, newly received authenticator has 
		// larger timestamp.
                addFirst(t);
            }
            else if (temp.kerberosTime == t.kerberosTime) {
                if (temp.cusec < t.cusec) {
                    addFirst(t);
                }
            }
            else { 
		//unless client clock being re-adjusted.
                ListIterator it = listIterator(1);               
                while (it.hasNext()) {
                    temp = (AuthTime)(it.next());
                    if (temp.kerberosTime < t.kerberosTime) {
                        add(indexOf(temp), t);
                        break;                                   
			//we always put the bigger timestamp at the front.
                    }
                    else if (temp.kerberosTime == t.kerberosTime) {
                        if (temp.cusec < t.cusec) {
                            add(indexOf(temp), t);
                            break;
                        }
                    }
                }
            }
        }

	// let us cleanup while we are here
	long tdiff = Krb5.DEFAULT_ALLOWABLE_CLOCKSKEW;
	try {
	    Config c = Config.getInstance();
	    if ((tdiff = c.getDefaultIntValue("clockskew", 
					      "libdefaults")) == Integer.MIN_VALUE) {
		//value is not defined
		tdiff = Krb5.DEFAULT_ALLOWABLE_CLOCKSKEW;
	    }
	} catch (KrbException e) {
	    if (DEBUG) {
		System.out.println ("KrbException in ReplayCache.put " +
				    e.toString());
		e.printStackTrace();
	    }
	}
	long timeLimit = System.currentTimeMillis() - tdiff * 1000L;
	ListIterator it = listIterator(0);
	AuthTime temp = null;
	int index = -1;
	while (it.hasNext()) {        
	    //search expired timestamps.
	    temp = (AuthTime)(it.next());
	    if (temp.kerberosTime < timeLimit) {
		index = indexOf(temp);
		break;
	    }
	}
	if (index > -1) {
	    do {
		//remove expired timestamps from the list.
		removeLast();                         
	    } while(size() > index);
	}
	if (DEBUG) {
	    printList();
	}
	
	//if there are no entries in the replay cache, 
	// remove the replay cache from the table.
	table.remove(principal); 
        if (DEBUG) {
            printList();
        }
    }


    /**
     * Printes out the debug message.
     */
    private void printList() {
        Object[] total = toArray();
        for (int i = 0; i < total.length; i++) {
	    System.out.println("object " + i + ": " + ((AuthTime)total[i]).kerberosTime + "/" 
			       + ((AuthTime)total[i]).cusec);
        }
    }

}

