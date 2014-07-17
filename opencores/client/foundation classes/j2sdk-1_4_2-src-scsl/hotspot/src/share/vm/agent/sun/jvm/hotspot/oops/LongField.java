/*
 * @(#)LongField.java	1.4 03/01/23 11:43:10
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.oops;

import sun.jvm.hotspot.debugger.*;

// The class for a long field simply provides access to the value.
public class LongField extends Field {
  public LongField(FieldIdentifier id, long offset, boolean isVMField) {
    super(id, offset, isVMField);
  }

  public LongField(sun.jvm.hotspot.types.JLongField vmField, long startOffset) {
    super(new NamedFieldIdentifier(vmField.getName()), vmField.getOffset() + startOffset, true); 
  }

  public LongField(InstanceKlass holder, int fieldArrayIndex) {
    super(holder, fieldArrayIndex);
  }

  public long getValue(Oop obj) { return obj.getHandle().getJLongAt(getOffset()); }
  public void setValue(Oop obj, long value) {
    // Fix this: setJLongAt is missing in Address
  }
}

