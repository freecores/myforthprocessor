/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)KrbException.java	1.8 03/06/24
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

package sun.security.krb5;

import sun.security.krb5.internal.Krb5;
import sun.security.krb5.internal.KRBError;

public class KrbException extends Exception {
    private int returnCode;
    private KRBError error;
    
    public KrbException (String s) {
	super(s);
    }
    
    public KrbException(int i) {
	returnCode = i;
    }
    
    public KrbException(int i, String s) {
	this(s);
	returnCode = i;
    }
    
    public KrbException(KRBError e) {
	returnCode = e.errorCode;
	error = e;
    }
    
    public KRBError getError() {
	return error;
    }
    
    
    public int returnCode() {
	return returnCode;
    }
    
    public String returnCodeSymbol() {
	return returnCodeSymbol(returnCode);
    }
    
    public static String returnCodeSymbol(int i) {
	return "not yet implemented";  
    }
    
    public String returnCodeMessage() {
	return (String)Krb5.errMsgList.get(new Integer(returnCode));
    }
    
    public static String errorMessage(int i) {
	return (String)Krb5.errMsgList.get(new Integer(i));
    }
   

    public String krbErrorMessage() {
	StringBuffer strbuf = new StringBuffer("krb_error " + returnCode);
	String msg =  getMessage();
	if (msg != null) {
	    strbuf.append(" ");
	    strbuf.append(msg);
	}
	return strbuf.toString(); 
    }

    /**
     * Returns messages like: 
     * "Integrity check on decrypted field failed (31) - \
     *                         Could not decrypt service ticket"
     * If the error code is 0 then the first half is skipped.
     */
    public String getMessage() {
	StringBuffer message = new StringBuffer();
	int returnCode = returnCode();
	if (returnCode != 0) {
	    message.append(returnCodeMessage());
	    message.append(" (").append(returnCode()).append(')');
	}
	String consMessage = super.getMessage();
	if (consMessage != null && consMessage.length() != 0) {
	    if (returnCode != 0)
		message.append(" - ");
	    message.append(consMessage);
	}
	return message.toString();
    }

    public String toString() {
	return ("KrbException: " + getMessage());
    }

}
