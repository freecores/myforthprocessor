/*
 * @(#)OopField.java	1.5 03/01/23 11:43:48
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.oops;

import sun.jvm.hotspot.debugger.*;

// The class for an oop field simply provides access to the value.
public class OopField extends Field {
  public OopField(FieldIdentifier id, long offset, boolean isVMField) {
    super(id, offset, isVMField);
  }
   
  public OopField(sun.jvm.hotspot.types.OopField vmField, long startOffset) {
    super(new NamedFieldIdentifier(vmField.getName()), vmField.getOffset() + startOffset, true); 
  }

  public OopField(InstanceKlass holder, int fieldArrayIndex) {
    super(holder, fieldArrayIndex);
  }

  public Oop getValue(Oop obj) {
    return obj.getHeap().newOop(getValueAsOopHandle(obj));
  }

  /** Debugging support */
  public OopHandle getValueAsOopHandle(Oop obj) {
    return obj.getHandle().getOopHandleAt(getOffset());
  }

  public void setValue(Oop obj) throws MutationException {
    // Fix this: setOopAt is missing in Address
  }
}
