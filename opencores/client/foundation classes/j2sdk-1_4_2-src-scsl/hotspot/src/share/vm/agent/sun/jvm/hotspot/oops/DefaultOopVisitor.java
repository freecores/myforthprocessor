/* 
 * @(#)DefaultOopVisitor.java	1.4 03/01/23 11:42:24
 * 
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.oops;

/** A DefaultOopVisitor implements basic no-op OopVisitor
    functionality except that the setObj() and getObj() methods are
    implemented properly. */

public class DefaultOopVisitor implements OopVisitor {
  private Oop obj;

  public void prologue()                        {}

  // Called after visiting an object
  public void epilogue()                        {}

  public void setObj(Oop obj) {
    this.obj = obj;
  }

  public Oop getObj() {
    return obj;
  }

  // Callback methods for each field type in an object
  public void doOop(OopField field, boolean isVMField)         {}
  public void doByte(ByteField field, boolean isVMField)       {}
  public void doChar(CharField field, boolean isVMField)       {}
  public void doBoolean(BooleanField field, boolean isVMField) {}
  public void doShort(ShortField field, boolean isVMField)     {}
  public void doInt(IntField field, boolean isVMField)         {}
  public void doLong(LongField field, boolean isVMField)       {}
  public void doFloat(FloatField field, boolean isVMField)     {}
  public void doDouble(DoubleField field, boolean isVMField)   {}
  public void doCInt(CIntField field, boolean isVMField)       {}
}
