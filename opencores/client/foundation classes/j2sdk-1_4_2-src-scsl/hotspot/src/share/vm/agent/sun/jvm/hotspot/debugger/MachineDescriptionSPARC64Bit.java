/*
 * @(#)MachineDescriptionSPARC64Bit.java	1.4 03/01/23 11:25:29
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger;

public class MachineDescriptionSPARC64Bit extends MachineDescriptionTwosComplement implements MachineDescription {
  public long getAddressSize() {
    return 8;
  }

  public long getOopSize() {
    return 8;
  }

  public boolean isBigEndian() {
    return true;
  }

  public boolean isLP64() {
    return true;
  }
}
