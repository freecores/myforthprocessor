/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)KDCOptions.java	1.7 03/06/24
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

import sun.security.krb5.Config;
import sun.security.krb5.KrbException;
import sun.security.krb5.Asn1Exception;
import sun.security.krb5.internal.util.KrbBitArray;   
import sun.security.util.*;   
import java.io.IOException;

/**
 * Implements the ASN.1 KDCOptions type.
 *
 * <xmp>
 * KDCOptions ::= BIT STRING {
 *	reserved(0),
 *	forwardable(1),
 *  forwarded(2),
 *  proxiable(3),
 *  proxy(4),
 *  allow-postdate(5),
 *  postdated(6),
 *  unused7(7),
 *  renewable(8),
 *  unused9(9),
 *	unused10(10),
 *	unused11(11),
 *	renewable-ok(27),
 *	enc-tkt-in-skey(28),
 * 	renew(30),
 *	validate(31)
 * }
 * </xmp>
 *
 * <p>
 * This definition reflects the Network Working Group RFC 1510
 * specifications dated September 1993 and available at
 * <a href="http://www.ietf.org/rfc/rfc1510.txt">http://www.ietf.org/rfc/rfc1510.txt</a>.
 * <p>
 * This class appears as data field in the initial request(KRB_AS_REQ) or subsequent request
 * (KRB_TGS_REQ)
 * to the KDC and indicates the flags that the client wants to set on the tickets.
 * The optional bits are:
 * <UL>
 *  <LI>KDCOptions.RESERVED
 *  <LI>KDCOptions.FORWARDABLE    
 *  <LI>KDCOptions.FORWARDED      
 *  <LI>KDCOptions.PROXIABLE       
 *  <LI>KDCOptions.PROXY           
 *  <LI>KDCOptions.ALLOW_POSTDATE  
 *  <LI>KDCOptions.POSTDATED              
 *  <LI>KDCOptions.RENEWABLE                     
 *  <LI>KDCOptions.RENEWABLE_OK    
 *  <LI>KDCOptions.ENC_TKT_IN_SKEY 
 *  <LI>KDCOptions.RENEW           
 *  <LI>KDCOptions.VALIDATE                     
 *  </UL>
 * <p>Various checks must be made before honoring an option. The restrictions on the use
 * of some options are as follows:
 * <ol>
 * <li> FORWARDABLE, FORWARDED, PROXIABLE, RENEWABLE options may be set in subsequent request only if the ticket_granting ticket 
 * on which it is based has the same options (FORWARDABLE, FORWARDED, PROXIABLE, RENEWABLE)  set.
 * <li> ALLOW_POSTDATE may be set in subsequent request only if the ticket-granting ticket on which it is based also has
 * its MAY_POSTDATE flag set. 
 * <li> POSTDATED may be set in subsequent request only if the ticket-granting ticket on which it is based also has
 * its MAY_POSTDATE flag set. 
 * <li> RENEWABLE or RENEW may be set in subsequent request only if the ticket-granting ticket on which it is based also has
 * its RENEWABLE flag set. 
 * <li> POXY may be set in subsequent request only if the ticket-granting ticket on which it is based also has
 * its PROXIABLE flag set, and the address(es) of the host from which the resulting ticket is to be valid should be included in the addresses 
 * field of the request.
 * <li>FORWARDED, PROXY, ENC_TKT_IN_SKEY, RENEW, VALIDATE are used only in subsequent requests. 
 * </ol><p>
 */
public class KDCOptions extends KrbBitArray {

    public final int KDC_OPT_PROXIABLE = 0x10000000;
    public final int KDC_OPT_RENEWABLE_OK = 0x00000010;
    public final int KDC_OPT_FORWARDABLE = 0x40000000;


    //KDC Options
    
    public static final int RESERVED        = 0;
    public static final int FORWARDABLE     = 1;
    public static final int FORWARDED       = 2;
    public static final int PROXIABLE       = 3;
    public static final int PROXY           = 4;
    public static final int ALLOW_POSTDATE  = 5;
    public static final int POSTDATED       = 6;
    public static final int UNUSED7         = 7;
    public static final int RENEWABLE       = 8;
    public static final int UNUSED9         = 9;
    public static final int UNUSED10        = 10;
    public static final int UNUSED11        = 11;
    public static final int RENEWABLE_OK    = 27;
    public static final int ENC_TKT_IN_SKEY = 28;
    public static final int RENEW           = 30;
    public static final int VALIDATE        = 31;
    public static final int MAX             = 31;
    
    private boolean DEBUG = Krb5.DEBUG;

    public KDCOptions() {
	super(KDCOptions.MAX + 1);
	setDefault();
    }
    
    public KDCOptions(int size, byte[] data) throws Asn1Exception {
	super(size, data);
	if ((size > data.length * 8) || (size > KDCOptions.MAX + 1))
	    throw new Asn1Exception(Krb5.BITSTRING_BAD_LENGTH);
    }
    
    /**
     * Constructs a KDCOptions from the specified bit settings.
     *
     * @param data the bits to be set for the KDCOptions.
     * @exception Asn1Exception if an error occurs while decoding an ASN1
     * encoded data.
     *
     */
    public KDCOptions(boolean[] data) throws Asn1Exception {
	super(data);
	if (data.length > KDCOptions.MAX + 1) {
	    throw new Asn1Exception(Krb5.BITSTRING_BAD_LENGTH);
	}
    }
    
    public KDCOptions(DerValue encoding) throws Asn1Exception, IOException {
	this(encoding.getUnalignedBitString(true).toBooleanArray());
    }
    
    /**
     * Constructs a KDCOptions from the passed bit settings.
     *
     * @param options the bits to be set for the KDCOptions.
     *
     */
    public KDCOptions(byte[] options) {
        super(options.length * 8, options);  	
    }
    
    /**
     * Parse (unmarshal) a KDCOptions from a DER input stream.  This form
     * parsing might be used when expanding a value which is part of
     * a constructed sequence and uses explicitly tagged type.
     *
     * @param data the Der input stream value, which contains one or more
     * marshaled value.
     * @param explicitTag tag number.
     * @param optional indicate if this data field is optional
     * @return an instance of KDCOptions.
     * @exception Asn1Exception if an error occurs while decoding an ASN1 encoded data.
     * @exception IOException if an I/O error occurs while reading encoded data.
     *
     */
    
    public static KDCOptions parse(DerInputStream data, byte explicitTag, boolean optional) throws Asn1Exception, IOException { 
	if ((optional) && (((byte)data.peekByte() & (byte)0x1F) != explicitTag)) 
	    return null;		
	DerValue der = data.getDerValue();
	if (explicitTag != (der.getTag() & (byte)0x1F))  {	
	    throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	}
	else {
	    DerValue subDer = der.getData().getDerValue();
	    return new KDCOptions(subDer);
	}
    }
    
    /**
     * Writes the encoded data.
     *
     * @exception IOException if an I/O error occurs while reading encoded data.
     * @return an byte array of encoded KDCOptions.
     */
    public byte[] asn1Encode() throws IOException {	
	DerOutputStream out = new DerOutputStream();
	out.putUnalignedBitString(new BitArray(this.toBooleanArray()));
	return out.toByteArray();
    }
    
    /**
     * Sets the value(true/false) for one of the <code>KDCOptions</code>.
     *
     * @param option an option bit. 	 
     * @param value true if the option is selected, false if the option is not selected.
     * @exception ArrayIndexOutOfBoundsException if array index out of bound occurs.
     * @see sun.security.krb5.internal.Krb5
     */
    public void set(int option, boolean value) throws ArrayIndexOutOfBoundsException {
	super.set(option, value);	
    }
    
    /**
     * Gets the value(true/false) for one of the <code>KDCOptions</code>.
     *
     * @param option an option bit. 	 
     * @return value true if the option is selected, false if the option is not selected.
     * @exception ArrayIndexOutOfBoundsException if array index out of bound occurs.
     * @see sun.security.krb5.internal.Krb5
     */
    
    public boolean get(int option) throws ArrayIndexOutOfBoundsException {
	return super.get(option);
    }
    
    
    private void setDefault() {
        try {
	    
            Config config = Config.getInstance();
	    
            /*
             * First see if the IBM hex format is being used.
             * If not, try the Sun's string (boolean) format.
             */
	    
            int options =config.getDefaultIntValue("kdc_default_options",
						   "libdefaults");
	    
            if ((options & RENEWABLE_OK) == RENEWABLE_OK) {
                set(RENEWABLE_OK, true);
            } else {
		if (config.getDefaultBooleanValue("renewable", "libdefaults")) {
		    set(RENEWABLE_OK, true);
		}
	    }
            if ((options & PROXIABLE) == PROXIABLE) {
                set(PROXIABLE, true);
            } else {
		if (config.getDefaultBooleanValue("proxiable", "libdefaults")) {
		    set(PROXIABLE, true);
		}
	    }

            if ((options & FORWARDABLE) == FORWARDABLE) {
               set(FORWARDABLE, true);
            } else {
		if (config.getDefaultBooleanValue("forwardable", "libdefaults")) {
		    set(FORWARDABLE, true);
		}
	    }
        } catch (KrbException e) {
	    if (DEBUG) {
		System.out.println("Exception in getting default values for " +
				   "KDC Options from the configuration ");
		e.printStackTrace();
				   
	    }
        }
   }
}
