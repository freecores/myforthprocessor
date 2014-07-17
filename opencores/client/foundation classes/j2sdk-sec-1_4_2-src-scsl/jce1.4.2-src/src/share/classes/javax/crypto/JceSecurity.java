/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package javax.crypto;

import java.util.*;
import java.util.jar.*;
import java.io.*;
import java.net.InetAddress;
import java.net.URL;
import java.security.*;
import java.security.cert.Certificate;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;

import sun.security.validator.Validator;

/**
 * This class instantiates implementations of JCE engine classes from
 * providers registered with the java.security.Security object.
 *
 * @author Jan Luehe
 * @author Sharon Liu
 * @version 1.47, 06/24/03
 * @since 1.4
 */

final class JceSecurity {
    
    // Are we debugging? -- for developers
    static final boolean debug = false;

    static final SecureRandom RANDOM = new SecureRandom();

    // The defaultPolicy and exemptPolicy will be set up
    // in the static initializer.
    private static CryptoPermissions defaultPolicy = null;
    private static CryptoPermissions exemptPolicy = null;
    
    // Once a provider has been verified successfully, keep it in this cache.
    private static Vector verifiedProvidersCache = new Vector(2);

    // The inProgressTable is used to avoid
    // multiple threads verifying the same
    // provider at the same time.
    private static Vector inProgressTable = new Vector(2);

    // Set the default value. May be changed in the static initializer.
    private static boolean isRestricted = true;

    // X.509 certificate factory used to setup the certificates
    private static CertificateFactory certificateFactory;
    
    // certificate used to sign the framework and the policy files
    private static X509Certificate jceCertificate;
    
    // validator instance to verify provider JAR files
    private static Validator providerValidator;
    
    // validator instance to verify exempt application JAR files
    // or null if the providerValidator should be used
    private static Validator exemptValidator;
    
    /*
     * Don't let anyone instantiate this. 
     */
    private JceSecurity() {
    }
    
    static {
/* FIRST CA CERTIFICATE TRUSTED TO SIGN JCE PROVIDERS
Owner: CN=JCE Code Signing CA, OU=Java Software Code Signing, O=Sun Microsystems Inc, L=Palo Alto, ST=CA, C=US
Issuer: CN=JCE Code Signing CA, OU=Java Software Code Signing, O=Sun Microsystems Inc, L=Palo Alto, ST=CA, C=US
Serial number: 10
Valid from: Wed Apr 25 00:00:00 PDT 2001 until: Sat Apr 25 00:00:00 PDT 2020
Certificate fingerprints:
         MD5:  66:25:5A:78:3E:1A:CA:06:C1:43:A6:15:AE:BE:A5:92
         SHA1: 57:37:D1:E1:16:2F:F6:FE:26:B9:87:88:D2:86:DA:66:7F:98:54:3C
*/
	final String CACERT1 =
"-----BEGIN CERTIFICATE-----\n" +
"MIIDwDCCA36gAwIBAgIBEDALBgcqhkjOOAQDBQAwgZAxCzAJBgNVBAYTAlVTMQswCQYDVQQIEwJD" +
"QTESMBAGA1UEBxMJUGFsbyBBbHRvMR0wGwYDVQQKExRTdW4gTWljcm9zeXN0ZW1zIEluYzEjMCEG" +
"A1UECxMaSmF2YSBTb2Z0d2FyZSBDb2RlIFNpZ25pbmcxHDAaBgNVBAMTE0pDRSBDb2RlIFNpZ25p" +
"bmcgQ0EwHhcNMDEwNDI1MDcwMDAwWhcNMjAwNDI1MDcwMDAwWjCBkDELMAkGA1UEBhMCVVMxCzAJ" +
"BgNVBAgTAkNBMRIwEAYDVQQHEwlQYWxvIEFsdG8xHTAbBgNVBAoTFFN1biBNaWNyb3N5c3RlbXMg" +
"SW5jMSMwIQYDVQQLExpKYXZhIFNvZnR3YXJlIENvZGUgU2lnbmluZzEcMBoGA1UEAxMTSkNFIENv" +
"ZGUgU2lnbmluZyBDQTCCAbcwggEsBgcqhkjOOAQBMIIBHwKBgQDrrzcEHspRHmldsPKP9rVJH8ak" +
"mQXXKb90t2r1Gdge5Bv4CgGamP9wq+JKVoZsU7P84ciBjDHwxPOwi+ZwBuz3aWjbg0xyKYkpNhdc" +
"O0oHoCACKkaXUR1wyAgYC84Mbpt29wXj5/vTYXnhYJokjQaVgzxRIOEwzzhXgqYacg3O0wIVAIQl" +
"ReG6ualiq3noWzC4iWsb/3t1AoGBAKvJdHt07+5CtWpTTTvdkAZyaJEPC6Qpdi5VO9WuTWVcfio6" +
"BKZnptBxqqXXt+LBcg2k0aoeklRMIAAJorAJQRkzALLDXK5C+LGLynyW2BB/N0Rbqsx4yNdydjdr" +
"QJmoVWb6qAMei0oRAmnLTLglBhygd9LJrNI96QoQ+nZwt/vcA4GEAAKBgC0JmFysuJzHmX7uIBkq" +
"NJD516urrt1rcpUNZvjvJ49Esu0oRMf+r7CmJ28AZ0WCWweoVlY70ilRYV5pOdcudHcSzxlK9S3I" +
"y3JhxE5v+kdDPxS7+rwYZijC2WaLei0vwmCSSxT+WD4hf2hivmxISfmgS16FnRkQ+RVFURtx1PcL" +
"o2YwZDARBglghkgBhvhCAQEEBAMCAAcwDwYDVR0TAQH/BAUwAwEB/zAfBgNVHSMEGDAWgBRl4vSG" +
"ydNO8JFOWKJq9dh4WprBpjAdBgNVHQ4EFgQUZeL0hsnTTvCRTliiavXYeFqawaYwCwYHKoZIzjgE" +
"AwUAAy8AMCwCFCr3zzyXXfl4tgjXQbTZDUVM5LScAhRFzXVpDiH6HdazKbLp9zMdM/38SQ==\n" +
"-----END CERTIFICATE-----";

/* SECOND CA CERTIFICATE TRUSTED TO SIGN JCE PROVIDERS
Owner: CN=JCE Code Signing CA, OU=IBM Code Signing, O=IBM Corporation, C=US
Issuer: CN=JCE Code Signing CA, OU=IBM Code Signing, O=IBM Corporation, C=US
Serial number: 3924a555
Valid from: Thu May 18 19:22:13 PDT 2000 until: Wed May 17 19:22:13 PDT 2006
Certificate fingerprints:
         MD5:  AC:83:A0:27:0C:A8:3A:48:BC:63:44:D2:E4:11:D1:39
         SHA1: 60:5B:A9:64:F3:6E:A9:7B:4C:97:E5:88:AC:60:7C:16:40:3A:75:B8
*/
	final String CACERT2 =
"-----BEGIN CERTIFICATE-----\n" +
"MIIDTzCCAw2gAwIBAgIEOSSlVTALBgcqhkjOOAQDBQAwYDELMAkGA1UEBhMCVVMxGDAWBgNVBAoT" +
"D0lCTSBDb3Jwb3JhdGlvbjEZMBcGA1UECxMQSUJNIENvZGUgU2lnbmluZzEcMBoGA1UEAxMTSkNF" +
"IENvZGUgU2lnbmluZyBDQTAeFw0wMDA1MTkwMjIyMTNaFw0wNjA1MTgwMjIyMTNaMGAxCzAJBgNV" +
"BAYTAlVTMRgwFgYDVQQKEw9JQk0gQ29ycG9yYXRpb24xGTAXBgNVBAsTEElCTSBDb2RlIFNpZ25p" +
"bmcxHDAaBgNVBAMTE0pDRSBDb2RlIFNpZ25pbmcgQ0EwggG4MIIBLAYHKoZIzjgEATCCAR8CgYEA" +
"/X9TgR11EilS30qcLuzk5/YRt1I870QAwx4/gLZRJmlFXUAiUftZPY1Y+r/F9bow9subVWzXgTuA" +
"HTRv8mZgt2uZUKWkn5/oBHsQIsJPu6nX/rfGG/g7V+fGqKYVDwT7g/bTxR7DAjVUE1oWkTL2dfOu" +
"K2HXKu/yIgMZndFIAccCFQCXYFCPFSMLzLKSuYKi64QL8Fgc9QKBgQD34aCF1ps93su8q1w2uFe5" +
"eZSvu/o66oL5V0wLPQeCZ1FZV4661FlP5nEHEIGAtEkWcSPoTCgWE7fPCTKMyKbhPBZ6i1R8jSjg" +
"o64eK7OmdZFuo38L+iE1YvH7YnoBJDvMpPG+qFGQiaiD3+Fa5Z8GkotmXoB7VSVkAUw7/s9JKgOB" +
"hQACgYEA6msAx98QO7l0NafhbWaCTfdbVnHCJkUncj1REGL/s9wQyftRE9Sti6glbl3JeNJbJ9MT" +
"QUcUBnzLgjhexgthoEyDLZTMjC6EkDqPQgppUtN0JnekFH0qcUGIiXemLWKaoViYbWzPzqjqut3o" +
"oRBEjIRCwbgfK7S8s110YICNQlSjUzBRMB8GA1UdIwQYMBaAFH49TU3MEFm6+a5CPeXJV1pSC36H" +
"MB0GA1UdDgQWBBR+PU1NzBBZuvmuQj3lyVdaUgt+hzAPBgNVHRMBAf8EBTADAQH/MAsGByqGSM44" +
"BAMFAAMvADAsAhQc7/bAXO2TcCLhS8kP8J16Ce6GhgIUEndHYsKfGWHcKfq5SaUZ1qW8RU0=\n" +
"-----END CERTIFICATE-----";

/* RSA CERTIFICATE USED TO SIGN JCE.JAR AND THE POLICY FILES
Owner: CN=Sun Microsystems Inc, OU=Java Software Code Signing, O=Sun Microsystems Inc
Issuer: CN=JCE Code Signing CA, OU=Java Software Code Signing, O=Sun Microsystems Inc, L=Palo Alto, ST=CA, C=US
Serial number: 15d
Valid from: Fri Oct 25 12:05:10 PDT 2002 until: Mon Oct 29 11:05:10 PST 2007
Certificate fingerprints:
         MD5:  63:AA:AA:43:78:9F:86:BC:AA:78:40:36:2D:6E:66:6D
         SHA1: 9D:F7:A6:11:C5:C1:F5:5D:61:01:F2:1D:A2:22:D7:58:79:6A:52:23
*/
	final String JCECERT =
"-----BEGIN CERTIFICATE-----\n" +
"MIICnjCCAlugAwIBAgICAV0wCwYHKoZIzjgEAwUAMIGQMQswCQYDVQQGEwJVUzELMAkGA1UECBMC" +
"Q0ExEjAQBgNVBAcTCVBhbG8gQWx0bzEdMBsGA1UEChMUU3VuIE1pY3Jvc3lzdGVtcyBJbmMxIzAh" +
"BgNVBAsTGkphdmEgU29mdHdhcmUgQ29kZSBTaWduaW5nMRwwGgYDVQQDExNKQ0UgQ29kZSBTaWdu" +
"aW5nIENBMB4XDTAyMTAyNTE5MDUxMFoXDTA3MTAyOTE5MDUxMFowYzEdMBsGA1UEChMUU3VuIE1p" +
"Y3Jvc3lzdGVtcyBJbmMxIzAhBgNVBAsTGkphdmEgU29mdHdhcmUgQ29kZSBTaWduaW5nMR0wGwYD" +
"VQQDExRTdW4gTWljcm9zeXN0ZW1zIEluYzCBnzANBgkqhkiG9w0BAQEFAAOBjQAwgYkCgYEA16bK" +
"o6tC3OHFDNfPXLKXMCMtIyeubNnsEtlvrH34HhfF+ZmpSliLCvQ15ms705vy4XgZUbZ3mgSOlLRM" +
"AGRo6596ePhc+0Z6yeKhbb3LZ8iz97ZIptkHGOshj9cfcSRPYmorUug9OsybMdIfQXazxT9mZJ9Y" +
"x5IDw6xak7kVbpUCAwEAAaOBiDCBhTARBglghkgBhvhCAQEEBAMCBBAwDgYDVR0PAQH/BAQDAgXg" +
"MB0GA1UdDgQWBBRI319jCbhc9DWJVltXgfrMybHNjzAfBgNVHSMEGDAWgBRl4vSGydNO8JFOWKJq" +
"9dh4WprBpjAgBgNVHREEGTAXgRV5dS1jaGluZy5wZW5nQHN1bi5jb20wCwYHKoZIzjgEAwUAAzAA" +
"MC0CFFmXXV97KWezNwPFiAJt5IWpPGVqAhUAggVpunP/Bo8BOcwIoUpgYYQ7ruY=\n" +
"-----END CERTIFICATE-----";

/* DSA CERTIFICATE USED TO SIGN SUNJCE_PROVIDER.JAR
Owner: CN=Sun Microsystems Inc, OU=Java Software Code Signing, O=Sun Microsystems Inc
Issuer: CN=JCE Code Signing CA, OU=Java Software Code Signing, O=Sun Microsystems Inc, L=Palo Alto, ST=CA, C=US
Serial number: 104
Valid from: Fri Oct 19 16:04:31 PDT 2001 until: Mon Oct 23 16:04:31 PDT 2006
Certificate fingerprints:
         MD5:  3F:B9:8C:64:7A:0D:9F:3B:C9:F3:1A:36:9E:89:39:09
         SHA1: C2:E7:9B:2C:98:43:E1:AB:AD:FB:35:3B:81:F6:AB:E8:02:3C:18:21
*/
	final String PROVIDERCERT =
"-----BEGIN CERTIFICATE-----\n" +
"MIIDtDCCA3KgAwIBAgICAQQwCwYHKoZIzjgEAwUAMIGQMQswCQYDVQQGEwJVUzELMAkGA1UECBMC" +
"Q0ExEjAQBgNVBAcTCVBhbG8gQWx0bzEdMBsGA1UEChMUU3VuIE1pY3Jvc3lzdGVtcyBJbmMxIzAh" +
"BgNVBAsTGkphdmEgU29mdHdhcmUgQ29kZSBTaWduaW5nMRwwGgYDVQQDExNKQ0UgQ29kZSBTaWdu" +
"aW5nIENBMB4XDTAxMTAxOTIzMDQzMVoXDTA2MTAyMzIzMDQzMVowYzEdMBsGA1UECgwUU3VuIE1p" +
"Y3Jvc3lzdGVtcyBJbmMxIzAhBgNVBAsMGkphdmEgU29mdHdhcmUgQ29kZSBTaWduaW5nMR0wGwYD" +
"VQQDDBRTdW4gTWljcm9zeXN0ZW1zIEluYzCCAbUwggEqBgUrDgMCDDCCAR8CgYEA/X9TgR11EilS" +
"30qcLuzk5/YRt1I870QAwx4/gLZRJmlFXUAiUftZPY1Y+r/F9bow9subVWzXgTuAHTRv8mZgt2uZ" +
"UKWkn5/oBHsQIsJPu6nX/rfGG/g7V+fGqKYVDwT7g/bTxR7DAjVUE1oWkTL2dfOuK2HXKu/yIgMZ" +
"ndFIAccCFQCXYFCPFSMLzLKSuYKi64QL8Fgc9QKBgQD34aCF1ps93su8q1w2uFe5eZSvu/o66oL5" +
"V0wLPQeCZ1FZV4661FlP5nEHEIGAtEkWcSPoTCgWE7fPCTKMyKbhPBZ6i1R8jSjgo64eK7OmdZFu" +
"o38L+iE1YvH7YnoBJDvMpPG+qFGQiaiD3+Fa5Z8GkotmXoB7VSVkAUw7/s9JKgOBhAACgYAHzPY4" +
"Os3TWJmQD3GvqtADJzt04WQ4Eb/6t78s57unki8Izif4tP3YFB2jlbsDFqa6vDXAzfn1bKeUWyMB" +
"+a71yeCBeujkaev49YAlBCyRc5ZZtAaDF7JQrE/rnVElPffusCQlDv60MqHEDrNmQeBXzp2+My6T" +
"msl6V9zNiGCnzqOBiDCBhTARBglghkgBhvhCAQEEBAMCBBAwDgYDVR0PAQH/BAQDAgXgMB0GA1Ud" +
"DgQWBBRVjR8qBaubzoYQrjtd9ro/IsVqyjAfBgNVHSMEGDAWgBRl4vSGydNO8JFOWKJq9dh4WprB" +
"pjAgBgNVHREEGTAXgRV5dS1jaGluZy5wZW5nQHN1bi5jb20wCwYHKoZIzjgEAwUAAy8AMCwCFHVL" +
"6CE3eHkK0LXcfjZ1ueQUtdBGAhRqUdy6bRprXBgjavHKIYp3wgUWQg==\n" +
"-----END CERTIFICATE-----";

	try {
	    AccessController.doPrivileged(new PrivilegedExceptionAction() {
	        public Object run() throws Exception {
		    certificateFactory = CertificateFactory.getInstance("X.509");
		    jceCertificate = parseCertificate(JCECERT);
		    X509Certificate[] providerCaCerts = new X509Certificate[] {
			parseCertificate(CACERT1),
			parseCertificate(CACERT2),
			// as an optimization, add our EE certs to the trusted
			// certs. this saves us the signature verification
			jceCertificate,
			parseCertificate(PROVIDERCERT),
		    };
		    providerValidator = Validator.getInstance(
		    				Validator.TYPE_SIMPLE,
						Validator.VAR_JCE_SIGNING, 
						Arrays.asList(providerCaCerts));
		    // Trust the JCE provider CAs for exempt JCE applications.
		    // If this changes, setup the validator for exempt apps here
		    exemptValidator = null;
		    testSignatures(providerCaCerts[0]);
		    setupJurisdictionPolicies();
		    certificateFactory = null;
		    return null;
		}
	    });

	    isRestricted = defaultPolicy.implies(CryptoAllPermission.INSTANCE)? 
	                false : true;
	} catch (Exception e) {
	    SecurityException se = new 
		SecurityException("Cannot set up certs for trusted CAs");
	    se.initCause(e);
	    throw se;
	    
	}
    }
    
    /**
     * Parse a PEM encoded X.509 certificate.
     */
    private static X509Certificate parseCertificate(String s) throws Exception {
	InputStream in = new ByteArrayInputStream(s.getBytes("UTF8"));
	return (X509Certificate)certificateFactory.generateCertificate(in);
    }
    
    /*
     * Retrieves the property with the given key from the given provider.
     */
    private static String getProviderProperty(String key, Provider prov) {
	String prop = prov.getProperty(key);
	if (prop != null) {
	    return prop;
	}
	// Is there a match if we do a case-insensitive property name
	// comparison? Let's try ...
	for (Enumeration e = prov.keys(); e.hasMoreElements(); ) {
	    String matchKey = (String)e.nextElement();
	    if (key.equalsIgnoreCase(matchKey)) {
		return prov.getProperty(matchKey);
	    }
	}
	return null;
    }

    /*
     * Converts an alias name to the standard name.
     */
    private static String getStandardName(String alias, String engineType,
					  Provider prov) {
	return getProviderProperty("Alg.Alias." + engineType + "." + alias,
				   prov);
    }

    /**
     * Instantiates the implementation of the requested algorithm
     * (of the requested engine type) from the requested provider.
     */
    private static Class getImplClass(String algName, String engineType,
				      Provider prov)
	throws NoSuchAlgorithmException
    {
	Class implClass = null;
	
	// get the implementation class name
	String key = engineType + "." + algName;
	String className = getProviderProperty(key, prov);
	if (className == null) {
	    // consider "algName" an alias name, and try to map it onto
	    // the corresponding standard name
	    String stdName = getStandardName(algName, engineType, prov);
	    if (stdName != null) {
		key = engineType + "." + stdName;
		className = getProviderProperty(key, prov);
	    } 
	    if ((stdName == null) ||
		((className = getProviderProperty(key, prov)) == null)) {
	 	throw new NoSuchAlgorithmException("No such algorithm: " +
			algName);
	    }
	}

	// Load the implementation class with the same class loader
	// that was used to load the provider.
	// In order to get the class loader of a class, the
	// caller's class loader must be the same as or an ancestor of
	// the class loader being returned. Otherwise, the caller must
	// have "getClassLoader" permission, or a SecurityException
	// will be thrown.
	try {
	    ClassLoader cl = prov.getClass().getClassLoader();
	    if (cl == null) {
		// system class
		implClass = Class.forName(className);
	    } else {
		implClass = cl.loadClass(className);
	    }
	} catch (ClassNotFoundException e) {
	    throw new NoSuchAlgorithmException("Class " + className + 
					       " configured for " +
					       engineType +
					       " not found: " +
					       e.getMessage());
	} catch (SecurityException e) {
	    throw new NoSuchAlgorithmException("Class " + className + 
					       " configured for " +
					       engineType +
					       " cannot be accessed: " + 
					       e.getMessage());
	}

	return implClass;
    }

    /**
     * Returns an array of objects: the first object in the array is
     * an instance of an implementation of the requested algorithm
     * and type, and the second object in the array identifies the provider
     * of that implementation.
     * The <code>provName</code> argument can be null, in which case all
     * configured providers will be searched in order of preference.
     */
    static Object[] getImpl(String algName, String engineType, String provName)
	throws NoSuchAlgorithmException, NoSuchProviderException
    {	
	Class implClass = null;
	Provider prov = null;
 
	if (provName != null) {
	    // check if the requested provider is installed
	    prov = Security.getProvider(provName);
	    if (prov == null) {
		throw new NoSuchProviderException("No such provider: " +
						  provName);
	    }
	    implClass = getImplClass(algName, engineType, prov);
	    // Try to authenticate the provider. Throws
	    // NoSuchProviderException if the verification
	    // fails.
	    verifyProvider(prov);
	} else {
	    // get all currently installed providers
	    Provider[] provs = Security.getProviders();

	    // get the implementation class from the first provider
	    // that supplies an implementation that we can load
	    boolean found = false;
	    for (int i = 0; (i < provs.length) && (!found); i++) {
		try {
		    implClass = getImplClass(algName, engineType, provs[i]);
		    // Try to authenticate the provider. Throws
		    // NoSuchProviderException if the verification
		    // fails.
		    verifyProvider(provs[i]);
		    found = true;
		    prov = provs[i];
		} catch (NoSuchAlgorithmException nsae) {
		    // do nothing, check the next provider
		} catch (NoSuchProviderException nspe) {
		    // do nothing, check the next provider
		}
	    }
	    if (!found) {
		throw new NoSuchAlgorithmException("Algorithm " + algName
						   + " not available");
	    }
	}

	return doGetImpl(algName, engineType, prov, implClass);
    }

    /**
     * Returns an array of objects: the first object in the array is
     * an instance of an implementation of the requested algorithm
     * and type, and the second object in the array identifies the provider
     * of that implementation.
     * The <code>provider</code> argument can not be null.
     */
    static Object[] getImpl(String algName, String engineType, Provider provider)
	throws NoSuchAlgorithmException
    {
	Class implClass = getImplClass(algName, engineType, provider);
	try {
	    verifyProvider(provider);
	} catch (NoSuchProviderException nspe) {
	    throw new SecurityException("The provider " + provider.getName() +
					" may not be signed by a trusted party");
	}
	return doGetImpl(algName, engineType, provider, implClass);
    }

    private static Object[] doGetImpl(String algName, String engineType, 
				      Provider prov, Class implClass) 
        throws NoSuchAlgorithmException {

	// Make sure the implementation class is a subclass of the
	// corresponding engine class.
	Class typeClass = null;
	try {
	    // (The following Class.forName() always works, because this class
	    // and all the SPI classes in javax.crypto are loaded by the same
	    // class loader.)
	    typeClass = Class.forName("javax.crypto." + engineType + "Spi");
	} catch (ClassNotFoundException e) {
	    throw new NoSuchAlgorithmException(e.getMessage());
	}

	if (!checkSuperclass(implClass, typeClass)) {
	    throw new NoSuchAlgorithmException("Class " +
					       implClass.getName() +
					       " configured for " + 
					       engineType + " is not a " +
					       engineType);
	}

	// If the requested crypto service is export-controlled,
	// determine the maximum allowable keysize.
	CryptoPermission cp = CryptoAllPermission.INSTANCE;
	ExemptionMechanism exmech = null;
	if (JceSecurity.isRestricted() && engineType.equals("Cipher")) {
	    // Determine the "algorithm" component of the requested
	    // transformation
	    String algComponent = algName;
	    int index = algName.indexOf('/');
	    if (index != -1) {
		algComponent = algName.substring(0, index);
	    }
	    JceSecurityManager jsm = (JceSecurityManager)
                  AccessController.doPrivileged(new PrivilegedAction() {
		      public Object run() {
			  return new JceSecurityManager();
		      }
		  });
	    cp = jsm.getCryptoPermission(algComponent.toUpperCase());

	    // Instantiate the exemption mechanism (if required)
	    String exmechName = cp.getExemptionMechanism();
	    if (exmechName != null) {
		try {
		    exmech = ExemptionMechanism.getInstance(exmechName);
		} catch (Exception e) {
		    // This never happens, because when we selected the
		    // CryptoPermission, me made sure that if it required
		    // an exemption mechanism, that exemption mechanism was
		    // available.
		}
	    }
	}

	// Instantiate the SPI implementation
	try {
	    Object obj = implClass.newInstance();
	    return new Object[] { obj, prov, cp, exmech,
				  new Boolean(JceSecurity.isRestricted()) };
	} catch (InstantiationException e) {
	    throw new NoSuchAlgorithmException("Class " +
					       implClass.getName() +
					       " configured for " +
					       engineType +
					       " cannot be instantiated: " + 
					       e.getMessage());
	} catch (IllegalAccessException e) {
	    throw new NoSuchAlgorithmException("Class " +
					       implClass.getName() +
					       " configured for " +
					       engineType +
					       " cannot be accessed: " + 
					       e.getMessage());
	}
    }

    /*
     * Checks whether one class is the superclass of another
     */
    private static boolean checkSuperclass(Class subclass, Class superclass) {
	while (!subclass.equals(superclass)) {
	    subclass = subclass.getSuperclass();
	    if (subclass == null) {
		return false;
	    }
	}
	return true;
    }
    
    /**
     * Verify if the JAR at URL codeBase is a signed exempt application
     * JAR file.
     *
     * @return the JarFile on success
     * @throws Exception on error
     */
    static JarFile verifyExemptJar(URL codeBase) throws Exception {
	if (exemptValidator != null) {
	    try {
		JarVerifier jv = new JarVerifier(codeBase, exemptValidator);
		jv.verify();
		return jv.getJarFile();
	    } catch (Exception e) {
		// ignore, also try provider CA certs
	    }
	}
	return verifyProviderJar(codeBase);
    }
    
    /**
     * Verify if the JAR at URL codeBase is a signed provider JAR file.
     *
     * @return the JarFile on success
     * @throws Exception on error
     */
    static JarFile verifyProviderJar(URL codeBase) throws Exception {
	// Verify the provider JAR file and all
	// supporting JAR files if there are any.
	JarVerifier jv = new JarVerifier(codeBase, providerValidator);
	jv.verify();
	return jv.getJarFile();
    }
    
    /*
     * Verify that the provider JAR files are signed properly, which
     * means the signer's certificate can be traced back to a 
     * JCE trusted CA. Throws NoSuchProviderException
     * if the verification fails.
     */
    private static void verifyProvider(Provider prov) 
	throws NoSuchProviderException
    {
	// If this provider hasn't been verified,
	// we must verify it now.
	if (mustVerify(prov)) {
	    try {
		URL providerURL = getCodeBase(prov.getClass());
		verifyProviderJar(providerURL);
		// Add this provider in verifiedProvidersCache
		verifiedProvidersCache.addElement(prov);
	    } catch (Exception e) {
		NoSuchProviderException nspe = new 
		    NoSuchProviderException("JCE cannot authenticate the " +
					    "provider " + prov.getName());
		nspe.initCause(e);
		throw nspe;
	    } finally {
		// Delete the provider name
		// from the inProgressTable, and
		// notify all threads waiting for
		// the monitor for inProgressTable.
		updateInProgressTable(prov);
	    }
	}
    }

    /*
     * Returns true if the current thread should
     * verify the provider. Retruns false if
     * the provider has been verified.
     *
     * If the provider has been verified,return
     * false since the current thread doesn't
     * have to verify it again. Otherwise, do
     * the following:
     * 1) If no other thread is verifying this provider,
     *     add this provider in the inProgressTable and
     *     return true so this thread will verify the
     *     provider.
     * 2) otherwise, do the folloing:
     * a) wait until the other thread finished the 
     *     verification.
     * b) check whether this provider is in the
     *     verifiedProvidersCache.
     * c) if yes, return false since this thread
     *     doesn't have to verify the provider.
     * d) if no, add this provider into the inProgressTable
     *    and return true so this thread will
     *    verify the provider.
     */
    private static boolean mustVerify(Provider prov) {
	if (verifiedProvidersCache.contains(prov)) {
	    return false;
	}

	// Now deal with the else case.
	synchronized (inProgressTable) {
	    // If the provider isn't in the inProgressTable,
	    // add it into the inProgressTable and set result
	    // to true. The caller must verify the provider.
	    if (!inProgressTable.contains(prov)) {
		inProgressTable.addElement(prov);
		return true;
	    }

	    // Now deal with the else case.
	    while (inProgressTable.contains(prov)) {
		try {
		    inProgressTable.wait();
		} catch (InterruptedException ie) {
		    // eat
		}
	    }

	    // This thread should retry to check
	    // whether this provider is in the verifiedProvidersCache.
	    // If not, it must verify this provider
	    // by itself.
	    if (verifiedProvidersCache.contains(prov)) {
		return false;
	    }

	    inProgressTable.addElement(prov);
	    return true;
	}
    }

    private static void updateInProgressTable(Provider prov) {
	synchronized (inProgressTable) {
	    inProgressTable.remove(prov);
	    inProgressTable.notifyAll();
	}
    }

    /*
     * Retuns the CodeBase for the given class.
     */
    static URL getCodeBase(final Class clazz) {
        return (URL)AccessController.doPrivileged(new PrivilegedAction() {
            public Object run() {
                ProtectionDomain pd = clazz.getProtectionDomain();
                if (pd != null) {
                        CodeSource cs = pd.getCodeSource();
                        if (cs != null) return cs.getLocation();
                }
                return null;
            }
        });
    }

    private static void setupJurisdictionPolicies() throws Exception {
	String javaHomeDir = System.getProperty("java.home");
       	String sep = File.separator;
	String pathToPolicyJar = javaHomeDir + sep + "lib" + sep +
            "security" + sep;

	File exportJar = new File(pathToPolicyJar, "US_export_policy.jar");
	File importJar = new File(pathToPolicyJar, "local_policy.jar");
        URL jceCipherURL = ClassLoader.getSystemResource
		("javax/crypto/Cipher.class");

	if ((jceCipherURL == null) || !exportJar.exists() || !importJar.exists()) {
	    throw new SecurityException
	    			("Cannot locate policy or framework files!");
	}
	
	URL exportPolicyURL = exportJar.toURI().toURL();
        URL importPolicyURL = importJar.toURI().toURL();
        
        if (debug) {
	    System.out.println("export policy at:" + exportPolicyURL);
	    System.out.println("import policy at:" + importPolicyURL);
	    System.out.println("jce Cipher class at:" + jceCipherURL);
        }
	
	// Enforce the signer restraint, i.e. signer of JCE framework
	// jar should also be the signer of the two jurisdiction policy
	// jar files.
	List signers = JarVerifier.getSignersOfJarEntry(jceCipherURL);
	for (Iterator t = signers.iterator(); t.hasNext(); ) {
	    X509Certificate[] chain = (X509Certificate[])t.next();
	    if (chain[0].equals(jceCertificate)) {
		// use signers as a success flag
		signers = null;
		break;
	    }
	}
	if (signers != null) {
	    throw new SecurityException("Jurisdiction policy files are " +
					"not signed by trusted signers!");
	}

	// Read jurisdiction policies.
	CryptoPermissions defaultExport = new CryptoPermissions();
	CryptoPermissions exemptExport = new CryptoPermissions();
	loadPolicies(exportJar, defaultExport, exemptExport, jceCertificate);

	CryptoPermissions defaultImport = new CryptoPermissions();
	CryptoPermissions exemptImport = new CryptoPermissions();
	loadPolicies(importJar, defaultImport, exemptImport, jceCertificate);

	// Merge the export and import policies for default applications.
	if (defaultExport.isEmpty() || defaultImport.isEmpty()) {
	    throw new SecurityException("Missing mandatory jurisdiction " +
					"policy files");
	}
	defaultPolicy = defaultExport.getMinimum(defaultImport);

	// Merge the export and import policies for exempt applications.
	if (exemptExport.isEmpty())  {
	    exemptPolicy = exemptImport.isEmpty() ? null : exemptImport;
	} else {
	    exemptPolicy = exemptExport.getMinimum(exemptImport);
	}
    }
    
    /**
     * Load the policies from the specified file. Also checks that the policies
     * are signed by the certificate <code>signer</code>.
     */
    private static void loadPolicies(File jarPathName,
				     CryptoPermissions defaultPolicy,
				     CryptoPermissions exemptPolicy,
				     X509Certificate signer)
	throws Exception {

	JarFile jf = new JarFile(jarPathName);

	Enumeration entries = jf.entries();
	while (entries.hasMoreElements()) {
	    JarEntry je = (JarEntry)entries.nextElement();
	    InputStream is = null;
	    try {
	        if (je.getName().startsWith("default_")) {
		    is = jf.getInputStream(je);
		    defaultPolicy.load(is);
	        } else if (je.getName().startsWith("exempt_")) {
		    is = jf.getInputStream(je);
		    exemptPolicy.load(is);
	        } else {
		    continue;
		}
	    } finally {
		if (is != null) {
		    is.close();
		}
	    }
	    Certificate[] certChains = je.getCertificates();
	    List paths = JarVerifier.convertCertsToChains(certChains);
	    boolean found = false;
	    for (Iterator t = paths.iterator(); t.hasNext(); ) {
		X509Certificate[] path = (X509Certificate[])t.next();
		X509Certificate cert = path[0];
		if (cert.equals(signer)) {
		    found = true;
		    break;
		}
	    }
	    if (found == false) {
		throw new SecurityException("Jurisdiction policy files are " +
					    "not signed by trusted signers!");
	    }
	}
	jf = null;
    }

    static CryptoPermissions getDefaultPolicy() {
	return defaultPolicy;
    }

    static CryptoPermissions getExemptPolicy() {
	return exemptPolicy;
    }

    static boolean isRestricted() {
	return isRestricted;
    }

    /**
     * Retrieve some system information, hashed.
     */
    private static byte[] getSystemEntropy() {
	final MessageDigest md;

	try {
    	    md = MessageDigest.getInstance("SHA");
    	} catch (NoSuchAlgorithmException nsae) {
    	    throw new InternalError("internal error: SHA-1 not available.");
	}

	// The current time in millis
	byte b =(byte)System.currentTimeMillis();
	md.update(b);

	try {
	    // System properties can change from machine to machine
	    String s;
	    Properties p = System.getProperties();
	    Enumeration e = p.propertyNames();
	    while (e.hasMoreElements()) {
		s =(String)e.nextElement();
		md.update(s.getBytes());
		md.update(p.getProperty(s).getBytes());
	    }
	    
	    md.update
		(InetAddress.getLocalHost().toString().getBytes());
	    
	    // The temporary dir
	    File f = new File(p.getProperty("java.io.tmpdir"));
	    String[] sa = f.list();
	    for(int i = 0; i < sa.length; i++)
		md.update(sa[i].getBytes());
	    
	} catch (Exception ex) {
	    md.update((byte)ex.hashCode());
	}
	
	// get Runtime memory stats
	Runtime rt = Runtime.getRuntime();
	byte[] memBytes = longToByteArray(rt.totalMemory());
	md.update(memBytes, 0, memBytes.length);
	memBytes = longToByteArray(rt.freeMemory());
	md.update(memBytes, 0, memBytes.length);
	
	return md.digest();
    }

    /**
     * Helper function to convert a long into a byte array (least significant
     * byte first).
     */
    private static byte[] longToByteArray(long l) {
	byte[] retVal = new byte[8];
 
	for (int i=0; i<8; i++) {
	    retVal[i] = (byte) l;
	    l >>= 8;
	}
 
	return retVal;
    }

    private static void testSignatures(X509Certificate providerCert)
	    throws Exception {
/* DSA CERTIFICATE
Owner: CN=JCE Development, OU=Java Software, O=Sun Microsystems, L=Cupertino, ST=CA, C=US
Issuer: CN=JCE Development, OU=Java Software, O=Sun Microsystems, L=Cupertino, ST=CA, C=US
Serial number: 37f939e5
Valid from: Mon Oct 04 16:36:05 PDT 1999 until: Tue Oct 03 16:36:05 PDT 2000
Certificate fingerprints:
         MD5:  F2:F9:D1:18:F3:CC:E5:67:39:48:2C:15:98:03:ED:28
         SHA1: 28:E5:32:F3:FD:AF:8E:01:97:66:47:61:92:90:F9:8D:1B:E4:59:D5
*/
	String DSA =
"-----BEGIN CERTIFICATE-----\n" +
"MIIDLDCCAukCBDf5OeUwCwYHKoZIzjgEAwUAMHsxCzAJBgNVBAYTAlVTMQswCQYDVQQIEwJDQTES" +
"MBAGA1UEBxMJQ3VwZXJ0aW5vMRkwFwYDVQQKExBTdW4gTWljcm9zeXN0ZW1zMRYwFAYDVQQLEw1K" +
"YXZhIFNvZnR3YXJlMRgwFgYDVQQDEw9KQ0UgRGV2ZWxvcG1lbnQwHhcNOTkxMDA0MjMzNjA1WhcN" +
"MDAxMDAzMjMzNjA1WjB7MQswCQYDVQQGEwJVUzELMAkGA1UECBMCQ0ExEjAQBgNVBAcTCUN1cGVy" +
"dGlubzEZMBcGA1UEChMQU3VuIE1pY3Jvc3lzdGVtczEWMBQGA1UECxMNSmF2YSBTb2Z0d2FyZTEY" +
"MBYGA1UEAxMPSkNFIERldmVsb3BtZW50MIIBuDCCASwGByqGSM44BAEwggEfAoGBAP1/U4EddRIp" +
"Ut9KnC7s5Of2EbdSPO9EAMMeP4C2USZpRV1AIlH7WT2NWPq/xfW6MPbLm1Vs14E7gB00b/JmYLdr" +
"mVClpJ+f6AR7ECLCT7up1/63xhv4O1fnxqimFQ8E+4P208UewwI1VBNaFpEy9nXzrith1yrv8iID" +
"GZ3RSAHHAhUAl2BQjxUjC8yykrmCouuEC/BYHPUCgYEA9+GghdabPd7LvKtcNrhXuXmUr7v6OuqC" +
"+VdMCz0HgmdRWVeOutRZT+ZxBxCBgLRJFnEj6EwoFhO3zwkyjMim4TwWeotUfI0o4KOuHiuzpnWR" +
"bqN/C/ohNWLx+2J6ASQ7zKTxvqhRkImog9/hWuWfBpKLZl6Ae1UlZAFMO/7PSSoDgYUAAoGBAOGs" +
"R8waR5aiuOk1yBLemRlVCY+APJv3xqmPRxWAF6nwV2xrFUB8ghSEMFcHywoe4vBDvkGSoAFzeB5j" +
"y5wjDiFsN5AFPEVRfveS4NNZ1dgRdHbbh3h5O1dZE4MAKQwQfUoh9Oa3aahlB+orRzKOHLlGDpbN" +
"RQLST5BClvohramCMAsGByqGSM44BAMFAAMwADAtAhRF46T3nS+inP9TA1pLd3LIV0NNDQIVAIaf" +
"i+1/+JKxu0rcoXWMFSxNaRb3\n" +
"-----END CERTIFICATE-----";

/* SELF SIGNED RSA CERTIFICATE 1
Owner: CN=JCE Development, OU=Java Software, O=Sun Microsystems, L=Cupertino, ST=CA, C=US
Issuer: CN=JCE Development, OU=Java Software, O=Sun Microsystems, L=Cupertino, ST=CA, C=US
Serial number: 1
Valid from: Thu Oct 31 15:27:44 GMT 2002 until: Wed Oct 31 15:27:44 GMT 2007
Certificate fingerprints:
         MD5:  62:D2:99:B7:5C:20:A7:9D:B1:4A:64:06:8D:31:B8:70
         SHA1: 53:B9:A1:1F:D5:9F:53:27:99:5D:6A:DF:E0:D3:59:9B:67:8B:5C:7F
*/
	final String RSA1 = 
"-----BEGIN CERTIFICATE-----\n" +
"MIIB4DCCAYoCAQEwDQYJKoZIhvcNAQEEBQAwezELMAkGA1UEBhMCVVMxCzAJBgNVBAgTAkNBMRIw" +
"EAYDVQQHEwlDdXBlcnRpbm8xGTAXBgNVBAoTEFN1biBNaWNyb3N5c3RlbXMxFjAUBgNVBAsTDUph" +
"dmEgU29mdHdhcmUxGDAWBgNVBAMTD0pDRSBEZXZlbG9wbWVudDAeFw0wMjEwMzExNTI3NDRaFw0w" +
"NzEwMzExNTI3NDRaMHsxCzAJBgNVBAYTAlVTMQswCQYDVQQIEwJDQTESMBAGA1UEBxMJQ3VwZXJ0" +
"aW5vMRkwFwYDVQQKExBTdW4gTWljcm9zeXN0ZW1zMRYwFAYDVQQLEw1KYXZhIFNvZnR3YXJlMRgw" +
"FgYDVQQDEw9KQ0UgRGV2ZWxvcG1lbnQwXDANBgkqhkiG9w0BAQEFAANLADBIAkEAo/4CddEOa3M6" +
"v9JFAhnBYgTq54Y30++F8yzCK9EeYaG3AzvzZqNshDy579647p0cOM/4VO6rU2PgbzgKXPcs8wID" +
"AQABMA0GCSqGSIb3DQEBBAUAA0EACqPlFmVdKdYSCTNltXKQnBqss9GNjbnB+CitvWrwN+oOK8qQ" +
"pvV+5LB6LruvRy6zCedCV95Z2kXKg/Fnj0gvsg==\n" +
"-----END CERTIFICATE-----";

/* SELF SIGNED RSA CERTIFICATE 2
Owner: CN=JCE Development, OU=Java Software, O=Sun Microsystems, L=Cupertino, ST=CA, C=US
Issuer: CN=JCE Development, OU=Java Software, O=Sun Microsystems, L=Cupertino, ST=CA, C=US
Serial number: 2
Valid from: Thu Oct 31 15:27:44 GMT 2002 until: Wed Oct 31 15:27:44 GMT 2007
Certificate fingerprints:
         MD5:  A5:18:04:0E:29:DB:2B:24:19:FE:0F:12:E8:0C:A7:CC
         SHA1: A9:3B:F5:AE:AC:92:5B:0B:EC:7E:FD:E6:59:28:07:F2:2C:2C:8A:2D
*/	 
	final String RSA2 = 
"-----BEGIN CERTIFICATE-----\n" +
"MIIB4DCCAYoCAQIwDQYJKoZIhvcNAQEEBQAwezELMAkGA1UEBhMCVVMxCzAJBgNVBAgTAkNBMRIw" +
"EAYDVQQHEwlDdXBlcnRpbm8xGTAXBgNVBAoTEFN1biBNaWNyb3N5c3RlbXMxFjAUBgNVBAsTDUph" +
"dmEgU29mdHdhcmUxGDAWBgNVBAMTD0pDRSBEZXZlbG9wbWVudDAeFw0wMjEwMzExNTI3NDRaFw0w" +
"NzEwMzExNTI3NDRaMHsxCzAJBgNVBAYTAlVTMQswCQYDVQQIEwJDQTESMBAGA1UEBxMJQ3VwZXJ0" +
"aW5vMRkwFwYDVQQKExBTdW4gTWljcm9zeXN0ZW1zMRYwFAYDVQQLEw1KYXZhIFNvZnR3YXJlMRgw" +
"FgYDVQQDEw9KQ0UgRGV2ZWxvcG1lbnQwXDANBgkqhkiG9w0BAQEFAANLADBIAkEAr1OSXaOzpnVo" +
"qL2LqS5+HLy1kVvBwiM/E5iYT9eZaghE8qvF+4fETipWUNTWCQzHR4cDJGJOl9Nm77tELhES4QID" +
"AQABMA0GCSqGSIb3DQEBBAUAA0EAL+WcVFyj+iXlEVNVQbNOOUlWmlmXGiNKKXnIdNcc1ZUyi+JW" +
"0zmlfZ7iU/eRYhEEJBwdrUoyiGOGLo7pi6JzAA==\n" +
"-----END CERTIFICATE-----";
	 
	final int NUM_TESTS = 12;
	byte[] randomBytes = getSystemEntropy(); 
	int random = (randomBytes[0] & 0xff)
	           | (randomBytes[1] & 0xff) << 8
	           | (randomBytes[2] & 0xff) << 16
	           | (randomBytes[3]       ) << 24;

	X509Certificate[] certs = new X509Certificate[] {
	    providerCert,
	    parseCertificate(DSA),
	    parseCertificate(RSA1),
	    parseCertificate(RSA2),
	};
	
	PublicKey[] publicKeys = new PublicKey[4];
	publicKeys[0] = providerCert.getPublicKey();
	publicKeys[1] = publicKeys[0];
	publicKeys[2] = certs[2].getPublicKey();
	publicKeys[3] = publicKeys[2];
	
	boolean[] expectedResult = new boolean[] {
	    true,
	    false,
	    true,
	    false,
	};

	for (int i = 0; i < NUM_TESTS; i++ ) {
	    int k = random & 3;
	    random >>= 2;
	    boolean result;
	    try {
		certs[k].verify(publicKeys[k]);
		result = true;
	    } catch (SignatureException e) {
		result = false;
	    }
	    if (result != expectedResult[k]) {
		throw new SecurityException("Signature classes have " +
					    "been tampered with");
	    }
	}
    }
}
