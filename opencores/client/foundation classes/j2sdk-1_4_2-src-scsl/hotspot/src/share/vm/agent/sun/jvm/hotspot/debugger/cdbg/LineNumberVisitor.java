/*
 * @(#)LineNumberVisitor.java	1.3 03/01/23 11:27:15
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.cdbg;

public interface LineNumberVisitor {
  public void doLineNumber(LineNumberInfo info);
}
