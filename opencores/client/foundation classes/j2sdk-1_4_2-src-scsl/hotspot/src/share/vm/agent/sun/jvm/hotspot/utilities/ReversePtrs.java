/*
 * @(#)ReversePtrs.java	1.1 02/10/07 16:10:34
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.utilities;

import java.util.*;
import sun.jvm.hotspot.oops.*;

/** ReversePtrs hashtable. */

public class ReversePtrs  {
  private HashMap rp;

  public ReversePtrs() {
    rp = new HashMap();
  }

  public void put(LivenessPathElement from, Oop to) {
    if (to == null) return;
    ArrayList al = (ArrayList) rp.get((Object) to);
    if (al == null) al = new ArrayList();
    al.add((Object) from);
    rp.put((Object) to, (Object) al);
  }

  public ArrayList get(Oop obj) {
    return (ArrayList) rp.get((Object)obj);
  }
}
