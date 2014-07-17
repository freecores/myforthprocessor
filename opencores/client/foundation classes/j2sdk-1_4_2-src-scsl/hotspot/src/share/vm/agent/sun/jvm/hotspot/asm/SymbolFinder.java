/*
 * @(#)SymbolFinder.java	1.2 03/01/23 11:15:32
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm;

// SymbolFinder gets Symbol for a given address.

public interface SymbolFinder {
   // return address value as hex string if no symbol associated with the given address.
   public String getSymbolFor(long address);
}
