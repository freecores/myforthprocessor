/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)CCacheOutputStream.java	1.6 03/06/24
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

package sun.security.krb5.internal.ccache;

import java.io.IOException;
import java.io.FileOutputStream;
import java.io.OutputStream;
import sun.security.krb5.internal.util.KrbDataOutputStream;
import sun.security.krb5.*;
import sun.security.krb5.internal.*;

/**
 * This class implements a buffered output stream. It provides functions to write FCC-format data to a disk file.
 *
 * @author Yanni Zhang
 * @version 1.00
 *
 */
public class CCacheOutputStream extends KrbDataOutputStream implements FileCCacheConstants {   
    public CCacheOutputStream(OutputStream os) {
	super(os);
    }

    public void writeHeader(PrincipalName p, int version) throws IOException {
	write((version & 0xff00) >> 8);
	write(version & 0x00ff);
	p.writePrincipal(this);
    }

    /**
     * Writes a credentials in FCC format to this cache output stream.
     *
     * @param creds the credentials to be written to the output stream.
     * @exception IOException if an I/O exception occurs.
     * @exception Asn1Exception	 if an Asn1Exception occurs.
     */
    /*For object data fields which themselves have multiple data fields, such as PrincipalName, EncryptionKey
      HostAddresses, AuthorizationData, I created corresponding write methods (writePrincipal,
      writeKey,...) in each class, since converting the object into FCC format data stream 
      should be encapsulated in object itself. 
    */
    public void addCreds(Credentials creds) throws IOException, Asn1Exception {
        creds.cname.writePrincipal(this);
	creds.sname.writePrincipal(this);
	creds.key.writeKey(this);
        write32((int)(creds.authtime.getTime()/1000));
        if (creds.starttime != null) 
            write32((int)(creds.starttime.getTime()/1000));			
	else write32(0);
        write32((int)(creds.endtime.getTime()/1000));
	if (creds.renewTill != null) 
            write32((int)(creds.renewTill.getTime()/1000));
			
	else write32(0);
	if (creds.isEncInSKey) {
	    write8(1);
	}
	else write8(0);
	writeFlags(creds.flags);
	if (creds.caddr == null) 
	    write32(0);
	else 
	    creds.caddr.writeAddrs(this);
		
	if (creds.authorizationData == null) {
	    write32(0);
	}
	else 
	    creds.authorizationData.writeAuth(this);
	writeTicket(creds.ticket);
	writeTicket(creds.secondTicket);
    }

    void writeTicket(Ticket t) throws IOException, Asn1Exception {	
	if (t == null) {   
	    write32(0);
	}
	else {
	    byte[] bytes = t.asn1Encode();
	    write32(bytes.length);
	    write(bytes, 0, bytes.length);
	}
    }
	
    void writeFlags(TicketFlags flags) throws IOException {
	int tFlags = 0;	
	boolean[] f = flags.toBooleanArray();
	if (f[1] == true) {
	    tFlags |= TKT_FLG_FORWARDABLE;
	}
	if (f[2] == true) {
	    tFlags |= TKT_FLG_FORWARDED;
	}
        if (f[3] == true) {
	    tFlags |= TKT_FLG_PROXIABLE;
	}
        if (f[4] == true) {
	    tFlags |= TKT_FLG_PROXY;
	}
        if (f[5] == true) {
	    tFlags |= TKT_FLG_MAY_POSTDATE;
	}
        if (f[6] == true) {
	    tFlags |= TKT_FLG_POSTDATED;
	}
        if (f[7] == true) {
	    tFlags |= TKT_FLG_INVALID;
	}
        if (f[8] == true) {
	    tFlags |= TKT_FLG_RENEWABLE;
	}
        if (f[9] == true) {
	    tFlags |= TKT_FLG_INITIAL;
	}
        if (f[10] == true) {
	    tFlags |= TKT_FLG_PRE_AUTH;
	}
	if (f[11] == true) {
	    tFlags |= TKT_FLG_HW_AUTH;
	}
	write32(tFlags);
		 
    }
}
