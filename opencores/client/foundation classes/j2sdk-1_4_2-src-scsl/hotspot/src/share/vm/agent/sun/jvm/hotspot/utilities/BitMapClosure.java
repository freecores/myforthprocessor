/*
 * @(#)BitMapClosure.java	1.3 03/01/23 11:51:10
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.utilities;

public interface BitMapClosure {
  /** Called when specified bit in map is set */
  public void doBit(int offset);
}
