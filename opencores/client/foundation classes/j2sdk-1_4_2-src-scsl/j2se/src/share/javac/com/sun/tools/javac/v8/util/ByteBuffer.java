/**
 * @(#)ByteBuffer.java	1.13 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8.util;
import java.io.*;


/**
 * A byte buffer is a flexible array which grows when elements are
 *  appended. There are also methods to append names to byte buffers
 *  and to convert byte buffers to names.
 */
public class ByteBuffer {

    /**
     * An array holding the bytes in this buffer; can be grown.
     */
    public byte[] elems;

    /**
     * The current number of defined bytes in this buffer.
     */
    public int length;

    /**
     * Create a new byte buffer.
     */
    public ByteBuffer() {
        this(64);
    }

    /**
      * Create a new byte buffer with an initial elements array
      *  of given size.
      */
    public ByteBuffer(int initialSize) {
        super();
        elems = new byte[initialSize];
        length = 0;
    }

    private void copy(int size) {
        byte[] newelems = new byte[size];
        System.arraycopy(elems, 0, newelems, 0, elems.length);
        elems = newelems;
    }

    /**
      * Append byte to this buffer.
      */
    public void appendByte(int b) {
        if (length >= elems.length)
            copy(elems.length * 2);
        elems[length++] = (byte) b;
    }

    /**
      * Append `len' bytes from byte array,
      *  starting at given `start' offset.
      */
    public void appendBytes(byte[] bs, int start, int len) {
        while (length + len > elems.length)
            copy(elems.length * 2);
        System.arraycopy(bs, start, elems, length, len);
        length += len;
    }

    /**
      * Append all bytes from given byte array.
      */
    public void appendBytes(byte[] bs) {
        appendBytes(bs, 0, bs.length);
    }

    /**
      * Append a character as a two byte number.
      */
    public void appendChar(int x) {
        while (length + 1 >= elems.length)
            copy(elems.length * 2);
        elems[length] = (byte)((x >> 8) & 255);
        elems[length + 1] = (byte)((x) & 255);
        length = length + 2;
    }

    /**
      * Append an integer as a four byte number.
      */
    public void appendInt(int x) {
        while (length + 3 >= elems.length)
            copy(elems.length * 2);
        elems[length] = (byte)((x >> 24) & 255);
        elems[length + 1] = (byte)((x >> 16) & 255);
        elems[length + 2] = (byte)((x >> 8) & 255);
        elems[length + 3] = (byte)((x) & 255);
        length = length + 4;
    }

    /**
      * Append a long as an eight byte number.
      */
    public void appendLong(long x) {
        ByteArrayOutputStream buffer = new ByteArrayOutputStream(8);
        DataOutputStream bufout = new DataOutputStream(buffer);
        try {
            bufout.writeLong(x);
            appendBytes(buffer.toByteArray(), 0, 8);
        } catch (IOException e) {
            throw new AssertionError("write");
        }
    }

    /**
      * Append a float as a four byte number.
      */
    public void appendFloat(float x) {
        ByteArrayOutputStream buffer = new ByteArrayOutputStream(4);
        DataOutputStream bufout = new DataOutputStream(buffer);
        try {
            bufout.writeFloat(x);
            appendBytes(buffer.toByteArray(), 0, 4);
        } catch (IOException e) {
            throw new AssertionError("write");
        }
    }

    /**
      * Append a double as a eight byte number.
      */
    public void appendDouble(double x) {
        ByteArrayOutputStream buffer = new ByteArrayOutputStream(8);
        DataOutputStream bufout = new DataOutputStream(buffer);
        try {
            bufout.writeDouble(x);
            appendBytes(buffer.toByteArray(), 0, 8);
        } catch (IOException e) {
            throw new AssertionError("write");
        }
    }

    /**
      * Append a name.
      */
    public void appendName(Name name) {
        appendBytes(name.table.names, name.index, name.len);
    }

    /**
      * Reset to zero length.
      */
    public void reset() {
        length = 0;
    }

    /**
      * Convert contents to name.
      */
    public Name toName(Name.Table names) {
        return names.fromUtf(elems, 0, length);
    }
}
