/*
 * @(#)RBColor.java	1.3 03/01/23 11:51:58
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.utilities;

/** Type-safe enum for the colors in a red-black tree. */

public class RBColor {
  public static final RBColor RED   = new RBColor("red");
  public static final RBColor BLACK = new RBColor("black");

  public String getName() {
    return name;
  }

  private RBColor(String name) {
    this.name = name;
  }
  private String name;
}
