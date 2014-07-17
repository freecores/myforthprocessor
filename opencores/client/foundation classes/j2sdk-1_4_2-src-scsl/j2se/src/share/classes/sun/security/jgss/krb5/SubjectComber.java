/*
 * @(#)SubjectComber.java	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
  
package sun.security.jgss.krb5;

import org.ietf.jgss.*;
import sun.security.jgss.spi.*;
import sun.security.krb5.*;
import sun.security.krb5.Config;
import javax.security.auth.kerberos.*;
import java.net.InetAddress;
import java.io.IOException;
import java.util.Date;
import javax.security.auth.Subject;
import java.security.AccessController;
import java.security.AccessControlContext;
import java.util.Iterator;
import java.security.PrivilegedAction;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import javax.security.auth.kerberos.*;
import sun.security.jgss.LoginUtility;


/**
 * This utility looks through the current Subject and retreives a TGT
 * for the desired client principal and the desired REALM.
 *
 * @author Ram Marti
 * @version 1.4, 01/23/03
 * @since 1.4.2 
 */

class SubjectComber implements PrivilegedAction {
    
    private AccessControlContext acc;
    private String serverPrincipal;
    private String clientPrincipal;
    private Subject subject;
    private Class credClass;
    public static final boolean DEBUG =
        ((Boolean)java.security.AccessController.doPrivileged(
	    new sun.security.action.GetBooleanAction
		("sun.security.krb5.debug"))).booleanValue();
    public SubjectComber(AccessControlContext acc,
			 String serverPrincipal, 
			 String clientPrincipal,
			 Class c ) {
	/*
	 * Can't get the Subject here because this is
	 * not privileged code
	 */
	this.acc = acc;
	this.serverPrincipal = serverPrincipal;
	this.clientPrincipal = clientPrincipal;
	this.credClass = c;
    }
    
    public SubjectComber(Subject subject,
			 String serverPrincipal, 
			 String clientPrincipal,
			 Class c ) {
	this.subject = subject;
	this.serverPrincipal = serverPrincipal;
	this.clientPrincipal = clientPrincipal;
	this.credClass = c;
    }
    
    public Object run() {
	if (subject == null)
	    subject = Subject.getSubject(acc); // This is privileged code
	if (subject == null) {
	    return null;
	} else {
	    if (credClass == KerberosKey.class) {
		// We are looking for a KerberosKey credentials for the
		// serverPrincipal
		Iterator iterator =
                    subject.getPrivateCredentials(KerberosKey.class).iterator();
                while (iterator.hasNext()) {
                    KerberosKey key = (KerberosKey) iterator.next();
                    if (serverPrincipal == null ||
                        serverPrincipal.equals(key.getPrincipal().getName())) {
			 if (DEBUG) {	
			     System.out.println("Found key for "
						+ key.getPrincipal());
			 }
                        return key;
                    }
                }
	    } else {
	        if (credClass == KerberosTicket.class) {
		    // we are looking for a KerberosTicket credentials
		    // for client-service principal pair
		    Iterator iterator = 
			subject.getPrivateCredentials(KerberosTicket.class).iterator();
		    while (iterator.hasNext()) {
			KerberosTicket ticket = (KerberosTicket) iterator.next();
			if (DEBUG) {
			    System.out.println("Found ticket for " 
					       + ticket.getClient()
					       + " to go to "
					       + ticket.getServer()
					       + " expiring on "
					       + ticket.getEndTime());
			}
			if (!ticket.isCurrent()) {
			    // let us remove the ticket from the Subject
			    // Note that both TGT and service ticket will be
			    // removed  upon expiration
			    if (!subject.isReadOnly()) {
				iterator.remove();
				if (DEBUG) {
				    System.out.println ("Removed the " +
							"expired Ticket \n" +
							ticket);
				    
				}
			    }
			} else {
			    if (ticket.getServer().getName().equals
				(serverPrincipal))  {
				
				if (clientPrincipal == null ||
				    clientPrincipal.equals(
							   ticket.getClient().getName())) {
				    return ticket;
				}
			    }
			}
		    }
		}
		
	    }
	   
	}
	return null;    
    }
    
}
    

