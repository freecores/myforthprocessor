/*
 * @(#)SPARCRegisterMap.java	1.4 03/01/23 11:46:40
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.runtime.sparc;

import java.util.*;

import sun.jvm.hotspot.asm.sparc.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;
import sun.jvm.hotspot.utilities.*;

public class SPARCRegisterMap extends RegisterMap {
  /** Register window save area (for L and I regs) */
  private Address window;
  /** Previous save area (for O regs, if needed) */
  private Address youngerWindow;

  private static int registerImplNumberOfRegisters;

  // C2 only
  private static Address matcherRegEncodeAddr;

  private static int[] R_L_nums;
  private static int[] R_I_nums;
  private static int[] R_O_nums;
  private static int[] R_G_nums;

  private static int once;
  private static int badMask;
  private static int R_LIO_mask;

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static void initialize(TypeDataBase db) {
    once = 0;
    badMask = 0;
    R_LIO_mask = 0;

    registerImplNumberOfRegisters = db.lookupIntConstant("RegisterImpl::number_of_registers").intValue();
    if (VM.getVM().isClientCompiler()) {
      R_L_nums = new int[] {0+16, 1+16, 2+16, 3+16, 4+16, 5+16, 6+16, 7+16};
      R_I_nums = new int[] {0+24, 1+24, 2+24, 3+24, 4+24, 5+24, 6+24, 7+24};
      R_O_nums = new int[] {0+ 8, 1+ 8, 2+ 8, 3+ 8, 4+ 8, 5+ 8, 6+ 8, 7+ 8};
      R_G_nums = new int[] {0+ 0, 1+ 0, 2+ 0, 3+ 0, 4+ 0, 5+ 0, 6+ 0, 7+ 0};
    }
    if (VM.getVM().isServerCompiler()) {
      Type type = db.lookupType("Matcher");
      Field f = type.getField("_regEncode");
      matcherRegEncodeAddr = f.getStaticFieldAddress();
      R_L_nums = new int[] {
        db.lookupIntConstant("R_L0_num").intValue(),
        db.lookupIntConstant("R_L1_num").intValue(),
        db.lookupIntConstant("R_L2_num").intValue(),
        db.lookupIntConstant("R_L3_num").intValue(),
        db.lookupIntConstant("R_L4_num").intValue(),
        db.lookupIntConstant("R_L5_num").intValue(),
        db.lookupIntConstant("R_L6_num").intValue(),
        db.lookupIntConstant("R_L7_num").intValue()
      };
      R_I_nums = new int[] {
        db.lookupIntConstant("R_I0_num").intValue(),
        db.lookupIntConstant("R_I1_num").intValue(),
        db.lookupIntConstant("R_I2_num").intValue(),
        db.lookupIntConstant("R_I3_num").intValue(),
        db.lookupIntConstant("R_I4_num").intValue(),
        db.lookupIntConstant("R_I5_num").intValue(),
        db.lookupIntConstant("R_FP_num").intValue(),
        db.lookupIntConstant("R_I7_num").intValue()
      };
      R_O_nums = new int[] {
        db.lookupIntConstant("R_O0_num").intValue(),
        db.lookupIntConstant("R_O1_num").intValue(),
        db.lookupIntConstant("R_O2_num").intValue(),
        db.lookupIntConstant("R_O3_num").intValue(),
        db.lookupIntConstant("R_O4_num").intValue(),
        db.lookupIntConstant("R_O5_num").intValue(),
        db.lookupIntConstant("R_SP_num").intValue(),
        db.lookupIntConstant("R_O7_num").intValue()
      };
      R_G_nums = new int[] {
        db.lookupIntConstant("R_G0_num").intValue(),
        db.lookupIntConstant("R_G1_num").intValue(),
        db.lookupIntConstant("R_G2_num").intValue(),
        db.lookupIntConstant("R_G3_num").intValue(),
        db.lookupIntConstant("R_G4_num").intValue(),
        db.lookupIntConstant("R_G5_num").intValue(),
        db.lookupIntConstant("R_G6_num").intValue(),
        db.lookupIntConstant("R_G7_num").intValue()
      };
    }
  }

  /** This is the only public constructor, and is only called by
      SolarisSPARCJavaThread */
  public SPARCRegisterMap(JavaThread thread, boolean updateMap) {
    super(thread, updateMap);
  }

  protected SPARCRegisterMap(RegisterMap map) {
    super(map);
  }

  public Object clone() {
    SPARCRegisterMap retval = new SPARCRegisterMap(this);
    return retval;
  }
 
  protected void clearPD() {
    if (thread.hasLastJavaFrame()) {
      Frame fr = thread.getLastFrame();
      window = fr.getSP();
    } else {
      window = null;
      if (VM.getVM().isDebugging()) {
        Frame fr = thread.getCurrentFrameGuess();
        if (fr != null) {
          window = fr.getSP();
        }
      }
    }
    youngerWindow = null;
  }

  protected Address getLocationPD(VMReg regname) {
    VM vm = VM.getVM();
    if (vm.isClientCompiler()) {
      if (Assert.ASSERTS_ENABLED) {
        Assert.that(0 <= regname.getValue() && regname.getValue() < regCount, "sanity check");
      }
      if (regname.getValue() < registerImplNumberOfRegisters) {
        SPARCRegister reg = new SPARCRegister(regname.getValue());
        if (reg.isOut()) {
          if (Assert.ASSERTS_ENABLED) {
            Assert.that(youngerWindow != null, "Younger window should be available");
          }
          return youngerWindow.addOffsetTo(reg.afterSave().spOffsetInSavedWindow());
        }
        if (reg.isLocal() || reg.isIn()) {
          if (Assert.ASSERTS_ENABLED) {
            Assert.that(window != null, "Window should be available");
          }
          return window.addOffsetTo(reg.spOffsetInSavedWindow());
        }
      }
    } else if (vm.isServerCompiler()) {
      if (Assert.ASSERTS_ENABLED) {
        Assert.that(0 <= regname.getValue() && regname.getValue() < regCount, "sanity check");
      }
      SPARCRegister reg = new SPARCRegister((int) matcherRegEncodeAddr.getCIntegerAt(regname.getValue(), 1, true));
      if (reg.isOut() && youngerWindow != null) {
        return youngerWindow.addOffsetTo(reg.afterSave().spOffsetInSavedWindow());
      }
      if ((reg.isLocal() || reg.isIn()) && window != null) {
        return window.addOffsetTo(reg.spOffsetInSavedWindow());
      }
    }

    return null;
  }
  
  protected String getRegisterNamePD(int i) {
    return SPARCRegisters.getRegisterName(i);
  }

  protected void initializePD() {
    window        = null;
    youngerWindow = null;
    // avoid the shift_individual_registers game
    makeIntegerRegsUnsaved();
  }

  protected void initializeFromPD(RegisterMap map) {
    SPARCRegisterMap srm = (SPARCRegisterMap) map;
    window        = srm.window;
    youngerWindow = srm.youngerWindow;
    // avoid the shift_individual_registers game
    makeIntegerRegsUnsaved();
  }

  public void shiftWindow(Address sp, Address youngerSP) {
    window        = sp;
    youngerWindow = youngerSP;
    // Throw away locations for %i, %o, and %l registers:
    // But do not throw away %g register locs.
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(registerImplNumberOfRegisters == Bits.BitsPerInt, "one word has valid bits for int-regs");
    }
    if (locationValid[0] != 0) {
      shiftIndividualRegisters();
    }
  }

  /** When popping out of compiled frames, we make all IRegs disappear. */
  public void makeIntegerRegsUnsaved() {
    locationValid[0] = 0;
  }

  //--------------------------------------------------------------------------------
  // Internals only below this point
  //

  private void shiftIndividualRegisters() {
    if (!getUpdateMap()) {
      return;
    }

    int lv = locationValid[0];
    int lv0 = lv;
    int i;

    if (Assert.ASSERTS_ENABLED) {
      if (once++ == 0) {
        int bitsPerWord = (int) VM.getVM().getAddressSize() * Bits.BitsPerByte;
        for (i = 0; i < 8; i++) {
          Assert.that(R_L_nums[i] < bitsPerWord, "in first chunk");
          Assert.that(R_I_nums[i] < bitsPerWord, "in first chunk");
          Assert.that(R_O_nums[i] < bitsPerWord, "in first chunk");
          Assert.that(R_G_nums[i] < bitsPerWord, "in first chunk");
        }
      }

      if (VM.getVM().isServerCompiler()) {
        if (badMask == 0) {
          int m = 0;
          m |= (1 << R_O_nums[6]); // SP
          m |= (1 << R_O_nums[7]); // cPC
          m |= (1 << R_I_nums[6]); // FP
          m |= (1 << R_I_nums[7]); // rPC
          m |= (1 << R_G_nums[2]); // TLS
          m |= (1 << R_G_nums[7]); // reserved by libthread
          badMask = m;
        }
      }

      Assert.that((lv & badMask) == 0, "cannot have special locations for SP,FP,TLS,etc.");
    }

    int mask = R_LIO_mask;
    if (mask == 0) {
      for (i = 0; i < 8; i++) {
        mask |= (1 << R_L_nums[i]);
        mask |= (1 << R_I_nums[i]);
        mask |= (1 << R_O_nums[i]);
      }
      R_LIO_mask = mask;
    }

    lv &= ~mask;  // clear %l, %o, %i regs

    // if we cleared some non-%g locations, we may have to do some shifting
    if (lv != lv0) {
      // copy %i0-%i5 to %o0-%o5, if they have special locations
      // This can happen in within stubs which spill argument registers
      // around a dynamic link operation, such as resolve_opt_virtual_call.
      for (i = 0; i < 8; i++) {
        if ((lv0 & (1 << R_I_nums[i])) != 0) {
          location[R_O_nums[i]] = location[R_I_nums[i]];
          lv |= (1 << R_O_nums[i]);
        }
      }
    }

    locationValid[0] = lv;
  }
}
