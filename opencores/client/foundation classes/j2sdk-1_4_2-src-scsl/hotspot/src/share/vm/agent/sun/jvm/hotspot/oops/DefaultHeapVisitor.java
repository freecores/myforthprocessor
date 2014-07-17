/*
 * @(#)DefaultHeapVisitor.java	1.3 03/01/23 11:42:22
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.oops;

/** A DefaultHeapVisitor implements basic no-op HeapVisitor
    functionality. */

public class DefaultHeapVisitor implements HeapVisitor {
  public void prologue(long usedSize) {}
  public void doObj(Oop obj)          {}
  public void epilogue()              {}
}
