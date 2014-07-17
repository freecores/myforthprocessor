/*
 * @(#)Operand.java	1.2 03/01/23 11:15:12
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm;

import sun.jvm.hotspot.utilities.Assert;

/*
 * Operand is used as RTL argument. An Operand is either
 * a Number or a Register or an Address.  
*/

public abstract class Operand {
   // few type testers
   public boolean isAddress() {
      return false;
   }

   public boolean isImmediate() {
      return false;
   }

   public boolean isRegister() {
      return false;
   }
}
