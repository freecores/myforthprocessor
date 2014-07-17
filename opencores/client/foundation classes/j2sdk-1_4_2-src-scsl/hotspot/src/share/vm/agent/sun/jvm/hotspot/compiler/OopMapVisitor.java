/*
 * @(#)OopMapVisitor.java	1.3 03/01/23 11:24:54
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.compiler;

import sun.jvm.hotspot.debugger.*;

/** Adaptation of the oop visitation mechanism to Java. */

public interface OopMapVisitor {
  public void visitOopLocation(Address oopAddr);
  public void visitDerivedOopLocation(Address baseOopAddr, Address derivedOopAddr);
  public void visitValueLocation(Address valueAddr);
  public void visitDeadLocation(Address deadAddr);
}
