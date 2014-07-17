/*
 * @(#)OffsetInputStream.java	1.3 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/**
 * a subclass of FilterInputStream that keeps track of the read offset.
 */

package sun.tools.crunch;

import java.io.*;

public class OffsetInputStream extends java.io.FilterInputStream {

    int offset = 0;

    public OffsetInputStream(InputStream in) {
	super(in);
    }

    public int read() throws IOException {
	int rc = super.read();
	if (rc != -1) {
	    offset++;
	}
	return rc;
    }

    public int read(byte[] b, int off, int len) throws IOException {
	int rc = super.read(b, off, len);
	if (rc > 0) {
	    offset += rc;
	}
	return rc;
    }

    public long skip(long len) throws IOException {
	long skipped = super.skip(len);
	offset += skipped;
	return skipped;
    }

    public int getOffset() {
	return offset;
    }

}
