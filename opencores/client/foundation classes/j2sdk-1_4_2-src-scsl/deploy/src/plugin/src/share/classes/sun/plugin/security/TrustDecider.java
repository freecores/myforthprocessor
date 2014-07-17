/*
 * @(#)TrustDecider.java	1.44 03/01/27
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.security;

import java.io.IOException;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;
import java.util.Collections;
import java.util.Date;
import java.util.Iterator;
import java.util.ArrayList;
import java.util.Map;
import java.util.HashMap;
import java.util.Enumeration;
import java.util.Collection;
import java.security.AccessController;
import java.security.CodeSource;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.GeneralSecurityException;
import java.security.Principal;
import java.security.PublicKey;
import java.security.KeyStore;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.security.cert.CertificateException;
import java.security.cert.CertificateEncodingException;
import java.security.cert.CertificateExpiredException;
import java.security.cert.CertificateParsingException;
import java.security.cert.CertificateNotYetValidException;
import javax.security.auth.x500.X500Principal;
import sun.plugin.util.DialogFactory;
import sun.plugin.util.Trace;
import sun.plugin.resources.ResourceHandler;
import sun.security.x509.NetscapeCertTypeExtension;
import sun.security.util.DerValue;
import sun.security.util.DerInputStream;
import sun.plugin.util.PluginSysUtil;
import sun.plugin.util.PluginSysAction;

public class TrustDecider 
{
    // Certificate stores used for signed applet verification
    //
    private static CertificateStore rootStore = new RootCACertificateStore();
    private static CertificateStore permanentStore = new PluginCertificateStore();
    private static CertificateStore sessionStore = new SessionCertificateStore();
    private static CertificateStore deniedStore = new DeniedCertificateStore();
    private static Map trustedX500Principals;
    private final static String OID_BASIC_CONSTRAINTS = "2.5.29.19";
    private final static String OID_KEY_USAGE = "2.5.29.15";
    private final static String OID_EXTENDED_KEY_USAGE = "2.5.29.37";
    private final static String OID_NETSCAPE_CERT_TYPE = "2.16.840.1.113730.1.1";
    private final static String OID_EKU_ANY_USAGE = "2.5.29.37.0";
    private final static String OID_EKU_CODE_SIGNING = "1.3.6.1.5.5.7.3.3";

    // Netscape certificate type extension
    private final static String NSCT_OBJECT_SIGNING_CA = NetscapeCertTypeExtension.OBJECT_SIGNING_CA;
    private final static String NSCT_OBJECT_SIGNING = NetscapeCertTypeExtension.OBJECT_SIGNING;
    private final static String NSCT_SSL_CA = NetscapeCertTypeExtension.SSL_CA;
    private final static String NSCT_S_MIME_CA = NetscapeCertTypeExtension.S_MIME_CA;

    /**
     * Reset the TrustDecider.
     */
    public static void reset()
    { 
	rootStore = new RootCACertificateStore();
	permanentStore = new PluginCertificateStore();
	sessionStore = new SessionCertificateStore();
	deniedStore = new DeniedCertificateStore();
    }

    public static boolean isAllPermissionGranted(CodeSource cs)
			  throws CertificateEncodingException, CertificateExpiredException, 
			         CertificateNotYetValidException, CertificateParsingException,
				 CertificateException, KeyStoreException, 
				 NoSuchAlgorithmException, IOException
    {
	Certificate[] certs = cs.getCertificates();
	String url = cs.getLocation().toString();

	// If no certificate is found, simply return false.
	if (certs == null)
	    return false;

	// Check our Session cert store, Permanent cert store and Deny cert store
	// first. If any cert in the certificate chain has been stored already,
	// we will give all permission to the applet.
	int 	  	start = 0;
	int 	  	end = 0;	    
	int		chainNum = 0;
        LinkedList	isDenyStoredList = new LinkedList();
	String		msg = null;

	// Loading certificate stores 
	rootStore.load();
	permanentStore.load();
	sessionStore.load();		
	deniedStore.load();		

	// determine each signer and its certificate chain
	while (end < certs.length) {
	    int i = start;
	    while ((i+1) < certs.length) 
	    {
	      if ((certs[i] instanceof X509Certificate)
			&& (certs[i+1] instanceof X509Certificate)
			&& isIssuerOf((X509Certificate)certs[i], (X509Certificate)certs[i+1])) 
		i++;
	      else 
		break;
	    }
	    end = i + 1;

	    // We first check if the certificate has been denied in 
	    // the denied certificate store
	    // If we found it in the Deny store, just add the TRUE flag into
	    // isDenyStoredList for this signer. We have to check any other certs
	    // int the same chain to see whether they have been trusted yet.
	    if (deniedStore.contains(certs[start]))
		isDenyStoredList.add(chainNum, new Boolean(true));
	    else
	    {
	    	// Check cert in Permanent and Session store only if cert not in Deny store.
		isDenyStoredList.add(chainNum, new Boolean(false));

	    	// We need to determine if the certificate has been stored
	    	// in the permanent certificate store.
	    	if (permanentStore.contains(certs[start]))
		    return true;

	    	// We need to determine if the certificate has been stored
	    	// in the session certificate store
	    	if (sessionStore.contains(certs[start]))
		    return true;
	    }
	
	    start = end;
	    chainNum++;
	}

	// If we get here, no cert in chain has been stored in Session or Permanent store.
	// If they are not in Deny store either, we have to pop up security dialog box 
	// for each signer's certificate one by one.
	boolean rootCANotValid = false;
	boolean timeNotValid = false;
	boolean trustDecision = false;
	start = 0;
	end = 0;	    
	
	// Substitute invalid or missing certs from trust store
	KeyStore ks = rootStore.getKeyStore();
	getCertMap(ks);
	Date date = new Date();
	certs = canonicalize(certs, date);

	chainNum = 0;
	while (end < certs.length) {
	    int i = start;
	    CertificateExpiredException certExpiredException = null;
	    CertificateNotYetValidException certNotYetValidException = null;

	    for (i = start; i < certs.length; i++)
	    {
		    X509Certificate currentCert = null;
		    X509Certificate issuerCert = null;

		    if (certs[i] instanceof X509Certificate)
			currentCert = (X509Certificate) certs[i];

		    if (((i+1)<certs.length) && certs[i+1] instanceof X509Certificate)
			issuerCert = (X509Certificate) certs[i+1];
		    else
			issuerCert = currentCert;
	
		    // Check if the certificate is valid and has not expired.
		    //
		    try
		    {
		    	currentCert.checkValidity();
		    }
		    catch (CertificateExpiredException e1)
		    {
		    	if (certExpiredException == null)
			   certExpiredException = e1;
		    }
		    catch (CertificateNotYetValidException e2)
		    {
		    	if (certNotYetValidException == null)
			   certNotYetValidException = e2;
		    }

		    // Check certificate extensions
		    // If the root CA is in our cacerts file,
		    // or it is the last cert in cert chain, skip the extensions check
		    if (rootStore.contains(currentCert) == false && (i+1) != certs.length 
						&& isIssuerOf(currentCert, issuerCert))
		    {
			Set critSet = currentCert.getCriticalExtensionOIDs();
			if (critSet == null)
			   critSet = Collections.EMPTY_SET;

			// Check for the basic constraints extension
			if (checkBasicConstraints(currentCert, critSet, (i-start)) == false)
			{
		           Trace.msgSecurityPrintln("trustdecider.check.basicconstraints");
			   msg = ResourceHandler.getMessage("trustdecider.check.basicconstraints");
			   throw new CertificateException(msg);
			}

			// Check for the key usage extension
			if (i == start)
			{
			   if (checkLeafKeyUsage(currentCert, critSet) == false)
			   {
		              Trace.msgSecurityPrintln("trustdecider.check.leafkeyusage");
			      msg = ResourceHandler.getMessage("trustdecider.check.leafkeyusage");
			      throw new CertificateException(msg);
			   }
			}
			else
			{
			   if (checkSignerKeyUsage(currentCert, critSet) == false)
			   {
		              Trace.msgSecurityPrintln("trustdecider.check.signerkeyusage");
			      msg = ResourceHandler.getMessage("trustdecider.check.signerkeyusage");
			      throw new CertificateException(msg);
			   }
			}

			// Certificate contains unknown critical extensions
			if (!critSet.isEmpty())
			{
		           Trace.msgSecurityPrintln("trustdecider.check.extensions");
			   msg = ResourceHandler.getMessage("trustdecider.check.extensions");
			   throw new CertificateException(msg);
			}
		    }

		    if (isIssuerOf(currentCert, issuerCert))
		    {
			// Check certificate signature
			// We verify that this issuer did indeed sign the certificate.
			try {
			   currentCert.verify(issuerCert.getPublicKey());
			} catch(GeneralSecurityException se) {
		            Trace.msgSecurityPrintln("trustdecider.check.signature");
			    msg = ResourceHandler.getMessage("trustdecider.check.signature");
			    throw new CertificateException(msg);
			}
		    } 
		    else 
			break;
	    } // loop certs in one chain
	    end = (i < certs.length) ? (i + 1): i;

	    // Get Boolean value from linkedlist. 
	    Boolean isDenyStored = (Boolean)isDenyStoredList.get(chainNum);

	    // Only pop up security dialog box for the cert chain not in Deny Session.
	    if (!isDenyStored.booleanValue())
	    {
		  // Otherwise, the browser doesn't provide any certificate
		  // verification support, and we need to popup a dialog and 
		  // determine the outcome
		  // First, we need to verify if the certificate chain is
		  // signed by a CA
		  //
		  if (rootStore.verify(certs[end-1]) == false)
		     rootCANotValid = true;

		  // Check if the user still want to proceed if the cert is expired.
		  if (certExpiredException != null || certNotYetValidException != null)
		     timeNotValid = true;

		  // This certificate chain has not been encountered before, popup
		  // certificate dialog
		  int action = showSecurityDialog(certs, start, end, rootCANotValid, timeNotValid); 

		  // Persist the action in either the permanent or session 
		  // certificate store.
		  if (action == TrustDeciderDialog.TrustOption_GrantThisSession)
		  {
		     Trace.msgSecurityPrintln("trustdecider.user.grant.session");

		     // Grant this session 
		     sessionStore.add(certs[start]);
		     sessionStore.save();
		     trustDecision = true;
		  }
		  else if (action == TrustDeciderDialog.TrustOption_GrantAlways)
		  {
		     Trace.msgSecurityPrintln("trustdecider.user.grant.forever");

		     // Grant always 
		     permanentStore.add(certs[start]);
		     permanentStore.save(); 
		     trustDecision = true;
	 	  }
		  else
		  { 
		     Trace.msgSecurityPrintln("trustdecider.user.deny");

		     // Deny
		     deniedStore.add(certs[start]);
		     deniedStore.save(); 
		  }

		  // If user Grant permission, just pass all security checks.
		  // If user Deny first signer, pop up security box for second signer certs  
		  if (trustDecision)
		     return true;
	    }
            start = end;
	    chainNum++;
	}

	return false;
    }

    /*
     * Check the Basic Constraints
     * If failed, we return false.
     */
    private static boolean checkBasicConstraints(X509Certificate cert, Set critSet, int index)
            throws CertificateException, IOException
    {
	critSet.remove(OID_BASIC_CONSTRAINTS);
	critSet.remove(OID_NETSCAPE_CERT_TYPE);
	// No basic constraints check for user cert
	if (index == 0)
	   return true;

	// CA certificate does not include basic constraints extension
        if (cert.getExtensionValue(OID_BASIC_CONSTRAINTS) == null)
        {
	   // Check Netscape certificate type extension
           if (cert.getExtensionValue(OID_NETSCAPE_CERT_TYPE) != null)
           {
              if (getNetscapeCertTypeBit(cert, NSCT_OBJECT_SIGNING_CA) == false)
	      {
		 Trace.msgSecurityPrintln("trustdecider.check.basicconstraints.certtypebit");
                 return false;
	      }
           }
	   else
	   {
	      Trace.msgSecurityPrintln("trustdecider.check.basicconstraints.extensionvalue");
	      return false;
	   }
        }
        else
        {
	   // Check Netscape certificate type extension
           if (cert.getExtensionValue(OID_NETSCAPE_CERT_TYPE) != null)
           {
	      // Require either all of bits 5,6,7 are flase or 
	      // that at least bit 7 be true
              if ((getNetscapeCertTypeBit(cert, NSCT_SSL_CA) != false || 
                  getNetscapeCertTypeBit(cert, NSCT_S_MIME_CA) != false ||
                  getNetscapeCertTypeBit(cert, NSCT_OBJECT_SIGNING_CA) != false) && 
                  getNetscapeCertTypeBit(cert, NSCT_OBJECT_SIGNING_CA) == false)
	      {
	         Trace.msgSecurityPrintln("trustdecider.check.basicconstraints.bitvalue");
		 return false;
	      }
	   }

           int constraints = cert.getBasicConstraints();
	   // End user tried to act as a CA
           if (constraints < 0)
	   {
	      Trace.msgSecurityPrintln("trustdecider.check.basicconstraints.enduser");
              return false;
	   }

           // Violated path length constraints
           if ( (index-1) > constraints)
	   {
	      Trace.msgSecurityPrintln("trustdecider.check.basicconstraints.pathlength");
              return false;
	   }
        }

	return true;
    }

    /*
     * Verify the key usage and extended key usage for intermediate
     * certificates.
     */
    private static boolean checkLeafKeyUsage(X509Certificate cert, Set critSet)
            throws CertificateException, IOException 
    { 
	critSet.remove(OID_KEY_USAGE);

        // check key usage extension
        boolean[] keyUsageInfo = cert.getKeyUsage();
        if (keyUsageInfo != null) 
	{
	   // Invalid key usage extension
	   if (keyUsageInfo.length == 0)
	   {
	      Trace.msgSecurityPrintln("trustdecider.check.leafkeyusage.length");
	      return false;
	   }

	   // keyUsageInfo[0] is for digitalSignature
	   // require digitalSignature to be set
	   boolean digitalSignature = keyUsageInfo[0];
	   if (!digitalSignature)
	   {
	      Trace.msgSecurityPrintln("trustdecider.check.leafkeyusage.digitalsignature");
	      return false;
	   }
	}

	// Check extened key usage extension only if critical
	// (interoperability problems with some certificates)
	List extKeyUsageInfo = cert.getExtendedKeyUsage();
	if ((extKeyUsageInfo != null) && critSet.contains(OID_EXTENDED_KEY_USAGE))
	{
	   critSet.remove(OID_EXTENDED_KEY_USAGE);
	   if (extKeyUsageInfo.contains(OID_EKU_ANY_USAGE) == false && 
	       extKeyUsageInfo.contains(OID_EKU_CODE_SIGNING) == false)
	   {
	      Trace.msgSecurityPrintln("trustdecider.check.leafkeyusage.extkeyusageinfo");
	      return false;
	   }
	}

	// Check Netscape certificate type extension
        if (cert.getExtensionValue(OID_NETSCAPE_CERT_TYPE) != null)
        {
           if (getNetscapeCertTypeBit(cert, NSCT_OBJECT_SIGNING) == false)
	   {
	      Trace.msgSecurityPrintln("trustdecider.check.leafkeyusage.certtypebit");
              return false;
	   }
        }

	return true;
    }

    /*
     * Verify the key usage and extended key usage for intermediate
     * certificates.
     */
    private static boolean checkSignerKeyUsage(X509Certificate cert, Set critSet)
            throws CertificateException, IOException 
    { 
	critSet.remove(OID_KEY_USAGE);

        // check key usage extension
        boolean[] keyUsageInfo = cert.getKeyUsage();
        if (keyUsageInfo != null) 
	{
	   // keyUsageInfo[5] is for keyCertSign.
	   if ((keyUsageInfo.length < 6) || (keyUsageInfo[5] == false))
	   {
	      Trace.msgSecurityPrintln("trustdecider.check.signerkeyusage.lengthandbit");
	      return false;
	   }
	}

	// Check extened key usage extension only if critical
	// (interoperability problems with some certificates)
	List extKeyUsageInfo = cert.getExtendedKeyUsage();
	if ((extKeyUsageInfo != null) && critSet.contains(OID_EXTENDED_KEY_USAGE))
	{
	   critSet.remove(OID_EXTENDED_KEY_USAGE);
	   if (extKeyUsageInfo.contains(OID_EKU_ANY_USAGE) == false)
	   {
	      Trace.msgSecurityPrintln("trustdecider.check.signerkeyusage.keyusage");
	      return false;
	   }
	}

	return true;
    }

    /*
     * Get the value of the specified bit in the Netscape certificate type
     * extension. If the extension is not present at all, we return false.
     */
    private static boolean getNetscapeCertTypeBit(X509Certificate cert, String type)
            throws CertificateException, IOException
    {
        byte[] extVal = cert.getExtensionValue(OID_NETSCAPE_CERT_TYPE);
        if (extVal == null)
            return false;

        DerInputStream in = new DerInputStream(extVal);
        byte[] encoded = in.getOctetString();
        encoded = new DerValue(encoded).getUnalignedBitString().toByteArray();

        NetscapeCertTypeExtension extn = new NetscapeCertTypeExtension(encoded);

        Boolean val = (Boolean)extn.get(type);
        return val.booleanValue();
    }

    /*
     * Returns true iff the issuer of <code>cert1</code> corresponds to the
     * subject (owner) of <code>cert2</code>.
     *
     * @return true iff the issuer of <code>cert1</code> corresponds to the
     * subject (owner) of <code>cert2</code>, false otherwise.
     */
    private static boolean isIssuerOf(X509Certificate cert1,
				      X509Certificate cert2)
    {
	Principal issuer = cert1.getIssuerDN();
	Principal subject = cert2.getSubjectDN();
	if (issuer.equals(subject))
	    return true;
	return false;
    }

    /**
     * Obtain all the certificates in the cacerts file
     */
    private static synchronized void getCertMap(KeyStore ks) throws KeyStoreException
    {
	trustedX500Principals = new HashMap();

	if (ks == null)
	   return;

	Enumeration aliases = ks.aliases();
	while (aliases.hasMoreElements())
	{
	  String alias = (String) aliases.nextElement();
	  if (ks.isCertificateEntry(alias))
	  {
	     Certificate cert = ks.getCertificate(alias);
	     if (cert instanceof X509Certificate)
		addTrustedCert((X509Certificate)cert);
	  }
	}
    }

    /**
     * Add a certificate as trusted.
     */
    private static void addTrustedCert(X509Certificate cert)
    {
	X500Principal principal = cert.getSubjectX500Principal();
	Collection coll = (Collection)trustedX500Principals.get(principal);

	if (coll == null)
	{
	   // This actually should be a set, but duplicate entries
	   // are not a problem and we can avoid the Set overhead
	   coll = new ArrayList();
	   trustedX500Principals.put(principal,coll);
	}
	coll.add(cert);
     }

    /*
     * Rewrite the certificate chain to substitue locally trusted
     * certificates in place of certificates. Also add missing self-signed
     * root certificates.
     */
    private static Certificate[] canonicalize(Certificate[] chain, Date date)
				throws CertificateException
    {
	List c = new ArrayList(chain.length);
	boolean updated = false;

	if (chain.length == 0)
	   return chain;

	for (int i = 0; i < chain.length; i++)
	{
	    X509Certificate currentCert = (X509Certificate)chain[i];
	    X509Certificate trustedCert = getTrustedCertificate(currentCert, date);

	    if (trustedCert != null)
	    {
	        Trace.msgSecurityPrintln("trustdecider.check.canonicalize.updatecert");
		currentCert = trustedCert;
	    	updated = true;
	    }

	    c.add(currentCert);	

	    // If the final cert in single signed chain is not self-signed, append a
	    // trusted certificate with a matching subject, if available.
	    Principal subjectName = ((X509Certificate)chain[i]).getSubjectX500Principal();
	    Principal issuerName = ((X509Certificate)chain[i]).getIssuerX500Principal();
	    Principal nextSubjectName = null;

	    if ( i < chain.length - 1)
	       nextSubjectName = ((X509Certificate)chain[i+1]).getSubjectX500Principal();

	    if (!issuerName.equals(subjectName) && !issuerName.equals(nextSubjectName)) 
	    { 
	 	X509Certificate issuer = getTrustedIssuerCertificate((X509Certificate)chain[i], date);
	   	if (issuer != null)
	   	{
	           // Add missing root cert
	      	   Trace.msgSecurityPrintln("trustdecider.check.canonicalize.missing");
	      	   updated = true;
	      	   c.add(issuer);
	   	}
	    }
	}

	if (updated)
	   return (Certificate[])c.toArray(new Certificate[c.size()]);
	else
	   return chain;
    }

    /*
     * Return a valid, trusted certificate that matches the input certificate,
     * or null if no such certificate can be found.
     * This method is used to replace a given certificate with a different
     * but equivalent certificate that is currently valid. This is often
     * useful as CAs reissue their root certificates with a new validity period. 
     */
    private static X509Certificate getTrustedCertificate(X509Certificate cert, Date date)
    {
	Principal certSubjectName = cert.getSubjectX500Principal();
	List list = (List)trustedX500Principals.get(certSubjectName);
	if (list == null)
	   return null;

	Principal certIssuerName = cert.getIssuerX500Principal();
	PublicKey certPublicKey = cert.getPublicKey();
	
	for (Iterator ir = list.iterator(); ir.hasNext(); )
	{
	    X509Certificate mycert = (X509Certificate)ir.next();
	    if (mycert.equals(cert))
	       continue;

	    if (!mycert.getIssuerX500Principal().equals(certIssuerName))
	       continue;

	    if (!mycert.getPublicKey().equals(certPublicKey))
	       continue;

	    try {
	       mycert.checkValidity(date);
	    }
	    catch (Exception e){
	       continue;
	    }

	    // All tests pass, this must be the one to use
	    Trace.msgSecurityPrintln("trustdecider.check.gettrustedcert.find");
	    return mycert;
	}
	return null;
    }

    private static X509Certificate getTrustedIssuerCertificate(X509Certificate cert, Date date)
    {
	Principal certIssuerName = cert.getIssuerX500Principal();
	List list = (List)trustedX500Principals.get(certIssuerName);

	if (list == null)
	   return null;

	for (Iterator ir = list.iterator(); ir.hasNext(); )
	{
	    X509Certificate mycert = (X509Certificate)ir.next();
	    try {
	       mycert.checkValidity(date);
	    }
	    catch (Exception e){
	       continue;
	    }
	    Trace.msgSecurityPrintln("trustdecider.check.gettrustedissuercert.find");
	    return mycert;
	}
	return null;
    }

    /**
     * <P> Display the security dialog about the certificate, and ask the user
     * for granting permission.
     * </P>
     *
     * @return 0 if "Yes" option is selected from the security dialog.
     * @return 1 if "No" option is selected from the security dialog.
     * @return 2 if "Always" option is selected from the security dialog.
     */
    static int showSecurityDialog(final Certificate[] certs, final int start, final int end, final boolean rootCANotValid, final boolean timeNotValid) 
    {
	int result = -1;
	try {
            result = ((Integer)PluginSysUtil.execute(new PluginSysAction() {
            public Object execute() throws Exception {
            	TrustDeciderDialog dialog = new TrustDeciderDialog(certs, start, end, rootCANotValid, timeNotValid);
            	return new Integer(dialog.DoModal());
            }})).intValue();
         }
         catch(Exception e) {
            // should never happen
            assert(false);
         }

	 return result;
    }
}
