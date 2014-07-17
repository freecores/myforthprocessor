/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)TicketFlags.java	1.8 03/06/24
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

import sun.security.krb5.Asn1Exception;
import sun.security.krb5.internal.util.KrbBitArray;
import sun.security.util.*;
import java.io.IOException;

/**
 * Implements the ASN.1TicketFlags type.
 *
 *    TicketFlags ::= BIT STRING   
 *                  {
 *                   reserved(0),
 *                   forwardable(1),
 *                   forwarded(2),
 *                   proxiable(3),
 *                   proxy(4),
 *                   may-postdate(5),
 *                   postdated(6),
 *                   invalid(7),
 *                   renewable(8),
 *                   initial(9),
 *                   pre-authent(10),
 *                   hw-authent(11)
 *                  }
 */
public class TicketFlags extends KrbBitArray {
    public TicketFlags() {
	super(Krb5.TKT_OPTS_MAX + 1);
    }
	
    public TicketFlags (boolean[] flags) throws Asn1Exception {
	super(flags);
        if (flags.length > Krb5.TKT_OPTS_MAX + 1) {
	    throw new Asn1Exception(Krb5.BITSTRING_BAD_LENGTH);
	}
    }   

    public TicketFlags(int size, byte[] data) throws Asn1Exception {
	super(size, data);
	if ((size > data.length * 8) || (size > Krb5.TKT_OPTS_MAX + 1))
	    throw new Asn1Exception(Krb5.BITSTRING_BAD_LENGTH);
    }

    public TicketFlags(DerValue encoding) throws IOException, Asn1Exception {
	this(encoding.getUnalignedBitString(true).toBooleanArray());
    }
	
    /**
     * Parse (unmarshal) a ticket flag from a DER input stream.  This form
     * parsing might be used when expanding a value which is part of
     * a constructed sequence and uses explicitly tagged type.
     *
     * @exception Asn1Exception on error.
     * @param data the Der input stream value, which contains one or more marshaled value.
     * @param explicitTag tag number.
     * @param optional indicate if this data field is optional
     * @return an instance of TicketFlags.
     *
     */
    public static TicketFlags parse(DerInputStream data, byte explicitTag, boolean optional) throws Asn1Exception, IOException {
	if ((optional) && (((byte)data.peekByte() & (byte)0x1F) != explicitTag)) 
	    return null;		
	DerValue der = data.getDerValue();
	if (explicitTag != (der.getTag() & (byte)0x1F))  {	
	    throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	}
	else {
	    DerValue subDer = der.getData().getDerValue();
	    return new TicketFlags(subDer);
	}
    }
    
    /**
     * Encodes a TicketFlags object.
     * @return an byte array of encoded TicketFlags object.
     * @exception IOException if an I/O error occurs while reading encoded data.
     */
    public byte[] asn1Encode() throws IOException {	
	DerOutputStream out = new DerOutputStream();
	out.putUnalignedBitString(new BitArray(this.toBooleanArray()));
	return out.toByteArray();
    }


    public Object clone() {
	try {
	    return new TicketFlags(this.toBooleanArray());
	}
	catch (Exception e) {
	    return null;
	}
    }

    public boolean match(LoginOptions options) {
        boolean matched = false;
        //We currently only consider if forwardable renewable and proxiable are match
        if (this.get(Krb5.TKT_OPTS_FORWARDABLE) == (options.get(KDCOptions.FORWARDABLE))) {
            if (this.get(Krb5.TKT_OPTS_PROXIABLE) == (options.get(KDCOptions.PROXIABLE))) {
                if (this.get(Krb5.TKT_OPTS_RENEWABLE) == (options.get(KDCOptions.RENEWABLE))) {
                    matched = true;
                }
            }
        }
        return matched;
    }
    public boolean match(TicketFlags flags) {
        boolean matched = true;
        for (int i = 0; i < 12; i++) {
            if (this.get(i) != flags.get(i)) {
                return false;
            }
        }
        return matched;
    }


    /**
     * Returns the string representative of ticket flags.
     */
    public String toString() {
        StringBuffer sb = new StringBuffer();
        boolean[] flags = toBooleanArray();
        for (int i = 0; i < flags.length; i++) {
            if (flags[i] == true) {
                switch (i) {
                case 0:
                    sb.append("RESERVED;");
                    break;
                case 1:
                    sb.append("FORWARDABLE;");
                    break;
                case 2:
                    sb.append("FORWARDED;");
                    break;
                case 3:
                    sb.append("PROXIABLE;");
                    break;
                case 4:
                    sb.append("PROXY;");
                    break;
                case 5:
                    sb.append("MAY-POSTDATE;");
                    break;
                case 6:
                    sb.append("POSTDATED;");
                    break;
                case 7:
                    sb.append("INVALID;");
                    break;
                case 8:
                    sb.append("RENEWABLE;");
                    break;
                case 9:
                    sb.append("INITIAL;");
                    break;
                case 10:
                    sb.append("PRE-AUTHENT;");
                    break;
                case 11:
                    sb.append("HW-AUTHENT;");
                    break;
                }
            }
        }
        String result = sb.toString();
        if (result.length() > 0) {
            result = result.substring(0, result.length() - 1);
        }
        return result;
    }
}
