/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)KRBError.java	1.10 03/06/24
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

import sun.security.krb5.internal.util.KrbBitArray;
import sun.security.krb5.PrincipalName;
import sun.security.krb5.Checksum;
import sun.security.krb5.Asn1Exception;
import sun.security.krb5.Realm;
import sun.security.krb5.RealmException;
import sun.security.util.*;
import java.util.Vector;
import java.io.IOException;
import java.math.BigInteger;
/**
 * Implements the ASN.1 KRBError type.
 *
 *  KRB-ERROR ::=   [APPLICATION 30] SEQUENCE {
 *                  pvno[0]               INTEGER,
 *                  msg-type[1]           INTEGER,
 *                  cTime[2]              KerberosTime OPTIONAL,
 *                  cuSec[3]              INTEGER OPTIONAL,
 *                  sTime[4]              KerberosTime,
 *                  suSec[5]              INTEGER,
 *                  error-code[6]         INTEGER,
 *                  crealm[7]             Realm OPTIONAL,
 *                  cname[8]              PrincipalName OPTIONAL,
 *                  realm[9]              Realm, -- Correct realm
 *                  sname[10]             PrincipalName, -- Correct name
 *                  e-text[11]            GeneralString OPTIONAL,
 *                  e-data[12]            OCTET STRING OPTIONAL
 *  }
 * </xmp>
 *
 * <p>
 * This definition reflects the Network Working Group RFC 1510
 * specifications dated September 1993 and available at
 * <a href="http://www.ietf.org/rfc/rfc1510.txt">http://www.ietf.org/rfc/rfc1510.txt</a>.
 */
public class KRBError {
    public int pvno;
    public int msgType;
    public KerberosTime cTime; //optional
    public Integer cuSec; //optional
    public KerberosTime sTime;
    public Integer suSec;
    public int errorCode;
    public Realm crealm; //optional
    public PrincipalName cname; //optional
    public Realm realm;
    public PrincipalName sname;
    public String eText; //optional
    public byte[] eData; //optional
    public Checksum eCksum; //optional

    private boolean DEBUG = Krb5.DEBUG;

    public KRBError(
		    APOptions new_apOptions,
		    KerberosTime new_cTime,
		    Integer new_cuSec,
		    KerberosTime new_sTime,
		    Integer new_suSec,
		    int new_errorCode,
		    Realm new_crealm,
		    PrincipalName new_cname,
		    Realm new_realm,
		    PrincipalName new_sname,
		    String new_eText,
		    byte[] new_eData
			) {
	pvno = Krb5.PVNO;
	msgType = Krb5.KRB_ERROR;
	cTime = new_cTime;
	cuSec = new_cuSec;
	sTime = new_sTime;
	suSec = new_suSec;
	errorCode = new_errorCode;
	crealm =  new_crealm;
	cname = new_cname;
	realm = new_realm;
	sname = new_sname;
	eText = new_eText;
	eData = new_eData;
    }

    public KRBError(
		    APOptions new_apOptions,
		    KerberosTime new_cTime,
		    Integer new_cuSec,
		    KerberosTime new_sTime,
		    Integer new_suSec,
		    int new_errorCode,
		    Realm new_crealm,
		    PrincipalName new_cname,
		    Realm new_realm,
		    PrincipalName new_sname,
		    String new_eText,
		    byte[] new_eData,
		    Checksum new_eCksum
			) {
	pvno = Krb5.PVNO;
	msgType = Krb5.KRB_ERROR;
	cTime = new_cTime;
	cuSec = new_cuSec;
	sTime = new_sTime;
	suSec = new_suSec;
	errorCode = new_errorCode;
	crealm =  new_crealm;
	cname = new_cname;
	realm = new_realm;
	sname = new_sname;
	eText = new_eText;
	eData = new_eData;
	eCksum = new_eCksum;
    }

    public KRBError(byte[] data) throws Asn1Exception,
    RealmException, KrbApErrException, IOException {
	init(new DerValue(data));
    }

    public KRBError(DerValue encoding) throws Asn1Exception,
    RealmException, KrbApErrException, IOException {
	init(encoding);	

	if (DEBUG) {
            System.out.println(">>>KRBError:");
            if (cTime != null)
                System.out.println("\t cTime is " + cTime.toDate().toString() + " " + cTime.toDate().getTime());
            if (cuSec != null) {
                System.out.println("\t cuSec is " + cuSec.intValue());
            }

            System.out.println("\t sTime is " + sTime.toDate().toString
			       () + " " + sTime.toDate().getTime());
            System.out.println("\t suSec is " + suSec);
            System.out.println("\t error code is " + errorCode);
            System.out.println("\t error Message is " + (String)Krb5.errMsgList.get(new Integer(errorCode)));
            if (crealm != null) {
                System.out.println("\t crealm is " + crealm.toString());
            }
            if (cname != null) {
                System.out.println("\t cname is " + cname.toString());
            }
            if (realm != null) {
                System.out.println("\t realm is " + realm.toString());
            }
            if (sname != null) {
                System.out.println("\t sname is " + sname.toString());
            }
            if (eText != null) {
                System.out.println("\t etext is " + eText);
            }
            if (eData != null) {
                System.out.println("\t eData provided.");
                if (eCksum != null) {
                    System.out.println("\t checksum provided.");
                }
            }
        }
    }

    public final KerberosTime getServerTime() {
	return sTime;
    }

    public final KerberosTime getClientTime() {
	return cTime;
    }

    public final Integer getServerMicroSeconds() {
	return suSec;
    }

    public final Integer getClientMicroSeconds() {
	return cuSec;
    }


    public final int getErrorCode() {
	return errorCode;
    }

    public final String getErrorString() {
	return eText;
    }

    /**
     * Initializes a KRBError object.
     * @param encoding a DER-encoded data.
     * @exception Asn1Exception if an error occurs while decoding an ASN1 encoded data.
     * @exception IOException if an I/O error occurs while reading encoded data.
     * @exception KrbApErrException if the value read from the DER-encoded data 
     *  stream does not match the pre-defined value.
     * @exception RealmException if an error occurs while parsing a Realm object.
     */
    private void init(DerValue encoding) throws Asn1Exception,
    RealmException, KrbApErrException, IOException {
        DerValue der, subDer;		
        if (((encoding.getTag() & (byte)0x1F) != (byte)0x1E)
	    || (encoding.isApplication() != true)
	    || (encoding.isConstructed() != true))
	    throw new Asn1Exception(Krb5.ASN1_BAD_ID);
        der = encoding.getData().getDerValue();
        if (der.getTag() != DerValue.tag_Sequence)
            throw new Asn1Exception(Krb5.ASN1_BAD_ID);
        subDer = der.getData().getDerValue();
	if ((subDer.getTag() & (byte)0x1F) == (byte)0x00) {
	    pvno = subDer.getData().getBigInteger().intValue();
            if (pvno != Krb5.PVNO)
		throw new KrbApErrException(Krb5.KRB_AP_ERR_BADVERSION);
	}
        else
	    throw new Asn1Exception(Krb5.ASN1_BAD_ID);
        subDer = der.getData().getDerValue();
	if ((subDer.getTag() & (byte)0x1F) == (byte)0x01) {
	    msgType = subDer.getData().getBigInteger().intValue();
	    if (msgType != Krb5.KRB_ERROR)
		throw new KrbApErrException(Krb5.KRB_AP_ERR_MSG_TYPE);
	}
        else
	    throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	cTime = KerberosTime.parse(der.getData(), (byte)0x02, true);
        if ((der.getData().peekByte() & 0x1F) == 0x03) {
	    subDer = der.getData().getDerValue();
	    cuSec = new Integer(subDer.getData().getBigInteger().intValue());
	}
	else cuSec = null;
	sTime = KerberosTime.parse(der.getData(), (byte)0x04, false);
        subDer = der.getData().getDerValue();
	if ((subDer.getTag() & (byte)0x1F) == (byte)0x05) {
	    suSec = new Integer (subDer.getData().getBigInteger().intValue());
	}
	else  throw new Asn1Exception(Krb5.ASN1_BAD_ID); 
        subDer = der.getData().getDerValue();
	if ((subDer.getTag() & (byte)0x1F) == (byte)0x06) {
	    errorCode = subDer.getData().getBigInteger().intValue();
	}
	else  throw new Asn1Exception(Krb5.ASN1_BAD_ID); 
	crealm = Realm.parse(der.getData(), (byte)0x07, true);
	cname = PrincipalName.parse(der.getData(), (byte)0x08, true);
	realm = Realm.parse(der.getData(), (byte)0x09, false);
	sname = PrincipalName.parse(der.getData(), (byte)0x0A, false);
	eText = null;
	eData = null;
	eCksum = null;
	if (der.getData().available() >0) {
	    if ((der.getData().peekByte() & 0x1F) == 0x0B) {
                subDer = der.getData().getDerValue();
		eText = subDer.getData().getGeneralString();				
	    }
	}
        if (der.getData().available() >0) {
	    if ((der.getData().peekByte() & 0x1F) == 0x0C) {
                subDer = der.getData().getDerValue();
		eData = subDer.getData().getOctetString();				
	    }
	}
        if (der.getData().available() >0) {
	    eCksum = Checksum.parse(der.getData(), (byte)0x0D, true);		
	}
        if (der.getData().available() >0)
	    throw new Asn1Exception(Krb5.ASN1_BAD_ID);
    }


    /**
     * Encodes an KRBError object.
     * @return the byte array of encoded KRBError object.
     * @exception Asn1Exception if an error occurs while decoding an ASN1 encoded data.
     * @exception IOException if an I/O error occurs while reading encoded data.
     */
    public byte[] asn1Encode() throws Asn1Exception, IOException {
        DerOutputStream temp = new DerOutputStream();
        DerOutputStream bytes = new DerOutputStream();

	temp.putInteger(BigInteger.valueOf(pvno));
        bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x00), temp);
        temp = new DerOutputStream();
        temp.putInteger(BigInteger.valueOf(msgType));
        bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x01), temp);
	if (cTime != null)
	bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x02), cTime.asn1Encode());
			
	if (cuSec != null) {
            temp = new DerOutputStream();
	    temp.putInteger(BigInteger.valueOf(cuSec.intValue()));
            bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x03), temp);
	}
        bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x04), sTime.asn1Encode());
        temp = new DerOutputStream();
        temp.putInteger(BigInteger.valueOf(suSec.intValue()));
        bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x05), temp);
	temp = new DerOutputStream();
	temp.putInteger(BigInteger.valueOf(errorCode));		
	bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x06), temp);
		
	if (crealm != null)
	bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x07), crealm.asn1Encode());
	if (cname != null)
	bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x08), cname.asn1Encode());
        bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x09), realm.asn1Encode());
        bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x0A), sname.asn1Encode());
	if (eText != null) {
            temp = new DerOutputStream();
	    temp.putGeneralString(eText);
            bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x0B), temp);
	}
	if (eData != null) {
            temp = new DerOutputStream();
	    temp.putOctetString(eData);
            bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x0C), temp);
			
	}
	if (eCksum != null)
	bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x0D), eCksum.asn1Encode());
        temp = new DerOutputStream();
	temp.write(DerValue.tag_Sequence, bytes);
        bytes = new DerOutputStream();
	bytes.write(DerValue.createTag(DerValue.TAG_APPLICATION, true, (byte)0x1E), temp);
	return bytes.toByteArray();
    }

}
