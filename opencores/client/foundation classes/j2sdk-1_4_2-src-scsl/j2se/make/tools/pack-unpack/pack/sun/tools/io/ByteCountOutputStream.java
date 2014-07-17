/*
 * @(#)ByteCountOutputStream.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.tools.io;

import java.io.OutputStream;
import java.io.FilterOutputStream;
import java.io.IOException;

/**
 * An output stream which counts the number of bytes written.
 */
public
class ByteCountOutputStream extends FilterOutputStream {
    private long count;

    public ByteCountOutputStream(OutputStream out) {
	super(out);
    }

    public long getCount() { return count; }
    public void setCount(long c) { count = c; }

    public void write(int b) throws IOException {
	count++;
	if (out != null)  out.write(b);
    }
    public void write(byte b[], int off, int len) throws IOException {
	count += len;
	if (out != null)  out.write(b, off, len);
    }
    public String toString() {
	return String.valueOf(getCount());
    }
}

