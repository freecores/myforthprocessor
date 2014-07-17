/*
 * @(#)SystemDictionary.java	1.5 03/01/23 11:41:23
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.memory;

import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.oops.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;

public class SystemDictionary {
  private static AddressField bucketsField;
  private static sun.jvm.hotspot.types.OopField javaSystemLoaderField;
  private static int loaderConstraintSize;
  private static int nofBuckets;

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) {
    Type type = db.lookupType("SystemDictionary");
    
    bucketsField = type.getAddressField("_buckets");
    javaSystemLoaderField = type.getOopField("_java_system_loader");
    loaderConstraintSize = db.lookupIntConstant("SystemDictionary::_loader_constraint_size").intValue();
    nofBuckets = db.lookupIntConstant("SystemDictionary::_nof_buckets").intValue();
  }

  public Oop javaSystemLoader() {
    return VM.getVM().getObjectHeap().newOop(javaSystemLoaderField.getValue());
  }

  /** Lookup an already loaded class. If not found null is returned. */
  public Klass find(String className, Oop classLoader, Oop protectionDomain) {
    Symbol sym = VM.getVM().getSymbolTable().probe(className);
    if (sym == null) return null;
    return find(sym, classLoader, protectionDomain);
  }

  /** Lookup an already loaded class. If not found null is returned. */
  public Klass find(Symbol className, Oop classLoader, Oop protectionDomain) {
    SystemDictionaryEntry entry = getEntry(className, classLoader);
    if (entry == null)            return null;
    if (!entry.klass().isKlass()) return null;
    return entry.isValidProtectionDomain(protectionDomain)
         ? ((Klass) entry.klass())
         : null;
  }

  /** Interface for iterating through all classes in dictionary */
  public static interface ClassVisitor {
    public void visit(Klass k);
  }

  /** Interface for iterating through all classes and their class
      loaders in dictionary */
  public static interface ClassAndLoaderVisitor {
    public void visit(Klass k, Oop loader);
  }

  /** Iterate over all klasses in dictionary; just the classes from
      declaring class loaders */
  public void classesDo(ClassVisitor v) {
    ObjectHeap heap = VM.getVM().getObjectHeap();
    for (int index = 0; index < nofBuckets; index++) {
      for (SystemDictionaryEntry probe = getEntryAt(index); probe != null; probe = probe.next()) {
        Oop k = probe.klass();
        if (k.isKlass() &&
            heap.equal(probe.loader(), ((InstanceKlass) k).getClassLoader())) {
          v.visit((Klass) k);
        }
      }
    }
  }

  /** All classes, and their class loaders */
  public void classesDo(ClassAndLoaderVisitor v) {
    for (int index = 0; index < nofBuckets; index++) {
      for (SystemDictionaryEntry probe = getEntryAt(index); probe != null; probe = probe.next()) {
        Oop k = probe.klass();
        if (k.isKlass()) {
          v.visit((Klass) k, probe.loader());
        }
      }
    }
  }

  /** All array classes of primitive type, and their class loaders */
  public void primArrayClassesDo(ClassAndLoaderVisitor v) {
    ObjectHeap heap = VM.getVM().getObjectHeap();
    for (int index = 0; index < nofBuckets; index++) {
      for (SystemDictionaryEntry probe = getEntryAt(index); probe != null; probe = probe.next()) {
        Oop k = probe.klass();
        // array of primitive arrays are stored in system dictionary as placeholders
        if (k.isSymbol()) {
          FieldType ft = new FieldType((Symbol) k);
          if (ft.isArray()) {
            FieldType.ArrayInfo info = ft.getArrayInfo();
            if (info.elementBasicType() != BasicType.getTObject()) {
              Klass arrayKlass = heap.typeArrayKlassObj(info.elementBasicType());
              arrayKlass = arrayKlass.arrayKlassOrNull(info.dimension());
              v.visit(arrayKlass, probe.loader());
            }
          }
        }
      }
    }
  }

  public List allClasses() {
    int count = 0;
    
    // First, find how many
    for (int index = 0; index < nofBuckets; index++) {
      for (SystemDictionaryEntry probe = getEntryAt(index); probe != null; probe = probe.next()) {
        Oop k = probe.klass();
        if (k.isKlass()) {
            // This is going to be generous since we won't be including ALL
            // symbols.
            count++;
        }
        if ( k.isSymbol()) {
            count++;
        }
      }
    }

    List retVal = new ArrayList(count);
    ObjectHeap heap = VM.getVM().getObjectHeap();
    for (int index = 0; index < nofBuckets; index++) {
      for (SystemDictionaryEntry probe = getEntryAt(index); probe != null; probe = probe.next()) {
        Oop k = probe.klass();
        if (k.isKlass()) {
            retVal.add(k);
        } else  if (k.isSymbol()) {
            FieldType ft = new FieldType((Symbol) k);
            if (ft.isArray()) {
                FieldType.ArrayInfo info = ft.getArrayInfo();
                if (info.elementBasicType() != BasicType.getTObject()) {
                    Klass arrayKlass = heap.typeArrayKlassObj(info.elementBasicType());
                    arrayKlass = arrayKlass.arrayKlassOrNull(info.dimension());
                    retVal.add(arrayKlass);
                }
            }
        }
        
      }
    }
    return retVal;
  }

  //----------------------------------------------------------------------
  // Internals only below this point
  //

  private SystemDictionaryEntry getEntry(Symbol className, Oop classLoader) {
    int index = indexFor(className, classLoader);
    ObjectHeap heap = VM.getVM().getObjectHeap();
    for (SystemDictionaryEntry entry = getEntryAt(index); entry != null; entry = entry.next()) {
      if (   heap.equal(entry.loader(), classLoader)
             && entry.klass().isKlass()
             && heap.equal(((Klass) entry.klass()).getName(), className)) {
        return entry;
      }
    }
    return null;
  }

  private SystemDictionaryEntry getEntryAt(int index) {
    Address array      = bucketsField.getValue();
    Address bucketAddr = array.getAddressAt(index * VM.getVM().getAddressSize());
    if (bucketAddr == null) return null;
    return (SystemDictionaryEntry) VMObjectFactory.newObject(SystemDictionaryEntry.class, bucketAddr);
  }

  private int indexFor(Symbol className, Oop classLoader) {
    int hash = hashValue(className, classLoader) % nofBuckets;
    return hash;
  }

  private int hashValue(Symbol className, Oop classLoader) {
    return (int) className.identityHash()
        ^  (int) (classLoader == null ? 0 : classLoader.identityHash());
  }
}
