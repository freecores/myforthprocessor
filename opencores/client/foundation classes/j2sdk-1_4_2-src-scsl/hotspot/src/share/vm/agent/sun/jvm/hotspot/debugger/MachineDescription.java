/*
 * @(#)MachineDescription.java	1.5 03/01/23 11:25:21
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger;

import java.io.Serializable;

/** Encapsulates machine-specific information that currently must be
    exported up to the Java level. Implementations must be
    serializable. */

public interface MachineDescription extends Serializable {
  /** Returns the size of an address in bytes. Currently needed to be
      able to traverse arrays of pointers or oops. */
  public long getAddressSize();

  /** Returns the size of an address in bytes. Currently needed to be
      able to traverse arrays of pointers or oops. (FIXME: since we're
      already reading the Java primitive types' sizes from the remote
      VM, it would be nice to remove this routine, using a similar
      mechanism to how the TypeDataBase deals with primitive types.) */
  public long getOopSize();

  /** Returns the maximum value of the C integer type with the given
      size in bytes and signedness. Throws IllegalArgumentException if
      the size in bytes is not legal for a C type (or can not be
      handled by this system). Note that the current implementation
      does not currently handle unsigned 8-byte longs properly. */
  public long cIntegerTypeMaxValue(long sizeInBytes, boolean isUnsigned);

  /** Returns the minimum value of the C integer type with the given
      size in bytes and signedness. Throws IllegalArgumentException if
      the size in bytes is not legal for a C type (or can not be
      handled by this system). */
  public long cIntegerTypeMinValue(long sizeInBytes, boolean isUnsigned);

  /** Indicates whether the CPU is big- or little-endian. This
      information is typically only needed by the Debugger
      implementation. */
  public boolean isBigEndian();

  /** Indicates whether the underlying machine supports the LP64 data
      model (currently only SPARC/64). */
  public boolean isLP64();
}
