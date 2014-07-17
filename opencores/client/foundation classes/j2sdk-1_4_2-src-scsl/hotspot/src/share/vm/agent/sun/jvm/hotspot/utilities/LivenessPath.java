/*
 * @(#)LivenessPath.java	1.3 03/01/23 11:51:35
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.utilities;

import java.util.*;
import sun.jvm.hotspot.oops.*;

/** Describes a path from a root to an object. Elements of the path
    are (object, field) pairs, where the object is expressed as a
    @link{sun.jvm.hotspot.oops.Oop}, and where the field is expressed
    as a @link{sun.jvm.hotspot.oops.FieldIdentifier}. If the element
    reflects a root, the Oop will be null. If the element is the end
    of the path, the FieldIdentifier will be null. */

public class LivenessPath {
  LivenessPath() {
    stack = new Stack();
  }
  
  /** Number of elements in the path */
  public int size() {
    return stack.size();
  }

  /** Fetch the element at the given index; 0-based */
  public LivenessPathElement get(int index) throws ArrayIndexOutOfBoundsException {
    return (LivenessPathElement) stack.get(index);
  }

  // Convenience routine for LivenessAnalysis
  void push(LivenessPathElement el) {
    stack.push(el);
  }

  // Convenience routine for LivenessAnalysis
  void pop() {
    stack.pop();
  }

  // Make a copy of the contents of the path -- the
  // LivenessPathElements are not duplicated, only the containing path
  LivenessPath copy() {
    LivenessPath dup = new LivenessPath();
    for (int i = 0; i < stack.size(); i++) {
      dup.stack.push(stack.get(i));
    }
    return dup;
  }

  //---------------------------------------------------------------------------
  // Internals only below this point
  //
  private Stack stack;
}
