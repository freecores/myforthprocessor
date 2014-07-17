/*
 * @(#)StoreInstruction.java	1.2 03/01/23 11:15:29
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm;

public interface StoreInstruction extends MemoryInstruction {
   public Register[]  getStoreSources();
   public Address     getStoreDestination();
}
