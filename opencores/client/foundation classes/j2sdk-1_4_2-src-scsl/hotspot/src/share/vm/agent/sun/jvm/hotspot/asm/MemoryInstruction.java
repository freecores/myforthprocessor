/*
 * @(#)MemoryInstruction.java	1.2 03/01/23 11:15:08
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm;

public interface MemoryInstruction extends RTLDataTypes {
   public int getDataType(); // one of the RTLDataTypes.
   public boolean isConditional(); // conditional store like swap or v9 like non-faulting loads
}
