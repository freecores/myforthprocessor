/*
 * @(#)RefType.java	1.4 03/01/23 11:27:35
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.cdbg;

public interface RefType extends Type {
  public Type getTargetType();
}
