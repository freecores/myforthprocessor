/*
 * @(#)IndexableFieldIdentifier.java	1.4 03/01/23 11:42:47
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.oops;

import sun.jvm.hotspot.*;
import sun.jvm.hotspot.types.*;
import java.io.*;

// An IndexableFieldIdentifier describes a field in an Oop accessed by an index

public class IndexableFieldIdentifier extends FieldIdentifier {

  public IndexableFieldIdentifier(int index) {
    this.index = index;
  }

  private int index;

  public int getIndex() { return index; } 

  public String getName() { return Integer.toString(getIndex()); }

  public void printOn(PrintStream tty) {
    tty.print(" - " + getIndex() + ":\t");
  }

  public boolean equals(Object obj) {
    if (obj == null) {
      return false;
    }
    
    if (!(obj instanceof IndexableFieldIdentifier)) {
      return false;
    }

    return (((IndexableFieldIdentifier) obj).getIndex() == index);
  }
};
