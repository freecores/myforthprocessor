/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)KerberosTime.java	1.10 03/06/24
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
import sun.security.krb5.Config;
import sun.security.krb5.KrbException;
import sun.security.krb5.Asn1Exception;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.Calendar;
import java.io.IOException;

/**
 * Implements the ASN.1 KerberosTime type.
 *
 * <xmp>
 * KerberosTime ::= GeneralizedTime --specifying UTC time zone (Z)
 * </xmp>
 *
 * <p>
 * This definition reflects the Network Working Group RFC 1510
 * specifications dated September 1993 and available at
 * <a href="http://www.ietf.org/rfc/rfc1510.txt">http://www.ietf.org/rfc/rfc1510.txt</a>.
 */
public class KerberosTime implements Cloneable {

    private long kerberosTime; // actually stores local time, not UTC
    private static long syncTime;
    private static boolean DEBUG = Krb5.DEBUG;

    public static final boolean NOW = true;
    public static final boolean UNADJUSTED_NOW = false;

    //defaults to zero instead of now; use setNow() for current time
    public KerberosTime() {
	kerberosTime = 0;
    }

    public KerberosTime(long time) {
	kerberosTime = time;
    }


    public Object clone() {
	return new KerberosTime(kerberosTime);
    }

    public KerberosTime(String time) throws Asn1Exception {
	kerberosTime = toKerberosTime(time);
    }

    /**
     * Constructs a KerberosTime object.
     * @param encoding a DER-encoded data.
     * @exception Asn1Exception if an error occurs while decoding an ASN1 encoded data.
     * @exception IOException if an I/O error occurs while reading encoded data.
     */
    public KerberosTime(DerValue encoding) throws Asn1Exception, IOException { 
	GregorianCalendar calendar = new GregorianCalendar();
	Date temp = encoding.getGeneralizedTime();
	kerberosTime = temp.getTime();
    }

    private static long toKerberosTime(String time) throws Asn1Exception {  
	// this method only used by KerberosTime class.

	// ASN.1 GeneralizedTime format:

	// "19700101000000Z"
	//  |   | | | | | |
	//  0   4 6 8 | | |
	//           10 | |
	//			   12 |
	//			     14

	if (time.length() != 15)
	    throw new Asn1Exception(Krb5.ASN1_BAD_TIMEFORMAT);
	if (time.charAt(14) != 'Z')
	    throw new Asn1Exception(Krb5.ASN1_BAD_TIMEFORMAT);
	int year = Integer.parseInt(time.substring(0, 4));
	Calendar calendar = Calendar.getInstance();
	calendar.set(year,
		     Integer.parseInt(time.substring(4, 6)) - 1,
		     Integer.parseInt(time.substring(6, 8)),
		     Integer.parseInt(time.substring(8, 10)),
		     Integer.parseInt(time.substring(10, 12)),
		     Integer.parseInt(time.substring(12, 14)));
		
	//The Date constructor assumes the setting are local relative
	//and converts the time to UTC before storing it.  Since we
	//want the internal representation to correspond to local
	//and not UTC time we subtract the UTC time offset.
	return (calendar.getTime().getTime());
	  
    }		    

    // should be moved to sun.security.krb5.util class
    public static String zeroPad(String s, int length) {
	StringBuffer temp = new StringBuffer(s);
	while (temp.length() < length)
	    temp.insert(0, '0');
	return temp.toString();
    }

    public KerberosTime(Date time) {
	kerberosTime = time.getTime(); // (time.getTimezoneOffset() * 60000L);
    }

    public KerberosTime(boolean initToNow) {
	if (initToNow) {
	    Date temp = new Date();
	    setTime(temp);
	}
	else
	    kerberosTime = 0;
    }

    /**
     * Returns a string representation of KerberosTime object.
     * @return a string representation of this object.
     */
    public String toGeneralizedTimeString() {
	Date temp = new Date(kerberosTime);
	Calendar calendar = Calendar.getInstance(); 
	calendar.clear();
		
	//Assumes Date is local relative so we need to revert
	//to UTC time before extracting the Date components.
        temp.setTime(temp.getTime() - (calendar.get(Calendar.ZONE_OFFSET) + calendar.get(Calendar.DST_OFFSET)));
	calendar.setTime(temp);
        return zeroPad(Integer.toString(calendar.get(Calendar.YEAR)), 4) +
	    zeroPad(Integer.toString(calendar.get(Calendar.MONTH) + 1), 2) +
	    zeroPad(Integer.toString(calendar.get(Calendar.DAY_OF_MONTH)), 2) +
	    zeroPad(Integer.toString(calendar.get(Calendar.HOUR_OF_DAY)), 2) +
	    zeroPad(Integer.toString(calendar.get(Calendar.MINUTE)), 2) +
	    zeroPad(Integer.toString(calendar.get(Calendar.SECOND)), 2) + 'Z';
		
    }

    /**
     * Encodes this object to a byte array.
     * @return a byte array of encoded data.
     * @exception Asn1Exception if an error occurs while decoding an ASN1 encoded data.
     * @exception IOException if an I/O error occurs while reading encoded data.
     */
    public byte[] asn1Encode() throws Asn1Exception, IOException {
	DerOutputStream out = new DerOutputStream();
	out.putGeneralizedTime(this.toDate());
	return out.toByteArray();
    }

    public long getTime() {
	return kerberosTime;
    }


    public void setTime(Date time) {
	kerberosTime = time.getTime(); // (time.getTimezoneOffset() * 60000L);
    }

    public void setTime(long time) {
	kerberosTime = time;
    }

    public Date toDate() {
	Date temp = new Date(kerberosTime);
        temp.setTime(temp.getTime());
	return temp;
    }

    public void setNow() {
	Date temp = new Date();
        setTime(temp);
    }

    public int getMicroSeconds() {
	Long temp_long = new Long((kerberosTime % 1000L) * 1000L);
	return temp_long.intValue();
    }

    public void setMicroSeconds(int usec) {
	Integer temp_int = new Integer(usec);
	long temp_long = temp_int.longValue() / 1000L;
	kerberosTime = kerberosTime - (kerberosTime % 1000L) + temp_long;
    }

    public void setMicroSeconds(Integer usec) {
	if (usec != null) {
	    long temp_long = usec.longValue() / 1000L;
	    kerberosTime = kerberosTime - (kerberosTime % 1000L) + temp_long;
	}
    }
    public boolean inClockSkew(int clockSkew) {
	KerberosTime now = new KerberosTime(KerberosTime.NOW);

	if (java.lang.Math.abs(kerberosTime - now.kerberosTime) >
	    clockSkew * 1000L)    
	    return false;
	return true;
    }

    public boolean inClockSkew() {
	return inClockSkew(getDefaultSkew());
    }

    public boolean greaterThanWRTClockSkew(KerberosTime time, int clockSkew) {
	if ((kerberosTime - time.kerberosTime) > clockSkew * 1000L)
	    return true;
	return false;
    }

    public boolean greaterThanWRTClockSkew(KerberosTime time) {
	return greaterThanWRTClockSkew(time, getDefaultSkew());
    }

    public boolean greaterThan(KerberosTime time) {
	return kerberosTime > time.kerberosTime;
    }

    public boolean equals(KerberosTime time) {
	return kerberosTime == time.kerberosTime;
    }

    public boolean isZero() {
	return kerberosTime == 0;
    }

    public int getSeconds() {
	Long temp_long = new Long(kerberosTime / 1000L);
	return temp_long.intValue();
    }

    public void setSeconds(int sec) {
	Integer temp_int = new Integer(sec);
	kerberosTime = temp_int.longValue() * 1000L;
    }

    /**
     * Parse (unmarshal) a kerberostime from a DER input stream.  This form
     * parsing might be used when expanding a value which is part of
     * a constructed sequence and uses explicitly tagged type.
     *
     * @exception Asn1Exception on error.
     * @param data the Der input stream value, which contains one or more marshaled value.
     * @param explicitTag tag number.
     * @param optional indicates if this data field is optional
     * @return an instance of KerberosTime.
     *
     */
    public static KerberosTime parse(DerInputStream data, byte explicitTag, boolean optional) throws Asn1Exception, IOException {
	if ((optional) && (((byte)data.peekByte() & (byte)0x1F)!= explicitTag)) 
	    return null;
	DerValue der = data.getDerValue();
	if (explicitTag != (der.getTag() & (byte)0x1F))  {	
	    throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	}
	else {
	    DerValue subDer = der.getData().getDerValue();
	    return new KerberosTime(subDer);
	}
    }

    private int getDefaultSkew() {
        int tdiff = Krb5.DEFAULT_ALLOWABLE_CLOCKSKEW;
        try {
            Config c = Config.getInstance();
            if ((tdiff = c.getDefaultIntValue("clockskew", 
					      "libdefaults")) == Integer.MIN_VALUE) {   //value is not defined
                tdiff = Krb5.DEFAULT_ALLOWABLE_CLOCKSKEW;
            }
        } catch (KrbException e) {
	    if (DEBUG) {
		System.out.println("Exception in getting clockskew from " +
				   "Configuration " + 
				   "using default value " + 
				   e.getMessage());
	    }
        }
        return tdiff;
    }

    public String toString() {
	return toGeneralizedTimeString();
    }
}
