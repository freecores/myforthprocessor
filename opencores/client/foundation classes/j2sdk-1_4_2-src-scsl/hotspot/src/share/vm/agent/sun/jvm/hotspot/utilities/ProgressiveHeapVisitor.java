/* 
 * @(#)ProgressiveHeapVisitor.java	1.3 03/01/23 11:51:56
 * 
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.utilities;

import sun.jvm.hotspot.memory.*;
import sun.jvm.hotspot.oops.*;

/** This class wraps a user's chosen HeapVisitor with the
    functionality that a chosen "thunk" is called periodically during
    the heap traversal. This allows a progress bar to be displayed
    during long heap scans. */

public class ProgressiveHeapVisitor implements HeapVisitor {
  private HeapVisitor userHeapVisitor;
  private Thunk thunk;
  private long usedSize;
  private long visitedSize;
  private double lastNotificationFraction;
  private static double MINIMUM_NOTIFICATION_FRACTION = 0.01;

  public interface Thunk {
    // This will be called with a number between 0 and 1
    public void heapIterationPercentageUpdate(double fractionVisited);

    // This will be called after the iteration is complete
    public void heapIterationComplete();
  }

  public ProgressiveHeapVisitor(HeapVisitor userHeapVisitor,
                                Thunk thunk) {
    this.userHeapVisitor = userHeapVisitor;
    this.thunk = thunk;
  }

  public void prologue(long usedSize) {
    this.usedSize = usedSize;
    visitedSize = 0;
    userHeapVisitor.prologue(usedSize);
  }

  public void doObj(Oop obj) {
    userHeapVisitor.doObj(obj);
    visitedSize += obj.getObjectSize();
    double curFrac = (double) visitedSize / (double) usedSize;
    if (curFrac > lastNotificationFraction + MINIMUM_NOTIFICATION_FRACTION) {
      thunk.heapIterationPercentageUpdate(curFrac);
      lastNotificationFraction = curFrac;
    }
  }
  
  public void epilogue() {
    userHeapVisitor.epilogue();
    thunk.heapIterationComplete();
  }
}
