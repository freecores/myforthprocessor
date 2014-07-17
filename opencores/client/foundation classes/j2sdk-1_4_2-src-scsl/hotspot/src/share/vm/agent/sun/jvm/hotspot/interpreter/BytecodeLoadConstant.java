/*
 * @(#)BytecodeLoadConstant.java	1.4 03/01/23 11:36:50
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.interpreter;

import sun.jvm.hotspot.oops.*;
import sun.jvm.hotspot.utilities.*;

public class BytecodeLoadConstant extends BytecodeWithCPIndex {
  BytecodeLoadConstant(Method method, int bci) {
    super(method, bci);
  }

  public int index() {
    return javaCode() == Bytecodes._ldc ?
                 (int) (0xFF & javaByteAt(1))
               : (int) (0xFFFF & javaShortAt(1));
  }

  public void verify() {
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(isValid(), "check load constant");
    }
  }

  public boolean isValid() {
    int jcode = javaCode();
    boolean codeOk = jcode == Bytecodes._ldc || jcode == Bytecodes._ldc_w || 
           jcode == Bytecodes._ldc2_w;
    if (! codeOk) return false;

    ConstantTag ctag = method().getConstants().getTagAt(index());
    if (jcode == Bytecodes._ldc2_w) {
       // has to be double or long
       return (ctag.isDouble() || ctag.isLong()) ? true: false;
    } else {
       // has to be int or float or String
       return (ctag.isUnresolvedString() || ctag.isString() 
               || ctag.isInt() || ctag.isFloat())? true: false;
    }
  }

  public static BytecodeLoadConstant at(Method method, int bci) {
    BytecodeLoadConstant b = new BytecodeLoadConstant(method, bci);
    if (Assert.ASSERTS_ENABLED) {
      b.verify();
    }
    return b;
  }

  /** Like at, but returns null if the BCI is not at ldc or ldc_w or ldc2_w  */
  public static BytecodeLoadConstant atCheck(Method method, int bci) {
    BytecodeLoadConstant b = new BytecodeLoadConstant(method, bci);
    return (b.isValid() ? b : null);
  }

  public static BytecodeLoadConstant at(BytecodeStream bcs) {
    return new BytecodeLoadConstant(bcs.method(), bcs.bci());
  }

  public String getConstantValue() {
    ConstantPool cpool = method().getConstants();
    int cpIndex = index();
    ConstantTag ctag = cpool.getTagAt(cpIndex);
    if (ctag.isInt()) {
       return "<int " + new Integer(cpool.getIntAt(cpIndex)).toString() +">";
    } else if (ctag.isLong()) {
       return "<long " + new Long(cpool.getLongAt(cpIndex)).toString() + "L>";
    } else if (ctag.isFloat()) {
       return "<float " + new Float(cpool.getFloatAt(cpIndex)).toString() + "F>";
    } else if (ctag.isDouble()) {
       return "<double " + new Double(cpool.getDoubleAt(cpIndex)).toString() + "D>";
    } else if (ctag.isString()) {
       Oop obj = cpool.getObjAt(cpIndex);
       return "<String \"" + OopUtilities.stringOopToString(obj) + "\">";
    } else if (ctag.isUnresolvedString()) {
       Symbol sym = cpool.getSymbolAt(cpIndex);
       return "<String \"" + sym.asString() + "\">";
    } else {
       if (Assert.ASSERTS_ENABLED) {
         Assert.that(false, "invalid load constant type");
       }
       return null;
    }
  }

  public String toString() {
    StringBuffer buf = new StringBuffer();
    buf.append(getJavaBytecodeName());
    buf.append(spaces);
    buf.append('#');
    buf.append(new Integer(index()).toString());
    buf.append(spaces);
    buf.append(getConstantValue());
    if (code() != javaCode()) {
       buf.append(spaces);
       buf.append('[');
       buf.append(getBytecodeName());
       buf.append(']');
    }
    return buf.toString();
  }
}
