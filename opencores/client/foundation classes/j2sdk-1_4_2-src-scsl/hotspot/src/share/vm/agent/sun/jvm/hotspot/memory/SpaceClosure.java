/*
 * @(#)SpaceClosure.java	1.3 03/01/23 11:41:16
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.memory;

public interface SpaceClosure {
  public void doSpace(Space s);
}
