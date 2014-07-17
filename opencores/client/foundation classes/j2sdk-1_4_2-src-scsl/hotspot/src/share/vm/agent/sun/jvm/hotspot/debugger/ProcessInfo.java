/*
 * @(#)ProcessInfo.java	1.4 03/01/23 11:25:51
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger;

/** Simple wrapper class for name and process ID information.
    Hopefully this will be generic enough to be portable. */

public class ProcessInfo {
  public ProcessInfo(String name, int pid) {
    this.name = name;
    this.pid = pid;
  }

  public String getName() {
    return name;
  }

  public int getPid() {
    return pid;
  }

  private String name;
  private int pid;
}
