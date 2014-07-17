/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)Confounder.java	1.4 03/06/24
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

/**
 * This class is a wrapper on the internal Confounder class. Perhaps at
 * some point, the two will be merged into one. Alternatively, this class
 * can be enhanced to utilize secure random if an entropy gathering device is
 * installed, still falling back on the internal Confounder if not.
 */
public final class Confounder {

    public static byte[] bytes(int size) 
	throws KrbCryptoException {
	    return sun.security.krb5.internal.crypto.Confounder.bytes(size);
	}

    public static int intValue() 
	throws KrbCryptoException {
	return sun.security.krb5.internal.crypto.Confounder.intValue();
    }

    public static long longValue() 
	throws KrbCryptoException {
	return sun.security.krb5.internal.crypto.Confounder.longValue();
    }
}
