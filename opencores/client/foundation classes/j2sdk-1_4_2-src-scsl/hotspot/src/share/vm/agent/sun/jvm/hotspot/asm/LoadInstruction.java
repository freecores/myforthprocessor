/*
 * @(#)LoadInstruction.java	1.2 03/01/23 11:15:03
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm;

public interface LoadInstruction extends MemoryInstruction {
   public Address  getLoadSource();
   public Register[] getLoadDestinations();
}
