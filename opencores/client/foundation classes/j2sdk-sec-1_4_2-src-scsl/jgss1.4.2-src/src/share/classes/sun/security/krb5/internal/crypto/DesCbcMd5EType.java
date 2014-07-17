/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)DesCbcMd5EType.java	1.6 03/06/24
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

package sun.security.krb5.internal.crypto;

import sun.security.krb5.internal.*;
import sun.security.krb5.Checksum;
import sun.security.krb5.EncryptedData;
import sun.security.krb5.KrbCryptoException;
import java.security.MessageDigest;
import java.security.Provider;
import java.security.Security;

public final class DesCbcMd5EType extends DesCbcEType {

    public DesCbcMd5EType() {
    }

    public int eType() {
	return EncryptedData.ETYPE_DES_CBC_MD5;
    }

    public int minimumPadSize() {
	return 0;
    }

    public int confounderSize() {
	return 8;
    }

    public int checksumType() {
	return Checksum.CKSUMTYPE_RSA_MD5;
    }

    public int checksumSize() {
	return 16;
    }

    /**
     * Calculates checksum using MD5.
     * @param data the input data.
     * @param size the length of data.
     * @return the checksum.
     *
     * @modified by Yanni Zhang, 12/06/99.
     */
    public byte[] calculateChecksum(byte[] data, int size) throws KrbCryptoException {
	MessageDigest md5 = null;
	try {
	    md5 = MessageDigest.getInstance("MD5");
	} catch (Exception e) {
	    throw new KrbCryptoException("JCE provider may not be installed. " + e.getMessage());
	}
	try {
	    md5.update(data);
	    return(md5.digest());
	} catch (Exception e) {
	    throw new KrbCryptoException(e.getMessage());		
	}
    }
}

