/*
 * @(#)ConstIterator.java	1.3 03/01/23 11:51:16
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.utilities;

import java.util.Iterator;

/** An Iterator which wraps another and prevents the "remove" method
    from being called */

public class ConstIterator implements Iterator {
  private Iterator iter;

  public ConstIterator(Iterator iter) {
    this.iter = iter;
  }

  public boolean hasNext() {
    return iter.hasNext();
  }

  public Object next() {
    return iter.next();
  }

  public void remove() {
    throw new UnsupportedOperationException();
  }
}
