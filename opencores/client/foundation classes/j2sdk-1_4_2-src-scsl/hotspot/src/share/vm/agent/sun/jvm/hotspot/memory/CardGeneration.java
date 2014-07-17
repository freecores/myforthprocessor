/*
 * @(#)CardGeneration.java	1.3 03/01/23 11:40:22
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.memory;

import sun.jvm.hotspot.debugger.*;

/** Class CardGeneration is a generation that is covered by a card
    table, and uses a card-size block-offset array to implement
    block_start. */

public abstract class CardGeneration extends Generation {
  public CardGeneration(Address addr) {
    super(addr);
  }

  // FIXME: not sure what I need to expose from here in order to have
  // verification similar to that of the old RememberedSet
}
