/*
 * @(#)ELFHashTable.java	1.4 03/01/23 11:30:19
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.posix.elf;

public interface ELFHashTable {
    /**
     * Returns the ELFSymbol that has the specified name or null if no symbol
     * with that name exists.  NOTE: Currently this method does not work and
     * willl always return null.
     */
    public ELFSymbol getSymbol(String symbolName);
}
