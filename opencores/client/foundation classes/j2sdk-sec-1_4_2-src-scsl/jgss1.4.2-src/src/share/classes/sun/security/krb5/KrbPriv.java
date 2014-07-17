/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)KrbPriv.java	1.6 03/06/24
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

import sun.security.krb5.EncryptionKey;
import sun.security.krb5.internal.*;
import sun.security.krb5.internal.crypto.*;
import sun.security.util.*;
import java.io.IOException;

class KrbPriv extends KrbAppMessage {
    private byte[] obuf;
    private byte[] userData;
    
    public KrbPriv(byte[] userData,
		   Credentials creds,
		   EncryptionKey subKey,
		   KerberosTime timestamp,
		   SeqNumber seqNumber,
		   HostAddress saddr,
		   HostAddress raddr
		   )  throws KrbException, IOException {
	EncryptionKey reqKey = null;
	if (subKey != null)
	    reqKey = subKey;
	else
	    reqKey = creds.key;
	
	obuf = mk_priv(
		       userData,
		       reqKey,
		       timestamp,
		       seqNumber,
		       saddr,
		       raddr
		       );
    }
    
    public KrbPriv(byte[] msg,
		   Credentials creds,
		   EncryptionKey subKey,
		   SeqNumber seqNumber,
		   HostAddress saddr,
		   HostAddress raddr,
		   boolean timestampRequired,
		   boolean seqNumberRequired
		   )  throws KrbException, IOException {
	
	KRBPriv krb_priv = new KRBPriv(msg);
	EncryptionKey reqKey = null;
	if (subKey != null)
	    reqKey = subKey;
	else
	    reqKey = creds.key;
	userData = rd_priv(krb_priv,
			   reqKey,
			   seqNumber,
			   saddr,
			   raddr,
			   timestampRequired,
			   seqNumberRequired,
			   creds.client,
			   creds.client.getRealm()
			   );
    }
    
    public byte[] getMessage() throws KrbException {
	return obuf;
    }
    
    public byte[] getData() {
	return userData;
    }
    
    private byte[] mk_priv(byte[] userData,
			   EncryptionKey key,
			   KerberosTime timestamp,
			   SeqNumber seqNumber,
			   HostAddress sAddress,
			   HostAddress rAddress
			   ) throws Asn1Exception, IOException, 
			   KdcErrException, KrbCryptoException {
	
			       Integer usec = null;
			       Integer seqno = null;
	
			       if (timestamp != null)
			       usec = new Integer(timestamp.getMicroSeconds());
	
			       if (seqNumber != null) {
				   seqno = new Integer(seqNumber.current());
				   seqNumber.step();
			       }
	
			       EncKrbPrivPart unenc_encKrbPrivPart = 
			       new EncKrbPrivPart(userData,
						  timestamp,
						  usec,
						  seqno,
						  sAddress,
						  rAddress
						  );
	
			       byte[] temp = unenc_encKrbPrivPart.asn1Encode();
	
			       EncryptedData encKrbPrivPart = 
			       new EncryptedData(key, temp);
	
			       KRBPriv krb_priv = new KRBPriv(encKrbPrivPart);
	
			       temp = krb_priv.asn1Encode();
	
			       return krb_priv.asn1Encode();
			   }

    private byte[] rd_priv(KRBPriv krb_priv,
			   EncryptionKey key,
			   SeqNumber seqNumber,
			   HostAddress sAddress,
			   HostAddress rAddress,
			   boolean timestampRequired,
			   boolean seqNumberRequired,
			   PrincipalName cname,
			   Realm crealm
			   ) throws Asn1Exception, KdcErrException, 
			   KrbApErrException, IOException, KrbCryptoException {
	
			       byte[] bytes = krb_priv.encPart.decrypt(key); 
			       byte[] temp = krb_priv.encPart.reset(bytes, true); 
			       DerValue ref = new DerValue(temp);
			       EncKrbPrivPart enc_part = new EncKrbPrivPart(ref);
	
			       check(enc_part.timestamp,
				     enc_part.usec,
				     enc_part.seqNumber,
				     enc_part.sAddress,
				     enc_part.rAddress,
				     seqNumber,
				     sAddress,
				     rAddress,
				     timestampRequired,
				     seqNumberRequired,
				     cname,
				     crealm
				     );
	
			       return enc_part.userData;
			   }
}

