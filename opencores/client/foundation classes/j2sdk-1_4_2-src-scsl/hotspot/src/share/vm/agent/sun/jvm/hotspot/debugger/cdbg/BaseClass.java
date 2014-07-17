/*
 * @(#)BaseClass.java	1.3 03/01/23 11:26:18
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.cdbg;

/** Models a base class; effectively just provides the "virtual"
    keyword as well as public/private derivation information. */
public interface BaseClass {
  /** See {@link sun.jvm.hotspot.debugger.cdbg.AccessControl} */
  public int     getAccessControl();
  public boolean isVirtual();
  public Type    getType();
}
