/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)Tag.java	1.5 03/06/24
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

import sun.security.krb5.*;
import java.io.ByteArrayOutputStream;

/**
 * tag field introduced in KRB5_FCC_FVNO_4 
 *
 * @author Yanni Zhang
 * @version 1.00
 */
public class Tag{
    int length;
    int tag;
    int tagLen;
    Integer time_offset;
    Integer usec_offset;

    public Tag(int len, int new_tag, Integer new_time, Integer new_usec) {
	tag = new_tag;
	tagLen = 8;
	time_offset = new_time;
	usec_offset = new_usec;
	length =  4 + tagLen;
    }
    public Tag(int new_tag) {
	tag = new_tag;
	tagLen = 0;
	length = 4 + tagLen;
    }
    public byte[] toByteArray() {
	ByteArrayOutputStream os = new ByteArrayOutputStream();
	os.write(length);
	os.write(tag);
	os.write(tagLen);
	if (time_offset != null) {
	    os.write(time_offset.intValue());
	}
	if (usec_offset != null) {
	    os.write(usec_offset.intValue());
	}
	return os.toByteArray();
    }
}
