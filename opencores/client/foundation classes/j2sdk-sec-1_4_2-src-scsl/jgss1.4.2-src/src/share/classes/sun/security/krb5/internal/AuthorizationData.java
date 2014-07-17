/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)AuthorizationData.java	1.9 03/06/24
 *
 * Portions Copyright 2002 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 * 
 * ===========================================================================
 *  IBM Confidential
 *  OCO Source Materials
 *  Licensed Materials - Property of IBM
 * 
 *  (C) Copyright IBM Corp. 1999 All Rights Reserved.
 * 
 *  The source code for this program is not published or otherwise divested of
 *  its trade secrets, irrespective of what has been deposited with the U.S.
 *  Copyright Office.
 * 
 *  Copyright 1997 The Open Group Research Institute.  All rights reserved.
 * ===========================================================================
 * 
 */

package sun.security.krb5.internal;

import sun.security.util.*;
import sun.security.krb5.Asn1Exception; 
import java.util.Vector;
import java.io.IOException;
import sun.security.krb5.internal.ccache.CCacheOutputStream;

/**
 * In RFC1510, the ASN.1 AuthorizationData is defined as:
 * AuthorizationData ::=   SEQUENCE OF SEQUENCE {
 *                          ad-type[0]  INTEGER,
 *                          ad-data[1]  OCTET STRING
 *                         }
 * Here, two classes are used to implement it and they can be represented as follows:
 * AuthorizationData ::= SEQUENCE OF AuthorizationDataEntry
 * AuthorizationDataEntry ::= SEQUENCE { 
 *  							ad-type[0]  INTEGER,
 *                          	ad-data[1]  OCTET STRING
 *							 }
 */
public class AuthorizationData implements Cloneable {
	private AuthorizationDataEntry[] entry = null;

	private AuthorizationData() {
	}

	public AuthorizationData(
		AuthorizationDataEntry[] new_entries
	) throws IOException {
		if (new_entries != null) {
		   entry = new AuthorizationDataEntry[new_entries.length];
		   for (int i = 0; i < new_entries.length; i++) {
			if (new_entries[i] == null) {
			   throw new IOException("Cannot create an AuthorizationData");
			} else {
			   entry[i] = (AuthorizationDataEntry)new_entries[i].clone();
			}
		   }
		}
	}

	public AuthorizationData(
		AuthorizationDataEntry new_entry
	) {
		entry = new AuthorizationDataEntry[1];
		entry[0] = new_entry;
	}

	public Object clone() {
		AuthorizationData new_authorizationData =
			new AuthorizationData();
		if (entry != null) {
			new_authorizationData.entry =
				new AuthorizationDataEntry[entry.length];
			for (int i = 0; i < entry.length; i++)
				new_authorizationData.entry[i] =
					(AuthorizationDataEntry)entry[i].clone();
		}
		return new_authorizationData;
	}

	/**
	 * Constructs a new <code>AuthorizationData,</code> instance.
	 * @param der a single DER-encoded value.
	 * @exception Asn1Exception if an error occurs while decoding an ASN1 encoded data.
	 * @exception IOException if an I/O error occurs while reading encoded data.
	 */
	public AuthorizationData(DerValue der) throws Asn1Exception, IOException {
		Vector v = new Vector();
		if (der.getTag() != DerValue.tag_Sequence) {
			throw new Asn1Exception(Krb5.ASN1_BAD_ID);
		}
		while (der.getData().available() > 0) {
			v.addElement(new AuthorizationDataEntry(der.getData().getDerValue()));
		}
		if (v.size() > 0) {
			entry = new AuthorizationDataEntry[v.size()];
			v.copyInto(entry);
		}
	}

	/**
	 * Encodes an <code>AuthorizationData</code> object.
	 * @return byte array of encoded <code>AuthorizationData</code> object.
	 * @exception Asn1Exception if an error occurs while decoding an ASN1 encoded data.
	 * @exception IOException if an I/O error occurs while reading encoded data.
	 */
	public byte[] asn1Encode() throws Asn1Exception, IOException {
		DerOutputStream bytes = new DerOutputStream();
		DerValue der[] = new DerValue[entry.length];
		for (int i = 0; i < entry.length; i++) {
			der[i] = new DerValue(entry[i].asn1Encode());
		}
		bytes.putSequence(der);
		return bytes.toByteArray();
	}

    /**
     * Parse (unmarshal) an <code>AuthorizationData</code> object from a DER input stream.  
     * This form of parsing might be used when expanding a value which is part of
     * a constructed sequence and uses explicitly tagged type.
     *
     * @exception Asn1Exception if an error occurs while decoding an ASN1 encoded data.
     * @exception IOException if an I/O error occurs while reading encoded data.
     * @param data the Der input stream value, which contains one or more marshaled value.
     * @param explicitTag tag number.
     * @param optional indicates if this data field is optional
     * @return an instance of AuthorizationData.
     *
     */
	public static AuthorizationData parse(DerInputStream data, byte explicitTag, boolean optional) throws Asn1Exception, IOException{  
		if ((optional) && (((byte)data.peekByte() & (byte)0x1F) != explicitTag)) {		
			return null;
		}
		DerValue der = data.getDerValue();
		if (explicitTag != (der.getTag() & (byte)0x1F))  {	
			throw new Asn1Exception(Krb5.ASN1_BAD_ID);
		}
		else {
			DerValue subDer = der.getData().getDerValue();
			return new AuthorizationData(subDer);
		}
	}
	
	/**
	 * Writes <code>AuthorizationData</code> data fields to a output stream.
	 *
	 * @param cos a <code>CCacheOutputStream</code> to be written to.
	 * @exception IOException if an I/O exception occurs.
	 */
   	public void writeAuth(CCacheOutputStream cos) throws IOException {
		for (int i = 0; i < entry.length; i++) {
			entry[i].writeEntry(cos);
		}
	}

    public String toString() {
	String retVal = "AuthorizationData:\n";
	for (int i = 0; i < entry.length; i++) {
	    retVal += entry[i].toString();
	}
	return retVal;
    }
}

