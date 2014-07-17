/*
 * @(#)Generation.java	1.9 03/01/23 11:40:48
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.memory;

import java.io.*;
import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.types.*;
import sun.jvm.hotspot.runtime.*;

/** <P> The (supported) Generation hierarchy currently looks like this: </P>

    <ul> 
    <li> Generation
      <ul>
      <li> CardGeneration
        <ul>
        <li> OneContigSpaceCardGeneration
          <ul>
          <li> CompactingPermGenGen
          <li> TenuredGeneration
          </ul>
        </ul>
      <li> DefNewGeneration
      </ul>
    </ul>
*/


public abstract class Generation extends VMObject {
  private static long          reservedFieldOffset;
  private static long          virtualSpaceFieldOffset;
  private static CIntegerField levelField;
  protected static final int  K = 1024;
  // Fields for class StatRecord
  private static Field         statRecordField;
  private static CIntegerField invocationField;

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) {
    Type type = db.lookupType("Generation");
    
    reservedFieldOffset     = type.getField("_reserved").getOffset();
    virtualSpaceFieldOffset = type.getField("_virtual_space").getOffset();
    levelField              = type.getCIntegerField("_level");
    // StatRecord
    statRecordField         = type.getField("_stat_record");
    type                    = db.lookupType("Generation::StatRecord");
    invocationField         = type.getCIntegerField("invocations");

  }

  public Generation(Address addr) {
    super(addr);
  }
  
  public GenerationName kind() {
    return GenerationName.OTHER;
  }

  public GenerationSpec spec() {
    return ((GenCollectedHeap) VM.getVM().getUniverse().heap()).spec(level());
  }

  public int level() {
    return (int) levelField.getValue(addr);
  }

  public int invocations() {
    return getStatRecord().getInvocations();
  }

  /** The maximum number of object bytes the generation can currently
      hold. */
  public abstract long capacity();

  /** The number of used bytes in the gen. */
  public abstract long used();

  /** The number of free bytes in the gen. */
  public abstract long free();

  /** The largest number of contiguous free words in the generation,
      including expansion. (VM's version assumes it is called at a
      safepoint.)  */
  public abstract long contiguousAvailable();

  public MemRegion reserved() {
    return new MemRegion(addr.addOffsetTo(reservedFieldOffset));
  }

  /** Returns a region guaranteed to contain all the objects in the
      generation. */
  public MemRegion usedRegion() {
    return reserved();
  }

  /* Returns "TRUE" iff "p" points into an allocated object in the
     generation. */
  public boolean isIn(Address p) {
    GenerationIsInClosure blk = new GenerationIsInClosure(p);
    spaceIterate(blk);
    return (blk.space() != null);
  }

  /** Returns "TRUE" iff "p" points into the reserved area of the
     generation. */
  public boolean isInReserved(Address p) {
    return reserved().contains(p);
  }

  protected VirtualSpace virtualSpace() {
    return (VirtualSpace) VMObjectFactory.newObject(VirtualSpace.class, addr.addOffsetTo(virtualSpaceFieldOffset));
  }

  public abstract String name();

  /** Equivalent to spaceIterate(blk, false) */
  public void spaceIterate(SpaceClosure blk) {
    spaceIterate(blk, false);
  }

  /** Iteration - do not use for time critical operations */
  public abstract void spaceIterate(SpaceClosure blk, boolean usedOnly);

  public void print() { printOn(System.out); }
  public abstract void printOn(PrintStream tty);

  public static class StatRecord extends VMObject {
    public StatRecord(Address addr) {
      super(addr);
    }

    public int getInvocations() {
      return (int) invocationField.getValue(addr);
    }

  }
  
  private StatRecord getStatRecord() {
    return (StatRecord) VMObjectFactory.newObject(Generation.StatRecord.class, addr.addOffsetTo(statRecordField.getOffset()));
  }
}
