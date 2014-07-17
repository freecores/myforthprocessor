/*
 * @(#)CompressedLineNumberReadStream.java	1.3 03/01/23 11:42:09
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.oops;

import sun.jvm.hotspot.code.*;
import sun.jvm.hotspot.debugger.*;

public class CompressedLineNumberReadStream extends CompressedReadStream {
  /** Equivalent to CompressedLineNumberReadStream(buffer, 0) */
  public CompressedLineNumberReadStream(Address buffer) {
    this(buffer, 0);
  }

  public CompressedLineNumberReadStream(Address buffer, int position) {
    super(buffer, position);
  }

  /** Read (bci, line number) pair from stream. Returns false at end-of-stream. */
  public boolean readPair() {
    int next = readByte() & 0xFF;
    // Check for terminator
    if (next == 0) return false;
    if (next == 0xFF) {
      // Escape character, regular compression used
      bci  += readInt();
      line += readInt();
    } else {
      // Single byte compression used
      bci  += next >> 3;
      line += next & 0x7;
    }
    return true;
  }

  public int bci()  { return bci;  }
  public int line() { return line; }

  //--------------------------------------------------------------------------------
  // Internals only below this point
  //

  private int bci;
  private int line;
}
