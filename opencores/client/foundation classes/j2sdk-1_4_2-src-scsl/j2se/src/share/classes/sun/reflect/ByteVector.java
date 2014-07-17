/*
 * @(#)ByteVector.java	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.reflect;

/** A growable array of bytes. */

interface ByteVector {
    public int  getLength();
    public byte get(int index);
    public void put(int index, byte value);
    public void add(byte value);
    public void trim();
    public byte[] getData();
}
