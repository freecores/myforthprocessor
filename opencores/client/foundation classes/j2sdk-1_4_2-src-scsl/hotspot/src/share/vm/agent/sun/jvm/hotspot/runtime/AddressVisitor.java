/*
 * @(#)AddressVisitor.java	1.3 03/01/23 11:44:22
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.runtime;

import sun.jvm.hotspot.debugger.*;

/** A generic interface for visiting addresses. Used by the frame/oop
    map iteration mechanisms. */

public interface AddressVisitor {
  public void visitAddress(Address addr);
}
