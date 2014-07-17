/*
 * @(#)CIntegerField.java	1.4 03/01/23 11:47:13
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.types;

import sun.jvm.hotspot.debugger.*;

/** A specialization of Field which represents a field referring to a
    C integer value. See CIntegerType for a discussion of C integer
    types and why this class is not specialized for various int sizes
    or signed/unsigned. */

public interface CIntegerField extends Field {
  public boolean isUnsigned();

  /** The field must be nonstatic and of integer type, or a
      WrongTypeException will be thrown. */
  public long getValue(Address addr) throws UnmappedAddressException, UnalignedAddressException, WrongTypeException;

  /** The field must be static and of integer type, or a
      WrongTypeException will be thrown. */
  public long getValue() throws UnmappedAddressException, UnalignedAddressException, WrongTypeException;
}
