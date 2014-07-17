/*
 * @(#)CompressedStream.java	1.4 03/01/23 11:23:38
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.code;

import sun.jvm.hotspot.debugger.*;

/** NOTE that this class takes the address of a buffer. This means
    that it can read previously-generated debug information directly
    from the target VM. However, it also means that you can't create a
    "wrapper" object for a CompressedStream down in the VM. It looks
    like these are only kept persistently in OopMaps, and the code has
    been special-cased in OopMap.java to handle this. */

public class CompressedStream {
  protected Address buffer;
  protected int     position;

  /** Equivalent to CompressedStream(buffer, 0) */
  public CompressedStream(Address buffer) {
    this(buffer, 0);
  }

  public CompressedStream(Address buffer, int position) {
    this.buffer   = buffer;
    this.position = position;
  }

  public Address getBuffer() {
    return buffer;
  }
  
  // Positioning
  public int getPosition() {
    return position;
  }
  public void setPosition(int position) {
    this.position = position;
  }
}
