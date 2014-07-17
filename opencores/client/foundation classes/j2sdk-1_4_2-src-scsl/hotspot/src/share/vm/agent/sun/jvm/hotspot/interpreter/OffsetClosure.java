/*
 * @(#)OffsetClosure.java	1.3 03/01/23 11:37:44
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.interpreter;

public interface OffsetClosure {
  public void offsetDo(int offset);
}
