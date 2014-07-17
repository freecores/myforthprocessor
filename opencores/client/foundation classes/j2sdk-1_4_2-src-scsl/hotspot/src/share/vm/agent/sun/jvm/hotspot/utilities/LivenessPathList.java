/*
 * @(#)LivenessPathList.java	1.3 03/01/23 11:51:40
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.utilities;

import java.util.*;

/** Simple container class for LivenessPaths */

public class LivenessPathList {
  public LivenessPathList() {
    list = new ArrayList();
  }

  public int size() {
    return list.size();
  }

  public LivenessPath get(int i) {
    return (LivenessPath) list.get(i);
  }

  void add(LivenessPath path) {
    list.add(path);
  }

  private ArrayList list;
}
