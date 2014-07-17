/*
 * @(#)LinuxVtblAccess.java	1.2 03/01/23 11:14:08
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.types.*;
import sun.jvm.hotspot.types.basic.*;

public class LinuxVtblAccess extends BasicVtblAccess {
  public LinuxVtblAccess(SymbolLookup symbolLookup,
                         String[] dllNames) {
    super(symbolLookup, dllNames);
  }

  protected String vtblSymbolForType(Type type) {
    // Note: this is for gcc-2.9x. gcc-3.x uses different mangling rules
    return "__vt_" + type.getName().length() + type;
  }
}
