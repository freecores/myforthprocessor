/*
 * @(#)CompressedReadStream.java	1.3 03/01/23 11:23:36
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.code;

import sun.jvm.hotspot.debugger.*;

public class CompressedReadStream extends CompressedStream {
  /** Equivalent to CompressedReadStream(buffer, 0) */
  public CompressedReadStream(Address buffer) {
    this(buffer, 0);
  }

  public CompressedReadStream(Address buffer, int position) {
    super(buffer, position);
  }

  public boolean readBoolean() {
    return (read() != 0);
  }

  public byte readByte() {
    return (byte) read();
  }

  public char readChar() {
    return (char) readInt();
  }

  public short readShort() {
    return (short) readInt();
  }
   
  public int readInt() {
    int shift = 0;
    int value = 0;
    short s = read();
    while (s < 0x80) {
      value += s << shift;
      shift += 7;
      s = read();
    }
    value += (s - 192) << shift;
    return value;
  }
  
  public float readFloat() {
    return Float.intBitsToFloat(readInt());
  }

  public double readDouble() {
    return Double.longBitsToDouble(readLong());
  }

  public long readLong() {
    long low = readInt() & 0x00000000FFFFFFFFL;
    long high = readInt();
    return (high << 32) | low;
  }

  //--------------------------------------------------------------------------------
  // Internals only below this point
  //

  /** Reads an unsigned byte, but returns it as a short */
  private short read() {
    short retval = (short) buffer.getCIntegerAt(position, 1, true);
    ++position;
    return retval;
  }
}
