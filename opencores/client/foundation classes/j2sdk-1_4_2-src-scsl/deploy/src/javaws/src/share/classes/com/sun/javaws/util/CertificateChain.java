/*
 * @(#)CertificateChain.java	1.3 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.util;

import java.security.cert.Certificate;

/** Object to encapsulates that certificate chain used to sign
 *  a resource (JAR file) with
 */
public class CertificateChain {
    private Certificate[] _certChain;
        
    public CertificateChain(Certificate[] chain) {
	_certChain = chain;
    }
}

