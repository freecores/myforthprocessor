/*
 * @(#)JCharField.java	1.3 03/01/23 11:47:25
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.types;

import sun.jvm.hotspot.debugger.*;

/** A specialization of Field which represents a field containing a
    Java char value (in either a C/C++ data structure or a Java
    object) and which adds typechecked getValue() routines returning
    chars. */

public interface JCharField extends Field {
  /** The field must be nonstatic and the type of the field must be a
      Java char, or a WrongTypeException will be thrown. */
  public char getValue(Address addr) throws UnmappedAddressException, UnalignedAddressException, WrongTypeException;

  /** The field must be static and the type of the field must be a
      Java char, or a WrongTypeException will be thrown. */
  public char getValue() throws UnmappedAddressException, UnalignedAddressException, WrongTypeException;
}