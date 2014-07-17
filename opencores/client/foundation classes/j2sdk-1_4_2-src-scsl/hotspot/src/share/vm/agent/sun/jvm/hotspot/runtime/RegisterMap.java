/*
 * @(#)RegisterMap.java	1.7 03/01/23 11:45:32
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.runtime;

import java.io.*;
import java.util.*;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.types.*;
import sun.jvm.hotspot.utilities.*;

/** <P> A companion structure used for stack traversal. The
    RegisterMap contains misc. information needed in order to do
    correct stack traversal of stack frames.  Hence, it must always be
    passed in as an argument to Frame.sender(RegisterMap). </P>

    <P> The use of RegisterMaps is slightly different in the
    Serviceability Agent APIs than in the VM itself. In the VM, a
    RegisterMap is created either for a particular thread or cloned
    from another RegisterMap. In these APIs, a JavaThread is the
    top-level factory for RegisterMaps, and RegisterMaps know how to
    copy themselves (through either the clone() or copy()
    methods). </P> */

public abstract class RegisterMap implements Cloneable {
  /** Location of registers */
  protected Address[]  location;
  protected int[]      locationValid;
  /** Should include argument_oop marked locations for compiler */
  protected boolean    includeArgumentOops;
  /** Reference to current thread */
  protected JavaThread thread;
  /** Tells if the register map needs to be updated when traversing the stack */
  protected boolean    updateMap;
  /** Location of a frame where the pc is not at a call (NULL if no frame exists) */
  protected Address    notAtCallID;

  protected static int regCount;
  protected static int locationValidSize;

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static void initialize(TypeDataBase db) {
    if (VM.getVM().isCore()) {
      regCount = db.lookupIntConstant("RegisterImpl::number_of_registers").intValue();
    } else {
      regCount = db.lookupIntConstant("REG_COUNT").intValue();
    }
    locationValidSize = (regCount + Bits.BitsPerInt - 1) / Bits.BitsPerInt;
  }

  protected RegisterMap(JavaThread thread, boolean updateMap) {
    this.thread    = thread;
    this.updateMap = updateMap;
    location      = new Address[regCount];
    locationValid = new int[locationValidSize];
    clear(thread.getNotAtCallID());
  }

  /** Makes a copy of map into this */
  protected RegisterMap(RegisterMap map) {
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(map != null, "RegisterMap must be present");
    }
    this.thread              = map.getThread();
    this.updateMap           = map.getUpdateMap();
    this.includeArgumentOops = map.getIncludeArgumentOops();
    this.notAtCallID         = map.notAtCallID;
    location                 = new Address[map.location.length];
    locationValid            = new int[map.locationValid.length];
    initializeFromPD(map);
    if (updateMap) {
      for (int i = 0; i < locationValidSize; i++) {
        int bits = (!getUpdateMap()) ? 0 : map.locationValid[i];
        locationValid[i] = bits;
        // for whichever bits are set, pull in the corresponding map->_location
        int j = i*Bits.BitsPerInt;
        while (bits != 0) {
          if ((bits & 1) != 0) {
            if (Assert.ASSERTS_ENABLED) {
              Assert.that(0 <= j && j < regCount, "range check");
            }
            location[j] = map.location[j];
          }
          bits >>>= 1;
          j += 1;
        }
      }
    }
  }

  public abstract Object clone();

  public RegisterMap copy() {
    return (RegisterMap) clone();
  }

  public void clear(Address notAtCallID) {
    setIncludeArgumentOops(true);
    this.notAtCallID = notAtCallID;
    if (!VM.getVM().isCore()) {
      if (updateMap) {
        for (int i = 0; i < locationValid.length; i++) {
          locationValid[i] = 0;
        }
        clearPD();
      } else {
        initializePD();
      }
    }
  }

  public Address getLocation(VMReg reg) {
    int i = reg.getValue();
    int index = i / Bits.BitsPerInt;
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(0 <= i && i < regCount, "sanity check");
      Assert.that(0 <= index && index < locationValidSize, "sanity check");
    }
    if ((locationValid[index] & (1 << i % Bits.BitsPerInt)) != 0) {
      return location[i];
    } else {
      return getLocationPD(reg);
    }
  }

  public void setLocation(VMReg reg, Address loc) {
    int i = reg.getValue();
    int index = i / Bits.BitsPerInt;
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(0 <= i && i < regCount, "sanity check");
      Assert.that(0 <= index && index < locationValidSize, "sanity check");
      Assert.that(updateMap, "updating map that does not need updating");
    }
    location[i]          = loc;
    locationValid[index] |= (1 << (i % Bits.BitsPerInt));
  }

  public boolean getIncludeArgumentOops() {
    return includeArgumentOops;
  }

  public void setIncludeArgumentOops(boolean f) {
    includeArgumentOops = f;
  }

  public JavaThread getThread() {
    return thread;
  }

  public boolean getUpdateMap() {
    return updateMap;
  }

  public void print() {
    printOn(System.out);
  }

  public void printOn(PrintStream tty) {
    tty.println("Register map");
    for (int i = 0; i < location.length; i++) {
      Address src = getLocation(new VMReg(i));
      if (src != null) {
        tty.println("  " + getRegisterNamePD(i) +
                    " [" + src + "] = " +
                    src.getAddressAt(0));
      }
    }
  }

  /** Checks if the pc is at a call for the given id. */
  public boolean isPCAtCall(Address id) {
    if (notAtCallID == null) {
      return (notAtCallID != id);
    } else {
      return (!notAtCallID.equals(id));
    }
  }

  /** Platform-dependent clear() functionality */
  protected abstract void clearPD();
  /** Platform-dependent initialize() functionality */
  protected abstract void initializePD();
  /** Platform-dependent initializeFrom() functionality */
  protected abstract void initializeFromPD(RegisterMap map);
  /** Platform-dependent getLocation() functionality */
  protected abstract Address getLocationPD(VMReg reg);
  /** Platform-dependent getRegisterName() functionality */
  protected abstract String getRegisterNamePD(int i);
}
