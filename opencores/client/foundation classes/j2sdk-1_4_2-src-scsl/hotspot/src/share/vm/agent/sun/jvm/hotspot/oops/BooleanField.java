/*
 * @(#)BooleanField.java	1.4 03/01/23 11:41:47
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.oops;

import sun.jvm.hotspot.debugger.*;

// The class for a boolean field simply provides access to the value.
public class BooleanField extends Field {
  public BooleanField(FieldIdentifier id, long offset, boolean isVMField) {
    super(id, offset, isVMField);
  }

  public BooleanField(sun.jvm.hotspot.types.JBooleanField vmField, long startOffset) {
    super(new NamedFieldIdentifier(vmField.getName()), vmField.getOffset() + startOffset, true); 
  }

  public BooleanField(InstanceKlass holder, int fieldArrayIndex) {
    super(holder, fieldArrayIndex);
  }

  public boolean getValue(Oop obj) { return obj.getHandle().getJBooleanAt(getOffset()); }
  public void setValue(Oop obj, boolean value) throws MutationException {
    // Fix this: setJBooleanAt is missing in Address
  }
}

