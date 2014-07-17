/*
 * @(#)CodeHeap.java	1.4 03/01/23 11:40:24
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.memory;

import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;

public class CodeHeap extends VMObject {
  private static Field         memoryField;
  private static Field         segmapField;
  //  private static CIntegerField numberOfCommittedSegmentsField;
  //  private static CIntegerField numberOfReservedSegmentsField;
  //  private static CIntegerField segmentSizeField;
  private static CIntegerField log2SegmentSizeField;
  //  private static CIntegerField nextSegmentField;
  //  private static AddressField  freelistField;
  //  private static CIntegerField freeSegmentsField;

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static void initialize(TypeDataBase db) {
    Type type = db.lookupType("CodeHeap");

    memoryField          = type.getField("_memory");
    segmapField          = type.getField("_segmap");
    log2SegmentSizeField = type.getCIntegerField("_log2_segment_size");
  }

  public CodeHeap(Address addr) {
    super(addr);
  }

  public Address begin() {
    return getMemory().low();
  }

  public Address end() {
    return getMemory().high();
  }

  public boolean contains(Address p) {
    return (begin().lessThanOrEqual(p) && end().greaterThan(p));
  }

  /** Returns the start of the block containing p or null */
  public Address findStart(Address p) {
    if (!contains(p)) return null;
    long i = segmentFor(p);
    Address b = getSegmentMap().low();
    if (b.getCIntegerAt(i, 1, true) == 0xFF) {
      return null;
    }
    while (b.getCIntegerAt(i, 1, true) > 0) {
      i -= b.getCIntegerAt(i, 1, true);
    }
    HeapBlock h = getBlockAt(i);
    if (h.isFree()) {
      return null;
    }
    return h.getAllocatedSpace();
  }

  //--------------------------------------------------------------------------------
  // Internals only below this point
  //

  private VirtualSpace getMemory() {
    return new VirtualSpace(addr.addOffsetTo(memoryField.getOffset()));
  }

  private VirtualSpace getSegmentMap() {
    return new VirtualSpace(addr.addOffsetTo(segmapField.getOffset()));
  }

  private long segmentFor(Address p) {
    return p.minus(getMemory().low()) >> getLog2SegmentSize();
  }

  private int getLog2SegmentSize() {
    return (int) log2SegmentSizeField.getValue(addr);
  }

  private HeapBlock getBlockAt(long i) {
    return (HeapBlock) VMObjectFactory.newObject(HeapBlock.class, getMemory().low().addOffsetTo(i << getLog2SegmentSize()));
  }
}
