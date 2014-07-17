/*
 * @(#)ResultTypeFinder.java	1.3 03/01/23 11:45:35
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.runtime;

import sun.jvm.hotspot.oops.*;

/** Specialized SignatureIterator: Used to compute the result type. */

public class ResultTypeFinder extends SignatureInfo {
  protected void set(int size, int type)    { if (isReturnType()) this.type = type; }
  public ResultTypeFinder(Symbol signature) { super(signature); }
}
