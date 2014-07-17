/*
 * @(#)FieldIdentifier.java	1.3 03/01/23 11:42:31
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.oops;

import java.io.*;

// A FieldIdentifier describes a field in an Oop with a name
public class FieldIdentifier {

  public String getName() { return ""; } 

  public void printOn(PrintStream tty) {
    tty.print(" - " + getName() + ":\t");
  }

};
