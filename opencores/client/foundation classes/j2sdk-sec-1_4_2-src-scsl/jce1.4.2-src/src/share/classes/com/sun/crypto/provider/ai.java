/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.crypto.provider;

import java.io.IOException;
import java.io.Serializable;
import java.io.ObjectStreamException;
import java.security.AlgorithmParameters;
import java.security.NoSuchAlgorithmException;
import java.security.NoSuchProviderException;
import javax.crypto.Cipher;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.SealedObject;
import javax.crypto.spec.*;

/**
 * This class is introduced to workaround a problem in 
 * the SunJCE provider shipped in JCE 1.2.1: the class
 * SealedObjectForKeyProtector was obfuscated due to a mistake.
 * 
 * In order to retrieve secret keys in a JCEKS KeyStore written 
 * by the SunJCE provider in JCE 1.2.1, this class will be used. 
 *
 * @author Valerie Peng
 *
 * @version 1.3, 06/24/03
 *
 * @see JceKeyStore
 */

final class ai extends javax.crypto.SealedObject {

    static final long serialVersionUID = -7051502576727967444L;

    ai(SealedObject so) {
	super(so);
    }

    Object readResolve() throws ObjectStreamException {
	return new SealedObjectForKeyProtector(this);
    }
}
