/*
 * @(#)PolicyMappingsExtension.java	1.14 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.x509;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.reflect.Array;
import java.security.cert.CertificateException;
import java.util.Vector;
import java.util.Enumeration;

import sun.security.util.*;

/**
 * Represent the Policy Mappings Extension.
 *
 * This extension, if present, identifies the certificate policies considered
 * identical between the issuing and the subject CA.
 * <p>Extensions are addiitonal attributes which can be inserted in a X509
 * v3 certificate. For example a "Driving License Certificate" could have
 * the driving license number as a extension.
 *
 * <p>Extensions are represented as a sequence of the extension identifier
 * (Object Identifier), a boolean flag stating whether the extension is to
 * be treated as being critical and the extension value itself (this is again
 * a DER encoding of the extension value).
 *
 * @author Amit Kapoor
 * @author Hemma Prafullchandra
 * @version 1.14
 * @see Extension
 * @see CertAttrSet
 */
public class PolicyMappingsExtension extends Extension
implements CertAttrSet {
    /**
     * Identifier for this attribute, to be used with the
     * get, set, delete methods of Certificate, x509 type.
     */
    public static final String IDENT = "x509.info.extensions.PolicyMappings";
    /**
     * Attribute names.
     */
    public static final String NAME = "PolicyMappings";
    public static final String MAP = "map";

    // Private data members
    private Vector maps = null;

    // Encode this extension value
    private void encodeThis() throws IOException {
        if (maps == null || maps.isEmpty()) {
            this.extensionValue = null;
            return;
        }
        DerOutputStream os = new DerOutputStream();
        DerOutputStream tmp = new DerOutputStream();

        for (int i = 0; i < maps.size(); i++) {
            ((CertificatePolicyMap)maps.elementAt(i)).encode(tmp);
        }
        os.write(DerValue.tag_Sequence, tmp);
        this.extensionValue = os.toByteArray();
    }

    /**
     * Create a PolicyMappings with the Vector of CertificatePolicyMap.
     *
     * @param maps the Vector of CertificatePolicyMap.
     */
    public PolicyMappingsExtension(Vector map) throws IOException {
        this.maps = map;
        this.extensionId = PKIXExtensions.PolicyMappings_Id;
        this.critical = false;
        encodeThis();
    }

    /**
     * Create a default PolicyMappingsExtension.
     */
    public PolicyMappingsExtension() {
        extensionId = PKIXExtensions.KeyUsage_Id;
        critical = false;
        maps = new Vector(1, 1);
    }

    /**
     * Create the extension from the passed DER encoded value.
     *
     * @params critical true if the extension is to be treated as critical.
     * @params value Array of DER encoded bytes of the actual value.
     * @exception IOException on error.
     */
    public PolicyMappingsExtension(Boolean critical, Object value)
    throws IOException {
        this.extensionId = PKIXExtensions.PolicyMappings_Id;
        this.critical = critical.booleanValue();

        int len = Array.getLength(value);
	byte [] extValue = new byte[len];
	for (int i = 0; i < len; i++) {
	  extValue[i] = Array.getByte(value, i);
	}
        this.extensionValue = extValue;
        DerValue val = new DerValue(extValue);
        if (val.tag != DerValue.tag_Sequence) {
            throw new IOException("Invalid encoding for " +
                                  "PolicyMappingsExtension.");
        }
        maps = new Vector(1, 1);
        while (val.data.available() != 0) {
            DerValue seq = val.data.getDerValue();
            CertificatePolicyMap map = new CertificatePolicyMap(seq);
            maps.addElement(map);
        }
    }

    /**
     * Returns a printable representation of the policy map.
     */
    public String toString() {
        if (maps == null) return "";
        String s = super.toString() + "PolicyMappings [\n"
                 + maps.toString() + "]\n";

        return (s);
    }

    /**
     * Write the extension to the OutputStream.
     *
     * @param out the OutputStream to write the extension to.
     * @exception IOException on encoding errors.
     */
    public void encode(OutputStream out) throws IOException {
        DerOutputStream tmp = new DerOutputStream();
        if (extensionValue == null) {
            extensionId = PKIXExtensions.PolicyMappings_Id;
            critical = false;
            encodeThis();
        }
        super.encode(tmp);
	out.write(tmp.toByteArray());
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
     * Set the attribute value.
     */
    public void set(String name, Object obj) throws IOException {
	if (name.equalsIgnoreCase(MAP)) {
	    if (!(obj instanceof Vector)) {
	      throw new IOException("Attribute value should be of" +
                                    " type Vector.");
	    }
	    maps = (Vector)obj;
	} else {
	  throw new IOException("Attribute name not recognized by " +
			"CertAttrSet:PolicyMappingsExtension.");
	}
        encodeThis();
    }

    /**
     * Get the attribute value.
     */
    public Object get(String name) throws IOException {
	if (name.equalsIgnoreCase(MAP)) {
	    return (maps);
	} else {
	  throw new IOException("Attribute name not recognized by " +
			"CertAttrSet:PolicyMappingsExtension.");
	}
    }

    /**
     * Delete the attribute value.
     */
    public void delete(String name) throws IOException {
	if (name.equalsIgnoreCase(MAP)) {
	    maps = null;
	} else {
	  throw new IOException("Attribute name not recognized by " +
			"CertAttrSet:PolicyMappingsExtension.");
	}
        encodeThis();
    }

    /**
     * Return an enumeration of names of attributes existing within this
     * attribute.
     */
    public Enumeration getElements () {
        AttributeNameEnumeration elements = new AttributeNameEnumeration();
        elements.addElement(MAP);

	return (elements.elements());
    }

    /**
     * Return the name of this attribute.
     */
    public String getName () {
        return (NAME);
    }
}
