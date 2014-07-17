/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)CredentialsUtil.java	1.5 03/06/24
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

package sun.security.krb5.internal;

import sun.security.krb5.*;
import sun.security.krb5.internal.ccache.CredentialsCache;
import java.util.StringTokenizer;
import sun.security.krb5.internal.ktab.*;
import java.io.File;
import java.io.IOException;
import java.util.Date;
import java.util.Vector;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.UnsupportedEncodingException;
import java.net.InetAddress;

/**
 * This class is a utility that contains much of the TGS-Exchange
 * protocol. It is used by ../Credentials.java for service ticket
 * acquisition in both the normal and the x-realm case.
 */
public class CredentialsUtil {
    
    private static boolean DEBUG = sun.security.krb5.internal.Krb5.DEBUG;

   /**
    * Acquires credentials for a specified service using initial credential. Wh
en the service has a different realm
    * from the initial credential, we do cross-realm authentication - first, we
 use the current credential to get
    * a cross-realm credential from the local KDC, then use that cross-realm cr
edential to request service credential
    * from the foreigh KDC.
    *
    * @param service the name of service principal using format components@real
m
    * @param ccreds client's initial credential.
    * @exception Exception general exception will be thrown when any error occu
rs.
    * @return a <code>Credentials</code> object.
    */
    public static Credentials acquireServiceCreds(
                String service, Credentials ccreds)
    throws KrbException, IOException {
        ServiceName sname = new ServiceName(service);
        String serviceRealm = sname.getRealmString();
        String localRealm = ccreds.getClient().getRealmString();
        String defaultRealm = Config.getInstance().getDefaultRealm();

        if (localRealm == null) {
            PrincipalName temp = null;
            if ((temp = ccreds.getServer()) != null)
                localRealm = temp.getRealmString();
        }
        if (localRealm == null) {
            localRealm = defaultRealm;
        }
        if (serviceRealm == null) {
            serviceRealm = localRealm;
            sname.setRealm(serviceRealm);
        }

	/*
	  if (!localRealm.equalsIgnoreCase(serviceRealm)) { //do cross-realm auth entication
	  if (DEBUG) {
	  System.out.println(">>>DEBUG: Credentails request cross realm ticket for " + "krbtgt/" + serviceRealm + "@" + localRealm);
	  }
	  Credentials crossCreds = serviceCreds(new ServiceName("krbtgt/" + serviceRealm + "@" + localRealm), ccreds);
	  if (DEBUG) {
	  printDebug(crossCreds);
	  }
	  Credentials result = serviceCreds(sname, crossCreds);
	  if (DEBUG) {
	  printDebug(result);
	  }
	  return result;
	  }
	  else return serviceCreds(sname, ccreds);
	*/

        if (localRealm.equals(serviceRealm))
        {
            if (DEBUG)
                System.out.println(">>> Credentials acquireServiceCreds: same realm");
            return serviceCreds(sname, ccreds);
        }

        // Get a list of realms to traverse
        String[] realms = Realm.getRealmsList(localRealm, serviceRealm);

        if (realms == null || realms.length == 0)
        {
            if (DEBUG)
                System.out.println(">>> Credentials acquireServiceCreds: no realms list");
            return null;
        }

        int i = 0, k = 0;
        Credentials cTgt = null, newTgt = null, theTgt = null;
        ServiceName tempService = null;
        String realm = null, newTgtRealm = null, theTgtRealm = null;

        for (cTgt = ccreds, i = 0; i < realms.length;)
        {
            tempService = new ServiceName(PrincipalName.TGS_DEFAULT_SRV_NAME,
                                          serviceRealm, realms[i]);

            if (DEBUG)
            {
                System.out.println(">>> Credentials acquireServiceCreds: main loop: [" + i +"] tempService=" + tempService);
            }

            try {
                newTgt = serviceCreds(tempService, cTgt);
            } catch (Exception exc) {
                newTgt = null;
            }

            if (newTgt == null)
            {
                if (DEBUG)
                {
                    System.out.println(">>> Credentials acquireServiceCreds: no tgt; searching backwards");
                }

                /*
                 * No tgt found. Try to get one for a
                 * realm as close to the target as possible.
                 * That means traversing the realms list backwards.
                 */

                for (newTgt = null, k = realms.length - 1; 
                     newTgt == null && k > i; k--)
                {

                    tempService = new ServiceName(
                                       PrincipalName.TGS_DEFAULT_SRV_NAME,
                                       realms[k], realms[i]);
                    if (DEBUG)
                    {
                        System.out.println(">>> Credentials acquireServiceCreds: inner loop: [" + k +"] tempService=" + tempService);
                    }

                    try {
                        newTgt = serviceCreds(tempService, cTgt);
                    } catch (Exception exc) {
                        newTgt = null;
                    }
                }
            } // Ends 'if (newTgt == null)'

            if (newTgt == null)
            {
                if (DEBUG)
                {
                    System.out.println(">>> Credentials acquireServiceCreds: no tgt; cannot get creds");
                }
                break;
            }

            /*
             * We have a tgt. It may or may not be for the target.
             * If it's for the target realm, we're done looking for a tgt.
             */

            newTgtRealm = newTgt.getServer().getInstanceComponent();

            if (DEBUG)
            {
                System.out.println(">>> Credentials acquireServiceCreds: got tgt"); 
                //printDebug(newTgt);
            }

            if (newTgtRealm.equals(serviceRealm))
            {
                /* We got the right tgt */
                theTgt = newTgt;
                theTgtRealm = newTgtRealm;
                break;
            }

            /*
             * The new tgt is not for the target realm.
             * See if the realm of the new tgt is in the list of realms
             * and continue looking from there.
             */

            for (k = i+1; k < realms.length; k++)
            {
                if (newTgtRealm.equals(realms[k]))
                {
                    break;
                }
            }

            if (k < realms.length)
            {
                /*
                 * (re)set the counter so we start looking
                 * from the realm we just obtained a tgt for.
                 */
                i = k;
                cTgt = newTgt;

                if (DEBUG)
                {
                    System.out.println(">>> Credentials acquireServiceCreds: continuing with main loop counter reset to " + i); 
                }

                continue;
            }
            else
            {
                /*
                 * The new tgt's realm is not in the heirarchy of realms. 
                 * It's probably not safe to get a tgt from
                 * a tgs that is outside the known list of realms.
                 * Give up now. 
                 */

                break;
            }
        } // Ends outermost/main 'for' loop

        Credentials theCreds = null;

        if (theTgt != null)
        {
            /* We have the right tgt. Let's get the service creds */

            if (DEBUG)
            {
                System.out.println(">>> Credentials acquireServiceCreds: got right tgt");

                //printDebug(theTgt);

                System.out.println(">>> Credentials acquireServiceCreds: obtaining service creds for " + sname);
            }

            try {
                theCreds = serviceCreds(sname, theTgt);
            } catch (Exception exc) {
	      if (DEBUG)
		System.out.println(exc);
	      theCreds = null;
            }
        }

        if (theCreds != null)
        {
            if (DEBUG)
            {
                System.out.println(">>> Credentials acquireServiceCreds: returning creds:");
                Credentials.printDebug(theCreds);
            }
            return theCreds;
        }
        throw new KrbApErrException(Krb5.KRB_AP_ERR_GEN_CRED,
				    "No service creds");
    }

   /*
    * This method does the real job to request the service credential.
    */
    private static Credentials serviceCreds(
                    ServiceName service, Credentials ccreds)
    throws KrbException, IOException {
            KrbTgsReq tgs_req = new KrbTgsReq(ccreds, service);
	    KrbTgsRep tgs_rep = null;
	    String kdc = null;
	    try {
		kdc = tgs_req.send();
		tgs_rep = tgs_req.getReply(ccreds);
	    } catch (KrbException ke) {
		if (ke.returnCode() == Krb5.KRB_ERR_RESPONSE_TOO_BIG) {
			// set useTCP and retry  
		    tgs_req.send(service.getRealmString(), kdc, true); 
	            tgs_rep = tgs_req.getReply(ccreds);
		} else {
		    throw ke;
		}
	    }
            return tgs_rep.getCreds();
    }
}
