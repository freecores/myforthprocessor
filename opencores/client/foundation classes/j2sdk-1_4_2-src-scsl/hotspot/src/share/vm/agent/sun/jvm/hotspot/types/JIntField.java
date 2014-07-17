/*
 * @(#)JIntField.java	1.3 03/01/23 11:47:32
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.types;

import sun.jvm.hotspot.debugger.*;

/** A specialization of Field which represents a field containing a
    Java int value (in either a C/C++ data structure or a Java
    object) and which adds typechecked getValue() routines returning
    ints. */

public interface JIntField extends Field {
  /** The field must be nonstatic and the type of the field must be a
      Java int, or a WrongTypeException will be thrown. */
  public int getValue(Address addr) throws UnmappedAddressException, UnalignedAddressException, WrongTypeException;

  /** The field must be static and the type of the field must be a
      Java int, or a WrongTypeException will be thrown. */
  public int getValue() throws UnmappedAddressException, UnalignedAddressException, WrongTypeException;
}
