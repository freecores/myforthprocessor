/*
 * @(#)BasicJByteField.java	1.3 03/01/23 11:48:05
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.types.basic;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.types.*;

/** A specialization of Field which represents a field containing a
    Java byte value (in either a C/C++ data structure or a Java
    object) and which adds typechecked getValue() routines returning
    bytes. */

public class BasicJByteField extends BasicField implements JByteField {
  public BasicJByteField(BasicTypeDataBase db, Type containingType, String name, Type type,
                         boolean isStatic, long offset, Address staticFieldAddress) {
    super(db, containingType, name, type, isStatic, offset, staticFieldAddress);

    if (!type.equals(db.getJByteType())) {
      throw new WrongTypeException("Type of a BasicJByteField must be db.getJByteType()");
    }
  }

  /** The field must be nonstatic and the type of the field must be a
      Java byte, or a WrongTypeException will be thrown. */
  public byte getValue(Address addr) throws UnmappedAddressException, UnalignedAddressException, WrongTypeException {
    return getJByte(addr);
  }

  /** The field must be static and the type of the field must be a
      Java byte, or a WrongTypeException will be thrown. */
  public byte getValue() throws UnmappedAddressException, UnalignedAddressException, WrongTypeException {
    return getJByte();
  }
}
