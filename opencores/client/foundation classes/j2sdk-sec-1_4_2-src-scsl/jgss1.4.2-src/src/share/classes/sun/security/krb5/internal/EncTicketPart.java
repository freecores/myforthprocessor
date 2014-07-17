/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)EncTicketPart.java	1.7 03/06/24
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

import sun.security.krb5.*;
import sun.security.util.*;
import java.util.Vector;
import java.io.IOException;
import java.io.*;

/**
 * Implements the ASN.1 EncTicketPart type.
 *
 * <xmp>
 *EncTicketPart ::= [APPLICATION 3] SEQUENCE {
 *                     flags[0]             TicketFlags,
 *                     key[1]               EncryptionKey,
 *                     crealm[2]            Realm,
 *                     cname[3]             PrincipalName,
 *                     transited[4]         TransitedEncoding,
 *                     authtime[5]          KerberosTime,
 *                     starttime[6]         KerberosTime OPTIONAL,
 *                     endtime[7]           KerberosTime,
 *                     renew-till[8]        KerberosTime OPTIONAL,
 *                     caddr[9]             HostAddresses OPTIONAL,
 *                     authorization-data[10]   AuthorizationData OPTIONAL
 *}
 * </xmp>
 * <p>
 * This definition reflects the Network Working Group RFC 1510
 * specifications dated September 1993 and available at
 * <a href="http://www.ietf.org/rfc/rfc1510.txt">http://www.ietf.org/rfc/rfc1510.txt</a>.
 */
public class EncTicketPart {
	public TicketFlags flags;
	public EncryptionKey key;
	public Realm crealm;
	public PrincipalName cname;
	public TransitedEncoding transited;
	public KerberosTime authtime;
	public KerberosTime starttime; //optional
	public KerberosTime endtime;
	public KerberosTime renewTill; //optional
	public HostAddresses caddr; //optional
	public AuthorizationData authorizationData; //optional

	public EncTicketPart(
		TicketFlags new_flags,
		EncryptionKey new_key,
		Realm new_crealm,
		PrincipalName new_cname,
		TransitedEncoding new_transited,
		KerberosTime new_authtime,
		KerberosTime new_starttime,
		KerberosTime new_endtime,
		KerberosTime new_renewTill,
		HostAddresses new_caddr,
		AuthorizationData new_authorizationData
	) {
		flags = new_flags;
		key = new_key;
		crealm = new_crealm;
		cname = new_cname;
		transited = new_transited;
		authtime = new_authtime;
		starttime = new_starttime;
		endtime = new_endtime;
		renewTill = new_renewTill;
		caddr = new_caddr;
		authorizationData = new_authorizationData;
	}

	public EncTicketPart(byte[] data)
		throws Asn1Exception, KrbException, IOException {
		init(new DerValue(data));
	}

	public EncTicketPart(DerValue encoding)
		throws Asn1Exception, KrbException, IOException {
		init(encoding);
	}

   /**
    * Initializes an EncTicketPart object.
    * @param encoding a single DER-encoded value.
    * @exception Asn1Exception if an error occurs while decoding an ASN1 encoded data.
    * @exception IOException if an I/O error occurs while reading encoded data.
    * @exception RealmException if an error occurs while parsing a Realm object.
    */

    private static String getHexBytes(byte[] bytes, int len) 
	throws IOException {
	
	StringBuffer sb = new StringBuffer();
	for (int i = 0; i < len; i++) {
	    
	    int b1 = (bytes[i]>>4) & 0x0f;
	    int b2 = bytes[i] & 0x0f;
	    
	    sb.append(Integer.toHexString(b1));
	    sb.append(Integer.toHexString(b2));
	    sb.append(' ');
	}
	return sb.toString();
    }

	private void init(DerValue encoding)
		throws Asn1Exception, IOException, RealmException {
	    DerValue der, subDer;

	    renewTill = null;
	    caddr = null;
	    authorizationData = null;
	    if (((encoding.getTag() & (byte)0x1F) != (byte)0x03)
		|| (encoding.isApplication() != true)
		|| (encoding.isConstructed() != true))
		throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	    der = encoding.getData().getDerValue();
	    if (der.getTag() != DerValue.tag_Sequence)
		throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	    flags = TicketFlags.parse(der.getData(), (byte)0x00, false);
	    key = EncryptionKey.parse(der.getData(), (byte)0x01, false);
	    crealm = Realm.parse(der.getData(), (byte)0x02, false);
	    cname = PrincipalName.parse(der.getData(), (byte)0x03, false);
	    transited = TransitedEncoding.parse(der.getData(), (byte)0x04, false);
	    authtime = KerberosTime.parse(der.getData(), (byte)0x05, false);
	    starttime = KerberosTime.parse(der.getData(), (byte)0x06, true);
	    endtime = KerberosTime.parse(der.getData(), (byte)0x07, false);
	    if (der.getData().available() > 0) {
		renewTill = KerberosTime.parse(der.getData(), (byte)0x08, true);
	    }
	    if (der.getData().available() > 0) {
		caddr = HostAddresses.parse(der.getData(), (byte)0x09, true);
	    }		
	    if (der.getData().available() > 0) {
		authorizationData = AuthorizationData.parse(der.getData(), (byte)0x0A, true);
	    }
	    if (der.getData().available() > 0) 
		throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	    
	}
    
       /**
        * Encodes an EncTicketPart object.
        * @return byte array of encoded EncTicketPart object.
        * @exception Asn1Exception if an error occurs while decoding an ASN1 encoded data.
        * @exception IOException if an I/O error occurs while reading encoded data.
        */

	public byte[] asn1Encode() throws Asn1Exception, IOException {
        DerOutputStream bytes = new DerOutputStream();
	    DerOutputStream temp = new DerOutputStream();
		bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x00), flags.asn1Encode());
        bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x01), key.asn1Encode());
        bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x02), crealm.asn1Encode());
        bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x03), cname.asn1Encode());
        bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x04), transited.asn1Encode());
        bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x05), authtime.asn1Encode());
		if (starttime != null)
           bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x06), starttime.asn1Encode());
        bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x07), endtime.asn1Encode());

		if (renewTill != null)
            bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x08), renewTill.asn1Encode());

		if (caddr != null)
            bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x09), caddr.asn1Encode());

		if (authorizationData != null)
            bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x0A), authorizationData.asn1Encode());
		temp.write(DerValue.tag_Sequence, bytes);
		bytes = new DerOutputStream();
		bytes.write(DerValue.createTag(DerValue.TAG_APPLICATION, true, (byte)0x03), temp);
		return bytes.toByteArray();
	}
}
