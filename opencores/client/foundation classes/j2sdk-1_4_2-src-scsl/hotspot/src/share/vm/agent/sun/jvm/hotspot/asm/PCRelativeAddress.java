/*
 * @(#)PCRelativeAddress.java	1.2 03/01/23 11:15:15
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm;

// address is specified as an offset from current PC

public class PCRelativeAddress extends IndirectAddress {
   private final long disp;

   public PCRelativeAddress(long disp) {
       this.disp = disp;
   }

   public String toString() {
      return new Long(disp).toString();
   }

   public long getDisplacement() {
      return disp;
   }
}
