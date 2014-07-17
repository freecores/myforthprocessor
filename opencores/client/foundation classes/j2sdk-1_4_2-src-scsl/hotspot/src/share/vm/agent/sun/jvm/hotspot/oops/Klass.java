/*
 * @(#)Klass.java	1.8 03/01/23 11:42:58
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

public class Klass extends Oop {
  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) throws WrongTypeException {
    Type type    = db.lookupType("Klass");
    javaMirror   = new OopField(type.getOopField("_java_mirror"), Oop.getHeaderSize());
    superField   = new OopField(type.getOopField("_super"), Oop.getHeaderSize());
    sizeHelper   = new CIntField(type.getCIntegerField("_size_helper"), Oop.getHeaderSize());
    name         = new OopField(type.getOopField("_name"), Oop.getHeaderSize());
    accessFlags  = new CIntField(type.getCIntegerField("_access_flags"), Oop.getHeaderSize());
    subklass     = new OopField(type.getOopField("_subklass"), Oop.getHeaderSize());
    nextSibling  = new OopField(type.getOopField("_next_sibling"), Oop.getHeaderSize());
    allocCount   = new CIntField(type.getCIntegerField("_alloc_count"), Oop.getHeaderSize());
  }

  Klass(OopHandle handle, ObjectHeap heap) {
    super(handle, heap);
  }

  public boolean isKlass()             { return true; }

  // Fields
  private static OopField  javaMirror;
  private static OopField  superField;
  private static CIntField sizeHelper;
  private static OopField  name;
  private static CIntField accessFlags;
  private static OopField  subklass;
  private static OopField  nextSibling;
  private static CIntField allocCount;

  // Accessors for declared fields
  public Instance getJavaMirror()       { return (Instance) javaMirror.getValue(this);   }
  public Klass    getSuper()            { return (Klass)    superField.getValue(this);   }
  public long     getSizeHelper()       { return            sizeHelper.getValue(this);   }
  public Symbol   getName()             { return (Symbol)   name.getValue(this);         }
  public long     getAccessFlags()      { return            accessFlags.getValue(this);  }
  // Convenience routine
  public AccessFlags getAccessFlagsObj(){ return new AccessFlags(getAccessFlags());      }
  public Klass    getSubklassKlass()    { return (Klass)    subklass.getValue(this);     }
  public Klass    getNextSiblingKlass() { return (Klass)    nextSibling.getValue(this);  }
  public long     getAllocCount()       { return            allocCount.getValue(this);   }

  public void printValueOn(PrintStream tty) {
    tty.print("Klass");
  }

  public void iterateFields(OopVisitor visitor, boolean doVMFields) {
    super.iterateFields(visitor, doVMFields);
    if (doVMFields) {
      visitor.doOop(javaMirror, true);
      visitor.doOop(superField, true);
      visitor.doCInt(sizeHelper, true);
      visitor.doOop(name, true);
      visitor.doCInt(accessFlags, true);
      visitor.doOop(subklass, true);
      visitor.doOop(nextSibling, true);
      visitor.doCInt(allocCount, true);
    }
  }

  public long getObjectSize() {
    System.out.println("should not reach here");
    return 0;
  }

  /** Array class with specific rank */
  public Klass arrayKlass(int rank)       { return arrayKlassImpl(false, rank); }
  /** Array class with this klass as element type */
  public Klass arrayKlass()               { return arrayKlassImpl(false);       }
  /** These will return null instead of allocating on the heap */
  public Klass arrayKlassOrNull(int rank) { return arrayKlassImpl(true, rank);  }
  public Klass arrayKlassOrNull()         { return arrayKlassImpl(true);        }

  public Klass arrayKlassImpl(boolean orNull, int rank) {
    throw new RuntimeException("array_klass should be dispatched to instanceKlass, objArrayKlass or typeArrayKlass");
  }

  public Klass arrayKlassImpl(boolean orNull) {
    throw new RuntimeException("array_klass should be dispatched to instanceKlass, objArrayKlass or typeArrayKlass");
  }

  // This returns the name in the form java/lang/String which isn't really a signature
  // The subclasses override this to produce the correct form, eg
  //   Ljava/lang/String;
  public String signature() { return getName().asString(); }

  // Convenience routines
  public boolean isPublic()                 { return getAccessFlagsObj().isPublic(); }
  public boolean isFinal()                  { return getAccessFlagsObj().isFinal(); }
  public boolean isInterface()              { return getAccessFlagsObj().isInterface(); }
  public boolean isAbstract()               { return getAccessFlagsObj().isAbstract(); }
  public boolean isSuper()                  { return getAccessFlagsObj().isSuper(); }
  public boolean isSynthetic()              { return getAccessFlagsObj().isSynthetic(); }
  public boolean hasFinalizer()             { return getAccessFlagsObj().hasFinalizer(); }
  public boolean isCloneable()              { return getAccessFlagsObj().isCloneable(); }
  public boolean hasVanillaConstructor()    { return getAccessFlagsObj().hasVanillaConstructor(); }
  public boolean hasMirandaMethods ()       { return getAccessFlagsObj().hasMirandaMethods(); }
}
