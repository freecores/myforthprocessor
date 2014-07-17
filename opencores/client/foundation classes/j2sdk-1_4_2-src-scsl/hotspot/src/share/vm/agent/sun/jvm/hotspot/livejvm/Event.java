/*
 * @(#)Event.java	1.2 03/01/23 11:40:07
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.livejvm;

public class Event {
  public static class Type {
    private Type() {}
    public static final Type BREAKPOINT = new Type();
    public static final Type EXCEPTION  = new Type();
  }

  private Type type;

  public Event(Type type) {
    this.type = type;
  }

  public Type getType() { return type; }
}
