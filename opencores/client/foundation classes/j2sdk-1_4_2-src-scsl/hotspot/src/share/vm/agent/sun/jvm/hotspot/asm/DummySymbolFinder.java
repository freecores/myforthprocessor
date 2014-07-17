/*
 * @(#)DummySymbolFinder.java	1.2 03/01/23 11:14:48
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm;

public class DummySymbolFinder implements SymbolFinder {
   public String getSymbolFor(long address) {
      return "0x" + Long.toHexString(address);
   }
}
