/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)TGSReq.java	1.7 03/06/24
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

public class TGSReq extends KDCReq {

    public TGSReq(PAData[] new_pAData, KDCReqBody new_reqBody) throws IOException {
	super(new_pAData, new_reqBody, Krb5.KRB_TGS_REQ);
    }

    public TGSReq(byte[] data) throws Asn1Exception,
    IOException, KrbException {
	init(new DerValue(data));
    }

    public TGSReq(DerValue encoding) throws Asn1Exception,
    IOException, KrbException {
	init(encoding);
    }

    private void init(DerValue encoding) throws Asn1Exception,
    IOException, KrbException {
	init(encoding, Krb5.KRB_TGS_REQ);
    }

}
