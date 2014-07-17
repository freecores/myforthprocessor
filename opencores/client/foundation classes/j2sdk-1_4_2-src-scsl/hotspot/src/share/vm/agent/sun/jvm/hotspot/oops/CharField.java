/*
 * @(#)CharField.java	1.4 03/01/23 11:41:59
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.oops;

import sun.jvm.hotspot.debugger.*;

// The class for a char field simply provides access to the value.
public class CharField extends Field {
  public CharField(FieldIdentifier id, long offset, boolean isVMField) {
    super(id, offset, isVMField);
  }

  public CharField(sun.jvm.hotspot.types.JCharField vmField, long startOffset) {
    super(new NamedFieldIdentifier(vmField.getName()), vmField.getOffset() + startOffset, true); 
  }

  public CharField(InstanceKlass holder, int fieldArrayIndex) {
    super(holder, fieldArrayIndex);
  }

  public char getValue(Oop obj) { return obj.getHandle().getJCharAt(getOffset()); }
  public void setValue(Oop obj, char value) throws MutationException {
    // Fix this: setJCharAt is missing in Address
  }
}

