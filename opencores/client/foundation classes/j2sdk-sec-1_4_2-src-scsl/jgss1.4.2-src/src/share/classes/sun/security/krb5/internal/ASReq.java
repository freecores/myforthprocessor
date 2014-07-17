/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)ASReq.java	1.8 03/06/24
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

public class ASReq extends KDCReq {

	public ASReq(PAData[] new_pAData, KDCReqBody new_reqBody) throws IOException {
		super(new_pAData, new_reqBody, Krb5.KRB_AS_REQ);
	}

	public ASReq(byte[] data) throws Asn1Exception, KrbException, IOException { 
		init(new DerValue(data));		
	}

    public ASReq(DerValue encoding) throws Asn1Exception, KrbException, IOException {
		init(encoding);
	}

	private void init(DerValue encoding) throws Asn1Exception, IOException, KrbException {
		super.init(encoding, Krb5.KRB_AS_REQ);
	}

}
