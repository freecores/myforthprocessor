/*
 * @(#)CIntField.java	1.3 03/01/23 11:41:53
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.oops;

import sun.jvm.hotspot.debugger.*;

// The class for an C int field simply provides access to the value.
public class CIntField extends Field {

  public CIntField(sun.jvm.hotspot.types.CIntegerField vmField, long startOffset) {
    super(new NamedFieldIdentifier(vmField.getName()), vmField.getOffset() + startOffset, true); 
    size       = vmField.getSize();
    isUnsigned = ((sun.jvm.hotspot.types.CIntegerType) vmField.getType()).isUnsigned(); 
  }

  private long size;
  private boolean isUnsigned;

  public long getValue(Oop obj) {
    return obj.getHandle().getCIntegerAt(getOffset(), size, isUnsigned);
  }
  public void setValue(Oop obj, long value) throws MutationException {
    // Fix this: set* missing in Address
  }
}

