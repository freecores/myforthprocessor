/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)EncASRepPart.java	1.7 03/06/24
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
import sun.security.util.*;
import java.io.IOException;

public class EncASRepPart extends EncKDCRepPart {

	public EncASRepPart(
		EncryptionKey new_key,
		LastReq new_lastReq,
		int new_nonce,
		KerberosTime new_keyExpiration,
		TicketFlags new_flags,
		KerberosTime new_authtime,
		KerberosTime new_starttime,
		KerberosTime new_endtime,
		KerberosTime new_renewTill,
		Realm new_srealm,
		PrincipalName new_sname,
		HostAddresses new_caddr
	) {
		super(
			new_key,
			new_lastReq,
			new_nonce,
			new_keyExpiration,
			new_flags,
			new_authtime,
			new_starttime,
			new_endtime,
			new_renewTill,
			new_srealm,
			new_sname,
			new_caddr,
			Krb5.KRB_ENC_AS_REP_PART
			//may need to use Krb5.KRB_ENC_TGS_REP_PART to mimic
			//behavior of other implementaions, instead of above
		);
	}

	public EncASRepPart(byte[] data) throws Asn1Exception,
		IOException, KrbException {
         init(new DerValue(data));
	}

	public EncASRepPart(DerValue encoding) throws Asn1Exception,
		IOException, KrbException {
		init(encoding);
	}

	private void init(DerValue encoding) throws Asn1Exception,
		IOException, KrbException {
		init(encoding, Krb5.KRB_ENC_AS_REP_PART);
	}

	public byte[] asn1Encode() throws Asn1Exception,
		IOException {
		return asn1Encode(Krb5.KRB_ENC_AS_REP_PART);
	}

}
