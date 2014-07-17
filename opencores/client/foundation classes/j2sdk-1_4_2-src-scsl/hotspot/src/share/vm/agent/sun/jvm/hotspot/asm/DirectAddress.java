/*
 * @(#)DirectAddress.java	1.2 03/01/23 11:14:43
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm;

public class DirectAddress extends Address {
   private long value;
   public DirectAddress(long value) {
      this.value = value;
   }

   public long getValue() {
      return value;
   }

   public String toString() {
      return Long.toHexString(value);
   }
}
