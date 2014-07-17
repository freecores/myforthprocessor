/*
 * @(#)Address.java	1.2 03/01/23 11:14:25
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm;

public abstract class Address extends Operand {
   public boolean isAddress() {
      return true;
   }

   public abstract String toString();
}
