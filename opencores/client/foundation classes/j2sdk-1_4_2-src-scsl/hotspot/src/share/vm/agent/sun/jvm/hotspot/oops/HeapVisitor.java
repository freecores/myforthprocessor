/*
 * @(#)HeapVisitor.java	1.4 03/01/23 11:42:44
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.oops;

// A HeapVisitor is used for visiting all object in the heap

public interface HeapVisitor {
  // This is called at the beginning of the iteration to provide the
  // HeapVisitor with information about the amount of memory which
  // will be traversed (for example, for displaying a progress bar)
  public void prologue(long usedSize);

  // Callback method for each object
  public void doObj(Oop obj);

  // This is called after the traversal is complete
  public void epilogue();
};
