/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package javax.crypto;

import java.io.*;
import java.security.AccessController;
import java.security.PublicKey;
import java.security.NoSuchProviderException;
import java.security.PrivilegedExceptionAction;
import java.security.cert.*;
import java.util.*;
import java.net.URL;
import java.net.JarURLConnection;
import java.net.MalformedURLException;
import java.util.jar.*;
import java.util.zip.ZipEntry;

import sun.security.validator.Validator;

/**
 * This class verifies a JAR file and all its supporting JAR files.
 * 
 * @version 1.16, 06/24/03
 * @author Sharon Liu
 * @since 1.4
 */
final class JarVerifier {

    private static final boolean debug = false;

    // Cache for holding provider certificates which has been verified
    private Vector verifiedSignerCache = null;

    // The URL for the JAR file we want to verify.
    private URL jarURL;

    // The JarFile for the JAR file we verified.
    private JarFile jarFile = null;
    
    private Validator validator;

    /**
     * Creates a JarVerifier object to verify the given URL.
     *
     * @param jarURL the JAR file to be verified.
     */
    JarVerifier(URL jarURL, Validator validator) {
	this.jarURL = jarURL;
	this.validator = validator;
	verifiedSignerCache = new Vector(2);
    }

    /**
     * Verify the JAR file is signed by an entity which has a certificate
     * issued by a trusted CA.
     *
     * @param trustedCaCerts certificates of trusted CAs.
     */
    void verify()
        throws JarException, IOException {
	try {
	    verifyJars(jarURL, null);
	} catch (NoSuchProviderException nspe) {
	    throw new JarException("Cannot verify " + jarURL.toString());
	} catch (CertificateException ce) {
	    throw new JarException("Cannot verify " + jarURL.toString());
	} finally {
	    verifiedSignerCache = null;
	}
    }

    /**
     * Returns a JarFile for the <code>jarURL</code> passed to the
     * constructor of this class.
     *
     * One should call verifyJar() before calling getJarFile(); otherwise,
     * null is returned.
     *
     * The caller of getJarFile() should close the returned JarFile.
     */
    JarFile getJarFile() {
	return jarFile;
    }

    /**
     * Verify a JAR file and all of its supporting JAR files are signed by 
     * a signer with a certificate which
     * can be traced back to a trusted CA.
     */
    private void verifyJars(URL jarURL, Vector verifiedJarsCache)
        throws NoSuchProviderException, CertificateException, IOException
    {
	String jarURLString = jarURL.toString();

	// Check whether this JAR file has been verified before.
	if ((verifiedJarsCache == null) ||
	    !verifiedJarsCache.contains(jarURLString)) {
	    
	    // Verify just one jar file and find out the information
	    // about supporting JAR files.
	    String supportingJars = verifySingleJar(jarURL);

	    // Add the url for the verified JAR into verifiedJarsCache.
	    if (verifiedJarsCache != null)
		verifiedJarsCache.addElement(jarURLString);

	    // Verify all supporting JAR files if there are any.	    
	    if (supportingJars != null) {
		if (verifiedJarsCache == null) {
		    verifiedJarsCache = new Vector();
		    verifiedJarsCache.addElement(jarURLString);
		}
		verifyManifestClassPathJars(jarURL, 
					    supportingJars, 
					    verifiedJarsCache);
	    }
	}
	
    }

    private void verifyManifestClassPathJars(URL baseURL, 
					   String supportingJars,
					   Vector verifiedJarsCache)
        throws NoSuchProviderException, CertificateException, IOException
    {
	// Get individual JAR file names
	String[] jarFileNames = parseAttrClasspath(supportingJars);

	try {
	    // For each JAR file, verify it
	    for (int i = 0; i < jarFileNames.length; i++) {
		URL url = new URL(baseURL, jarFileNames[i]);
		verifyJars(url, verifiedJarsCache);
	    }
	} catch (MalformedURLException mue) {
	    MalformedURLException ex = new MalformedURLException(
		"The JAR file " + baseURL.toString() +
		" contains invalid URLs in its Class-Path attribute");
	    ex.initCause(mue);
	    throw ex;
	}
    }

    /*
     * Verify the signature on the JAR file and return
     * the value of the manifest attribute "CLASS_PATH".
     * If the manifest doesn't contain the attribute
     * "CLASS_PATH", return null.
     */
    private String verifySingleJar(URL jarURL) 
        throws NoSuchProviderException, CertificateException, IOException
    {
	// If the protocol of jarURL isn't "jar", we should 
	// construct a JAR URL so we can open a JarURLConnection
	// for verifying this provider.
	final URL url = jarURL.getProtocol().equalsIgnoreCase("jar")?
	                jarURL : new URL("jar:" + jarURL.toString() + "!/");

	JarFile jf = null;
	boolean isCached = true;

	try {
	    try {
		jf = (JarFile) AccessController.doPrivileged(
				   new PrivilegedExceptionAction() {
		    public Object run() throws Exception {
			JarURLConnection conn = (JarURLConnection)
			    url.openConnection();
			return conn.getJarFile();
		    }
		});
	    } catch (java.security.PrivilegedActionException pae) {
		SecurityException se = new SecurityException(
		    "Cannot verify " + url.toString());
		se.initCause(pae);
		throw se;
	    }

	    // Read in each jar entry, so the subsequent call
	    // JarEntry.getCertificates() will return Certificates
	    // for signed entries.
	    // Note: Since jars signed by 3rd party tool, e.g., 
	    // netscape signtool, may have its manifest at the 
	    // end, two separate loops maybe necessary.
	    byte[] buffer = new byte[8192]; 
	    Vector entriesVec = new Vector(); 
	    
	    Enumeration entries = jf.entries(); 
	    while (entries.hasMoreElements()) { 
		JarEntry je = (JarEntry)entries.nextElement(); 
		entriesVec.addElement(je); 
		BufferedInputStream is = 
		    new BufferedInputStream(jf.getInputStream(je)); 
		int n; 
		try {
		    while ((n = is.read(buffer, 0, buffer.length)) != -1) { 
		        // we just read. this will throw a SecurityException 
		        // if  a signature/digest check fails. 
		    }  
		} finally {
		    is.close(); 
		}
	    } 
	
	    if (this.jarURL.equals(jarURL)) {
		this.jarFile = jf;
	    } else {
		// have to close the jar file at the end of this routine
		isCached = false;
	    }

	    // Throws JarException if the JAR has no manifest
	    // which means the JAR isn't signed.
	    Manifest man = jf.getManifest();
	    if (man == null)
		throw new JarException(jarURL.toString() + " is not signed.");
	    
	    // Make sure every class file in the JAR is signed properly:
	    // We must check whether the signer's certificate
	    // can be traced back to a trusted CA
	    // Once we've verified that a signer's cert can be
	    // traced back to a trusted CA, the signer's cert
	    // is kept in the cache 'verifiedSignerCache'.
	    Enumeration e = jf.entries();
	    while (e.hasMoreElements()) {
		JarEntry je = (JarEntry) e.nextElement();
	    
		if (je.isDirectory())
		    continue;
		
		// Every file must be signed except files under META-INF.
		Certificate[] certs = je.getCertificates();
		if ((certs == null) || (certs.length == 0)) {
		    if (!je.getName().startsWith("META-INF"))
			throw new JarException(jarURL.toString() +
					       " has unsigned entries - " 
					       + je.getName() );
		} else {
		    // A JAR file may be signed by multiple signers.
		    // So certs may contain mutiple certificate
		    // chains. Check whether at least one of the signers 
		    // can be trusted.
		    // The signer is trusted iff
		    // 1) its certificate chain can be verified
		    // 2) the last certificate on its chain is one of 
		    //    JCE's trusted CAs
		    //         OR
		    //    the last certificate on its chain is issued
		    //    by one of JCE's trusted CAs.

		    int startIndex = 0;
		    X509Certificate[] certChain;
		    boolean signedAsExpected = false;
		    
		    while ((certChain = getAChain(certs, startIndex)) != null) {
			// First, check if the signer has been successfully
			// verified before.
			// If not, verify the signature for all certificates 
			// in the chain with the specified Trusted CA 
			// certificates. 
			if (verifiedSignerCache.contains(
				(X509Certificate)certChain[0])) {
			    signedAsExpected = true;
			    break;
			} else if (isTrusted(certChain)) {
			    signedAsExpected = true;
			    verifiedSignerCache.addElement(certChain[0]);
			    break;
			}
			// Proceed to the next chain.
			startIndex += certChain.length;
		    }

		    if (!signedAsExpected) {
			throw new JarException(jarURL.toString() +
					       " is not signed by a" +
					       " trusted signer.");
		    }
		}
	    }
	    
	    // Return the value of the attribute CLASS_PATH
	    return man.getMainAttributes().getValue(
		       Attributes.Name.CLASS_PATH);
	} finally {
	    if ((jf != null) && (isCached == false)) { 
		jf = null;
	    }
	}
    }

    /*
     * Parse the manifest attribute Class-Path.
     */
    private static String[] parseAttrClasspath(String supportingJars)
        throws JarException
    {
	supportingJars = supportingJars.trim();

	int endIndex = supportingJars.indexOf(' ');
	String name = null;
	Vector values = new Vector();
	boolean done = false;

	do {
	    if (endIndex > 0) {
		name = supportingJars.substring(0, endIndex);
		// Since supportingJars has been trimmed,
		// endIndex + 1 must be less than
		// supportingJars.length().
		supportingJars = supportingJars.substring(endIndex + 1).trim();
		endIndex = supportingJars.indexOf(' ');
	    } else {
		name = supportingJars;
		done = true;
	    }
	    if (name.endsWith(".jar")) {
		values.addElement(name);
	    } else {
		// Cannot verify. Throw a JarException.
		throw new JarException("The provider contains " +
				       "un-verifiable components");
	    }
	} while (!done);
	
	return (String[]) values.toArray(new String[0]);
    } 

    private boolean isTrusted(X509Certificate chain[]) {
	try {
	    validator.validate(chain);
	    return true;
	} catch (CertificateException e) {
	    return false;
	}
    }

    // Retrieve a certificate chain from the specified certificate
    // array which may hold multiple cert chains starting from index
    // 'startIndex'.
    private static X509Certificate[] getAChain(Certificate[] certs,
					int startIndex) {
	int i;

	if (startIndex > certs.length - 1)
	    return null;

	for (i = startIndex; i < certs.length - 1; i++) {
	    if (!((X509Certificate)certs[i + 1]).getSubjectDN().equals(
		  ((X509Certificate)certs[i]).getIssuerDN())) {
		break;
	    }
	}
	int certChainSize = (i-startIndex) + 1;
	X509Certificate[] ret = new X509Certificate[certChainSize];
	for (int j = 0; j < certChainSize; j++ ) {
	    ret[j] = (X509Certificate) certs[startIndex + j];
	}
	return ret;	
    }

    // Convert the specified certificate array into an array of 
    // certificate chains.
    // NOTE: returns an empty List if certs is null
    static List convertCertsToChains(Certificate[] certs) 
	    throws CertificateException {
	if (certs == null) {
	    return Collections.EMPTY_LIST;
	}
	List list = new ArrayList();
	X509Certificate[] oneChain = null;
	int index = 0;
	while ((oneChain = getAChain(certs, index)) != null) {
	    list.add(oneChain);
	    // Proceed to the next chain.
	    index += oneChain.length;
	}
	
	return list;
    }

    // Retrieve the signers information for the specified jar entry.
    // NOTE: returns an empty List if no certificate chain is found
    static List getSignersOfJarEntry(URL jarEntryURL) throws Exception {
	JarURLConnection conn = (JarURLConnection) jarEntryURL.openConnection();
	byte[] buffer = new byte[8192];
	// Read in the JarEntry data to retrieve the cert chains
	BufferedInputStream is = new BufferedInputStream(conn.getInputStream()); 
	try {
	    while (is.read(buffer, 0, buffer.length) != -1) { 
	        // Read - this will throw a SecurityException 
	        // if  a signature/digest check fails. 
	    }
	} finally { 
	    is.close();
	}
	
	return convertCertsToChains(conn.getCertificates());
    }

    protected void finalize() throws Throwable {
	jarFile = null;
	super.finalize();
    }
}
