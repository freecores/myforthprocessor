/*
 * @(#)MachineDescriptionSPARC32Bit.java	1.3 03/01/23 11:25:26
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger;

public class MachineDescriptionSPARC32Bit extends MachineDescriptionTwosComplement implements MachineDescription {
  public long getAddressSize() {
    return 4;
  }

  public long getOopSize() {
    return 4;
  }

  public boolean isBigEndian() {
    return true;
  }
}
