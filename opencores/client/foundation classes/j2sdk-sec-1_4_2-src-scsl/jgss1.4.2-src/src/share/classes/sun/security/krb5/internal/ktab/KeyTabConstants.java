/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)KeyTabConstants.java	1.5 03/06/24
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

package sun.security.krb5.internal.ktab;

import sun.security.krb5.internal.*;

/**
 * This class represents a Key Table entry. Each entry contains the service principal of
 * the key, time stamp, key version and secret key itself.
 *
 * @author Yanni Zhang
 * @version 1.00  09 Mar 2000
 */
public interface KeyTabConstants {
    final int principalComponentSize = 2;
    final int realmSize = 2;
    final int principalSize = 2;
    final int principalTypeSize = 4;
    final int timestampSize = 4;
    final int keyVersionSize = 1;
    final int keyTypeSize = 2;
    final int keySize = 2;
    static final int KRB5_KT_VNO_1 = 0x0501;	/* krb v5, keytab version 1 (DCE compat) */
    static final int KRB5_KT_VNO	= 0x0502;	/* krb v5, keytab version 2 (standard)  */
}
