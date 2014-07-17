/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)LoginOptions.java	1.5 03/06/24
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
 */
public class LoginOptions extends KDCOptions { 
    
    //Login Options

    public static final int RESERVED        = 0;
    public static final int FORWARDABLE     = 1;
    public static final int PROXIABLE       = 3;
    public static final int ALLOW_POSTDATE  = 5;
    public static final int RENEWABLE       = 8;
    public static final int RENEWABLE_OK    = 27;
    public static final int ENC_TKT_IN_SKEY = 28;
    public static final int RENEW           = 30;
    public static final int VALIDATE        = 31;
    public static final int MAX             = 31;

}
