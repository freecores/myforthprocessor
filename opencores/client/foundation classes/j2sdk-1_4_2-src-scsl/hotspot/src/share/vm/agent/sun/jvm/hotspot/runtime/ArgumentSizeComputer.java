/*
 * @(#)ArgumentSizeComputer.java	1.3 03/01/23 11:44:25
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.runtime;

import sun.jvm.hotspot.oops.*;

/** Specialized SignatureIterator: Used to compute the argument size. */

public class ArgumentSizeComputer extends SignatureInfo {
  protected void set(int size, int type)        { if (!isReturnType()) this.size += size; }
  public ArgumentSizeComputer(Symbol signature) { super(signature); }
}
