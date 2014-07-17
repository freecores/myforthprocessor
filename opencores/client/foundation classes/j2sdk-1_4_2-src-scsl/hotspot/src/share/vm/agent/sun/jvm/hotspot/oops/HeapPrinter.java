/*
 * @(#)HeapPrinter.java	1.5 03/01/23 11:42:41
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.oops;

import java.io.*;

public class HeapPrinter implements HeapVisitor {

  public HeapPrinter(PrintStream tty) {
    oopPrinter = new OopPrinter(tty);
  }

  private OopPrinter oopPrinter;

  public void prologue(long size) {}

  public void doObj(Oop obj) { obj.iterate(oopPrinter, true); }

  public void epilogue() {}
}
