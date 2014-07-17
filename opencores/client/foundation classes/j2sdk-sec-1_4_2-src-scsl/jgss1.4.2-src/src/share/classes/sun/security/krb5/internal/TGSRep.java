/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)TGSRep.java	1.9 03/06/24
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

import sun.security.krb5.PrincipalName;
import sun.security.krb5.Realm;
import sun.security.krb5.RealmException;
import sun.security.krb5.EncryptedData;
import sun.security.krb5.Asn1Exception;
import sun.security.util.*;
import java.io.IOException;

public class TGSRep extends KDCRep {

    public TGSRep(
		  PAData[] new_pAData,
		  Realm new_crealm,
		  PrincipalName new_cname,
		  Ticket new_ticket,
		  EncryptedData new_encPart
		      ) throws IOException {
	super(new_pAData, new_crealm, new_cname, new_ticket,
	      new_encPart, Krb5.KRB_TGS_REP);
    }

    public TGSRep(byte[] data) throws Asn1Exception,
    RealmException, KrbApErrException, IOException {
	init(new DerValue(data));
    }

    public TGSRep(DerValue encoding) throws Asn1Exception,
    RealmException, KrbApErrException, IOException {
	init(encoding);
    }

    private void init(DerValue encoding) throws Asn1Exception,
    RealmException, KrbApErrException, IOException {
	init(encoding, Krb5.KRB_TGS_REP);
    }

}
