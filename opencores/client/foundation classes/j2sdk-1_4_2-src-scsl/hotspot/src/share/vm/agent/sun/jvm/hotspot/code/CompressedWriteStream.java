/*
 * @(#)CompressedWriteStream.java	1.3 03/01/23 11:23:40
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.code;

import sun.jvm.hotspot.debugger.*;

/** Currently only used for oop map parsing (getBuffer() method) */

public class CompressedWriteStream extends CompressedStream {
  /** Equivalent to CompressedWriteStream(buffer, 0) */
  public CompressedWriteStream(Address buffer) {
    this(buffer, 0);
  }

  /** In a real implementation there would need to be some way to
      allocate the buffer */
  public CompressedWriteStream(Address buffer, int position) {
    super(buffer, position);
  }
}
