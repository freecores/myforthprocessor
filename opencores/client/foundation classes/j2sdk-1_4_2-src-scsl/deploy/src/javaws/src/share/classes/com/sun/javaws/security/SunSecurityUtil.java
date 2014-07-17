/*
 * @(#)SunSecurityUtil.java	1.2 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.security;

import com.sun.javaws.debug.Debug;
import com.sun.javaws.debug.Globals;

import java.io.*;
import java.util.*;
import java.security.*;
import java.security.cert.*;
import sun.security.provider.X509Factory;
import sun.security.x509.NetscapeCertTypeExtension;
import sun.security.x509.Extension;
import sun.security.util.DerValue;
import sun.security.util.DerInputStream;
import sun.security.util.DerOutputStream;
import sun.security.util.ObjectIdentifier;


/**
 * @version 1.2 01/23/03
 */
public class SunSecurityUtil {

    // Constants for the OIDs we need
    private final static String OID_BASIC_CONSTRAINTS = "2.5.29.19";
    private final static String OID_KEY_USAGE = "2.5.29.15";
    private final static String OID_EXTENDED_KEY_USAGE = "2.5.29.37";
    private final static String OID_NETSCAPE_CERT_TYPE= "2.16.840.1.113730.1.1";
    
    // extended key usage OID
    private final static String OID_EKU_CODE_SIGNING = "1.3.6.1.5.5.7.3.3";
    private final static String OID_EKU_ANY_USAGE = "2.5.29.37.0";
    
    // Netscape certificate type extension
    // see http://www.netscape.com/eng/security/cert-exts.html
    private final static String NSCT_OBJECT_SIGNING_CA = 
                         NetscapeCertTypeExtension.OBJECT_SIGNING_CA;
    private final static String NSCT_OBJECT_SIGNING = 
                         NetscapeCertTypeExtension.OBJECT_SIGNING;
    private final static String NSCT_SSL_CA = 
			 NetscapeCertTypeExtension.SSL_CA;
    private final static String NSCT_S_MIME_CA = 
			 NetscapeCertTypeExtension.S_MIME_CA;

    public static String getBeginCert() {
        return X509Factory.BEGIN_CERT;
    }

    public static String getEndCert() {
        return X509Factory.END_CERT;
    }

    public static void checkTrustedChain(java.security.cert.Certificate [] 
                chain) throws CertificateException {

        for (int i=0; i < chain.length; i++) {

            if (KeyStoreManager.isCertificateTrusted(chain[i])) {
                break; // If we have the cert on file, trust its signature.
            }

            // 1.) Check that extensions are valid:
            try {
                checkExtensions((X509Certificate)chain[i], i);
            } catch (Exception e) {
                if (Globals.TraceSecurity) {
                    Debug.println("failed extension check: " + chain[i]);
                    Debug.println("exception was: " + e);
                }
                if (e instanceof CertificateException) {
                    throw (CertificateException) e;
                } else {
                    throw new CertificateException("failed extensions check");
                }
            }   

	    int next = (i < chain.length - 1) ? i+1 : i;

            // 2.) verify linkage: issuer(chain[x] == subject(chain[x+1])
            Principal issuer = ((X509Certificate)chain[i]).getIssuerDN();
            Principal subject = ((X509Certificate)chain[next]).getSubjectDN();
            if (!(subject.equals(issuer))) {
                throw new CertificateException("Incomplete certificate chain");
            }

            // 3. ) verify each cert in the ordered chain
            PublicKey pubKey = chain[next].getPublicKey();
            try {
                chain[i].verify(pubKey);
            } catch (Exception e) {
                if (Globals.TraceSecurity) {
                    Debug.println("exception checking extensions: "+e);
                }
                if (e instanceof CertificateException) {
                    throw (CertificateException) e;
                } else {
                    throw new CertificateException("Invalid certificate chain");
                }
            }
        }
        if (Globals.TraceSecurity) {
            Debug.println("certificate chain validated");
        }
    }

    private static void checkExtensions(X509Certificate cert, int index) 
                               throws CertificateException, IOException {

        Set critSet = cert.getCriticalExtensionOIDs();
        if (critSet == null) {
            critSet = Collections.EMPTY_SET;
        }

        // Check for the basic constraints extension.
        checkBasicConstraints(cert, critSet, index);

        // Check for the key usage extension.
        if (index == 0) {
            checkLeafKeyUsage(cert, critSet);
        } else {
            checkSignerKeyUsage(cert, critSet);
        }
        
        if (!critSet.isEmpty()) {
            throw new CertificateException("Certificate contains unknown "
                + "critical extensions: " + critSet);
        }
    }    

    private static void checkBasicConstraints(X509Certificate cert, 
	Set critSet, int index) throws CertificateException, IOException {

        critSet.remove(OID_BASIC_CONSTRAINTS);
	critSet.remove(OID_NETSCAPE_CERT_TYPE);

        // no basic constraints check for user cert
        if (index == 0) {
            return;
        }
        // For some reason, getBasicConstraints() returns -1 for both
        // end user certificates with a basic constraints extension
        // and certificates that do not include the extension at all.
        // So do separate check to see if the extension is present.
        if (cert.getExtensionValue(OID_BASIC_CONSTRAINTS) == null) {
	    if (cert.getExtensionValue(OID_NETSCAPE_CERT_TYPE) == null) {
                throw new CertificateException("CA certificate does not "+
			"include basic constraints extension or "+
			"netscape cert type extension");
	    } else 
	    if (getNetscapeCertTypeBit(cert, NSCT_OBJECT_SIGNING_CA) == false) {
                     throw new CertificateException
                            ("Invalid Netscape CertType extension");
	    }
        } else {
	    // Check Netscape certificate type extension
	    if (cert.getExtensionValue(OID_NETSCAPE_CERT_TYPE) != null) {
	        // Require either bit 7 is true, or all of bits 5,6,7 are false
	        if ((getNetscapeCertTypeBit(cert, NSCT_SSL_CA) != false || 
	         getNetscapeCertTypeBit(cert, NSCT_S_MIME_CA) != false) &&
	         getNetscapeCertTypeBit(cert, NSCT_OBJECT_SIGNING_CA) == false) 
                {
                     throw new CertificateException
                            ("Invalid Netscape CertType extension");
                }
	    }

            // does include basic constraints extension
            int constraints = cert.getBasicConstraints();
            // reject, if not a CA (constraints == -1)
            if (constraints < 0) {
                throw new CertificateException("End user tried to act as a CA");            }
            if (index - 1 > constraints) {
                throw new CertificateException
                        ("Violated path length constraints");
            }
        }
    }


    /*
     * Verify the key usage, extended key usage, and Netscape certificate
     * type extension for end (client or server) certificates.
     *
     */
    private static void checkLeafKeyUsage(X509Certificate cert, Set critSet) 
	throws CertificateException, IOException {
            
        critSet.remove(OID_KEY_USAGE);
        
        // check key usage extension
        boolean[] keyUsageInfo = cert.getKeyUsage();
        if (keyUsageInfo != null) {
            /**
             * keyUsageInfo[0] is for digitalSignature,
             */
            if (keyUsageInfo.length == 0) {
                throw new CertificateException("Invalid key usage extension.");
            }
            boolean digitalSignature = keyUsageInfo[0];
                
            if (!digitalSignature) {
                    // key usage should include signing
                throw new CertificateException(
		    "Wrong key usage. Expected digitalSignature.");
            }
        }
 
        // check extended key usage extension only if critical
        // (interoperability problems with some certificates)

	List extKeyUsageInfo = getExtendedKeyUsage(cert);
        if ((extKeyUsageInfo != null) && 
	    critSet.contains(OID_EXTENDED_KEY_USAGE)) {
            /**
             * If the extension includes the wildcard OID, we always accept it.
             */
            if (!extKeyUsageInfo.contains(OID_EKU_ANY_USAGE)  && 
                !extKeyUsageInfo.contains(OID_EKU_CODE_SIGNING)) {
                    throw new CertificateException(
			"Extended key usage does not permit use");
	    }
        }
        critSet.remove(OID_EXTENDED_KEY_USAGE);
 
        // check Netscape certificate type extension
        if (cert.getExtensionValue(OID_NETSCAPE_CERT_TYPE) != null) {
            if (getNetscapeCertTypeBit(cert, NSCT_OBJECT_SIGNING) == false) {
                throw new CertificateException(
			"Invalid Netscape CertType extension");
	    }
        }
    }

    /*
     * Verify the key usage and extended key usage for intermediate
     * certificates.
     */
    private static void checkSignerKeyUsage(X509Certificate cert, Set critSet)
            throws CertificateException, IOException {
            
        critSet.remove(OID_KEY_USAGE);
 
        // check key usage extension
        boolean[] keyUsageInfo = cert.getKeyUsage();
        if (keyUsageInfo != null) {
            // keyUsageInfo[5] is for keyCertSign.
            if ((keyUsageInfo.length < 6) || (keyUsageInfo[5] == false)) {
                throw new CertificateException
                        ("Wrong key usage: expect keyCertSign");
            }
        }
        
        // check extended key usage extension only if critical
        // (interoperability problems with some certificates)

        List extKeyUsageInfo = getExtendedKeyUsage(cert);
        if ((extKeyUsageInfo != null) && 
	    critSet.contains(OID_EXTENDED_KEY_USAGE)) {
            if (extKeyUsageInfo.contains(OID_EKU_ANY_USAGE) == false) {
                throw new CertificateException("Extended key usage in CA "
                        + "certificates must include anyExtendedKeyUsage");
            }
        }
        critSet.remove(OID_EXTENDED_KEY_USAGE);
    }

    /*
     * getExtendedKeyUsage - must be inline since only in 1.4+
     */
    private static List getExtendedKeyUsage(X509Certificate cert)
            throws CertificateException, IOException {

        byte[] extVal = cert.getExtensionValue(OID_EXTENDED_KEY_USAGE);
        if (extVal == null) {
           return null;
        }

        DerInputStream in = new DerInputStream(extVal);
        byte[] encoded = in.getOctetString();
        DerValue val = new DerValue(encoded);
   
        Vector keyUsages = new Vector(1, 1);
        while (val.data.available() != 0) {
             DerValue seq = val.data.getDerValue();
             ObjectIdentifier usage = seq.getOID();
             keyUsages.addElement(usage);
        }

        ArrayList al = new ArrayList(keyUsages.size());
        for (int i = 0; i < keyUsages.size(); i++) {
            al.add(keyUsages.elementAt(i).toString());
        }
        return al;
    }

    private static boolean getNetscapeCertTypeBit(X509Certificate cert, 
                String type) throws CertificateException, IOException {

        byte[] extVal = cert.getExtensionValue(OID_NETSCAPE_CERT_TYPE);
        if (extVal == null) {
            return false;
        }

        DerInputStream in = new DerInputStream(extVal);
        byte[] encoded = in.getOctetString();
        encoded = new DerValue(encoded).getUnalignedBitString().toByteArray();

        NetscapeCertTypeExtension extn = new NetscapeCertTypeExtension(encoded);
        Boolean val = (Boolean)extn.get(type);
        return val.booleanValue();
    }
}


