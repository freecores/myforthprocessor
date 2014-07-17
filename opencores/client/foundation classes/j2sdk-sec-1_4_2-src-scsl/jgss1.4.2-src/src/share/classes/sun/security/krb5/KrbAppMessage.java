/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)KrbAppMessage.java	1.6 03/06/24
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

package sun.security.krb5;

import sun.security.krb5.internal.*;

abstract class KrbAppMessage {

    private static boolean DEBUG = Krb5.DEBUG;
    /**
     * Common checks for KRB-PRIV and KRB-SAFE
     */
    void check(KerberosTime packetTimestamp,
	       Integer packetUsec,
	       Integer packetSeqNumber,
	       HostAddress packetSAddress,
	       HostAddress packetRAddress,
	       SeqNumber seqNumber,
	       HostAddress sAddress,
	       HostAddress rAddress,
	       boolean timestampRequired,
	       boolean seqNumberRequired,
	       PrincipalName packetPrincipal,
	       Realm packetRealm)
	throws KrbApErrException {

	if (!Krb5.AP_EMPTY_ADDRESSES_ALLOWED || sAddress != null) {
	    if (packetSAddress == null || sAddress == null ||
		!packetSAddress.equals(sAddress)) { 
		if (DEBUG && packetSAddress == null) {
		    System.out.println("packetSAddress is null"); 
		}
		if (DEBUG && sAddress == null) {
		    System.out.println("sAddress is null"); 
		}
		throw new KrbApErrException(Krb5.KRB_AP_ERR_BADADDR);
	    } 
	}
	
	if (!Krb5.AP_EMPTY_ADDRESSES_ALLOWED || rAddress != null) {
	    if (packetRAddress == null || rAddress == null ||
		!packetRAddress.equals(rAddress))
		throw new KrbApErrException(Krb5.KRB_AP_ERR_BADADDR);
	}
	
	if (packetTimestamp != null) {
	    packetTimestamp.setMicroSeconds(packetUsec);
	    if (!packetTimestamp.inClockSkew())
		throw new KrbApErrException(Krb5.KRB_AP_ERR_SKEW);
	} else
	    if (timestampRequired)
		throw new KrbApErrException(Krb5.KRB_AP_ERR_SKEW);
	
	// replay cache not yet implemented
	// if (rcache.repeated(packetTimestamp, packetUsec, packetSAddress))
	//	throw new KrbApErrException(Krb5.KRB_AP_ERR_REPEAT);
	
	//  consider moving up to api level
	if (seqNumber == null && seqNumberRequired == true)
	    throw new KrbApErrException(Krb5.API_INVALID_ARG);
	
	if (packetSeqNumber != null && seqNumber != null) {
	    if (packetSeqNumber.intValue() != seqNumber.current())
		throw new KrbApErrException(Krb5.KRB_AP_ERR_BADORDER);
	    // should be done only when no more exceptions are possible
	    seqNumber.step();
	} else {
	    if (seqNumberRequired) {
		throw new KrbApErrException(Krb5.KRB_AP_ERR_BADORDER);
	    }
	}
	
	// Must not be relaxed, per RFC 1510
	if (packetTimestamp == null && packetSeqNumber == null)
	    throw new KrbApErrException(Krb5.KRB_AP_ERR_MODIFIED);

	// replay cache not yet implemented
	// rcache.save_identifier(packetTimestamp, packetUsec, packetSAddress,
	// packetPrincipal, pcaketRealm);
    }

}
