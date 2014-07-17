/*
 * @(#)OopPrinter.java	1.5 03/01/23 11:43:50
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.oops;

import java.io.*;

public class OopPrinter implements OopVisitor {
  public OopPrinter(PrintStream tty) {
    this.tty = tty;
  }

  PrintStream tty;

  public void prologue() {
    getObj().printValueOn(tty);
    tty.println(" (object size = " + getObj().getObjectSize() + ")");
  }

  public void epilogue() {
    tty.println();
  }


  private Oop obj;
  public void setObj(Oop obj) { this.obj = obj; }
  public Oop getObj() { return obj; }


  private void printField(Field field) {
    field.printOn(tty);
  }

  public void doOop(OopField field, boolean isVMField) {
    printField(field);
    Oop.printOopValueOn(field.getValue(getObj()), tty);
    tty.println();
  }
  public void doChar(CharField field, boolean isVMField) {
    printField(field);
    char c = field.getValue(getObj()); 
    // Fix me: not yet complete
    if (Character.isLetterOrDigit(c)) tty.println(c);
    else tty.println((int)c);
  }
  public void doByte(ByteField field, boolean isVMField) {
    printField(field);
    tty.println(field.getValue(getObj()));
  }
  public void doBoolean(BooleanField field, boolean isVMField) {
    printField(field);
    tty.println(field.getValue(getObj()));
  }
  public void doShort(ShortField field, boolean isVMField) {
    printField(field);
    tty.println(field.getValue(getObj()));
  }
  public void doInt(IntField field, boolean isVMField) {
    printField(field);
    tty.println(field.getValue(getObj()));
  }
  public void doLong(LongField field, boolean isVMField) {
    printField(field);
    tty.println(field.getValue(getObj()));
  }
  public void doFloat(FloatField field, boolean isVMField) {
    printField(field);
    tty.println(field.getValue(getObj()));
  }
  public void doDouble(DoubleField field, boolean isVMField) {
    printField(field);
    tty.println(field.getValue(getObj()));
  }
  public void doCInt(CIntField field, boolean isVMField) {
    printField(field);
    tty.println(field.getValue(getObj()));
  }
}
