/*
 * @(#)ObjectHistogramElement.java	1.5 03/01/23 11:43:43
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.oops;

import java.io.*;
import java.util.*;

public class ObjectHistogramElement {

  private Klass klass;
  private long  count; // Number of instances of klass
  private long  size;  // Total size of all these instances

  public ObjectHistogramElement(Klass k) {
     klass = k;
     count = 0;
     size  = 0;
  }

  public void updateWith(Oop obj) {
    count = count + 1;
    size  = size  + obj.getObjectSize();
  }

  public int compare(ObjectHistogramElement other) {
    return (int) (other.size - size);
  }

  /** Klass for this ObjectHistogramElement */
  public Klass getKlass() {
    return klass;
  }

  /** Number of instances of klass */
  public long getCount() {
    return count;
  }

  /** Total size of all these instances */
  public long getSize() {
    return size;
  }

    /** Human readable description **/
    public String getDescription() {
	ByteArrayOutputStream bos = new ByteArrayOutputStream();
	getKlass().printValueOn(new PrintStream(bos));
	return bos.toString();
    }


  public static void titleOn(PrintStream tty) {
    tty.println("Object Histogram:");
    tty.println();
    tty.println("Size" + "\t" + "Count" + "\t" + "Class description");
    tty.println("-------------------------------------------------------");
  }

  public void printOn(PrintStream tty) {
    tty.print(size + "\t" + count + "\t");
    klass.printValueOn(tty);
    tty.println();
  }
}
