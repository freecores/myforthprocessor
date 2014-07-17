/*
 * @(#)AuxFileRecord.java	1.3 03/01/23 11:32:17
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.win32.coff;

/** Describes an Auxiliary File record, which follows a symbol with
    storage class FILE. The symbol name itself should be <B>.file</B>.
    (Some of the descriptions are taken directly from Microsoft's
    documentation and are copyrighted by Microsoft.)  */

public interface AuxFileRecord extends AuxSymbolRecord {
  public String getName();
}
