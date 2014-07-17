/*
 * @(#)BytecodeVisitor.java	1.2 03/01/23 11:37:22
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.interpreter;

import sun.jvm.hotspot.oops.Method;

public interface BytecodeVisitor {
   public void prologue(Method method);
   public void visit(Bytecode bytecode);
   public void epilogue();
}
