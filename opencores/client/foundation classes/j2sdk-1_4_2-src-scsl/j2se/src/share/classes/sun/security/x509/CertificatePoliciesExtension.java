/*
 * @(#)CertificatePoliciesExtension.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.x509;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.reflect.Array;
import java.util.Enumeration;
import java.util.Vector;

import sun.security.util.DerValue;
import sun.security.util.DerOutputStream;

/**
 * This class defines the certificate policies extension which specifies the
 * policies under which the certificate has been issued
 * and the purposes for which the certificate may be used.
 * <p>
 * Applications with specific policy requirements are expected to have a
 * list of those policies which they will accept and to compare the
 * policy OIDs in the certificate to that list.  If this extension is
 * critical, the path validation software MUST be able to interpret this
 * extension (including the optional qualifier), or MUST reject the
 * certificate.
 * <p>
 * Optional qualifiers are not supported in this implementation, as they are
 * not recommended by RFC2459.
 *
 * The ASN.1 syntax for this is (IMPLICIT tagging is defined in the
 * module definition):
 * <pre>
 * id-ce-certificatePolicies OBJECT IDENTIFIER ::=  { id-ce 32 }
 *
 * certificatePolicies ::= SEQUENCE SIZE (1..MAX) OF PolicyInformation
 *
 * PolicyInformation ::= SEQUENCE {
 *      policyIdentifier   CertPolicyId,
 *      policyQualifiers   SEQUENCE SIZE (1..MAX) OF
 *                              PolicyQualifierInfo OPTIONAL }
 *
 * CertPolicyId ::= OBJECT IDENTIFIER
 * </pre>
 * @author Anne Anderson
 * @version 1.3, 01/23/03
 * @since	1.4
 * @see Extension
 * @see CertAttrSet
 */
public class CertificatePoliciesExtension extends Extension
implements CertAttrSet {
    /**
     * Identifier for this attribute, to be used with the
     * get, set, delete methods of Certificate, x509 type.
     */
    public static final String IDENT = "x509.info.extensions.CertificatePolicies";
    /**
     * Attribute names.
     */
    public static final String NAME = "CertificatePolicies";
    public static final String POLICIES = "policies";

    /**
     * Vector of PolicyInformation for this object.
     */
    private Vector certPolicies;

    // Encode this extension value.
    private void encodeThis() throws IOException {
        if (certPolicies == null || certPolicies.isEmpty()) {
            this.extensionValue = null;
        } else {
	    DerOutputStream os = new DerOutputStream();
	    DerOutputStream tmp = new DerOutputStream();

	    for (int i = 0; i < certPolicies.size(); i++) {
		((PolicyInformation)certPolicies.elementAt(i)).encode(tmp);
	    }

	    os.write(DerValue.tag_Sequence, tmp);
	    this.extensionValue = os.toByteArray();
	}
    }

    /**
     * Create a CertificatePoliciesExtension object from
     * a Vector of PolicyInformation; the criticality is set to false.
     *
     * @param certPolicies the Vector of PolicyInformation.
     */
    public CertificatePoliciesExtension(Vector certPolicies)
    throws IOException {
	this(Boolean.FALSE, certPolicies);
    }

    /**
     * Create a CertificatePoliciesExtension object from
     * a Vector of PolicyInformation with specified criticality.
     *
     * @param critical true if the extension is to be treated as critical.
     * @param certPolicies the Vector of PolicyInformation.
     */
    public CertificatePoliciesExtension(Boolean critical, Vector certPolicies)
    throws IOException {
	this.certPolicies = certPolicies;
	this.extensionId = PKIXExtensions.CertificatePolicies_Id;
	this.critical = critical.booleanValue();
        encodeThis();
    }

    /**
     * Create the extension from its DER encoded value and criticality.
     *
     * @param critical true if the extension is to be treated as critical.
     * @param value Array of DER encoded bytes of the actual value.
     * @exception IOException on error.
     */
    public CertificatePoliciesExtension(Boolean critical, Object value)
    throws IOException {
	this.extensionId = PKIXExtensions.CertificatePolicies_Id;
	this.critical = critical.booleanValue();
	//Array.getLength will throw an IllegalArgumentException if
	//value is not an array, and Array.getByte will do so if the
	//values are not bytes, so a type check is not needed.
        int len = Array.getLength(value);
	byte[] extValue = new byte[len];
	for (int i=0; i < len; i++)
	    extValue[i] = Array.getByte(value, i);
	this.extensionValue = extValue;
	DerValue val = new DerValue(extValue);
	if (val.tag != DerValue.tag_Sequence) {
	    throw new IOException("Invalid encoding for " +
				   "CertificatePoliciesExtension.");
	}
	certPolicies = new Vector(1, 1);
	while (val.data.available() != 0) {
	    DerValue seq = val.data.getDerValue();
	    PolicyInformation policy = new PolicyInformation(seq);
	    certPolicies.addElement(policy);
	}
    }

    /**
     * Return the extension as user readable string.
     */
    public String toString() {
	if (certPolicies == null) return "";
	String s = super.toString();
	s += "CertificatePolicies [\n";
	Enumeration enum = certPolicies.elements();
	while (enum.hasMoreElements()) {
	    s += ((PolicyInformation)(enum.nextElement())).toString();
	}
	s += "]\n";
	return s;
    }

    /**
     * Decode the extension from the InputStream.
     *
     * @param in the InputStream to unmarshal the contents from.
     * @exception IOException on decoding or validity errors.
     */
    public void decode(InputStream in) throws IOException {
        throw new IOException("Method not to be called directly.");
    }

    /**
     * Write the extension to the DerOutputStream.
     *
     * @param out the DerOutputStream to write the extension to.
     * @exception IOException on encoding errors.
     */
    public void encode(OutputStream out) throws IOException {
        DerOutputStream tmp = new DerOutputStream();
        if (extensionValue == null) {
	  extensionId = PKIXExtensions.CertificatePolicies_Id;
	  critical = false;
	  encodeThis();
	}
	super.encode(tmp);
	out.write(tmp.toByteArray());
    }

    /**
     * Set the attribute value.
     */
    public void set(String name, Object obj) throws IOException {
	if (name.equalsIgnoreCase(POLICIES)) {
	    if (!(obj instanceof Vector)) {
	        throw new IOException("Attribute value should be of type Vector.");
	    }
	    this.certPolicies = (Vector)obj;
	} else {
	  throw new IOException("Attribute name [" + name + 
                                "] not recognized by " +
				"CertAttrSet:CertificatePoliciesExtension.");
	}
        encodeThis();
    }

    /**
     * Get the attribute value.
     */
    public Object get(String name) throws IOException {
	if (name.equalsIgnoreCase(POLICIES)) {
	    //XXXX May want to consider cloning this
	    return certPolicies;
	} else {
	  throw new IOException("Attribute name [" + name + 
				"] not recognized by " +
				"CertAttrSet:CertificatePoliciesExtension.");
	}
    }

    /**
     * Delete the attribute value.
     */
    public void delete(String name) throws IOException {
	if (name.equalsIgnoreCase(POLICIES)) {
	    certPolicies = null;
	} else {
	  throw new IOException("Attribute name [" + name + 
			        "] not recognized by " +
				"CertAttrSet:CertificatePoliciesExtension.");
	}
        encodeThis();
    }

    /**
     * Return an enumeration of names of attributes existing within this
     * attribute.
     */
    public Enumeration getElements() {
        AttributeNameEnumeration elements = new AttributeNameEnumeration();
        elements.addElement(POLICIES);

	return (elements.elements());
    }

    /**
     * Return the name of this attribute.
     */
    public String getName() {
        return (NAME);
    }
}
