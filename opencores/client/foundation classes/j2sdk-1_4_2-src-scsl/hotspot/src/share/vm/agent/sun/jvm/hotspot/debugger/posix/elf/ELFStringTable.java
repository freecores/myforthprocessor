/*
 * @(#)ELFStringTable.java	1.4 03/01/23 11:30:28
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.posix.elf;

public interface ELFStringTable {
    public String get(int index);
    public int getNumStrings();
}
