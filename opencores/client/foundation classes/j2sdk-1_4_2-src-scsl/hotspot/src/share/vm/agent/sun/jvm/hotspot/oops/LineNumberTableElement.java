/*
 * @(#)LineNumberTableElement.java	1.3 03/01/23 11:43:03
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.oops;

import java.io.*;
import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.interpreter.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;
import sun.jvm.hotspot.utilities.*;

public class LineNumberTableElement {
  private int start_bci;
  private int line_number;

  public LineNumberTableElement(int start_bci, int line_number) {
    this.start_bci = start_bci;
    this.line_number = line_number;
  }

  public int getStartBCI() {
    return this.start_bci;
  }

  public int getLineNumber() {
    return this.line_number;
  }

}
