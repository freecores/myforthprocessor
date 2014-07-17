/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)HostAddress.java	1.12 03/06/24
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
import sun.security.krb5.Asn1Exception;
import sun.security.util.*;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.io.IOException;

/**
 * Implements the ASN.1 HostAddress type.
 *
 * <xmp>
 * HostAddress ::= SEQUENCE {
 *  				addr-type[0] INTEGER,
 *					address[1] OCTET STRING
 * }
 * </xmp>
 *
 * <p>
 * This definition reflects the Network Working Group RFC 1510
 * specifications dated September 1993 and available at
 * <a href="http://www.ietf.org/rfc/rfc1510.txt">http://www.ietf.org/rfc/rfc1510.txt</a>.
 */
public class HostAddress implements Cloneable {
    int addrType;
    byte[] address = null;

    private static byte[] localInetAddress; //caches local inet address
    private boolean DEBUG = Krb5.DEBUG;
    private volatile int hashCode = 0;

    private HostAddress(int dummy) {
    }
    
    public Object clone() {
	HostAddress new_hostAddress = new HostAddress(0);
	new_hostAddress.addrType = addrType;
	if (address != null) {
	    new_hostAddress.address = new byte[address.length];
	    System.arraycopy(address, 0, new_hostAddress.address,
			     0, address.length);
	}
	return new_hostAddress;
    }
    
    
    public int hashCode() {
	if (hashCode == 0) {
	    int result = 17;
	    result = 37*addrType;
	    if (address != null) {
		for (int i=0; i < address.length; i++)  {
		    result = 37*result + address[i];
		}
	    }
	    hashCode = result;
	}
	return hashCode;
	
    }
    
    public boolean equals(HostAddress h) {
	if (addrType != h.addrType ||
	    (address != null && h.address == null) ||
	    (address == null && h.address != null))
	    return false;
	if (address != null && h.address != null) {
	    if (address.length != h.address.length)
		return false;
	    for (int i = 0; i < address.length; i++)
		if (address[i] != h.address[i])
		    return false;
	}
	return true;
    }
    
    private static void refreshLocalInetAddress() throws UnknownHostException {
	localInetAddress = InetAddress.getLocalHost().getAddress();
    }
    
    public static synchronized byte[] getLocalInetAddress() throws UnknownHostException {
	if (localInetAddress == null) {
	   refreshLocalInetAddress();
	}
	if (localInetAddress == null) {
	    throw new UnknownHostException(); 
	}
	byte[] result = new byte[localInetAddress.length];
	System.arraycopy(localInetAddress, 0, result, 0,
			 localInetAddress.length);
	return result;
    }
    
    /**
     * Gets the InetAddress of this HostAddress.
     * @return the IP address for this specified host.
     * @exception if no IP address for the host could be found.
     *
     */
    public InetAddress getInetAddress() throws UnknownHostException {
	// the type of internet addresses is 2.
	if (addrType == 2) {
	    String host = (address[0] & 0x000000ff)	+ "." 
		+ (address[1] & 0x000000ff) + "."
		+ (address[2] & 0x000000ff) + "."
		+ (address[3] & 0x000000ff);
	    return InetAddress.getByName(host);
	} else {
	    // if it is other type (ISO address, XNS address, etc)
	    return null;
	}
	
    }
    // implicit default not in Config.java
    public HostAddress() throws UnknownHostException {
	addrType = Krb5.ADDRTYPE_INET;
	address = getLocalInetAddress();
    }
    
    /**
     * Creates a HostAddress from the specified address and address type.
     *
     * @param new_addrType the value of the address type which matches the defined
     *                       address family constants in the Berkeley Standard
     *                       Distributions of Unix.
     * @param new_address network address.
     * @exception KrbApErrException if address type and address length do not match defined value.
     *
     */
    public HostAddress(int new_addrType, byte[] new_address) 
	throws KrbApErrException{
	switch(new_addrType) {
	case 2: 					//Internet address
	    if (new_address.length != 4)
		throw new KrbApErrException(0, "Invalid Internet address");
	    break;
	case 5:
	    if (new_address.length != 2) //CHAOSnet address
		throw new KrbApErrException(0, "Invalid CHAOSnet address");
	    break;
	case 7:   		  // ISO address
	    break;
	case 6:			  // XNS address    
	    if (new_address.length != 6) 
		throw new KrbApErrException(0, "Invalid XNS address");
	    break;
	case 16:
	    if (new_address.length !=3)      //AppleTalk DDP address
		throw new KrbApErrException(0, "Invalid DDP address");
	    break;
	case 12:
	    if (new_address.length !=2)  //DECnet Phase IV address
		throw new KrbApErrException(0, "Invalid DECnet Phase IV address");
	    break;
	}
	addrType = new_addrType;
	if (new_address != null) {
	   address = (byte[])new_address.clone();
	}
	if (DEBUG) {
	    if (addrType == 2) {
		System.out.println("Host address is " + (address[0] & 0xff) + "."
				   + (address[1] & 0xff) + "." + 
				   (address[2] & 0xff) + "." +
				   (address[3] & 0xff));
	    }
	}
    }
    
    public HostAddress(InetAddress inetAddress) {
	addrType = Krb5.ADDRTYPE_INET;
	byte[] new_address = inetAddress.getAddress();
	address = new byte[new_address.length];
	System.arraycopy(new_address, 0, address, 0, new_address.length);
    }
    
    /**
     * Constructs a host address from a single DER-encoded value.
     * @param encoding a single DER-encoded value.
     * @exception Asn1Exception if an error occurs while decoding an ASN1 encoded data.
     * @exception IOException if an I/O error occurs while reading encoded data.
     * 
     */
    public HostAddress(DerValue encoding) throws Asn1Exception, IOException {
	DerValue der = encoding.getData().getDerValue();
	if ((der.getTag() & (byte)0x1F) == (byte)0x00) {
	    addrType = der.getData().getBigInteger().intValue();
	}
	else
	    throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	der = encoding.getData().getDerValue();
	if ((der.getTag() & (byte)0x1F) == (byte)0x01) {
	    address = der.getData().getOctetString();
	}
	else
	    throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	if (encoding.getData().available() > 0)
	    throw new Asn1Exception(Krb5.ASN1_BAD_ID);
    }

    /**
	 * Encodes a HostAddress object.
	 * @return a byte array of encoded HostAddress object.
	 * @exception Asn1Exception if an error occurs while decoding an ASN1 encoded data.
	 * @exception IOException if an I/O error occurs while reading encoded data.
	 * 
	 */

    public byte[] asn1Encode() throws Asn1Exception, IOException {  	
        DerOutputStream bytes = new DerOutputStream();
	DerOutputStream temp = new DerOutputStream();
	temp.putInteger(this.addrType);
	bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x00), temp);
	temp = new DerOutputStream();
	temp.putOctetString(address);
	bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x01), temp);
	temp = new DerOutputStream();
	temp.write(DerValue.tag_Sequence, bytes);
	return temp.toByteArray();
    }
        
    /**
     * Parses (unmarshal) a host address from a DER input stream.  This form
     * parsing might be used when expanding a value which is part of
	 * a constructed sequence and uses explicitly tagged type.
     *
     * @exception Asn1Exception on error.
     * @exception IOException if an I/O error occurs while reading encoded data.
     * @param data the Der input stream value, which contains one or more marshaled value.
     * @param explicitTag tag number.
     * @param optional indicates if this data field is optional
     * @return an instance of HostAddress.
     *
     */
    public static HostAddress parse(DerInputStream data, byte explicitTag,
				    boolean optional) 
	throws Asn1Exception, IOException{ 
	if ((optional) && 
	    (((byte)data.peekByte() & (byte)0x1F) != explicitTag)) {	
	    return null;
	}
	DerValue der = data.getDerValue();
	if (explicitTag != (der.getTag() & (byte)0x1F))  {	
	    throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	}
	else {
	    DerValue subDer = der.getData().getDerValue();
	    return new HostAddress(subDer);
	}
    }
    
}
