/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)ServiceName.java	1.9 03/06/24
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

import java.net.InetAddress;
import java.net.UnknownHostException;

public class ServiceName extends PrincipalName {

    public ServiceName(String name, int type) throws RealmException {
        super(name, type);

    }
    public ServiceName(String name) throws RealmException {
	this(name, PrincipalName.KRB_NT_UNKNOWN);
    }

    public ServiceName(String name, String realm) throws RealmException {
	this(name, PrincipalName.KRB_NT_UNKNOWN);
	setRealm(realm);
    }

    public ServiceName (String service, String instance, String realm)
        throws KrbException
    {
        super(service, instance, realm, PrincipalName.KRB_NT_SRV_INST);
    }

}

