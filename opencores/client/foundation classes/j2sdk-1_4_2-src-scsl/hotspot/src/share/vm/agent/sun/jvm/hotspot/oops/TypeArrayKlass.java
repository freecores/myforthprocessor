/*
 * @(#)TypeArrayKlass.java	1.8 03/01/23 11:44:15
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.oops;

import java.io.*;
import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;
import sun.jvm.hotspot.utilities.*;

// TypeArrayKlass is a proxy for typeArrayKlass in the JVM

public class TypeArrayKlass extends ArrayKlass {
  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) throws WrongTypeException {
    Type t             = db.lookupType("typeArrayKlass");
    type               = new CIntField(t.getCIntegerField("_type"), Oop.getHeaderSize());
    scale              = new CIntField(t.getCIntegerField("_scale"), Oop.getHeaderSize());
    maxLength          = new CIntField(t.getCIntegerField("_max_length"), Oop.getHeaderSize());
  }

  TypeArrayKlass(OopHandle handle, ObjectHeap heap) {
    super(handle, heap);
  }

  private static CIntField  type;
  private static CIntField  scale;
  private static CIntField  maxLength;

  public long getType()               { return  type.getValue(this); }
  public long getScale()              { return  scale.getValue(this); }
  public long getMaxLength()          { return  maxLength.getValue(this); }

  public static final int T_BOOLEAN = 4;
  public static final int T_CHAR    = 5;
  public static final int T_FLOAT   = 6;
  public static final int T_DOUBLE  = 7;
  public static final int T_BYTE    = 8;
  public static final int T_SHORT   = 9;
  public static final int T_INT     = 10;
  public static final int T_LONG    = 11;

  public String getTypeName() {
    switch ((int) getType()) {
      case T_BOOLEAN: return "[Z";
      case T_CHAR:    return "[C";
      case T_FLOAT:   return "[F";
      case T_DOUBLE:  return "[D";
      case T_BYTE:    return "[B";
      case T_SHORT:   return "[S";
      case T_INT:     return "[I";
      case T_LONG:    return "[J";
    }
    return "Unknown TypeArray";
  }
  
  public void printValueOn(PrintStream tty) {
    tty.print("TypeArrayKlass for " + getTypeName());
  }

  public void iterateFields(OopVisitor visitor, boolean doVMFields) {
    super.iterateFields(visitor, doVMFields);
    if (doVMFields) {
      visitor.doCInt(type, true);
      visitor.doCInt(scale, true);
      visitor.doCInt(maxLength, true);
    }
  }

  public Klass arrayKlassImpl(boolean orNull, int n) {
    int dimension = (int) getDimension();
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(dimension <= n, "check order of chain");
    }
    if (dimension == n) 
      return this;
    ObjArrayKlass ak = (ObjArrayKlass) getHigherDimension();
    if (ak == null) {
      if (orNull) return null;
      // FIXME: would need to change in reflective system to actually
      // allocate klass
      throw new RuntimeException("Can not allocate array klasses in debugging system");
    }
    if (orNull) {
      return ak.arrayKlassOrNull(n);
    }
    return ak.arrayKlass(n);
  }

  public Klass arrayKlassImpl(boolean orNull) {
    return arrayKlassImpl(orNull, (int) (getDimension() + 1));
  }
}
