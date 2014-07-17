/*
 * @(#)Location.java	1.6 03/01/23 11:24:08
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.code;

import java.io.*;
import java.util.*;

import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;
import sun.jvm.hotspot.utilities.*;

/** <P> A Location describes a concrete machine variable location
    (such as integer or floating point register or a stack-held
    variable). Used when generating debug-information for
    nmethods. </P>

    <P> Encoding: </P>
    <PRE>
    bits:
    Where:  [15]
    Type:   [14..12]
    Offset: [11..0]
    </PRE>
*/

public class Location {
  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static void initialize(TypeDataBase db) {
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(!VM.getVM().isCore(), "Debug info not used in core build");
    }
    
    OFFSET_MASK  = db.lookupIntConstant("Location::OFFSET_MASK").intValue();
    OFFSET_SHIFT = db.lookupIntConstant("Location::OFFSET_SHIFT").intValue();
    TYPE_MASK    = db.lookupIntConstant("Location::TYPE_MASK").intValue();
    TYPE_SHIFT   = db.lookupIntConstant("Location::TYPE_SHIFT").intValue();
    WHERE_MASK   = db.lookupIntConstant("Location::WHERE_MASK").intValue();
    WHERE_SHIFT  = db.lookupIntConstant("Location::WHERE_SHIFT").intValue();
  }

  private short value;

  public static class Where extends IntegerEnum {
    public static final Where ON_STACK    = new Where(0);
    public static final Where IN_REGISTER = new Where(1);

    protected Where(int value) {
      super(value);
    }
  }

  // FIXME: should read these enums from the target process
  private static final Where[] wheres = {
    Where.ON_STACK,
    Where.IN_REGISTER
  };

  public static class Type extends IntegerEnum {
    /** Ints, floats, double halves */
    public static final Type NORMAL       = new Type(0);
    /** Oop (please GC me!) */
    public static final Type OOP          = new Type(1);
    /** Long held in one register */
    public static final Type LNG          = new Type(2);
    /** Float held in double register */
    public static final Type FLOAT_IN_DBL = new Type(3);
    /** Double held in one register */
    public static final Type DBL          = new Type(4);
    /** JSR return address */
    public static final Type ADDR         = new Type(5);
    /** Invalid location */
    public static final Type INVALID      = new Type(6);

    protected Type(int value) {
      super(value);
    }
  }

  private static final Type[] types = {
    Type.NORMAL,
    Type.OOP,
    Type.LNG,
    Type.FLOAT_IN_DBL,
    Type.DBL,
    Type.ADDR,
    Type.INVALID
  };

  private static int OFFSET_MASK;
  private static int OFFSET_SHIFT;
  private static int TYPE_MASK;
  private static int TYPE_SHIFT;
  private static int WHERE_MASK;
  private static int WHERE_SHIFT;

  /** Create a bit-packed Location */
  Location(Where where, Type type, short offset) {
    setWhere(where);
    setType(type);
    setOffset(offset);
  }

  public Where getWhere() {
    return wheres[(value & WHERE_MASK) >> WHERE_SHIFT];
  }

  public Type getType() {
    return types[(value & TYPE_MASK) >> TYPE_SHIFT];
  }

  public short getOffset() {
    return (short) ((value & OFFSET_MASK) >> OFFSET_SHIFT);
  }

  public boolean isRegister() {
    return getWhere() == Where.IN_REGISTER;
  }

  public boolean isStack() {
    return getWhere() == Where.ON_STACK;
  }

  public boolean holdsOop() {
    return getType() == Type.OOP;
  }

  public boolean holdsFloat() {
    return getType() == Type.FLOAT_IN_DBL;
  }

  public boolean holdsDouble() {
    return getType() == Type.DBL;
  }

  public boolean holdsLong() {
    return getType() == Type.LNG;
  }

  public boolean holdsAddr() {
    return getType() == Type.ADDR;
  }

  public boolean isIllegal() {
    return getType() == Type.INVALID;
  }

  public int getStackOffset() {
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(getWhere() == Where.ON_STACK, "wrong Where");
    }
    return getOffset() << VM.getVM().getLogAddressSize();
  }
  
  public int getRegisterNumber() {
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(getWhere() == Where.IN_REGISTER, "wrong Where");
    }
    return getOffset();
  }
  
  public void print() {
    printOn(System.out);
  }

  public void printOn(PrintStream tty) {
    tty.print("Value " + value + ", ");
    if (isIllegal()) {
      tty.print("Illegal");
    } else {
      Where w = getWhere();
      if (w == Where.ON_STACK) {
        tty.print("stack[" + getStackOffset() + "]");
      } else if (w == Where.IN_REGISTER) {
        tty.print("reg " + getRegisterNumber());
      }

      Type type = getType();
      if (type == Type.NORMAL) {
      } else if (type == Type.OOP) {
        tty.print(",oop");
      } else if (type == Type.LNG) {
        tty.print(",long");
      } else if (type == Type.FLOAT_IN_DBL) {
        tty.print(",float");
      } else if (type == Type.DBL) {
        tty.print(",double");
      } else if (type == Type.ADDR) {
        tty.print(",address");
      } else {
        tty.print("Wrong location type " + type.getValue());
      }
    }
  }

  /** Serialization of debugging information */
  public Location(DebugInfoReadStream stream) {
    value = stream.readShort();
  }

  // FIXME: not yet implementable
  // void write_on(DebugInfoWriteStream* stream);
  

  //--------------------------------------------------------------------------------
  // Internals only below this point
  //

  private void setWhere(Where where) {
    value |= (where.getValue() << WHERE_SHIFT);
  }

  private void setType(Type type) {
    value |= (type.getValue() << TYPE_SHIFT);
  }
  
  private void setOffset(short offset) {
    value |= (offset << OFFSET_SHIFT);
  }
}
