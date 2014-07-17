/*
 * @(#)ShortField.java	1.4 03/01/23 11:44:00
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.oops;

import sun.jvm.hotspot.debugger.*;

// The class for a short field simply provides access to the value.
public class ShortField extends Field {
  public ShortField(FieldIdentifier id, long offset, boolean isVMField) {
    super(id, offset, isVMField);
  }

  public ShortField(sun.jvm.hotspot.types.JShortField vmField, long startOffset) {
    super(new NamedFieldIdentifier(vmField.getName()), vmField.getOffset(), true); 
  }

  public ShortField(InstanceKlass holder, int fieldArrayIndex) {
    super(holder, fieldArrayIndex);
  }

  public short getValue(Oop obj) { return obj.getHandle().getJShortAt(getOffset()); }
  public void setValue(Oop obj, short value) throws MutationException {
    // Fix this: setJFloatAt is missing in Address
  }
}

