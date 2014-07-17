/*
 * @(#)MemoizedFloat.java	1.3 03/01/23 11:52:33
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.utilities.memo;

/** A memoized float. Override {@link #computeValue} in subclasses;
    call {@link #getValue} in using code. */

public abstract class MemoizedFloat {
  private boolean computed;
  private float value;

  /** Should compute the value of this memoized object. This will only
      be called once, upon the first call to {@link #getValue}. */
  protected abstract float computeValue();

  /** Public accessor for the memoized value. */
  public float getValue() {
    if (!computed) {
      value = computeValue();
      computed = true;
    }
    return value;
  }
}
