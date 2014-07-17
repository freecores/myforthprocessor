/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)KrbBitArray.java	1.6 03/06/24
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

package sun.security.krb5.internal.util;

import sun.security.util.BitArray;

/**
 * A wrapper class around sun.security.util.BitArray, so that KDCOptions and ApOptions in krb5 classes
 * can utilize some functions in BitArray classes.
 *
 * @author Yanni Zhang
 * @version 1.0 22 June 2000
 */
public class KrbBitArray {
    BitArray bits;

    public KrbBitArray(int length) throws IllegalArgumentException {
        bits =  new BitArray(length);
    }

    public KrbBitArray(int length, byte[] a) throws IllegalArgumentException {
        bits = new BitArray(length, a);
    }

    public KrbBitArray(boolean[] bools) {
        bits = new BitArray(bools);
    }

    public void set(int index, boolean value) {
        bits.set(index, value);
    }

    public boolean get(int index) {
        return bits.get(index);
    }

    public boolean[] toBooleanArray() {
        return bits.toBooleanArray();
    }

    public String toString() {
	return bits.toString();
    }
}
