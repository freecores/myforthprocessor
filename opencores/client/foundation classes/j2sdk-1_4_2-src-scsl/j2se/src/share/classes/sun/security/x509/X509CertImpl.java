/*
 * @(#)X509CertImpl.java	1.123 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.x509;

import java.io.BufferedReader;
import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.math.BigInteger;
import java.security.*;
import java.security.cert.*;
import java.security.cert.Certificate;
import java.util.*;

import javax.security.auth.x500.X500Principal;

import sun.misc.HexDumpEncoder;
import sun.misc.BASE64Decoder;
import sun.security.util.*;
import sun.security.provider.X509Factory;

/**
 * The X509CertImpl class represents an X.509 certificate. These certificates
 * are widely used to support authentication and other functionality in
 * Internet security systems.  Common applications include Privacy Enhanced
 * Mail (PEM), Transport Layer Security (SSL), code signing for trusted
 * software distribution, and Secure Electronic Transactions (SET).  There
 * is a commercial infrastructure ready to manage large scale deployments
 * of X.509 identity certificates.
 *
 * <P>These certificates are managed and vouched for by <em>Certificate
 * Authorities</em> (CAs).  CAs are services which create certificates by
 * placing data in the X.509 standard format and then digitally signing
 * that data.  Such signatures are quite difficult to forge.  CAs act as
 * trusted third parties, making introductions between agents who have no
 * direct knowledge of each other.  CA certificates are either signed by
 * themselves, or by some other CA such as a "root" CA.
 *
 * <P>RFC 1422 is very informative, though it does not describe much
 * of the recent work being done with X.509 certificates.  That includes
 * a 1996 version (X.509v3) and a variety of enhancements being made to
 * facilitate an explosion of personal certificates used as "Internet
 * Drivers' Licences", or with SET for credit card transactions.
 *
 * <P>More recent work includes the IETF PKIX Working Group efforts,
 * especially RFC2459.
 *
 * @author Dave Brownell
 * @author Amit Kapoor
 * @author Hemma Prafullchandra
 * @version 1.123 01/23/03
 * @see X509CertInfo
 */
public class X509CertImpl extends X509Certificate implements DerEncoder {

    private static final String DOT = ".";
    /**
     * Public attribute names.
     */
    public static final String NAME = "x509";
    public static final String INFO = X509CertInfo.NAME;
    public static final String ALG_ID = "algorithm";
    public static final String SIGNATURE = "signature";
    public static final String SIGNED_CERT = "signed_cert";

    /**
     * The following are defined for ease-of-use. These
     * are the most frequently retrieved attributes.
     */
    // x509.info.subject.dname
    public static final String SUBJECT_DN = NAME + DOT + INFO + DOT +
                               X509CertInfo.SUBJECT + DOT +
                               CertificateSubjectName.DN_NAME;
    // x509.info.issuer.dname
    public static final String ISSUER_DN = NAME + DOT + INFO + DOT +
                               X509CertInfo.ISSUER + DOT +
                               CertificateIssuerName.DN_NAME;
    // x509.info.serialNumber.number
    public static final String SERIAL_ID = NAME + DOT + INFO + DOT +
                               X509CertInfo.SERIAL_NUMBER + DOT +
                               CertificateSerialNumber.NUMBER;
    // x509.info.key.value
    public static final String PUBLIC_KEY = NAME + DOT + INFO + DOT +
                               X509CertInfo.KEY + DOT +
                               CertificateX509Key.KEY;

    // x509.info.version.value
    public static final String VERSION = NAME + DOT + INFO + DOT +
	                       X509CertInfo.VERSION + DOT +
                               CertificateVersion.VERSION;

    // x509.algorithm
    public static final String SIG_ALG = NAME + DOT + ALG_ID;

    // x509.signature
    public static final String SIG = NAME + DOT + SIGNATURE;

    // when we sign and decode we set this to true
    // this is our means to make certificates immutable
    private boolean readOnly = false;

    // Certificate data, and its envelope
    private byte[] 		signedCert = null;
    protected X509CertInfo	info = null;
    protected AlgorithmId	algId = null;
    protected byte[]		signature = null;

    // recognized extension OIDS
    private static final String KEY_USAGE_OID = "2.5.29.15";
    private static final String EXTENDED_KEY_USAGE_OID = "2.5.29.37";
    private static final String BASIC_CONSTRAINT_OID = "2.5.29.19";
    private static final String SUBJECT_ALT_NAME_OID = "2.5.29.17";
    private static final String ISSUER_ALT_NAME_OID = "2.5.29.18";

    // number of standard key usage bits.
    private static final int NUM_STANDARD_KEY_USAGE = 9;

    // SubjectAlterntativeNames cache
    private Collection subjectAlternativeNames;

    // IssuerAlternativeNames cache
    private Collection issuerAlternativeNames;

    // ExtendedKeyUsage cache
    private List extKeyUsage;

    /**
     * PublicKey that has previously been used to verify
     * the signature of this certificate. Null if the certificate has not 
     * yet been verified.
     */
    private PublicKey verifiedPublicKey;
    /**
     * If verifiedPublicKey is not null, name of the provider used to
     * successfully verify the signature of this certificate, or the
     * empty String if no provider was explicitly specified.
     */
    private String verifiedProvider;
    /**
     * If verifiedPublicKey is not null, result of the verification using
     * verifiedPublicKey and verifiedProvider. If true, verification was
     * successful, if false, it failed.
     */
    private boolean verificationResult;
    
    /**
     * Default constructor.
     */
    public X509CertImpl() { }

    /**
     * Unmarshals a certificate from its encoded form, parsing the
     * encoded bytes.  This form of constructor is used by agents which
     * need to examine and use certificate contents.  That is, this is
     * one of the more commonly used constructors.  Note that the buffer
     * must include only a certificate, and no "garbage" may be left at
     * the end.  If you need to ignore data at the end of a certificate,
     * use another constructor.
     *
     * @param certData the encoded bytes, with no trailing padding.
     * @exception CertificateException on parsing and initialization errors.
     */
    public X509CertImpl(byte[] certData) throws CertificateException {
        try {
            parse(new DerValue(certData));
        } catch (IOException e) {
            signedCert = null;
	    CertificateException ce = new 
		CertificateException("Unable to initialize, " + e);
	    ce.initCause(e);
	    throw ce;
        }
    }

    /**
     * unmarshals an X.509 certificate from an input stream.  If the
     * certificate is RFC1421 hex-encoded, then it must begin with
     * the line X509Factory.BEGIN_CERT and end with the line
     * X509Factory.END_CERT.
     *
     * @param in an input stream holding at least one certificate that may
     *        be either DER-encoded or RFC1421 hex-encoded version of the
     *        DER-encoded certificate.
     * @exception CertificateException on parsing and initialization errors.
     */
    public X509CertImpl(InputStream in) throws CertificateException {

	DerValue der = null;

	BufferedInputStream inBuffered = new BufferedInputStream(in);

	// First try reading stream as HEX-encoded DER-encoded bytes, 
	// since not mistakable for raw DER
	try {
	    inBuffered.mark(Integer.MAX_VALUE);
	    der = readRFC1421Cert(inBuffered);
	} catch (IOException ioe) {
	    try {
		// Next, try reading stream as raw DER-encoded bytes
		inBuffered.reset();
		der = new DerValue(inBuffered);
	    } catch (IOException ioe1) {
		CertificateException ce = new 
		    CertificateException("Input stream must be " +
				         "either DER-encoded bytes " +
					 "or RFC1421 hex-encoded " +
					 "DER-encoded bytes: " +
					 ioe1.getMessage());
		ce.initCause(ioe1);
		throw ce;
	    }
	}
        try {
            parse(der);
        } catch (IOException ioe) {
            signedCert = null;
            CertificateException ce = new 
		CertificateException("Unable to parse DER value of " +
				     "certificate, " + ioe);
	    ce.initCause(ioe);
	    throw ce;
        }
    }

    /**
     * read input stream as HEX-encoded DER-encoded bytes
     *
     * @param in InputStream to read
     * @returns DerValue corresponding to decoded HEX-encoded bytes
     * @throws IOException if stream can not be interpreted as RFC1421 
     *                     encoded bytes
     */
    private DerValue readRFC1421Cert(InputStream in) throws IOException {
	DerValue der = null;
	String line = null;
	BufferedReader certBufferedReader =
	                   new BufferedReader(new InputStreamReader(in));
	try {
	    line = certBufferedReader.readLine();
	} catch (IOException ioe1) {
	    throw new IOException("Unable to read InputStream: " +
				  ioe1.getMessage());
	}
	if (line.equals(X509Factory.BEGIN_CERT)) {
	    /* stream appears to be hex-encoded bytes */
	    BASE64Decoder         decoder   = new BASE64Decoder();
	    ByteArrayOutputStream decstream = new ByteArrayOutputStream();
	    try {
		while ((line = certBufferedReader.readLine()) != null) {
		    if (line.equals(X509Factory.END_CERT)) {
			der = new DerValue(decstream.toByteArray());
			break;
		    } else {
			decstream.write(decoder.decodeBuffer(line));
		    }
		}
	    } catch (IOException ioe2) {
		throw new IOException("Unable to read InputStream: " 
				      + ioe2.getMessage());
	    }
	} else {
	    throw new IOException("InputStream is not RFC1421 hex-encoded " +
				  "DER bytes");
	}
	return der;
    }

    /**
     * Construct an initialized X509 Certificate. The certificate is stored
     * in raw form and has to be signed to be useful.
     *   
     * @params info the X509CertificateInfo which the Certificate is to be
     *              created from.
     */
    public X509CertImpl(X509CertInfo certInfo) {
        this.info = certInfo;
    }

    /**
     * Unmarshal a certificate from its encoded form, parsing a DER value.
     * This form of constructor is used by agents which need to examine
     * and use certificate contents.
     *   
     * @param derVal the der value containing the encoded cert.
     * @exception CertificateException on parsing and initialization errors.
     */  
    public X509CertImpl(DerValue derVal) throws CertificateException {
        try {
            parse(derVal);
        } catch (IOException e) {
            signedCert = null;
            CertificateException ce = new 
		CertificateException("Unable to initialize, " + e);
	    ce.initCause(e);
	    throw ce;
        }
    }

    /**
     * Appends the certificate to an output stream.
     *
     * @param out an input stream to which the certificate is appended.
     * @exception CertificateEncodingException on encoding errors.
     */
    public void encode(OutputStream out)
    throws CertificateEncodingException {
        if (signedCert == null)
            throw new CertificateEncodingException(
                          "Null certificate to encode");
        try {
            out.write((byte[])signedCert.clone());
        } catch (IOException e) {
            throw new CertificateEncodingException(e.toString());
        }
    }

    /**
     * DER encode this object onto an output stream.
     * Implements the <code>DerEncoder</code> interface.
     *
     * @param out the output stream on which to write the DER encoding.
     *
     * @exception IOException on encoding error.
     */
    public void derEncode(OutputStream out) throws IOException {
        if (signedCert == null)
            throw new IOException("Null certificate to encode");
	out.write((byte[])signedCert.clone());
    }

    /**
     * Returns the encoded form of this certificate. It is
     * assumed that each certificate type would have only a single
     * form of encoding; for example, X.509 certificates would
     * be encoded as ASN.1 DER.
     *   
     * @exception CertificateEncodingException if an encoding error occurs.
     */  
    public byte[] getEncoded() throws CertificateEncodingException {
	return (byte[])getEncodedInternal().clone();
    }
    
    /**
     * Returned the encoding as an uncloned byte array. Callers must
     * guarantee that they neither modify it nor expose it to untrusted
     * code.
     */
    public byte[] getEncodedInternal() throws CertificateEncodingException {
        if (signedCert == null) {
            throw new CertificateEncodingException(
                          "Null certificate to encode");
	}
	return signedCert;
    }

    /**
     * Throws an exception if the certificate was not signed using the
     * verification key provided.  Successfully verifying a certificate
     * does <em>not</em> indicate that one should trust the entity which
     * it represents.
     *
     * @param key the public key used for verification.
     *   
     * @exception InvalidKeyException on incorrect key.
     * @exception NoSuchAlgorithmException on unsupported signature
     * algorithms.
     * @exception NoSuchProviderException if there's no default provider.
     * @exception SignatureException on signature errors.
     * @exception CertificateException on encoding errors.
     */
    public void verify(PublicKey key)
    throws CertificateException, NoSuchAlgorithmException,
        InvalidKeyException, NoSuchProviderException, SignatureException {

        verify(key, "");
    }

    /**
     * Throws an exception if the certificate was not signed using the
     * verification key provided.  Successfully verifying a certificate
     * does <em>not</em> indicate that one should trust the entity which
     * it represents.
     *
     * @param key the public key used for verification.
     * @param sigProvider the name of the provider.
     *   
     * @exception NoSuchAlgorithmException on unsupported signature
     * algorithms.
     * @exception InvalidKeyException on incorrect key.
     * @exception NoSuchProviderException on incorrect provider.
     * @exception SignatureException on signature errors.
     * @exception CertificateException on encoding errors.
     */
    public synchronized void verify(PublicKey key, String sigProvider)
	    throws CertificateException, NoSuchAlgorithmException,
	    InvalidKeyException, NoSuchProviderException, SignatureException {
	if (sigProvider == null) {
	    sigProvider = "";
	}
	if ((verifiedPublicKey != null) && verifiedPublicKey.equals(key)) {
	    // this certificate has already been verified using
	    // this public key. Make sure providers match, too.
	    if (sigProvider.equals(verifiedProvider)) {
		if (verificationResult) {
		    return;
		} else {
		    throw new SignatureException("Signature does not match.");
		}
	    }
	}
        if (signedCert == null) {
            throw new CertificateEncodingException("Uninitialized certificate");
        }
        // Verify the signature ...
        Signature sigVerf = null;
	if (sigProvider.length() == 0) {
	    sigVerf = Signature.getInstance(algId.getName());
	} else {
	    sigVerf = Signature.getInstance(algId.getName(), sigProvider);
	}
        sigVerf.initVerify(key);

        byte[] rawCert = info.getEncodedInfo();
        sigVerf.update(rawCert, 0, rawCert.length);
	
	// verify may throw SignatureException for invalid encodings, etc.
	verificationResult = sigVerf.verify(signature);
	verifiedPublicKey = key;
	verifiedProvider = sigProvider;
      
        if (verificationResult == false) {
            throw new SignatureException("Signature does not match.");
        }
    }
  
    /**
     * Creates an X.509 certificate, and signs it using the given key
     * (associating a signature algorithm and an X.500 name).
     * This operation is used to implement the certificate generation
     * functionality of a certificate authority.
     *
     * @param key the private key used for signing.
     * @param algorithm the name of the signature algorithm used.
     *
     * @exception InvalidKeyException on incorrect key.
     * @exception NoSuchAlgorithmException on unsupported signature
     * algorithms.
     * @exception NoSuchProviderException if there's no default provider.
     * @exception SignatureException on signature errors.
     * @exception CertificateException on encoding errors.
     */
    public void sign(PrivateKey key, String algorithm)
    throws CertificateException, NoSuchAlgorithmException,
        InvalidKeyException, NoSuchProviderException, SignatureException {
        sign(key, algorithm, null);
    }

    /**
     * Creates an X.509 certificate, and signs it using the given key
     * (associating a signature algorithm and an X.500 name).
     * This operation is used to implement the certificate generation
     * functionality of a certificate authority.
     *
     * @param key the private key used for signing.
     * @param algorithm the name of the signature algorithm used.
     * @param provider the name of the provider.
     *
     * @exception NoSuchAlgorithmException on unsupported signature
     * algorithms.
     * @exception InvalidKeyException on incorrect key.
     * @exception NoSuchProviderException on incorrect provider.
     * @exception SignatureException on signature errors.
     * @exception CertificateException on encoding errors.
     */
    public void sign(PrivateKey key, String algorithm, String provider)
    throws CertificateException, NoSuchAlgorithmException,
        InvalidKeyException, NoSuchProviderException, SignatureException {
        try {
            if (readOnly)
                throw new CertificateEncodingException(
                              "cannot over-write existing certificate");
            Signature sigEngine = null;
            if ((provider == null) || (provider.length() == 0))
                sigEngine = Signature.getInstance(algorithm);
            else
                sigEngine = Signature.getInstance(algorithm, provider);

            sigEngine.initSign(key);

				// in case the name is reset
            algId = AlgorithmId.get(sigEngine.getAlgorithm());

            DerOutputStream out = new DerOutputStream();
            DerOutputStream tmp = new DerOutputStream();

            // encode certificate info
            info.encode(tmp);
            byte[] rawCert = tmp.toByteArray();

            // encode algorithm identifier
            algId.encode(tmp);

            // Create and encode the signature itself.
            sigEngine.update(rawCert, 0, rawCert.length);
            signature = sigEngine.sign();
            tmp.putBitString(signature);

            // Wrap the signed data in a SEQUENCE { data, algorithm, sig }
            out.write(DerValue.tag_Sequence, tmp);
            signedCert = out.toByteArray();
            readOnly = true;

        } catch (IOException e) {
            throw new CertificateEncodingException(e.toString());
      }
    }

    /**
     * Checks that the certificate is currently valid, i.e. the current
     * time is within the specified validity period.
     * 
     * @exception CertificateExpiredException if the certificate has expired.
     * @exception CertificateNotYetValidException if the certificate is not
     * yet valid.
     */  
    public void checkValidity()
    throws CertificateExpiredException, CertificateNotYetValidException {
        Date date = new Date();
        checkValidity(date);
    }
 
    /**
     * Checks that the specified date is within the certificate's 
     * validity period, or basically if the certificate would be 
     * valid at the specified date/time. 
     * 
     * @param date the Date to check against to see if this certificate
     *        is valid at that date/time.
     *
     * @exception CertificateExpiredException if the certificate has expired
     * with respect to the <code>date</code> supplied.
     * @exception CertificateNotYetValidException if the certificate is not
     * yet valid with respect to the <code>date</code> supplied.
     */
    public void checkValidity(Date date)
    throws CertificateExpiredException, CertificateNotYetValidException {

        CertificateValidity interval = null;
        try {
            interval = (CertificateValidity)info.get(CertificateValidity.NAME);
        } catch (Exception e) {
            throw new CertificateNotYetValidException("Incorrect validity period");
        }
        if (interval == null)
            throw new CertificateNotYetValidException("Null validity period");
        interval.valid(date);
    }

    /**
     * Return the requested attribute from the certificate.
     *
     * Note that the X509CertInfo is not cloned for performance reasons.
     * Callers must ensure that they do not modify it. All other
     * attributes are cloned.
     *
     * @param name the name of the attribute.
     * @exception CertificateParsingException on invalid attribute identifier.
     */
    public Object get(String name)
    throws CertificateParsingException {
        X509AttributeName attr = new X509AttributeName(name);
        String id = attr.getPrefix();
        if (!(id.equalsIgnoreCase(NAME))) {
            throw new CertificateParsingException("Invalid root of "
                          + "attribute name, expected [" + NAME +
                          "], received " + "[" + id + "]");
        }
        attr = new X509AttributeName(attr.getSuffix());
        id = attr.getPrefix();

        if (id.equalsIgnoreCase(INFO)) {
	    if (info == null) {
	        return null;
	    }
	    if (attr.getSuffix() != null) {
		try {
		    return info.get(attr.getSuffix());
		} catch (IOException e) {
		    throw new CertificateParsingException(e.toString());
		} catch (CertificateException e) {
		    throw new CertificateParsingException(e.toString());
		}
	    } else {
		return info;
	    }
        } else if (id.equalsIgnoreCase(ALG_ID)) {
            return(algId);
        } else if (id.equalsIgnoreCase(SIGNATURE)) {
	    if (signature != null)
		return signature.clone();
	    else
                return null;
        } else if (id.equalsIgnoreCase(SIGNED_CERT)) {
	    if (signedCert != null)
	        return signedCert.clone();
	    else
		return null;
        } else {
            throw new CertificateParsingException("Attribute name not "
                 + "recognized or get() not allowed for the same: " + id);
        }
    }

    /**
     * Set the requested attribute in the certificate.
     *
     * @param name the name of the attribute.
     * @param obj the value of the attribute.
     * @exception CertificateException on invalid attribute identifier.
     * @exception IOException on encoding error of attribute.
     */
    public void set(String name, Object obj)
    throws CertificateException, IOException {
        // check if immutable
        if (readOnly)
            throw new CertificateException("cannot over-write existing"
                                           + " certificate");

        X509AttributeName attr = new X509AttributeName(name);
        String id = attr.getPrefix();
        if (!(id.equalsIgnoreCase(NAME))) {
            throw new CertificateException("Invalid root of attribute name,"
                           + " expected [" + NAME + "], received " + id);
        }
        attr = new X509AttributeName(attr.getSuffix());
        id = attr.getPrefix();

        if (id.equalsIgnoreCase(INFO)) {
            if (attr.getSuffix() == null) {
	        if (!(obj instanceof X509CertInfo)) {
	            throw new CertificateException("Attribute value should"
                                    + " be of type X509CertInfo.");
	        }
	        info = (X509CertInfo)obj;
                signedCert = null;  //reset this as certificate data has changed
            } else {
	        info.set(attr.getSuffix(), obj);
                signedCert = null;  //reset this as certificate data has changed
            }
        } else {
            throw new CertificateException("Attribute name not recognized or " +
                              "set() not allowed for the same: " + id);
        }
    }

    /**
     * Delete the requested attribute from the certificate.
     *
     * @param name the name of the attribute.
     * @exception CertificateException on invalid attribute identifier.
     * @exception IOException on other errors.
     */
    public void delete(String name)
    throws CertificateException, IOException {
        // check if immutable
        if (readOnly)
            throw new CertificateException("cannot over-write existing"
                                           + " certificate");

        X509AttributeName attr = new X509AttributeName(name);
        String id = attr.getPrefix();
        if (!(id.equalsIgnoreCase(NAME))) {
            throw new CertificateException("Invalid root of attribute name,"
                                   + " expected ["
                                   + NAME + "], received " + id);
        }
        attr = new X509AttributeName(attr.getSuffix());
        id = attr.getPrefix();

        if (id.equalsIgnoreCase(INFO)) {
            if (attr.getSuffix() != null) {
	        info = null;
            } else {
	        info.delete(attr.getSuffix());
            }
        } else if (id.equalsIgnoreCase(ALG_ID)) {
            algId = null;
        } else if (id.equalsIgnoreCase(SIGNATURE)) {
            signature = null;
        } else if (id.equalsIgnoreCase(SIGNED_CERT)) {
            signedCert = null;
        } else {
            throw new CertificateException("Attribute name not recognized or " +
                              "delete() not allowed for the same: " + id);
        }
    }

    /**
     * Return an enumeration of names of attributes existing within this
     * attribute.
     */
    public Enumeration getElements() {
        AttributeNameEnumeration elements = new AttributeNameEnumeration();
        elements.addElement(NAME + DOT + INFO);
        elements.addElement(NAME + DOT + ALG_ID);
        elements.addElement(NAME + DOT + SIGNATURE);
        elements.addElement(NAME + DOT + SIGNED_CERT);

        return(elements.elements());
    }

    /**
     * Return the name of this attribute.
     */
    public String getName() {
        return(NAME);
    }

    /**
     * Returns a printable representation of the certificate.  This does not
     * contain all the information available to distinguish this from any
     * other certificate.  The certificate must be fully constructed
     * before this function may be called.
     */
    public String toString() {
        if (info == null || algId == null || signature == null)
            return "";

        StringBuffer sb = new StringBuffer();

        sb.append("[\n");
        sb.append(info.toString() + "\n");
        sb.append("  Algorithm: [" + algId.toString() + "]\n");

        HexDumpEncoder encoder = new HexDumpEncoder();
        sb.append("  Signature:\n" + encoder.encodeBuffer(signature));
        sb.append("\n]");

        return sb.toString();
    }

    // the strongly typed gets, as per java.security.cert.X509Certificate

    /**
     * Gets the publickey from this certificate.
     *   
     * @return the publickey.
     */  
    public PublicKey getPublicKey() {
        if (info == null)
            return null;
        try {
            PublicKey key = (PublicKey)info.get(CertificateX509Key.NAME
                                + DOT + CertificateX509Key.KEY);
            return key;
        } catch (Exception e) {
            return null;
        }
    }

    /**
     * Gets the version number from the certificate.
     *
     * @return the version number, i.e. 1, 2 or 3.
     */
    public int getVersion() {
        if (info == null)
            return -1;
        try {
            int vers = ((Integer)info.get(CertificateVersion.NAME
                        + DOT + CertificateVersion.VERSION)).intValue();
            return vers+1;
        } catch (Exception e) {
            return -1;
        }
    }

    /**
     * Gets the serial number from the certificate.
     *
     * @return the serial number.
     */
    public BigInteger getSerialNumber() {
        if (info == null)
            return null;
        try {
            SerialNumber ser = (SerialNumber)info.get(
                              CertificateSerialNumber.NAME + DOT +
                              CertificateSerialNumber.NUMBER);
           return ser.getNumber();
        } catch (Exception e) {
            return null;
        }
    }

    /**
     * Gets the subject distinguished name from the certificate.
     *
     * @return the subject name.
     */
    public Principal getSubjectDN() {
        if (info == null)
            return null;
        try {
            Principal subject = (Principal)info.get(
                                 CertificateSubjectName.NAME + DOT +
                                 CertificateSubjectName.DN_NAME);
            return subject;
        } catch (Exception e) {
            return null;
        }
    }

    /**
     * Get subject name as X500Principal. Overrides implementation in
     * X509Certificate with a slightly more efficient version that is
     * also aware of X509CertImpl mutability.
     */
    public X500Principal getSubjectX500Principal() {
        if (info == null) {
	    return null;
	}
        try {
            X500Principal subject = (X500Principal)info.get(
                                CertificateSubjectName.NAME + DOT +
                                CertificateSubjectName.DN_PRINCIPAL);
            return subject;
        } catch (Exception e) {
            return null;
        }
    }

    /**
     * Gets the issuer distinguished name from the certificate.
     *
     * @return the issuer name.
     */
    public Principal getIssuerDN() {
        if (info == null)
            return null;
        try {
            Principal issuer = (Principal)info.get(
                                CertificateIssuerName.NAME + DOT +
                                CertificateIssuerName.DN_NAME);
            return issuer;
        } catch (Exception e) {
            return null;
        }
    }
    
    /**
     * Get issuer name as X500Principal. Overrides implementation in
     * X509Certificate with a slightly more efficient version that is
     * also aware of X509CertImpl mutability.
     */
    public X500Principal getIssuerX500Principal() {
        if (info == null) {
	    return null;
	}
        try {
            X500Principal issuer = (X500Principal)info.get(
                                CertificateIssuerName.NAME + DOT +
                                CertificateIssuerName.DN_PRINCIPAL);
            return issuer;
        } catch (Exception e) {
            return null;
        }
    }

    /**
     * Gets the notBefore date from the validity period of the certificate.
     *
     * @return the start date of the validity period.
     */
    public Date getNotBefore() {
        if (info == null)
            return null;
        try {
            Date d = (Date) info.get(CertificateValidity.NAME + DOT +
                                        CertificateValidity.NOT_BEFORE);
            return d;
        } catch (Exception e) {
            return null;
        }
    }

    /**
     * Gets the notAfter date from the validity period of the certificate.
     *
     * @return the end date of the validity period.
     */
    public Date getNotAfter() {
        if (info == null)
            return null;
        try {
            Date d = (Date) info.get(CertificateValidity.NAME + DOT +
                                     CertificateValidity.NOT_AFTER);
            return d;
        } catch (Exception e) {
            return null;
        }
    }

    /**
     * Gets the DER encoded certificate informations, the
     * <code>tbsCertificate</code> from this certificate. 
     * This can be used to verify the signature independently.   
     *   
     * @return the DER encoded certificate information.
     * @exception CertificateEncodingException if an encoding error occurs.
     */  
    public byte[] getTBSCertificate() throws CertificateEncodingException {
        if (info != null) {
            return info.getEncodedInfo();
        } else
            throw new CertificateEncodingException("Uninitialized certificate");
    }

    /**
     * Gets the raw Signature bits from the certificate.
     *
     * @return the signature.
     */
    public byte[] getSignature() {
        if (signature == null)
            return null;
        byte[] dup = new byte[signature.length];
        System.arraycopy(signature, 0, dup, 0, dup.length);
        return dup;
    }

    /**
     * Gets the signature algorithm name for the certificate
     * signature algorithm.
     * For example, the string "SHA-1/DSA" or "DSS".
     *
     * @return the signature algorithm name.
     */
    public String getSigAlgName() {
        if (algId == null)
            return null;
        return (algId.getName());
    }

    /**
     * Gets the signature algorithm OID string from the certificate.
     * For example, the string "1.2.840.10040.4.3"
     *
     * @return the signature algorithm oid string.
     */
    public String getSigAlgOID() {
        if (algId == null)
            return null;
        ObjectIdentifier oid = algId.getOID();
        return (oid.toString());
    }

    /**
     * Gets the DER encoded signature algorithm parameters from this
     * certificate's signature algorithm.
     *   
     * @return the DER encoded signature algorithm parameters, or
     *         null if no parameters are present.
     */  
    public byte[] getSigAlgParams() {
        if (algId == null)
            return null;
        try {
            return algId.getEncodedParams();
        } catch (IOException e) {
            return null;
        }
    }

    /**
     * Gets the Issuer Unique Identity from the certificate.
     *
     * @return the Issuer Unique Identity.
     */
    public boolean[] getIssuerUniqueID() {
        if (info == null)
            return null;
        try {
            UniqueIdentity id = (UniqueIdentity)info.get(
                                 CertificateIssuerUniqueIdentity.NAME
                            + DOT + CertificateIssuerUniqueIdentity.ID);
            if (id == null)
                return null;
            else
                return (id.getId());
        } catch (Exception e) {
            return null;
        }
    }

    /**
     * Gets the Subject Unique Identity from the certificate.
     *
     * @return the Subject Unique Identity.
     */
    public boolean[] getSubjectUniqueID() {
        if (info == null)
            return null;
        try {
            UniqueIdentity id = (UniqueIdentity)info.get(
                                 CertificateSubjectUniqueIdentity.NAME
                            + DOT + CertificateSubjectUniqueIdentity.ID);
            if (id == null)
                return null;
            else
                return (id.getId());
        } catch (Exception e) {
            return null;
        }
    }

    /**
     * Get AuthorityKeyIdentifier extension
     * @return AuthorityKeyIdentifier object or null (if no such object 
     * in certificate)
     */
    public AuthorityKeyIdentifierExtension getAuthorityKeyIdentifierExtension() 
    { 
        return (AuthorityKeyIdentifierExtension)
	    getExtension(PKIXExtensions.AuthorityKey_Id);
    }

    /**
     * Get BasicConstraints extension
     * @return BasicConstraints object or null (if no such object in 
     * certificate)
     */
    public BasicConstraintsExtension getBasicConstraintsExtension() {
	return (BasicConstraintsExtension)
	    getExtension(PKIXExtensions.BasicConstraints_Id);
    }

    /**
     * Get CertificatePoliciesExtension
     * @return CertificatePoliciesExtension or null (if no such object in 
     * certificate)
     */
    public CertificatePoliciesExtension getCertificatePoliciesExtension() {
	return (CertificatePoliciesExtension)
	    getExtension(PKIXExtensions.CertificatePolicies_Id);
    }

    /**
     * Get ExtendedKeyUsage extension
     * @return ExtendedKeyUsage extension object or null (if no such object 
     * in certificate)
     */
    public ExtendedKeyUsageExtension getExtendedKeyUsageExtension() {
	return (ExtendedKeyUsageExtension)
	    getExtension(PKIXExtensions.ExtendedKeyUsage_Id);
    }

    /**
     * Get IssuerAlternativeName extension
     * @return IssuerAlternativeName object or null (if no such object in 
     * certificate)
     */
    public IssuerAlternativeNameExtension getIssuerAlternativeNameExtension() {
	return (IssuerAlternativeNameExtension)
	    getExtension(PKIXExtensions.IssuerAlternativeName_Id);
    }

    /**
     * Get NameConstraints extension
     * @return NameConstraints object or null (if no such object in certificate)
     */
    public NameConstraintsExtension getNameConstraintsExtension() {
	return (NameConstraintsExtension)
	    getExtension(PKIXExtensions.NameConstraints_Id);
    }

    /**
     * Get PolicyConstraints extension
     * @return PolicyConstraints object or null (if no such object in 
     * certificate)
     */
    public PolicyConstraintsExtension getPolicyConstraintsExtension() {
	return (PolicyConstraintsExtension)
	    getExtension(PKIXExtensions.PolicyConstraints_Id);
    }

    /**
     * Get PolicyMappingsExtension extension
     * @return PolicyMappingsExtension object or null (if no such object 
     * in certificate)
     */
    public PolicyMappingsExtension getPolicyMappingsExtension() {
	return (PolicyMappingsExtension)
	    getExtension(PKIXExtensions.PolicyMappings_Id);
    }

    /**
     * Get PrivateKeyUsage extension
     * @return PrivateKeyUsage object or null (if no such object in certificate)
     */
    public PrivateKeyUsageExtension getPrivateKeyUsageExtension() {
	return (PrivateKeyUsageExtension)
	    getExtension(PKIXExtensions.PrivateKeyUsage_Id);
    }

    /**
     * Get SubjectAlternativeName extension
     * @return SubjectAlternativeName object or null (if no such object in 
     * certificate)
     */
    public SubjectAlternativeNameExtension getSubjectAlternativeNameExtension() 
    {
	return (SubjectAlternativeNameExtension)
	    getExtension(PKIXExtensions.SubjectAlternativeName_Id);
    }

    /**
     * Get SubjectKeyIdentifier extension
     * @return SubjectKeyIdentifier object or null (if no such object in 
     * certificate)
     */
    public SubjectKeyIdentifierExtension getSubjectKeyIdentifierExtension() {
	return (SubjectKeyIdentifierExtension)
	    getExtension(PKIXExtensions.SubjectKey_Id);
    }

    /**
     * Get CRLDistributionPoints extension
     * @return CRLDistributionPoints object or null (if no such object in 
     * certificate)
     */
    public CRLDistributionPointsExtension getCRLDistributionPointsExtension() {
	return (CRLDistributionPointsExtension)
	    getExtension(PKIXExtensions.CRLDistributionPoints_Id);
    }

    /** 
     * Return true if a critical extension is found that is
     * not supported, otherwise return false. 
     */   
    public boolean hasUnsupportedCriticalExtension() {
        if (info == null)
            return false;
        try {
            CertificateExtensions exts = (CertificateExtensions)info.get(
                                         CertificateExtensions.NAME);
            if (exts == null)
                return false;
            return exts.hasUnsupportedCriticalExtension(); 
        } catch (Exception e) {
            return false;
        }
    } 
 
    /**
     * Gets a Set of the extension(s) marked CRITICAL in the
     * certificate. In the returned set, each extension is
     * represented by its OID string.
     *   
     * @return a set of the extension oid strings in the
     * certificate that are marked critical.
     */  
    public Set getCriticalExtensionOIDs() {
        if (info == null)
            return null;
        try {
            CertificateExtensions exts = (CertificateExtensions)info.get(
                                         CertificateExtensions.NAME);
            if (exts == null)
                return null;
            HashSet extSet = new HashSet(11);
            Extension ex;
            for (Enumeration e=exts.getElements(); e.hasMoreElements(); ) {
                ex = (Extension)e.nextElement();
                if (ex.isCritical())
                    extSet.add(((ObjectIdentifier)ex.getExtensionId()).toString());
            }
            return extSet;
        } catch (Exception e) {
            return null;
        }
    }
   
    /**
     * Gets a Set of the extension(s) marked NON-CRITICAL in the
     * certificate. In the returned set, each extension is
     * represented by its OID string.
     *   
     * @return a set of the extension oid strings in the
     * certificate that are NOT marked critical.
     */  
    public Set getNonCriticalExtensionOIDs() {
        if (info == null)
            return null;
        try {
            CertificateExtensions exts = (CertificateExtensions)info.get(
                                         CertificateExtensions.NAME);
            if (exts == null)
                return null;
            
            HashSet extSet = new HashSet(11);
            Extension ex;
            for (Enumeration e=exts.getElements(); e.hasMoreElements(); ) {
                ex = (Extension)e.nextElement();
                if ( ! ex.isCritical())
                    extSet.add(((ObjectIdentifier)ex.getExtensionId()).toString());
            }
            return extSet;
        } catch (Exception e) {
            return null;
        }
    }

    /**
     * Gets the extension identified by the given ObjectIdentifier
     *
     * @param oid the Object Identifier value for the extension.
     * @return Extension or null if certificate does not contain this
     *         extension
     */
    public Extension getExtension(ObjectIdentifier oid) {
	if (info == null) {
	    return null;
	}
	try {
	    CertificateExtensions extensions = null;
	    try {
		extensions = (CertificateExtensions)info.get(CertificateExtensions.NAME);
	    } catch (CertificateException ce) {
		return null;
	    }
	    if (extensions == null) {
		return null;
	    } else {
		Extension ex = null;
		for (Enumeration e=extensions.getElements(); e.hasMoreElements();) {
		    ex = (Extension)e.nextElement();
		    if (ex.getExtensionId().equals(oid))
			//XXXX May want to consider cloning this
			return ex;
		}
		/* no such extension in this certificate */
  		return null;
  	    }
	} catch (IOException ioe) {
	    return null;
	}
    }

    /**
     * Gets the DER encoded extension identified by the given
     * oid String.
     *
     * @param oid the Object Identifier value for the extension.
     */
    public byte[] getExtensionValue(String oid) {
        try {
	    ObjectIdentifier findOID = new ObjectIdentifier(oid);
            String extAlias = OIDMap.getName(findOID);
            Extension certExt = null;

            if (extAlias == null) { // may be unknown
                // get the extensions, search thru' for this oid
                CertificateExtensions exts = (CertificateExtensions)info.get(
                                         CertificateExtensions.NAME);
                if (exts == null)
                    return null;
                
                Extension ex = null;;
                ObjectIdentifier inCertOID;
                for (Enumeration e=exts.getElements(); e.hasMoreElements();) {
                    ex = (Extension)e.nextElement();
                    inCertOID = ex.getExtensionId();
                    if (inCertOID.equals(findOID)) {
                        certExt = ex;
                        break;
                    }
                }
            } else { // there's sub-class that can handle this extension
                certExt = (Extension)this.get(extAlias);
            }
            if (certExt == null)
                return null;
            byte[] extData = certExt.getExtensionValue();
            if (extData == null)
                return null;

            DerOutputStream out = new DerOutputStream();
            out.putOctetString(extData);
            return out.toByteArray();
        } catch (Exception e) {
            return null;
        }
    }

    /**
     * Get a boolean array representing the bits of the KeyUsage extension,
     * (oid = 2.5.29.15).
     * @return the bit values of this extension as an array of booleans.
     */  
    public boolean[] getKeyUsage() {
        try {
            String extAlias = OIDMap.getName(PKIXExtensions.KeyUsage_Id);
            if (extAlias == null)
                return null;

            KeyUsageExtension certExt = (KeyUsageExtension)this.get(extAlias);
            if (certExt == null)
                return null;

            boolean[] ret = certExt.getBits();
	    if (ret.length < NUM_STANDARD_KEY_USAGE) {	 
		boolean[] usageBits = new boolean[NUM_STANDARD_KEY_USAGE];
		System.arraycopy(ret, 0, usageBits, 0, ret.length);
		ret = usageBits;
	    }
	    return ret;
        } catch (Exception e) {
            return null;
        }
    }

    /**
     * This method are the overridden implementation of
     * getExtendedKeyUsage method in X509Certificate in the Sun
     * provider. It is better performance-wise since it returns cached
     * values.
     */
    public synchronized List getExtendedKeyUsage()
	throws CertificateParsingException {
	if (readOnly && extKeyUsage != null) {
	    return extKeyUsage;
	} else {
	    ExtendedKeyUsageExtension ext = getExtendedKeyUsageExtension();
	    if (ext == null) {
		return null;
	    }
	    extKeyUsage =
		Collections.unmodifiableList(ext.getExtendedKeyUsage());
	    return extKeyUsage;
	}
    }

    /**
     * This static method is the default implementation of the
     * getExtendedKeyUsage method in X509Certificate. A
     * X509Certificate provider generally should overwrite this to
     * provide among other things caching for better performance.
     */
    public static List getExtendedKeyUsage(X509Certificate cert)
	throws CertificateParsingException {
	try {
	    byte[] ext = cert.getExtensionValue(EXTENDED_KEY_USAGE_OID);
	    if (ext == null)
		return null;
	    DerValue val = new DerValue(ext);
	    byte[] data = val.getOctetString();
	    
	    ExtendedKeyUsageExtension ekuExt =
		new ExtendedKeyUsageExtension(Boolean.FALSE, data);
	    return Collections.unmodifiableList(ekuExt.getExtendedKeyUsage());
	} catch (IOException ioe) {
	    CertificateParsingException cpe =
		new CertificateParsingException();
	    cpe.initCause(ioe);
	    throw cpe;
	}
    }

    /**
     * Get the certificate constraints path length from the
     * the critical BasicConstraints extension, (oid = 2.5.29.19).
     * @return the length of the constraint.
     */
    public int getBasicConstraints() {
        try {
            String extAlias = OIDMap.getName(PKIXExtensions.BasicConstraints_Id);
            if (extAlias == null)
                return -1;
	    BasicConstraintsExtension certExt =
		        (BasicConstraintsExtension)this.get(extAlias);	    
            if (certExt == null)
                return -1;

            if (((Boolean)certExt.get(BasicConstraintsExtension.IS_CA)
                 ).booleanValue() == true)
                return ((Integer)certExt.get(
                        BasicConstraintsExtension.PATH_LEN)).intValue();
            else
                return -1;
        } catch (Exception e) {
            return -1;
        }
    }

    /**
     * Converts a GeneralNames structure into an immutable Collection of 
     * alternative names (subject or issuer) in the form required by 
     * {@link #getSubjectAlternativeNames} or 
     * {@link #getIssuerAlternativeNames}.
     *
     * @param names the GeneralNames to be converted
     * @return an immutable Collection of alternative names
     */
    private static Collection makeAltNames(GeneralNames names) {
        if (names.isEmpty())
	    return Collections.EMPTY_SET;

	HashSet newNames = new HashSet(names.size());
	Iterator i = names.iterator();
	while (i.hasNext()) {
	    GeneralName gname = (GeneralName) i.next();
	    GeneralNameInterface name = gname.getName();
	    ArrayList nameEntry = new ArrayList(2);
	    nameEntry.add(new Integer(name.getType()));
            switch (name.getType()) {
            case GeneralNameInterface.NAME_RFC822:
       	        nameEntry.add(((RFC822Name) name).getName());
                break;
       	    case GeneralNameInterface.NAME_DNS:
       	        nameEntry.add(((DNSName) name).getName());
                break;
            case GeneralNameInterface.NAME_DIRECTORY:
       	        nameEntry.add(((X500Name) name).getRFC2253Name());
                break;
            case GeneralNameInterface.NAME_URI:
     	        nameEntry.add(((URIName) name).getName());
                break;
            case GeneralNameInterface.NAME_IP:
		try {
       	            nameEntry.add(((IPAddressName) name).getName());
		} catch (IOException ioe) {
		    // IPAddressName in cert is bogus
		    throw new RuntimeException("IPAddress cannot be parsed",
			ioe);
		}
                break;
            case GeneralNameInterface.NAME_OID:
       	        nameEntry.add(((OIDName) name).getOID().toString());
                break;
            default:
		// add DER encoded form
	        DerOutputStream derOut = new DerOutputStream();
		try {
		    name.encode(derOut);
	        } catch (IOException ioe) {
		    // should not occur since name has already been decoded
		    // from cert (this would indicate a bug in our code)
		    throw new RuntimeException("name cannot be encoded", ioe);
		}
                nameEntry.add(derOut.toByteArray());
		break;
	    }
	    newNames.add(Collections.unmodifiableList(nameEntry));
	}
	return Collections.unmodifiableCollection(newNames);
    }

    /**
     * Checks a Collection of altNames and clones any name entries of type 
     * byte [].
     */
    private static Collection cloneAltNames(Collection altNames) {
	boolean mustClone = false;
	Iterator i = altNames.iterator();
	while (i.hasNext() && !mustClone) {
	    List nameEntry = (List) i.next();
	    if (nameEntry.get(1) instanceof byte []) {
		// must clone names
	        mustClone = true;
	    }
	}
	if (mustClone) {
	    HashSet namesCopy = new HashSet(altNames.size());
            i = altNames.iterator();
            while (i.hasNext()) {
                List nameEntry = (List) i.next();
		Object nameObject = nameEntry.get(1);
	        if (nameObject instanceof byte []) {
		    ArrayList nameEntryCopy = new ArrayList((List) nameEntry);
		    nameEntryCopy.set(1, ((byte []) nameObject).clone());
                    namesCopy.add(Collections.unmodifiableList(nameEntryCopy));
		} else 
		    namesCopy.add(nameEntry);
	    }
	    return Collections.unmodifiableCollection(namesCopy);
	} else
	    return altNames;
    }

    /**
     * This method are the overridden implementation of
     * getSubjectAlternativeNames method in X509Certificate in the Sun
     * provider. It is better performance-wise since it returns cached
     * values.
     */
    public synchronized Collection getSubjectAlternativeNames()
 	throws CertificateParsingException {
	// return cached value if we can
	if (readOnly && subjectAlternativeNames != null) 
	    return cloneAltNames(subjectAlternativeNames);
	
	SubjectAlternativeNameExtension subjectAltNameExt =
	    getSubjectAlternativeNameExtension();
	if (subjectAltNameExt == null)
	    return null;
	
	GeneralNames names;
	try {
	    names = (GeneralNames) subjectAltNameExt.get
		(SubjectAlternativeNameExtension.SUBJECT_NAME);
	} catch (IOException ioe) { 
	    // should not occur
	    return Collections.EMPTY_SET;
	}
	subjectAlternativeNames = makeAltNames(names);
	return subjectAlternativeNames;
    }

    /**
     * This static method is the default implementation of the
     * getSubjectAlternaitveNames method in X509Certificate. A
     * X509Certificate provider generally should overwrite this to
     * provide among other things caching for better performance.
     */
    public static Collection getSubjectAlternativeNames(X509Certificate cert)
 	throws CertificateParsingException {
	try {
	    byte[] ext = cert.getExtensionValue(SUBJECT_ALT_NAME_OID);
	    if (ext == null) {
		return null;
	    }
	    DerValue val = new DerValue(ext);
	    byte[] data = val.getOctetString();

	    SubjectAlternativeNameExtension subjectAltNameExt =
		new SubjectAlternativeNameExtension(Boolean.FALSE,
						    data);

	    GeneralNames names;
	    try {
		names = (GeneralNames) subjectAltNameExt.get
		    (SubjectAlternativeNameExtension.SUBJECT_NAME);
	    }  catch (IOException ioe) { 
		// should not occur
		return Collections.EMPTY_SET;
	    }
	    return makeAltNames(names);
	} catch (IOException ioe) {
	    CertificateParsingException cpe =
		new CertificateParsingException();
	    cpe.initCause(ioe);
	    throw cpe;
	}
    }

    /**
     * This method are the overridden implementation of
     * getIssuerAlternativeNames method in X509Certificate in the Sun
     * provider. It is better performance-wise since it returns cached
     * values.
     */
    public synchronized Collection getIssuerAlternativeNames()
	throws CertificateParsingException {
	// return cached value if we can
	if (readOnly && issuerAlternativeNames != null)
	    return cloneAltNames(issuerAlternativeNames);
	
	IssuerAlternativeNameExtension issuerAltNameExt =
	    getIssuerAlternativeNameExtension();
	if (issuerAltNameExt == null)
	    return null;
	
	GeneralNames names;
	try {
	    names = (GeneralNames) issuerAltNameExt.get
		(IssuerAlternativeNameExtension.ISSUER_NAME);
	} catch (IOException ioe) { 
	    // should not occur
	    return Collections.EMPTY_SET; 
	}
	issuerAlternativeNames = makeAltNames(names);
	return issuerAlternativeNames;
    }

    /**
     * This static method is the default implementation of the
     * getIssuerAlternaitveNames method in X509Certificate. A
     * X509Certificate provider generally should overwrite this to
     * provide among other things caching for better performance.
     */
    public static Collection getIssuerAlternativeNames(X509Certificate cert)
	throws CertificateParsingException {
	try {
	    byte[] ext = cert.getExtensionValue(ISSUER_ALT_NAME_OID);
	    if (ext == null) {
		return null;
	    }

	    DerValue val = new DerValue(ext);
	    byte[] data = val.getOctetString();

	    IssuerAlternativeNameExtension issuerAltNameExt =
		new IssuerAlternativeNameExtension(Boolean.FALSE,
						    data);
	    GeneralNames names;
	    try {
		names = (GeneralNames) issuerAltNameExt.get
		    (IssuerAlternativeNameExtension.ISSUER_NAME);
	    }  catch (IOException ioe) { 
		// should not occur
		return Collections.EMPTY_SET;
	    }
	    return makeAltNames(names);
	} catch (IOException ioe) {
	    CertificateParsingException cpe =
		new CertificateParsingException();
	    cpe.initCause(ioe);
	    throw cpe;
	}
    }

    /************************************************************/

    /*
     * Cert is a SIGNED ASN.1 macro, a three elment sequence:
     *
     *	- Data to be signed (ToBeSigned) -- the "raw" cert
     *	- Signature algorithm (SigAlgId)
     *	- The signature bits
     *
     * This routine unmarshals the certificate, saving the signature
     * parts away for later verification.
     */
    private void parse(DerValue val)
    throws CertificateException, IOException {
        // check if can over write the certificate
        if (readOnly)
            throw new CertificateParsingException(
                      "cannot over-write existing certificate");

	if (val.data == null)
	    throw new CertificateParsingException(
		      "invalid DER-encoded certificate data");

        signedCert = val.toByteArray();
        DerValue[] seq = new DerValue[3];

        seq[0] = val.data.getDerValue();
        seq[1] = val.data.getDerValue();
        seq[2] = val.data.getDerValue();
	
        if (val.data.available() != 0) {
            throw new CertificateParsingException("signed overrun, bytes = "
                                     + val.data.available());
        }
        if (seq[0].tag != DerValue.tag_Sequence) {
            throw new CertificateParsingException("signed fields invalid");
        }

        algId = AlgorithmId.parse(seq[1]);
        signature = seq[2].getBitString();

        if (seq[1].data.available() != 0) {
            throw new CertificateParsingException("algid field overrun");
        }
        if (seq[2].data.available() != 0)
            throw new CertificateParsingException("signed fields overrun");

        // The CertificateInfo
        info = new X509CertInfo(seq[0]);

        // the "inner" and "outer" signature algorithms must match
        AlgorithmId infoSigAlg = (AlgorithmId)info.get(
                                              CertificateAlgorithmId.NAME
                                              + DOT +
                                              CertificateAlgorithmId.ALGORITHM);
        if (! algId.equals(infoSigAlg))
            throw new CertificateException("Signature algorithm mismatch");
        readOnly = true;
    }
    
    /**
     * Extract the subject or issuer X500Principal from an X509Certificate. 
     * Parses the encoded form of the cert to preserve the principal's 
     * ASN.1 encoding. 
     */
    private static X500Principal getX500Principal(X509Certificate cert,
	    boolean getIssuer) throws Exception {
	byte[] encoded = cert.getEncoded();
	DerInputStream derIn = new DerInputStream(encoded);
	DerValue tbsCert = derIn.getSequence(3)[0];
	DerInputStream tbsIn = tbsCert.data;
	DerValue tmp;
	tmp = tbsIn.getDerValue();
	// skip version number if present
	if (tmp.isContextSpecific((byte)0)) {
	  tmp = tbsIn.getDerValue();
	}
	// tmp always contains serial number now
	tmp = tbsIn.getDerValue();		// skip signature
	tmp = tbsIn.getDerValue();		// issuer
	if (getIssuer == false) {
	    tmp = tbsIn.getDerValue();		// skip validity
	    tmp = tbsIn.getDerValue();		// subject
	}
	byte[] principalBytes = tmp.toByteArray();
	return new X500Principal(principalBytes);
    }

    /**
     * Extract the subject X500Principal from an X509Certificate.
     * Called from java.security.cert.X509Certificate.getSubjectX500Principal().
     */
    public static X500Principal getSubjectX500Principal(X509Certificate cert) {
    	try {
	    return getX500Principal(cert, false);
	} catch (Exception e) {
	    throw new RuntimeException("Could not parse subject", e);
	}
    }
    
    /**
     * Extract the issuer X500Principal from an X509Certificate.
     * Called from java.security.cert.X509Certificate.getIssuerX500Principal().
     */
    public static X500Principal getIssuerX500Principal(X509Certificate cert) {
    	try {
	    return getX500Principal(cert, true);
	} catch (Exception e) {
	    throw new RuntimeException("Could not parse issuer", e);
	}
    }
    
    /**
     * Returned the encoding of the given certificate for internal use.
     * Callers must guarantee that they neither modify it nor expose it 
     * to untrusted code. Uses getEncodedInternal() if the certificate
     * is instance of X509CertImpl, getEncoded() otherwise.
     */
    public static byte[] getEncodedInternal(Certificate cert) 
	    throws CertificateEncodingException {
	if (cert instanceof X509CertImpl) {
	    return ((X509CertImpl)cert).getEncodedInternal();
	} else {
	    return cert.getEncoded();
	}
    }
    
    /**
     * Utility method to convert an arbitrary instance of X509Certificate
     * to a X509CertImpl. Does a cast if possible, otherwise reparses
     * the encoding.
     */
    public static X509CertImpl toImpl(X509Certificate cert) 
	    throws CertificateException {
	if (cert instanceof X509CertImpl) {
	    return (X509CertImpl)cert;
	} else {
	    return X509Factory.intern(cert);
	}
    }
    
    /**
     * Utility method to test if a certificate is self-issued. This is
     * the case iff the subject and issuer X500Principals are equal.
     */
    public static boolean isSelfIssued(X509Certificate cert) {
	X500Principal subject = cert.getSubjectX500Principal();
	X500Principal issuer = cert.getIssuerX500Principal();
	return subject.equals(issuer);
    }
    
}
