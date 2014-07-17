/*
 * @(#)MachineDescriptionIntelX86.java	1.3 03/01/23 11:25:24
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger;

public class MachineDescriptionIntelX86 extends MachineDescriptionTwosComplement implements MachineDescription {
  public long getAddressSize() {
    return 4;
  }

  public long getOopSize() {
    return 4;
  }

  public boolean isBigEndian() {
    return false;
  }
}
