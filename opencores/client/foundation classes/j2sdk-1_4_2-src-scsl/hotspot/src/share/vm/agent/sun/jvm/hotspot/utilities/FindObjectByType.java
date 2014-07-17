/* 
 * @(#)FindObjectByType.java	1.3 03/01/23 11:51:21
 * 
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.utilities;

import java.util.*;

import sun.jvm.hotspot.oops.*;

public class FindObjectByType implements HeapVisitor {
  private Klass type;
  private List results = new ArrayList();

  public FindObjectByType(Klass type) {
    this.type = type;
  }
  
  /** Returns a List of Oops */
  public List getResults() {
    return results;
  }

  public void prologue(long size) {}
  public void epilogue()          {}

  public void doObj(Oop obj) {
    if (obj.getKlass().equals(type)) {
      results.add(obj);
    }
  }
}
