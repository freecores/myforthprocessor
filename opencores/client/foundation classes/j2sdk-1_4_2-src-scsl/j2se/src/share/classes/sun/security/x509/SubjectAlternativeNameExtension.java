/*
 * @(#)SubjectAlternativeNameExtension.java	1.20 03/01/23
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

import sun.security.util.*;

/**
 * This represents the Subject Alternative Name Extension.
 *
 * This extension, if present, allows the subject to specify multiple
 * alternative names.
 *
 * <p>Extensions are represented as a sequence of the extension identifier
 * (Object Identifier), a boolean flag stating whether the extension is to
 * be treated as being critical and the extension value itself (this is again
 * a DER encoding of the extension value).
 * <p>
 * The ASN.1 syntax for this is:
 * <pre>
 * SubjectAltName ::= GeneralNames
 * GeneralNames ::= SEQUENCE SIZE (1..MAX) OF GeneralName
 * </pre>
 * @author Amit Kapoor
 * @author Hemma Prafullchandra
 * @version 1.20
 * @see Extension
 * @see CertAttrSet
 */
public class SubjectAlternativeNameExtension extends Extension
implements CertAttrSet {
    /**
     * Identifier for this attribute, to be used with the
     * get, set, delete methods of Certificate, x509 type.
     */
    public static final String IDENT =
                         "x509.info.extensions.SubjectAlternativeName";
    /**
     * Attribute names.
     */
    public static final String NAME = "SubjectAlternativeName";
    public static final String SUBJECT_NAME = "subject_name";

    // private data members
    GeneralNames	names = null;

    // Encode this extension
    private void encodeThis() throws IOException {
        if (names == null || names.isEmpty()) {
            this.extensionValue = null;
            return;
        }
        DerOutputStream os = new DerOutputStream();
	names.encode(os);
        this.extensionValue = os.toByteArray();
    }

    /**
     * Create a SubjectAlternativeNameExtension with the passed GeneralNames.
     * The extension is marked non-critical.
     *
     * @param names the GeneralNames for the subject.
     * @exception IOException on error.
     */
    public SubjectAlternativeNameExtension(GeneralNames names)
    throws IOException {
	this(Boolean.FALSE, names);
    }

    /**
     * Create a SubjectAlternativeNameExtension with the specified
     * criticality and GeneralNames.
     *
     * @param critical true if the extension is to be treated as critical.
     * @param names the GeneralNames for the subject.
     * @exception IOException on error.
     */
    public SubjectAlternativeNameExtension(Boolean critical, GeneralNames names)
    throws IOException {
        this.names = names;
        this.extensionId = PKIXExtensions.SubjectAlternativeName_Id;
        this.critical = critical.booleanValue();
        encodeThis();
    }

    /**
     * Create a default SubjectAlternativeNameExtension. The extension
     * is marked non-critical.
     */
    public SubjectAlternativeNameExtension() {
        extensionId = PKIXExtensions.SubjectAlternativeName_Id;
        critical = false;
        names = new GeneralNames();
    }

    /**
     * Create the extension from the passed DER encoded value.
     *
     * @param critical true if the extension is to be treated as critical.
     * @param value Array of DER encoded bytes of the actual value.
     * @exception IOException on error.
     */
    public SubjectAlternativeNameExtension(Boolean critical, Object value)
    throws IOException {
        this.extensionId = PKIXExtensions.SubjectAlternativeName_Id;
        this.critical = critical.booleanValue();

        if (!(value instanceof byte[]))
            throw new IOException("SubjectAlternativeName: "
                                  + "Illegal argument type");

        int len = Array.getLength(value);
	byte[] extValue = new byte[len];
        System.arraycopy(value, 0, extValue, 0, len);

        this.extensionValue = extValue;
        DerValue val = new DerValue(extValue);
	if (val.data == null) {
	    names = new GeneralNames();
	    return;
	}

	names = new GeneralNames(val);
    }

    /**
     * Returns a printable representation of the SubjectAlternativeName.
     */
    public String toString() {
         return super.toString() + "SubjectAlternativeName [\n" 
             + String.valueOf(names) + "]\n";
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
     * Write the extension to the OutputStream.
     *
     * @param out the OutputStream to write the extension to.
     * @exception IOException on encoding errors.
     */
    public void encode(OutputStream out) throws IOException {
        DerOutputStream tmp = new DerOutputStream();
        if (extensionValue == null) {
	    extensionId = PKIXExtensions.SubjectAlternativeName_Id;
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
	if (name.equalsIgnoreCase(SUBJECT_NAME)) {
	    if (!(obj instanceof GeneralNames)) {
	      throw new IOException("Attribute value should be of " +
                                    "type GeneralNames.");
	    }
	    names = (GeneralNames)obj;
	} else {
	  throw new IOException("Attribute name not recognized by " +
			"CertAttrSet:SubjectAlternativeName.");
	}
        encodeThis();
    }

    /**
     * Get the attribute value.
     */
    public Object get(String name) throws IOException {
	if (name.equalsIgnoreCase(SUBJECT_NAME)) {
	    return (names);
	} else {
	  throw new IOException("Attribute name not recognized by " +
			"CertAttrSet:SubjectAlternativeName.");
	}
    }

    /**
     * Delete the attribute value.
     */
    public void delete(String name) throws IOException {
	if (name.equalsIgnoreCase(SUBJECT_NAME)) {
	    names = null;
	} else {
	  throw new IOException("Attribute name not recognized by " +
			"CertAttrSet:SubjectAlternativeName.");
	}
        encodeThis();
    }

    /**
     * Return an enumeration of names of attributes existing within this
     * attribute.
     */
    public Enumeration getElements() {
        AttributeNameEnumeration elements = new AttributeNameEnumeration();
        elements.addElement(SUBJECT_NAME);

	return (elements.elements());
    }

    /**
     * Return the name of this attribute.
     */
    public String getName() {
        return (NAME);
    }
}
