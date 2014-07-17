/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)LocalSeqNumber.java	1.6 03/06/24
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

import sun.security.krb5.internal.crypto.Confounder;
import sun.security.krb5.KrbCryptoException;

public class LocalSeqNumber implements SeqNumber {
    private int lastSeqNumber;

    public LocalSeqNumber() throws KrbCryptoException {
	randInit();
    }

    public LocalSeqNumber(int start) {
	init(start);
    }

    public LocalSeqNumber(Integer start) {
	init(start.intValue());
    }

    public synchronized void randInit() throws KrbCryptoException {
	lastSeqNumber = Confounder.intValue();
    }

    public synchronized void init(int start) {
	lastSeqNumber = start;
    }

    public synchronized int current() {
	return lastSeqNumber;
    }

    public synchronized int next() {
	return lastSeqNumber + 1;
    }

    public synchronized int step() {
	return ++lastSeqNumber;
    }

}

