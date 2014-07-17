/*
 * @(#)BasicVtblAccess.java	1.2 03/01/23 11:48:28
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.types.basic;

import java.util.*;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.types.*;

public abstract class BasicVtblAccess implements VtblAccess {
  protected SymbolLookup symbolLookup;
  protected String[] dllNames;

  private Map typeToVtblMap = new HashMap();

  public BasicVtblAccess(SymbolLookup symbolLookup,
                         String[] dllNames) {
    this.symbolLookup = symbolLookup;
    this.dllNames = dllNames;
  }

  public Address getVtblForType(Type type) {
    if (type == null) {
      return null;
    }
    Address addr = (Address) typeToVtblMap.get(type);
    if (addr != null) {
      return addr;
    }
    String vtblSymbol = vtblSymbolForType(type);
    for (int i = 0; i < dllNames.length; i++) {
      addr = symbolLookup.lookup(dllNames[i], vtblSymbol);
      if (addr != null) {
        typeToVtblMap.put(type, addr);
        return addr;
      }
    }
    return null;
  }

  public void clearCaches() {
    typeToVtblMap.clear();
  }

  protected abstract String vtblSymbolForType(Type type);
}
