/*
 * @(#)OopMapValue.java	1.4 03/01/23 11:24:51
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.compiler;

import java.util.*;

import sun.jvm.hotspot.code.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;
import sun.jvm.hotspot.utilities.*;

public class OopMapValue {
  private short value;
  private short contentReg;

  /** Read from target VM; located in compiler/oopMap.hpp */
  // How bits are organized
  static int TYPE_BITS;
  static int REGISTER_BITS;
  static int TYPE_SHIFT;
  static int REGISTER_SHIFT;
  static int TYPE_MASK;
  static int TYPE_MASK_IN_PLACE;
  static int REGISTER_MASK;
  static int REGISTER_MASK_IN_PLACE;

  // Types of OopValues
  static int UNUSED_VALUE;
  static int OOP_VALUE;
  static int VALUE_VALUE;
  static int DEAD_VALUE;
  static int CALLEE_SAVED_VALUE;
  static int DERIVED_OOP_VALUE;

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static void initialize(TypeDataBase db) {
    TYPE_BITS              = db.lookupIntConstant("OopMapValue::type_bits").intValue();
    REGISTER_BITS          = db.lookupIntConstant("OopMapValue::register_bits").intValue();
    TYPE_SHIFT             = db.lookupIntConstant("OopMapValue::type_shift").intValue();
    REGISTER_SHIFT         = db.lookupIntConstant("OopMapValue::register_shift").intValue();
    TYPE_MASK              = db.lookupIntConstant("OopMapValue::type_mask").intValue();
    TYPE_MASK_IN_PLACE     = db.lookupIntConstant("OopMapValue::type_mask_in_place").intValue();
    REGISTER_MASK          = db.lookupIntConstant("OopMapValue::register_mask").intValue();
    REGISTER_MASK_IN_PLACE = db.lookupIntConstant("OopMapValue::register_mask_in_place").intValue();
    UNUSED_VALUE           = db.lookupIntConstant("OopMapValue::unused_value").intValue();
    OOP_VALUE              = db.lookupIntConstant("OopMapValue::oop_value").intValue();
    VALUE_VALUE            = db.lookupIntConstant("OopMapValue::value_value").intValue();
    DEAD_VALUE             = db.lookupIntConstant("OopMapValue::dead_value").intValue();
    CALLEE_SAVED_VALUE     = db.lookupIntConstant("OopMapValue::callee_saved_value").intValue();
    DERIVED_OOP_VALUE      = db.lookupIntConstant("OopMapValue::derived_oop_value").intValue();
  }

  public static abstract class OopTypes {
    public static final OopTypes UNUSED_VALUE       = new OopTypes() { int getValue() { return OopMapValue.UNUSED_VALUE;       }};
    public static final OopTypes OOP_VALUE          = new OopTypes() { int getValue() { return OopMapValue.OOP_VALUE;          }};
    public static final OopTypes VALUE_VALUE        = new OopTypes() { int getValue() { return OopMapValue.VALUE_VALUE;        }};
    public static final OopTypes DEAD_VALUE         = new OopTypes() { int getValue() { return OopMapValue.DEAD_VALUE;         }};
    public static final OopTypes CALLEE_SAVED_VALUE = new OopTypes() { int getValue() { return OopMapValue.CALLEE_SAVED_VALUE; }};
    public static final OopTypes DERIVED_OOP_VALUE  = new OopTypes() { int getValue() { return OopMapValue.DERIVED_OOP_VALUE;  }};

    abstract int getValue();
    protected OopTypes() {}
  }

  public OopMapValue()                                  { setValue((short) 0); setContentReg(new VMReg(0)); }
  public OopMapValue(VMReg reg, OopTypes t)             { setReg(reg); setType(t);                      }
  public OopMapValue(VMReg reg, OopTypes t, VMReg reg2) { setReg(reg); setType(t); setContentReg(reg2); }
  public OopMapValue(CompressedReadStream stream)       { readFrom(stream);                             }

  public void readFrom(CompressedReadStream stream) {
    setValue((short) stream.readInt());
    if (isCalleeSaved() || isDerivedOop()) {
      setContentReg(new VMReg(stream.readInt()));
    }
  }

  // Querying
  public boolean isOop()         { return (getValue() & TYPE_MASK_IN_PLACE) == OOP_VALUE;          }
  public boolean isValue()       { return (getValue() & TYPE_MASK_IN_PLACE) == VALUE_VALUE;        }
  public boolean isDead()        { return (getValue() & TYPE_MASK_IN_PLACE) == DEAD_VALUE;         }
  public boolean isCalleeSaved() { return (getValue() & TYPE_MASK_IN_PLACE) == CALLEE_SAVED_VALUE; }
  public boolean isDerivedOop()  { return (getValue() & TYPE_MASK_IN_PLACE) == DERIVED_OOP_VALUE;  }

  public VMReg getReg() { return new VMReg((getValue() & REGISTER_MASK_IN_PLACE) >> REGISTER_SHIFT); }
  public void  setReg(VMReg r) { setValue((short) (r.getValue() << REGISTER_SHIFT | (getValue() & TYPE_MASK_IN_PLACE))); }

  public OopTypes getType() {
    int which = (getValue() & TYPE_MASK_IN_PLACE);
         if (which == UNUSED_VALUE) return OopTypes.UNUSED_VALUE;
    else if (which == OOP_VALUE)    return OopTypes.OOP_VALUE;
    else if (which == VALUE_VALUE)  return OopTypes.VALUE_VALUE;
    else if (which == DEAD_VALUE)   return OopTypes.DEAD_VALUE;
    else if (which == CALLEE_SAVED_VALUE) return OopTypes.CALLEE_SAVED_VALUE;
    else if (which == DERIVED_OOP_VALUE)  return OopTypes.DERIVED_OOP_VALUE;
    else throw new InternalError("unknown which " + which + " (TYPE_MASK_IN_PLACE = " + TYPE_MASK_IN_PLACE + ")");
  }
  public void setType(OopTypes t) { setValue((short) ((getValue() & REGISTER_MASK_IN_PLACE) | t.getValue())); }

  public VMReg getContentReg()        { return new VMReg(contentReg); }
  public void  setContentReg(VMReg r) { contentReg = (short) r.getValue(); }

  /** Physical location queries */
  public boolean isRegisterLoc()      { return (getReg().lessThan(VM.getVM().getSharedInfo().getStack0())); }
  public boolean isStackLoc()         { return (getReg().greaterThanOrEqual(VM.getVM().getSharedInfo().getStack0())); }

  /** Returns offset from sp. */
  public int getStackOffset() {
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(isStackLoc(), "must be stack location");
    }
    return getReg().minus(VM.getVM().getSharedInfo().getStack0());
  }

  //--------------------------------------------------------------------------------
  // Internals only below this point
  //

  private void setValue(short value) {
    this.value = value;
  }
  
  private int getValue() {
    return value;
  }
}
