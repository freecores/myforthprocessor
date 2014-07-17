/*
 * @(#)DebugInfoReadStream.java	1.3 03/01/23 11:23:53
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.code;

import sun.jvm.hotspot.debugger.*;

public class DebugInfoReadStream extends CompressedReadStream {
  private NMethod code;

  public DebugInfoReadStream(NMethod code, int offset) {
    super(code.scopesDataBegin(), offset);
    this.code = code;
  }

  public OopHandle readOopHandle() {
    return code.getOopAt(readInt());
  }
}
