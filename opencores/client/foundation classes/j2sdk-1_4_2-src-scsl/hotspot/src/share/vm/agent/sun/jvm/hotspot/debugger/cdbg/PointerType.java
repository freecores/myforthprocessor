/*
 * @(#)PointerType.java	1.4 03/01/23 11:27:30
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.cdbg;

public interface PointerType extends Type {
  public Type getTargetType();
}
