/*
 * @(#)CodeBlob.java	1.12 03/01/23 11:23:31
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.code;

import java.io.*;
import java.util.*;

import sun.jvm.hotspot.asm.x86.*;
import sun.jvm.hotspot.compiler.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;
import sun.jvm.hotspot.utilities.*;

public class CodeBlob extends VMObject {
  private static CIntegerField sizeField;
  private static CIntegerField headerSizeField;
  private static CIntegerField relocationSizeField;
  private static CIntegerField instructionsOffsetField;
  private static CIntegerField dataOffsetField;
  private static CIntegerField oopsOffsetField;
  private static CIntegerField oopsLengthField;
  private static CIntegerField frameSizeField;
  private static AddressField  oopMapsField;

  // Only used by server compiler on x86; computed over in SA rather
  // than relying on computation in target VM
  private static final int     NOT_YET_COMPUTED = -2;
  private static final int     UNDEFINED        = -1;
  private              int     linkOffset       = NOT_YET_COMPUTED;
  private static       int     matcherInterpreterFramePointerReg;

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static void initialize(TypeDataBase db) {
    Type type = db.lookupType("CodeBlob");

    sizeField                = type.getCIntegerField("_size");
    headerSizeField          = type.getCIntegerField("_header_size");
    relocationSizeField      = type.getCIntegerField("_relocation_size");
    instructionsOffsetField  = type.getCIntegerField("_instructions_offset");
    dataOffsetField          = type.getCIntegerField("_data_offset");
    oopsOffsetField          = type.getCIntegerField("_oops_offset");
    oopsLengthField          = type.getCIntegerField("_oops_length");
    frameSizeField           = type.getCIntegerField("_frame_size");
    oopMapsField             = type.getAddressField("_oop_maps");

    if (VM.getVM().isServerCompiler()) {
      matcherInterpreterFramePointerReg =
        db.lookupIntConstant("Matcher::interpreter_frame_pointer_reg").intValue();
    }
  }

  public CodeBlob(Address addr) {
    super(addr);
  }

  // Typing
  public boolean isBufferBlob()         { return false; }
  public boolean isNMethod()            { return false; }
  public boolean isRuntimeStub()        { return false; }
  public boolean isI2CAdapter()         { return false; }
  public boolean isC2IAdapter()         { return false; }
  public boolean isOSRAdapter()         { return false; }
  public boolean isDeoptimizationStub() { return false; }
  public boolean isUncommonTrapStub()   { return false; }
  public boolean isExceptionStub()      { return false; }
  public boolean isSafepointStub()      { return false; }

  // Fine grain nmethod support: isNmethod() == isJavaMethod() || isNativeMethod() || isOSRMethod()
  public boolean isJavaMethod()         { return false; }
  public boolean isNativeMethod()       { return false; }
  /** On-Stack Replacement method */
  public boolean isOSRMethod()          { return false; }

  // Boundaries
  public Address headerBegin() {
    return addr;
  }

  public Address headerEnd() {
    return addr.addOffsetTo(headerSizeField.getValue(addr));
  }

  // FIXME: add RelocInfo
  //  public RelocInfo relocationBegin();
  //  public RelocInfo relocationEnd();

  public Address instructionsBegin() {
    return headerBegin().addOffsetTo(instructionsOffsetField.getValue(addr));
  }

  public Address instructionsEnd() {
    return headerBegin().addOffsetTo(dataOffsetField.getValue(addr));
  }

  public Address dataBegin() {
    return headerBegin().addOffsetTo(dataOffsetField.getValue(addr));
  }

  public Address dataEnd() {
    return headerBegin().addOffsetTo(sizeField.getValue(addr));
  }

  public Address oopsBegin() {
    return headerBegin().addOffsetTo(oopsOffsetField.getValue(addr));
  }

  public Address oopsEnd() {
    return oopsBegin().addOffsetTo(getOopsLength());
  }

  // Offsets
  public int getRelocationOffset()   { return (int) headerSizeField.getValue(addr);         }
  public int getInstructionsOffset() { return (int) instructionsOffsetField.getValue(addr); }
  public int getDataOffset()         { return (int) dataOffsetField.getValue(addr);         }
  public int getOopsOffset()         { return (int) oopsOffsetField.getValue(addr);         }

  // Sizes
  public int getSize()             { return (int) sizeField.getValue(addr);                     }
  public int getHeaderSize()       { return (int) headerSizeField.getValue(addr);               }
  // FIXME: add getRelocationSize()
  public int getInstructionsSize() { return (int) instructionsEnd().minus(instructionsBegin()); }
  public int getDataSize()         { return (int) dataEnd().minus(dataBegin());                 }
  
  // Containment
  public boolean blobContains(Address addr)         { return headerBegin().lessThanOrEqual(addr) && dataEnd().greaterThan(addr);               }
  // FIXME: add relocationContains
  public boolean instructionsContains(Address addr) { return instructionsBegin().lessThanOrEqual(addr) && instructionsEnd().greaterThan(addr); }
  public boolean dataContains(Address addr)         { return dataBegin().lessThanOrEqual(addr) && dataEnd().greaterThan(addr);                 }
  public boolean oopsContains(Address addr)         { return oopsBegin().lessThanOrEqual(addr) && oopsEnd().greaterThan(addr);                 }
  public boolean contains(Address addr)             { return instructionsContains(addr);                                                       }
  
  /** Support for oops in scopes and relocs. Note: index 0 is reserved for null. */
  public OopHandle getOopAt(int index) {
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(index > 0 && index <= getOopsLength(), "must be a valid non-zero index");
    }
    return oopsBegin().getOopHandleAt((index - 1) * VM.getVM().getOopSize());
  }

  // Reclamation support (really only used by the nmethods, but in order to get asserts to work
  // in the CodeCache they are defined virtual here)
  public boolean isZombie()             { return false; }
  public boolean isLockedByVM()         { return false; }

  /** OopMap for frame; can return null if none available */
  public OopMapSet getOopMaps() {
    Address oopMapsAddr = oopMapsField.getValue(addr);
    if (oopMapsAddr == null) {
      return null;
    }
    return new OopMapSet(oopMapsAddr);
  }
  // FIXME: not yet implementable
  //  void set_oop_maps(OopMapSet* p);

  public OopMap getOopMapForReturnAddress(Address returnAddress, boolean atCall, boolean debugging) {
    Address pc = returnAddress;
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(getOopMaps() != null, "nope");
    }
    if (isNativeMethod() || debugging) {
      // do it unprecise; for natives we are using the value stored in
      // TLS and not the return address when we cook the last frame
      return getOopMaps().findMapAtOffset(pc.minus(instructionsBegin()), atCall, false, debugging);
    } else {
      return getOopMaps().findMapAtOffset(pc.minus(instructionsBegin()), atCall, true, debugging);
    }
  }

  //  virtual void preserve_callee_argument_oops(frame fr, const RegisterMap* reg_map, void f(oop*)) { ShouldNotReachHere(); }
  //  FIXME;

  /** NOTE: this returns a size in BYTES in this system! */
  public long getFrameSize() {
    return VM.getVM().getAddressSize() * frameSizeField.getValue(addr);
  }
  
  // Returns true, if the next frame is responsible for GC'ing oops passed as arguments
  public boolean callerMustGCArguments(JavaThread thread) { return false; }

  public int getLinkOffset() {
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(VM.getVM().isServerCompiler(), "Only used on C2 x86");
      Assert.that(VM.getVM().getCPU().equals("x86"), "Only used on C2 x86");
    }

    if (linkOffset == NOT_YET_COMPUTED) {
      // No prior EBP saved for nmethods (prior frame is NOT an
      // interpreted -- i.e., it's an adapter frame).
      if (isNMethod()) {
        linkOffset = UNDEFINED;
      } else {
        // The Adaptor spills EBP so it must be in a stack location somewhere.
        // Find it.
        OopMap map = getOopMaps().getMapAt(0);// Should be in first map

        OopMapValue omv = null;
        for (OopMapStream oms = new OopMapStream(map, OopMapValue.OopTypes.CALLEE_SAVED_VALUE);
             !oms.isDone();
             oms.next()) {
          omv = oms.getCurrent();
          if (omv.getContentReg().getValue() == matcherInterpreterFramePointerReg) {
            linkOffset = omv.getStackOffset();
            break;
          }
        }
        if (Assert.ASSERTS_ENABLED) {
          Assert.that(linkOffset != NOT_YET_COMPUTED, "didn't find EBP in first oopmap?");
        }
      }
    }
    return linkOffset;
  }

  // FIXME: NOT FINISHED

  // FIXME: add more accessors
    
  public void print() {
    printOn(System.out);
  }

  public void printOn(PrintStream tty) {
    tty.print(getClass().getName());
    printComponentsOn(tty);
  }

  protected void printComponentsOn(PrintStream tty) {
    // FIXME: add relocation information
    tty.println(" instructions: [" + instructionsBegin() + ", " + instructionsEnd() + "), " +
                " data: [" + dataBegin() + ", " + dataEnd() + "), " +
                " oops: [" + oopsBegin() + ", " + oopsEnd() + "), " +
                " frame size: " + getFrameSize());
  }

  //--------------------------------------------------------------------------------
  // Internals only below this point
  //

  private int getOopsLength() {
    return (int) oopsLengthField.getValue(addr);
  }
}
