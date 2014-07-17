/*
 * @(#)X509CRLEntryImpl.java	1.22 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package sun.security.x509;

import java.io.InputStream;
import java.io.IOException;
import java.security.cert.CRLException;
import java.security.cert.CertificateException;
import java.security.cert.X509CRLEntry;
import java.math.BigInteger;
import java.util.Collection;
import java.util.Date;
import java.util.Enumeration;
import java.util.Set;
import java.util.HashSet;

import sun.security.util.*;
import sun.misc.HexDumpEncoder;

/**
 * <p>Abstract class for a revoked certificate in a CRL.
 * This class is for each entry in the <code>revokedCertificates</code>,
 * so it deals with the inner <em>SEQUENCE</em>.
 * The ASN.1 definition for this is:
 * <pre>
 * revokedCertificates    SEQUENCE OF SEQUENCE  {
 *     userCertificate    CertificateSerialNumber,
 *     revocationDate     ChoiceOfTime,
 *     crlEntryExtensions Extensions OPTIONAL
 *                        -- if present, must be v2
 * }  OPTIONAL
 *
 * CertificateSerialNumber  ::=  INTEGER
 *
 * Extensions  ::=  SEQUENCE SIZE (1..MAX) OF Extension
 *
 * Extension  ::=  SEQUENCE  {
 *     extnId        OBJECT IDENTIFIER,
 *     critical      BOOLEAN DEFAULT FALSE,
 *     extnValue     OCTET STRING
 *                   -- contains a DER encoding of a value
 *                   -- of the type registered for use with
 *                   -- the extnId object identifier value
 * }
 * </pre>
 *
 * @author Hemma Prafullchandra
 * @version 1.22 01/23/03
 */

public class X509CRLEntryImpl extends X509CRLEntry {

    private SerialNumber serialNumber = null;
    private Date revocationDate = null;
    private CRLExtensions extensions = null;
    private byte[] revokedCert = null;
    private final static boolean isExplicit = false;
    private static final long YR_2050 = 2524636800000L;

    /**
     * Should not be used.
     */
    private X509CRLEntryImpl() { }

    /**
     * Constructs a revoked certificate entry using the given
     * serial number and revocation date.
     *
     * @param num the serial number of the revoked certificate.
     * @param date the Date on which revocation took place.
     */
    public X509CRLEntryImpl(BigInteger num, Date date) {
        this.serialNumber = new SerialNumber(num);
        this.revocationDate = date;
    }

    /**
     * Constructs a revoked certificate entry using the given
     * serial number, revocation date and the entry
     * extensions.
     *
     * @param num the serial number of the revoked certificate.
     * @param date the Date on which revocation took place.
     * @param crlEntryExts the extensions for this entry.
     */
    public X509CRLEntryImpl(BigInteger num, Date date,
                           CRLExtensions crlEntryExts) {
        this.serialNumber = new SerialNumber(num);
        this.revocationDate = date;
        this.extensions = crlEntryExts;
    }

    /**
     * Unmarshals a revoked certificate from its encoded form.
     *
     * @param revokedCert the encoded bytes.
     * @exception CRLException on parsing errors.
     */
    public X509CRLEntryImpl(byte[] revokedCert) throws CRLException {
        try {
            parse(new DerValue(revokedCert));
        } catch (IOException e) {
            this.revokedCert = null;
            throw new CRLException("Parsing error: " + e.toString());
        }
    }

    /**
     * Unmarshals a revoked certificate from its encoded form.
     *
     * @param derVal the DER value containing the revoked certificate.
     * @exception CRLException on parsing errors.
     */
    public X509CRLEntryImpl(DerValue derValue) throws CRLException {
        try {
            parse(derValue);
        } catch (IOException e) {
            revokedCert = null;
            throw new CRLException("Parsing error: " + e.toString());
        }
    }

    /**
     * Returns true if this revoked certificate entry has
     * extensions, otherwise false.
     *
     * @return true if this CRL entry has extensions, otherwise
     * false.
     */
    public boolean hasExtensions() {
        if (extensions == null)
            return false;
        else
            return true;
    }

    /**
     * Encodes the revoked certificate to an output stream.
     *
     * @param outStrm an output stream to which the encoded revoked
     * certificate is written.
     * @exception CRLException on encoding errors.
     */
    public void encode(DerOutputStream outStrm) throws CRLException {
        try {
            if (revokedCert == null) {
                DerOutputStream tmp = new DerOutputStream();
                // sequence { serialNumber, revocationDate, extensions }
                serialNumber.encode(tmp);

                if (revocationDate.getTime() < YR_2050) {
                    tmp.putUTCTime(revocationDate);
                } else {
                    tmp.putGeneralizedTime(revocationDate);
                }

                if (extensions != null)
                    extensions.encode(tmp, isExplicit);

                DerOutputStream seq = new DerOutputStream();
                seq.write(DerValue.tag_Sequence, tmp);
 
                revokedCert = seq.toByteArray();
            }
            outStrm.write(revokedCert);
        } catch (IOException e) {
             throw new CRLException("Encoding error: " + e.toString());
        }
    }

    /**
     * Returns the ASN.1 DER-encoded form of this CRL Entry,
     * which corresponds to the inner SEQUENCE.
     *   
     * @exception CRLException if an encoding error occurs.
     */  
    public byte[] getEncoded() throws CRLException {
        if (revokedCert == null)
            this.encode(new DerOutputStream());
        return (byte[])revokedCert.clone();
    }

    /**
     * Gets the serial number from this X509CRLEntry,
     * i.e. the <em>userCertificate</em>.
     * 
     * @return the serial number.
     */
    public BigInteger getSerialNumber() {
        return serialNumber.getNumber();
    }

    /**
     * Gets the revocation date from this X509CRLEntry,
     * the <em>revocationDate</em>.
     * 
     * @return the revocation date.
     */
    public Date getRevocationDate() {
        return (new Date(revocationDate.getTime()));
    }

    /**
     * get Reason Code from CRL entry.
     *   
     * @returns Integer or null, if no such extension
     * @throws IOException on error
     */  
    public Integer getReasonCode() throws IOException {
        Object obj = getExtension(PKIXExtensions.ReasonCode_Id);
        if (obj == null)
            return null;
        CRLReasonCodeExtension reasonCode = (CRLReasonCodeExtension)obj;
        return (Integer)(reasonCode.get(reasonCode.REASON));
    }

    /**
     * Returns a printable string of this revoked certificate.
     *
     * @return value of this revoked certificate in a printable form.
     */
    public String toString() {
        StringBuffer sb = new StringBuffer();

        sb.append(serialNumber.toString());
        sb.append("  On: " + revocationDate.toString());
        if (extensions != null) {
            Collection allEntryExts = extensions.getAllExtensions();
            Object[] objs = allEntryExts.toArray();

            sb.append("\n    CRL Entry Extensions: " + objs.length);
            for (int i = 0; i < objs.length; i++) {
                sb.append("\n    [" + (i+1) + "]: ");
                Extension ext = (Extension)objs[i];
                try {
                    if (OIDMap.getClass(ext.getExtensionId()) == null) {
                        sb.append(ext.toString());
                        byte[] extValue = ext.getExtensionValue();
                        if (extValue != null) {
                            DerOutputStream out = new DerOutputStream();
                            out.putOctetString(extValue);
                            extValue = out.toByteArray();
                            HexDumpEncoder enc = new HexDumpEncoder();
                            sb.append("Extension unknown: "
                                      + "DER encoded OCTET string =\n"
                                      + enc.encodeBuffer(extValue) + "\n");
                        }
                    } else
                        sb.append(ext.toString()); //sub-class exists
                } catch (Exception e) {
                    sb.append(", Error parsing this extension");
                }
            }
        }
        sb.append("\n");
        return (sb.toString());
    }

    /** 
     * Return true if a critical extension is found that is
     * not supported, otherwise return false. 
     */   
    public boolean hasUnsupportedCriticalExtension() {
        if (extensions == null)
            return false;
        return extensions.hasUnsupportedCriticalExtension(); 
    } 
 
    /**
     * Gets a Set of the extension(s) marked CRITICAL in this
     * X509CRLEntry.  In the returned set, each extension is
     * represented by its OID string.
     *
     * @return a set of the extension oid strings in the
     * Object that are marked critical.
     */  
    public Set getCriticalExtensionOIDs() {
        if (extensions == null)
            return null;
        HashSet extSet = new HashSet(11);
        Extension ex;
        for (Enumeration e = extensions.getElements();
                                      e.hasMoreElements();) {
            ex = (Extension)e.nextElement();
            if (ex.isCritical())
                extSet.add(((ObjectIdentifier)ex.getExtensionId()).toString());
        }
        return extSet;
    }

    /**
     * Gets a Set of the extension(s) marked NON-CRITICAL in this
     * X509CRLEntry. In the returned set, each extension is
     * represented by its OID string.
     *   
     * @return a set of the extension oid strings in the
     * Object that are marked critical.
     */  
    public Set getNonCriticalExtensionOIDs() {
        if (extensions == null)
            return null;
        HashSet extSet = new HashSet(11);
        Extension ex;
        for (Enumeration e = extensions.getElements();
                                      e.hasMoreElements();) {
            ex = (Extension)e.nextElement();
            if ( ! ex.isCritical())
                extSet.add(((ObjectIdentifier)ex.getExtensionId()).toString());
        }
        return extSet;
    }

    /**
     * Gets the DER encoded OCTET string for the extension value
     * (<em>extnValue</em>) identified by the passed in oid String.
     * The <code>oid</code> string is
     * represented by a set of positive whole number separated
     * by ".", that means,<br>
     * &lt;positive whole number&gt;.&lt;positive whole number&gt;.&lt;positive
     * whole number&gt;.&lt;...&gt;
     *
     * @param oid the Object Identifier value for the extension.
     * @return the DER encoded octet string of the extension value.
     */
    public byte[] getExtensionValue(String oid) {
        if (extensions == null)
            return null;
        try {
            String extAlias = OIDMap.getName(new ObjectIdentifier(oid));
            Extension crlExt = null;

            if (extAlias == null) { // may be unknown
                ObjectIdentifier findOID = new ObjectIdentifier(oid);
                Extension ex = null;
                ObjectIdentifier inCertOID;
                for (Enumeration e=extensions.getElements();
                                                 e.hasMoreElements();) {
                    ex = (Extension)e.nextElement();
                    inCertOID = ex.getExtensionId();
                    if (inCertOID.equals(findOID)) {
                        crlExt = ex;
                        break;
                    }
                }
            } else
                crlExt = extensions.get(extAlias);
            if (crlExt == null)
                return null;
            byte[] extData = crlExt.getExtensionValue();
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
     * get an extension
     *
     * @param oid ObjectIdentifier of extension desired
     * @returns Extension of type <extension> or null, if not found
     */
    public Extension getExtension(ObjectIdentifier oid) {
        if (extensions == null)
            return null;

        // following returns null if no such OID in map
        //XXX consider cloning this
        return extensions.get(OIDMap.getName(oid));
    }

    private void parse(DerValue derVal)
    throws CRLException, IOException {

        if (derVal.tag != DerValue.tag_Sequence) {
            throw new CRLException("Invalid encoded RevokedCertificate, " +
                                  "starting sequence tag missing.");
        }
        if (derVal.data.available() == 0)
            throw new CRLException("No data encoded for RevokedCertificates");

        revokedCert = derVal.toByteArray();
        // serial number
        DerInputStream in = derVal.toDerInputStream();
        DerValue val = in.getDerValue();
        this.serialNumber = new SerialNumber(val);

        // revocationDate
        int nextByte = derVal.data.peekByte();
        if ((byte)nextByte == DerValue.tag_UtcTime) {
            this.revocationDate = derVal.data.getUTCTime();
        } else if ((byte)nextByte == DerValue.tag_GeneralizedTime) {
            this.revocationDate = derVal.data.getGeneralizedTime();
        } else
            throw new CRLException("Invalid encoding for revocation date");
        
        if (derVal.data.available() == 0)
            return;  // no extensions

        // crlEntryExtensions
        this.extensions = new CRLExtensions(derVal.toDerInputStream());
    }
    
    /**
     * Utility method to convert an arbitrary instance of X509CRLEntry
     * to a X509CRLEntryImpl. Does a cast if possible, otherwise reparses
     * the encoding.
     */
    public static X509CRLEntryImpl toImpl(X509CRLEntry entry) 
	    throws CRLException {
	if (entry instanceof X509CRLEntryImpl) {
	    return (X509CRLEntryImpl)entry;
	} else {
	    return new X509CRLEntryImpl(entry.getEncoded());
	}
    }
}
