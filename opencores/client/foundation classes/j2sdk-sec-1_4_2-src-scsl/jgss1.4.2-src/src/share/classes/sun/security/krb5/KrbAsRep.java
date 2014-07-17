/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)KrbAsRep.java	1.7 03/06/24
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
import sun.security.util.*;
import java.io.IOException;

/**
 * This class encapsulates a AS-REP message that the KDC sends to the
 * client.
 */
public class KrbAsRep extends KrbKdcRep {

    private ASRep rep;
    private Credentials creds;

    private boolean DEBUG = Krb5.DEBUG;

    KrbAsRep(byte[] ibuf, EncryptionKey key, KrbAsReq asReq) throws
    KrbException, Asn1Exception, IOException {
	if (key == null)
	    throw new KrbException(Krb5.API_INVALID_ARG);
	DerValue encoding = new DerValue(ibuf);
	ASReq req = asReq.getMessage();
	ASRep rep = null;
	try {
	    rep = new ASRep(encoding);
    	} catch (Asn1Exception e) {
	    rep = null;
	    KRBError err = new KRBError(encoding);
	    String eText = null; // pick up text sent by the server (if any)
	    if (err.eText != null && err.eText.length() > 0) { 
    		if (err.eText.charAt(err.eText.length() - 1) == 0)
    		    eText = err.eText.substring(0, err.eText.length() - 1);
    		else
    		    eText = err.eText;
	    }
	    KrbException ke; 
	    if (eText == null) {
		// no text sent from server
		ke = new KrbException(err.errorCode); 
	    } else { 
		// override default text with server text
		ke = new KrbException(err.errorCode, eText); 
	    }
	    ke.initCause(e);
	    throw ke;
    	}	  
        byte[] enc_as_rep_bytes = rep.encPart.decrypt(key);
        byte[] enc_as_rep_part = rep.encPart.reset(enc_as_rep_bytes, true);
	
	encoding = new DerValue(enc_as_rep_part);
	EncASRepPart enc_part = new EncASRepPart(encoding);
	rep.ticket.sname.setRealm(rep.ticket.realm);
	rep.encKDCRepPart = enc_part;
	
	check(req, rep);
	
	creds = new Credentials(
				rep.ticket,
				req.reqBody.cname,
				rep.ticket.sname,
				enc_part.key,
				enc_part.flags,
				enc_part.authtime,
				enc_part.starttime,
				enc_part.endtime,
				enc_part.renewTill,
				enc_part.caddr );
        if (DEBUG) {
            System.out.println(">>> KrbAsRep cons in KrbAsReq.getReply " + 
			       req.reqBody.cname.getNameString());
        }

	this.rep = rep;
	this.creds = creds;
    }
    
    public Credentials getCreds() {
	return creds;
    }
    
    // made public for Kinit
    public sun.security.krb5.internal.ccache.Credentials setCredentials() {	
	return new sun.security.krb5.internal.ccache.Credentials(rep);
    }
}
