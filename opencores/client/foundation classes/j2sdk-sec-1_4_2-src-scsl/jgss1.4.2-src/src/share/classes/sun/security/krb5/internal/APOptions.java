/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)APOptions.java	1.7 03/06/24
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
 * Implements the ASN.1 APOptions type.
 *
 * <xmp>
 * APOptions ::= BIT STRING {
 * reserved(0),
 * use-session-key(1),
 * mutual-required(2)
 * }
 * </xmp>
 *
 * <p>
 * This definition reflects the Network Working Group RFC 1510
 * specifications dated September 1993 and available at
 * <a href="http://www.ietf.org/rfc/rfc1510.txt">http://www.ietf.org/rfc/rfc1510.txt</a>.
 */

  public class APOptions extends KrbBitArray {
	public APOptions() {
		super(Krb5.AP_OPTS_MAX + 1);
	} 

	public APOptions(int oneBit) throws Asn1Exception {
		super(Krb5.AP_OPTS_MAX + 1);
		set(oneBit, true);
	}
	public APOptions(int size, byte[] data) throws Asn1Exception {
		super(size, data);
		if ((size > data.length * 8) || (size > Krb5.AP_OPTS_MAX + 1)) {
			throw new Asn1Exception(Krb5.BITSTRING_BAD_LENGTH);
		}
	}

    public APOptions(boolean[] data) throws Asn1Exception {
		super(data);
		if (data.length > Krb5.AP_OPTS_MAX + 1) {
             throw new Asn1Exception(Krb5.BITSTRING_BAD_LENGTH);
		}
	}

	public APOptions(DerValue encoding) throws IOException, Asn1Exception {
	   this(encoding.getUnalignedBitString(true).toBooleanArray());
    }
    	
    /**
     * Parse (unmarshal) an APOptions from a DER input stream.  This form
     * parsing might be used when expanding a value which is part of
     * a constructed sequence and uses explicitly tagged type.
     *
     * @param data the Der input stream value, which contains one or more marshaled value.
     * @param explicitTag tag number.
     * @param optional indicate if this data field is optional.
     * @return an instance of APOptions.
     * @exception Asn1Exception if an error occurs while decoding an ASN1 encoded data.
     * @exception IOException if an I/O error occurs while reading encoded data.
     *
     */
	public static APOptions parse(DerInputStream data, byte explicitTag, boolean optional) throws Asn1Exception, IOException {
		if ((optional) && (((byte)data.peekByte() & (byte)0x1F) != explicitTag)) 
		   return null;		
		DerValue der = data.getDerValue();
		if (explicitTag != (der.getTag() & (byte)0x1F))  {	
			throw new Asn1Exception(Krb5.ASN1_BAD_ID);
		}
		else {
			DerValue subDer = der.getData().getDerValue();
			return new APOptions(subDer);
		}
	}
	
    /**
     * Write the extension to the DerOutputStream.
     *
     * @exception IOException if an I/O error occurs while reading encoded data.
     *
     */
	public byte[] asn1Encode() throws IOException {	
		DerOutputStream out = new DerOutputStream();
		out.putUnalignedBitString(new BitArray(this.toBooleanArray()));
		return out.toByteArray();
	}
}
