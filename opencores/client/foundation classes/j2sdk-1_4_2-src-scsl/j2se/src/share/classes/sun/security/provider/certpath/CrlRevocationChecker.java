/*
 * @(#)CrlRevocationChecker.java	1.12 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.provider.certpath;

import java.math.BigInteger;
import java.util.Collection;
import java.util.Date;
import java.util.List;
import java.util.HashSet;
import java.util.Set;
import java.util.Iterator;
import java.security.PublicKey;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.security.cert.X509CRL;
import java.security.cert.X509CRLEntry;
import java.security.cert.PKIXCertPathChecker;
import java.security.cert.CertPathValidatorException;
import java.security.cert.CertStore;
import java.security.cert.X509CRLSelector;
import java.security.interfaces.DSAPublicKey;
import javax.security.auth.x500.X500Principal;
import sun.security.util.Debug;
import sun.security.x509.X509CRLEntryImpl;
import sun.security.x509.CRLReasonCodeExtension;
import sun.security.x509.KeyUsageExtension;
import sun.security.x509.PKIXExtensions;

/**
 * CrlRevocationChecker is a <code>PKIXCertPathChecker</code> that checks
 * revocation status information on a PKIX certificate using CRLs obtained
 * from one or more <code>CertStores</code>. This is based on revision draft
 * three of rfc2459 
 * (http://www.ietf.org/internet-drafts/draft-ietf-pkix-new-part1-02.txt).
 *
 * @version 	1.12, 01/23/03
 * @since	1.4
 * @author	Seth Proctor
 */
class CrlRevocationChecker extends PKIXCertPathChecker {
 
    private static final Debug debug = Debug.getInstance("certpath");
    private final PublicKey mInitPubKey;
    private final List mStores;
    private final String mSigProvider;
    private final Date mCurrentTime;
    private PublicKey mPrevPubKey;
    private boolean mCRLSignFlag;
    private HashSet mPossibleCRLs;
    private HashSet mApprovedCRLs;

    /**
     * Default Constructor.
     *
     * @param initPubKey initial PublicKey in the path
     * @param stores a List of CertStores for retreiving CRLs
     * @param sigProvider a String used to validate CRLs
     * @param testDate a Date representing the time against which to test, or
     * null which uses the current time
     */
    CrlRevocationChecker(PublicKey initPubKey, List stores, 
	String sigProvider, Date testDate) throws CertPathValidatorException
    {
	mInitPubKey = initPubKey;
	mStores = stores;
	mSigProvider = sigProvider;

	if (testDate != null)
	    mCurrentTime = testDate;
	else
	    mCurrentTime = new Date();

	init(false);
    }
    
    /**
     * Initializes the internal state of the checker from parameters
     * specified in the constructor
     */
    public void init(boolean forward) throws CertPathValidatorException
    {
	if (!forward) {
	    mPrevPubKey = mInitPubKey;
	    mCRLSignFlag = true;
	} else {
	    throw new CertPathValidatorException("forward checking "
				+ "not supported");
	}
    }

    public boolean isForwardCheckingSupported() {
	return false;
    }

    public Set getSupportedExtensions() {
	return null;
    }

    /**
     * Performs the revocation status check on the certificate using
     * its internal state. 
     *
     * @param cert the Certificate
     * @param unresolvedCritExts a Collection of the unresolved critical
     * extensions
     * @exception CertPathValidatorException Exception thrown if
     * certificate does not verify
     */
    public void check(Certificate cert, Collection unresolvedCritExts)
        throws CertPathValidatorException
    {
        X509Certificate currCert = (X509Certificate) cert;
        verifyRevocationStatus(currCert, mPrevPubKey, mCRLSignFlag);

        // Make new public key if parameters are missing
        PublicKey cKey = currCert.getPublicKey();
        if (cKey instanceof DSAPublicKey &&
            ((DSAPublicKey)cKey).getParams() == null) {
            // cKey needs to inherit DSA parameters from prev key
            cKey = BasicChecker.makeInheritedParamsKey(cKey, mPrevPubKey);
        }
        mPrevPubKey = cKey;
        mCRLSignFlag = certCanSignCrl(currCert);
    }

    /**
     * Performs the revocation status check on the certificate using
     * the provided state variables, as well as the constant internal
     * data.
     *
     * @param currCert the Certificate
     * @param prevKey the previous PublicKey in the chain
     * @param signFlag a boolean as returned from the last call, or true
     * if this is the first cert in the chain
     * @return a boolean specifying if the cert is allowed to vouch for the
     * validity of a CRL for the next iteration
     * @exception CertPathValidatorException Exception thrown if
     *            certificate does not verify.
     */
    public boolean check(X509Certificate currCert, PublicKey prevKey,
	boolean signFlag) throws CertPathValidatorException
    {
	verifyRevocationStatus(currCert, prevKey, signFlag);
	return certCanSignCrl(currCert);
    }

    /**
     * Checks that a cert can be used to verify a CRL.
     *
     * @param currCert an X509Certificate to check
     * @return a boolean specifying if the cert is allowed to vouch for the
     * validity of a CRL
     */
    public boolean certCanSignCrl(X509Certificate currCert) {
	// if the cert doesn't include the key usage ext, or 
	// the key usage ext asserts cRLSigning, set CRL_sign_flag to
	// true, otherwise set it to false.
	try {
	    boolean [] kbools = currCert.getKeyUsage();

	    if (kbools != null) {
		KeyUsageExtension ku = new KeyUsageExtension(kbools);
		Boolean b = (Boolean) ku.get(KeyUsageExtension.CRL_SIGN);
		if (! b.booleanValue())
		    return false;
		else
		    return true;
	    } else
		return true;
	} catch (Exception e) {
	    if (debug != null) {
	       	debug.println("CrlRevocationChecker.certCanSignCRL() "
		    + "unexpected exception");
	       	e.printStackTrace();
	    }
	    return false;
	}
    }

    /**
     * Internal method to start the verification of a cert
     */
    private void verifyRevocationStatus(X509Certificate currCert, 
        PublicKey prevKey, boolean signFlag) throws CertPathValidatorException
    {
	String msg = "revocation status";
	if (debug != null)
	    debug.println("CrlRevocationChecker.verifyRevocationStatus()" +
	        " ---checking " + msg + "...");

	if (!signFlag)
	    throw new CertPathValidatorException("cert can't vouch for CRL");

	// to start, get the entire list of possible CRLs
	X500Principal certIssuer = currCert.getIssuerX500Principal();

	// init the state for this run
	mPossibleCRLs = new HashSet();
	mApprovedCRLs = new HashSet();

	try {
	    X509CRLSelector sel = new X509CRLSelector();
	    sel.setCertificateChecking(currCert);
	    sel.setDateAndTime(mCurrentTime);

	    // add the default issuer string
	    CertPathHelper.addIssuer(sel, certIssuer);

	    Iterator i = mStores.iterator();
	    while (i.hasNext()) {
		mPossibleCRLs.addAll(((CertStore) i.next()).getCRLs(sel));
	    }
	    DistributionPointFetcher store =
	    	DistributionPointFetcher.getInstance();
	    mPossibleCRLs.addAll(store.getCRLs(sel));
        } catch (Exception e) {
            if (debug != null) {
	        debug.println("CrlRevocationChecker.verifyRevocationStatus() " 
		    + "unexpected exception: " + e.getMessage());
       	        e.printStackTrace();
            }
	    throw new CertPathValidatorException(e);
        }
	    
        if (mPossibleCRLs.isEmpty()) {
	    // we are assuming the directory is not secure,
	    // so someone may have removed all the CRLs.
	    throw new CertPathValidatorException(msg + 
		" check failed: no CRL found");
	}
		
	if (debug != null) {
	    debug.println("CrlRevocationChecker.verifyRevocationStatus() " +
	        "crls.size() = " + mPossibleCRLs.size());
	}
	// Now that we have a list of possible CRLs, see which ones can
	// be approved
	Iterator iter = mPossibleCRLs.iterator();
	while (iter.hasNext()) {
	    X509CRL crl = (X509CRL) iter.next();
	    if (verifyPossibleCRL(crl, certIssuer, prevKey)) {
		mApprovedCRLs.add(crl);
	    }
	}

	// make sure that we have at least one CRL that _could_ cover
	// the certificate in question
	if (mApprovedCRLs.isEmpty()) {
	    throw new CertPathValidatorException("no possible CRLs");
	}
	/*
	 * SECTION 6.3.3.4
	 */

	// See if the cert is in the set of approved crls. If the
	// cert is listed on hold in one crl, and revoked in another, ignore
	// the hold. 
	if (debug != null) {
	    debug.println("starting the final sweep...");
	}
	iter = mApprovedCRLs.iterator();
	BigInteger sn = currCert.getSerialNumber();
        if (debug != null)
            debug.println("CrlRevocationChecker.verifyRevocationStatus" +
                          " cert SN: " + sn.toString());

	boolean hold = false;
	while (iter.hasNext()) {
	    X509CRL crl = (X509CRL) iter.next();
	    X509CRLEntry entry = (X509CRLEntry) crl.getRevokedCertificate(sn);
	    if (entry != null) {
                if (debug != null)
                    debug.println("CrlRevocationChecker.verifyRevocationStatus" 
			+ " CRL entry: " + entry.toString());

		int reasonCode = 0;
	        try { 
	            X509CRLEntryImpl entryImpl = X509CRLEntryImpl.toImpl(entry);
		    Integer reason = entryImpl.getReasonCode();
		    // if reasonCode extension is absent, this is equivalent
		    // to a reasonCode value of unspecified (0)
		    if (reason == null) {
			reasonCode = CRLReasonCodeExtension.UNSPECIFIED;
		    } else {
		        reasonCode = reason.intValue();
		    }
 	        } catch (Exception e) {
		    throw new CertPathValidatorException(e);
	        }

		/*
		 * If reason code is CERTIFICATE_HOLD, continue to look
		 * for other revoked entries with different reasons before
		 * exiting loop.
		 */
		hold = (reasonCode == CRLReasonCodeExtension.CERTIFICATE_HOLD);

		/*
		 * The certificate fails the revocation check if it is not
		 * on hold and the reason code is not REMOVE_FROM_CRL, which
		 * indicates a certificate that used to be but is no longer on 
		 * hold status. It should not be considered fatal.
		 */
		if (!hold 
		    && reasonCode != CRLReasonCodeExtension.REMOVE_FROM_CRL) 
		{
		    throw new CertPathValidatorException("Certificate has been"
			+ " revoked, reason: " + reasonToString(reasonCode));
		}

		/*
		 * Throw an exception if any unresolved critical extensions 
		 * remain in the CRL entry
		 */
		Set unresCritExts = entry.getCriticalExtensionOIDs();
		if ((unresCritExts != null) && !unresCritExts.isEmpty()) {
		    /* remove any that we have processed */
		    unresCritExts.remove
			(PKIXExtensions.ReasonCode_Id.toString());
		    if (!unresCritExts.isEmpty())
                	throw new CertPathValidatorException("Unrecognized "
			    + "critical extension(s) in revoked CRL entry: " + unresCritExts);
		}
	    }
	}

	if (hold) {
	    throw new CertPathValidatorException("Certificate is on hold");
	}
    }

    /**
     * Return a String describing the reasonCode value
     */
    private static String reasonToString(int reasonCode) {
        switch (reasonCode) {
            case CRLReasonCodeExtension.UNSPECIFIED: 
		return "unspecified";
            case CRLReasonCodeExtension.KEY_COMPROMISE: 
		return "key compromise";
            case CRLReasonCodeExtension.CA_COMPROMISE: 
		return "CA compromise";
            case CRLReasonCodeExtension.AFFLIATION_CHANGED: 
		return "affiliation changed";
            case CRLReasonCodeExtension.SUPERSEDED: 
		return "superseded";
            case CRLReasonCodeExtension.CESSATION_OF_OPERATION: 
		return "cessation of operation";
            case CRLReasonCodeExtension.CERTIFICATE_HOLD: 
		return "certificate hold";
            case CRLReasonCodeExtension.REMOVE_FROM_CRL: 
		return "remove from CRL";
            default: 
		return "unrecognized reason code";
        }
    }
	
    /**
     * Internal method that accepts a crl from the list of possible_crls,
     * and sees if it is approved, based on the cert.
     *
     * @param crl a possible CRL to test for acceptability
     * @param certIssuer the issuer DN of the certificate whose revocation
     * status is being checked
     * @param prevKey the public key of the issuer of cert
     * @return true if the crl is acceptable, false otherwise.
     */
    private boolean verifyPossibleCRL(X509CRL crl, X500Principal certIssuer, 
	PublicKey prevKey) throws CertPathValidatorException
    {
	/*
	 * SECTION 6.3.3.1.b (verify that the CRL issuer matches
	 * the certificate issuer)
	 */
	if (!crl.getIssuerX500Principal().equals(certIssuer)) {
	    if (debug != null)
		debug.println("CRL issuer does not match cert issuer");
	    return false;
	}

	/*
	 * SECTION 6.3.3.1.d (validate the signature on the CRL)
	 */
        try {
            crl.verify(prevKey, mSigProvider);
        } catch (Exception e) {
            if (debug != null) {
	        debug.println("CRL signature failed to verify");
	        e.printStackTrace();
            }
	    return false;
        }

	/*
	 * SECTION 6.3.3.2.a (if the value of the next update field
	 * is before the current time, obtain an appropriate delta
	 * CRL or discard the CRL).
	 */
        Date nextUpdate = crl.getNextUpdate();
        if (nextUpdate != null && nextUpdate.before(mCurrentTime)) {
	    if (debug != null)
		debug.println("discarding stale CRL (nextUpdate is before "
		    + "required validation time)");
	    return false;
	}

	/*
	 * Section 6.3.3.2.b-d omitted since Delta CRLs not supported
	 */

	/*
	 * Throw an exception if any unresolved critical extensions 
	 * remain in the CRL.
	 */
        Set unresCritExts = crl.getCriticalExtensionOIDs();
	if (unresCritExts != null && !unresCritExts.isEmpty()) {
	    if (debug != null) {
	        Iterator i = unresCritExts.iterator();
		while (i.hasNext())
		    debug.println((String) i.next());
	    }
       	    throw new CertPathValidatorException("Unrecognized "
	        + "critical extension(s) in CRL: " + unresCritExts);
        }

        return true;
    }
}
