/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)Ticket.java	1.8 03/06/24
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

import sun.security.krb5.PrincipalName;
import sun.security.krb5.EncryptedData;
import sun.security.krb5.Asn1Exception; 
import sun.security.krb5.Realm;
import sun.security.krb5.RealmException;
import sun.security.util.*;
import java.io.IOException;
import java.math.BigInteger;
/**
 * Implements the ASN.1 Ticket type.
 *
 * <xmp>
 * Ticket ::= [APPLICATION 1] SEQUENCE {
 *	tkt-vno[0] INTEGER,
 *	realm[1] Realm,
 *  sname[2] PrincipalName,
 *  enc-part[3] EncryptedData
 * }
 * </xmp>
 *
 * <p>
 * This definition reflects the Network Working Group RFC 1510
 * specifications dated September 1993 and available at
 * <a href="http://www.ietf.org/rfc/rfc1510.txt">http://www.ietf.org/rfc/rfc1510.txt</a>.
 */
public class Ticket implements Cloneable {
    public int tkt_vno;
    public Realm realm;
    public PrincipalName sname;
    public EncryptedData encPart;

    private Ticket() {
    }

    public Object clone() {
	Ticket new_ticket = new Ticket();
	new_ticket.realm = (Realm)realm.clone();
	new_ticket.sname = (PrincipalName)sname.clone();
	new_ticket.encPart = (EncryptedData)encPart.clone();
	new_ticket.tkt_vno = tkt_vno;
	return new_ticket;
    }

    public Ticket(
		  Realm new_realm,
		  PrincipalName new_sname,
		  EncryptedData new_encPart
		      ) {
	tkt_vno = Krb5.TICKET_VNO;
	realm = new_realm;
	sname = new_sname;
	encPart = new_encPart;
    }

    public Ticket(byte[] data) throws Asn1Exception,
    RealmException, KrbApErrException, IOException {
	init(new DerValue(data));
    }

    public Ticket(DerValue encoding) throws Asn1Exception,	  
    RealmException, KrbApErrException, IOException {
	init(encoding);
    }

    /**
     * Initializes a Ticket object.
     * @param encoding a single DER-encoded value.
     * @exception Asn1Exception if an error occurs while decoding an ASN1 encoded data.
     * @exception IOException if an I/O error occurs while reading encoded data.
     * @exception KrbApErrException if the value read from the DER-encoded data stream does not match the pre-defined value.
     * @exception RealmException if an error occurs while parsing a Realm object.
     */

    private void init(DerValue encoding) throws Asn1Exception,	 
    RealmException, KrbApErrException, IOException {
	DerValue der;
	DerValue subDer;
	if (((encoding.getTag() & (byte)0x1F) != Krb5.KRB_TKT)
	    || (encoding.isApplication() != true)
	    || (encoding.isConstructed() != true))
	    throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	der = encoding.getData().getDerValue();
	if (der.getTag() != DerValue.tag_Sequence)
            throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	subDer = der.getData().getDerValue();
	if ((subDer.getTag() & (byte)0x1F) != (byte)0x00)
	    throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	tkt_vno = subDer.getData().getBigInteger().intValue();
        if (tkt_vno != Krb5.TICKET_VNO)	
	    throw new KrbApErrException(Krb5.KRB_AP_ERR_BADVERSION);
	realm = Realm.parse(der.getData(), (byte)0x01, false);
	sname = PrincipalName.parse(der.getData(), (byte)0x02, false);
	encPart = EncryptedData.parse(der.getData(), (byte)0x03, false);
	if (der.getData().available() > 0) 
	    throw new Asn1Exception(Krb5.ASN1_BAD_ID);
    }
   	
    /**
     * Encodes a Ticket object.
     * @return byte array of encoded ticket object.
     * @exception Asn1Exception if an error occurs while decoding an ASN1 encoded data.
     * @exception IOException if an I/O error occurs while reading encoded data.
     */
    public byte[] asn1Encode() throws Asn1Exception, IOException {	
	DerOutputStream bytes = new DerOutputStream();
	DerOutputStream temp = new DerOutputStream();
	DerValue der[] = new DerValue[4];
	temp.putInteger(BigInteger.valueOf(tkt_vno));
	bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x00), temp);
	bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x01), realm.asn1Encode());
	bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x02), sname.asn1Encode());
	bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x03), encPart.asn1Encode());
	temp = new DerOutputStream();
	temp.write(DerValue.tag_Sequence, bytes);
	DerOutputStream ticket = new DerOutputStream();
	ticket.write(DerValue.createTag(DerValue.TAG_APPLICATION, true, (byte)0x01), temp);
	return ticket.toByteArray();
    }

    /**
     * Parse (unmarshal) a Ticket from a DER input stream.  This form
     * parsing might be used when expanding a value which is part of
     * a constructed sequence and uses explicitly tagged type.
     *
     * @exception Asn1Exception on error.
     * @param data the Der input stream value, which contains one or more marshaled value.
     * @param explicitTag tag number.
     * @param optional indicate if this data field is optional
     * @return an instance of Ticket.
     */
    public static Ticket parse(DerInputStream data, byte explicitTag, boolean optional) throws Asn1Exception, IOException, RealmException, KrbApErrException {
	if ((optional) && (((byte)data.peekByte() & (byte)0x1F)!= explicitTag)) 
	    return null;
	DerValue der = data.getDerValue();
	if (explicitTag != (der.getTag() & (byte)0x1F))  {	
	    throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	}
	else {
	    DerValue subDer = der.getData().getDerValue();
	    return new Ticket(subDer);
	}
    }


}
