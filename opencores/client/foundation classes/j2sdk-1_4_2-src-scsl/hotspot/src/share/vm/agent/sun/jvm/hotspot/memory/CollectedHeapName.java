/*
 * @(#)CollectedHeapName.java	1.3 03/01/23 11:40:29
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.memory;

/** Mimics the enums in the VM under CollectedHeap::Name */

public class CollectedHeapName {
  private String name;

  private CollectedHeapName(String name) { this.name = name; }

  public static final CollectedHeapName GENERATIONAL = new CollectedHeapName("GENERATIONAL");
  public static final CollectedHeapName OTHER        = new CollectedHeapName("OTHER");

  public String toString() {
    return name;
  }
}
