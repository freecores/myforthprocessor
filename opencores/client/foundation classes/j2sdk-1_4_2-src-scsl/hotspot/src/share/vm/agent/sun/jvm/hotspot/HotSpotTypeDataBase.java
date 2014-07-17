/*
 * @(#)HotSpotTypeDataBase.java	1.13 03/01/23 11:14:06
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot;

import java.io.*;
import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.types.*;
import sun.jvm.hotspot.types.basic.*;
import sun.jvm.hotspot.utilities.*;

/** <P> This is the cross-platform TypeDataBase used by the Oop
    hierarchy. The decision was made to make this cross-platform by
    having the VM export the necessary symbols via a built-in table;
    see src/share/vm/runtime/vmStructs.[ch]pp for more details. </P>

    <P> <B>WARNING</B>: clients should refer to this class through the
    TypeDataBase interface and not directly to the HotSpotTypeDataBase
    type. </P>

    <P> NOTE: since we are fetching the sizes of the Java primitive types 
 */

public class HotSpotTypeDataBase extends BasicTypeDataBase {
  private Debugger symbolLookup;
  private String[] jvmLibNames;
  private static final int UNINITIALIZED_SIZE = -1;
  private static final int C_INT8_SIZE  = 1;
  private static final int C_INT32_SIZE = 4;
  private static final int C_INT64_SIZE = 8;

  /** <P> This requires a SymbolLookup mechanism as well as the
      MachineDescription. Note that we do not need a NameMangler since
      we use the vmStructs mechanism to avoid looking up C++
      symbols. </P>

      <P> NOTE that it is guaranteed that this constructor will not
      attempt to fetch any Java values from the remote process, only C
      integers and addresses. This is required because we are fetching
      the sizes of the Java primitive types from the remote process,
      implying that attempting to fetch them before their sizes are
      known is illegal. </P> 

      <P> Throws NoSuchSymbolException if a problem occurred while
      looking up one of the bootstrapping symbols related to the
      VMStructs table in the remote VM; this may indicate that the
      remote process is not actually a HotSpot VM. </P>
  */
  public HotSpotTypeDataBase(MachineDescription machDesc,
                             VtblAccess vtblAccess,
                             Debugger symbolLookup,
                             String[] jvmLibNames) throws NoSuchSymbolException {
    super(machDesc, vtblAccess);
    this.symbolLookup = symbolLookup;
    this.jvmLibNames = jvmLibNames;

    readVMTypes();
    initializePrimitiveTypes();
    readVMStructs();
    readVMIntConstants();
    readVMLongConstants();
  }

  private void readVMTypes() {
    // Get the variables we need in order to traverse the VMTypeEntry[]
    long typeEntryTypeNameOffset;
    long typeEntrySuperclassNameOffset;
    long typeEntryIsOopTypeOffset;
    long typeEntryIsIntegerTypeOffset;
    long typeEntryIsUnsignedOffset;
    long typeEntrySizeOffset;
    long typeEntryArrayStride;
    
    typeEntryTypeNameOffset       = getLongValueFromProcess("gHotSpotVMTypeEntryTypeNameOffset");
    typeEntrySuperclassNameOffset = getLongValueFromProcess("gHotSpotVMTypeEntrySuperclassNameOffset");
    typeEntryIsOopTypeOffset      = getLongValueFromProcess("gHotSpotVMTypeEntryIsOopTypeOffset");
    typeEntryIsIntegerTypeOffset  = getLongValueFromProcess("gHotSpotVMTypeEntryIsIntegerTypeOffset");
    typeEntryIsUnsignedOffset     = getLongValueFromProcess("gHotSpotVMTypeEntryIsUnsignedOffset");
    typeEntrySizeOffset           = getLongValueFromProcess("gHotSpotVMTypeEntrySizeOffset");
    typeEntryArrayStride          = getLongValueFromProcess("gHotSpotVMTypeEntryArrayStride");

    // Fetch the address of the VMTypeEntry*
    Address entryAddr = lookupInProcess("gHotSpotVMTypes");
    //    System.err.println("gHotSpotVMTypes address = " + entryAddr);
    // Dereference this once to get the pointer to the first VMTypeEntry
    //    dumpMemory(entryAddr, 80);
    entryAddr = entryAddr.getAddressAt(0);

    if (entryAddr == null) {
      throw new RuntimeException("gHotSpotVMTypes was not initialized properly in the remote process; can not continue");
    }

    // Start iterating down it until we find an entry with no name
    Address typeNameAddr = null;
    do {
      // Fetch the type name first
      typeNameAddr = entryAddr.getAddressAt(typeEntryTypeNameOffset);
      if (typeNameAddr != null) {
        String typeName = CStringUtilities.getString(typeNameAddr);
        
        String superclassName = null;
        Address superclassNameAddr = entryAddr.getAddressAt(typeEntrySuperclassNameOffset);
        if (superclassNameAddr != null) {
          superclassName = CStringUtilities.getString(superclassNameAddr);
        }
        
        boolean isOopType     = (entryAddr.getCIntegerAt(typeEntryIsOopTypeOffset, C_INT32_SIZE, false) != 0);
        boolean isIntegerType = (entryAddr.getCIntegerAt(typeEntryIsIntegerTypeOffset, C_INT32_SIZE, false) != 0);
        boolean isUnsigned    = (entryAddr.getCIntegerAt(typeEntryIsUnsignedOffset, C_INT32_SIZE, false) != 0);
        long size             = entryAddr.getCIntegerAt(typeEntrySizeOffset, C_INT64_SIZE, true);

        // See whether we have a superclass
        BasicType superclass = null;
        if (superclassName != null) {
          // Fetch or create it (FIXME: would get oop types wrong if
          // they had a hierarchy; consider using lookupOrFail)
          superclass = lookupOrCreateClass(superclassName, false, false, false);
        }
        
        // Lookup or create the current type
        BasicType curType = lookupOrCreateClass(typeName, isOopType, isIntegerType, isUnsigned);
        // Set superclass and/or ensure it's correct
        if (superclass != null) {
          if (curType.getSuperclass() == null) {
            // Set the superclass in the current type
            curType.setSuperclass(superclass);
          }

          if (curType.getSuperclass() != superclass) {
            throw new RuntimeException("Error: the type \"" + typeName + "\" (declared in the remote VM in VMStructs::localHotSpotVMTypes) " +
                                       "had its superclass redefined (old was " + curType.getSuperclass().getName() + ", new is " +
                                       superclass.getName() + ").");
          }
        }

        // Classes are created with a size of UNINITIALIZED_SIZE.
        // Set size if necessary.
        if (curType.getSize() == UNINITIALIZED_SIZE) {
          curType.setSize(size);
        } else {
          if (curType.getSize() != size) {
            throw new RuntimeException("Error: the type \"" + typeName + "\" (declared in the remote VM in VMStructs::localHotSpotVMTypes) " +
                                       "had its size redefined (old was " + curType.getSize() + ", new is " + size + ").");
          }

          System.err.println("Warning: the type \"" + typeName + "\" (declared in the remote VM in VMStructs::localHotSpotVMTypes) " +
                             "had its size declared as " + size + " twice. Continuing.");
        }
      }

      entryAddr = entryAddr.addOffsetTo(typeEntryArrayStride);
    } while (typeNameAddr != null);
  }

  private void initializePrimitiveTypes() {
    // Look up the needed primitive types by name...they had better be present
    setJBooleanType(lookupPrimitiveType("jboolean"));
    setJByteType   (lookupPrimitiveType("jbyte"));
    setJCharType   (lookupPrimitiveType("jchar"));
    setJDoubleType (lookupPrimitiveType("jdouble"));
    setJFloatType  (lookupPrimitiveType("jfloat"));
    setJIntType    (lookupPrimitiveType("jint"));
    setJLongType   (lookupPrimitiveType("jlong"));
    setJShortType  (lookupPrimitiveType("jshort"));

    // Indicate that these are the Java primitive types
    ((BasicType) getJBooleanType()).setIsJavaPrimitiveType(true);
    ((BasicType) getJByteType()).setIsJavaPrimitiveType(true);
    ((BasicType) getJCharType()).setIsJavaPrimitiveType(true);
    ((BasicType) getJDoubleType()).setIsJavaPrimitiveType(true);
    ((BasicType) getJFloatType()).setIsJavaPrimitiveType(true);
    ((BasicType) getJIntType()).setIsJavaPrimitiveType(true);
    ((BasicType) getJLongType()).setIsJavaPrimitiveType(true);
    ((BasicType) getJShortType()).setIsJavaPrimitiveType(true);
  }

  private Type lookupPrimitiveType(String typeName) {
    Type type = lookupType(typeName, false);
    if (type == null) {
      throw new RuntimeException("Error initializing the HotSpotDataBase: could not find the primitive type \"" +
                                 typeName + "\" in the remote VM's VMStructs table. This type is required in " +
                                 "order to determine the size of Java primitive types. Can not continue.");
    }
    return type;
  }

  private void readVMStructs() {
    // Get the variables we need in order to traverse the VMStructEntry[]
    long structEntryTypeNameOffset;
    long structEntryFieldNameOffset;
    long structEntryTypeStringOffset;
    long structEntryIsStaticOffset;
    long structEntryOffsetOffset;
    long structEntryAddressOffset;
    long structEntryArrayStride;

    structEntryTypeNameOffset     = getLongValueFromProcess("gHotSpotVMStructEntryTypeNameOffset");
    structEntryFieldNameOffset    = getLongValueFromProcess("gHotSpotVMStructEntryFieldNameOffset");
    structEntryTypeStringOffset   = getLongValueFromProcess("gHotSpotVMStructEntryTypeStringOffset");
    structEntryIsStaticOffset     = getLongValueFromProcess("gHotSpotVMStructEntryIsStaticOffset");
    structEntryOffsetOffset       = getLongValueFromProcess("gHotSpotVMStructEntryOffsetOffset");
    structEntryAddressOffset      = getLongValueFromProcess("gHotSpotVMStructEntryAddressOffset");
    structEntryArrayStride        = getLongValueFromProcess("gHotSpotVMStructEntryArrayStride");
    
    // Fetch the address of the VMStructEntry*
    Address entryAddr = lookupInProcess("gHotSpotVMStructs");
    // Dereference this once to get the pointer to the first VMStructEntry
    entryAddr = entryAddr.getAddressAt(0);
    if (entryAddr == null) {
      throw new RuntimeException("gHotSpotVMStructs was not initialized properly in the remote process; can not continue");
    }

    // Start iterating down it until we find an entry with no name
    Address fieldNameAddr = null;
    String typeName = null;
    String fieldName = null;
    String typeString = null;
    boolean isStatic = false;
    long offset = 0;
    Address staticFieldAddr = null;
    long size = 0;
    long index = 0;
    String opaqueName = "<opaque>";
    lookupOrCreateClass(opaqueName, false, false, false);

    do {
      // Fetch the field name first
      fieldNameAddr = entryAddr.getAddressAt(structEntryFieldNameOffset);
      if (fieldNameAddr != null) {
        fieldName = CStringUtilities.getString(fieldNameAddr);

        // Now the rest of the names. Keep in mind that the type name
        // may be NULL, indicating that the type is opaque.
        Address addr = entryAddr.getAddressAt(structEntryTypeNameOffset);
        if (addr == null) {
          throw new RuntimeException("gHotSpotVMStructs unexpectedly had a NULL type name at index " + index);
        }
        typeName = CStringUtilities.getString(addr);

        addr = entryAddr.getAddressAt(structEntryTypeStringOffset);
        if (addr == null) {
          typeString = opaqueName;
        } else {
          typeString = CStringUtilities.getString(addr);
        }
      
        isStatic = !(entryAddr.getCIntegerAt(structEntryIsStaticOffset, C_INT32_SIZE, false) == 0);
        if (isStatic) {
          staticFieldAddr = entryAddr.getAddressAt(structEntryAddressOffset);
          offset = 0;
        } else {
          offset = entryAddr.getCIntegerAt(structEntryOffsetOffset, C_INT64_SIZE, true);
          staticFieldAddr = null;
        }

        // The containing Type must already be in the database -- no exceptions
        BasicType containingType = lookupOrFail(typeName);

        // The field's Type must already be in the database -- no exceptions
        BasicType fieldType = lookupOrFail(typeString);

        // Create field by type
        Field field = createField(this, containingType,
                                  fieldName, fieldType, isStatic,
                                  offset, staticFieldAddr);
        // Add field to containing type
        containingType.addField(field);
      }

      ++index;
      entryAddr = entryAddr.addOffsetTo(structEntryArrayStride);
    } while (fieldNameAddr != null);
  }

  private void readVMIntConstants() {
    // Get the variables we need in order to traverse the VMIntConstantEntry[]
    long intConstantEntryNameOffset;
    long intConstantEntryValueOffset;
    long intConstantEntryArrayStride;
    
    intConstantEntryNameOffset  = getLongValueFromProcess("gHotSpotVMIntConstantEntryNameOffset");
    intConstantEntryValueOffset = getLongValueFromProcess("gHotSpotVMIntConstantEntryValueOffset");
    intConstantEntryArrayStride = getLongValueFromProcess("gHotSpotVMIntConstantEntryArrayStride");

    // Fetch the address of the VMIntConstantEntry*
    Address entryAddr = lookupInProcess("gHotSpotVMIntConstants");
    // Dereference this once to get the pointer to the first VMIntConstantEntry
    entryAddr = entryAddr.getAddressAt(0);
    if (entryAddr == null) {
      throw new RuntimeException("gHotSpotVMIntConstants was not initialized properly in the remote process; can not continue");
    }

    // Start iterating down it until we find an entry with no name
    Address nameAddr = null;
    do {
      // Fetch the type name first
      nameAddr = entryAddr.getAddressAt(intConstantEntryNameOffset);
      if (nameAddr != null) {
        String name = CStringUtilities.getString(nameAddr);
        int value = (int) entryAddr.getCIntegerAt(intConstantEntryValueOffset, C_INT32_SIZE, false);

        // Be a little resilient
        Integer oldValue = lookupIntConstant(name, false);
        if (oldValue == null) {
          addIntConstant(name, value);
        } else {
          if (oldValue.intValue() != value) {
            throw new RuntimeException("Error: the integer constant \"" + name +
                                       "\" had its value redefined (old was " + oldValue +
                                       ", new is " + value + ". Aborting.");
          } else {
            System.err.println("Warning: the int constant \"" + name + "\" (declared in the remote VM in VMStructs::localHotSpotVMIntConstants) " +
                               "had its value declared as " + value + " twice. Continuing.");
          }
        }
      }

      entryAddr = entryAddr.addOffsetTo(intConstantEntryArrayStride);
    } while (nameAddr != null);
  }

  private void readVMLongConstants() {
    // Get the variables we need in order to traverse the VMLongConstantEntry[]
    long longConstantEntryNameOffset;
    long longConstantEntryValueOffset;
    long longConstantEntryArrayStride;
    
    longConstantEntryNameOffset  = getLongValueFromProcess("gHotSpotVMLongConstantEntryNameOffset");
    longConstantEntryValueOffset = getLongValueFromProcess("gHotSpotVMLongConstantEntryValueOffset");
    longConstantEntryArrayStride = getLongValueFromProcess("gHotSpotVMLongConstantEntryArrayStride");

    // Fetch the address of the VMLongConstantEntry*
    Address entryAddr = lookupInProcess("gHotSpotVMLongConstants");
    // Dereference this once to get the pointer to the first VMLongConstantEntry
    entryAddr = entryAddr.getAddressAt(0);
    if (entryAddr == null) {
      throw new RuntimeException("gHotSpotVMLongConstants was not initialized properly in the remote process; can not continue");
    }

    // Start iterating down it until we find an entry with no name
    Address nameAddr = null;
    do {
      // Fetch the type name first
      nameAddr = entryAddr.getAddressAt(longConstantEntryNameOffset);
      if (nameAddr != null) {
        String name = CStringUtilities.getString(nameAddr);
        int value = (int) entryAddr.getCIntegerAt(longConstantEntryValueOffset, C_INT64_SIZE, true);

        // Be a little resilient
        Long oldValue = lookupLongConstant(name, false);
        if (oldValue == null) {
          addLongConstant(name, value);
        } else {
          if (oldValue.longValue() != value) {
            throw new RuntimeException("Error: the long constant \"" + name +
                                       "\" had its value redefined (old was " + oldValue +
                                       ", new is " + value + ". Aborting.");
          } else {
            System.err.println("Warning: the long constant \"" + name + "\" (declared in the remote VM in VMStructs::localHotSpotVMLongConstants) " +
                               "had its value declared as " + value + " twice. Continuing.");
          }
        }
      }

      entryAddr = entryAddr.addOffsetTo(longConstantEntryArrayStride);
    } while (nameAddr != null);
  }

  private BasicType lookupOrFail(String typeName) {
    BasicType type = (BasicType) lookupType(typeName, false);
    if (type == null) {
      throw new RuntimeException("Type \"" + typeName + "\", referenced in VMStructs::localHotSpotVMStructs in the remote VM, " +
                                 "was not present in the remote VMStructs::localHotSpotVMTypes table (should have been caught " +
                                 "in the debug build of that VM). Can not continue.");
    }
    return type;
  }

  private long getLongValueFromProcess(String symbol) {
    return lookupInProcess(symbol).getCIntegerAt(0, C_INT64_SIZE, true);
  }

  private Address lookupInProcess(String symbol) throws NoSuchSymbolException {
    // FIXME: abstract away the loadobject name
    for (int i = 0; i < jvmLibNames.length; i++) {
      Address addr = symbolLookup.lookup(jvmLibNames[i], symbol);
      if (addr != null) {
        return addr;
      }
    }
    String errStr = "(";
    for (int i = 0; i < jvmLibNames.length; i++) {
      errStr += jvmLibNames[i];
      if (i < jvmLibNames.length - 1) {
        errStr += ", ";
      }
    }
    errStr += ")";
    throw new NoSuchSymbolException(symbol,
                                    "Could not find symbol \"" + symbol +
                                    "\" in any of the known library names " +
                                    errStr);
  }

  private BasicType lookupOrCreateClass(String typeName, boolean isOopType,
                                        boolean isIntegerType, boolean isUnsigned) {
    BasicType type = (BasicType) lookupType(typeName, false);
    if (type == null) {
      // Create a new type
      type = createBasicType(typeName, isOopType, isIntegerType, isUnsigned);
    }
    return type;
  }

  /** Creates a new BasicType, initializes its size to -1 so we can
      test to ensure that all types' sizes are initialized by VMTypes,
      and adds it to the database. Takes care of initializing integer
      and oop types properly. */
  private BasicType createBasicType(String typeName, boolean isOopType,
                                    boolean isIntegerType, boolean isUnsigned) {
    
    BasicType type = null;

    if (isIntegerType) {
      type = new BasicCIntegerType(this, typeName, isUnsigned);
    } else {
      type = new BasicType(this, typeName);

      if (isOopType) {
        // HACK: turn markOop into a C integer type. This allows
        // proper handling of it in the Serviceability Agent. (FIXME
        // -- consider doing something different here)
        if (typeName.equals("markOop")) {
          type = new BasicCIntegerType(this, typeName, true);
        } else {
          type.setIsOopType(true);
        }
      }
    }

    type.setSize(UNINITIALIZED_SIZE);
    addType(type);
    return type;
  }
  
  /** "Virtual constructor" for fields based on type */
  private static Field createField(BasicTypeDataBase db, BasicType containingType,
                                   String name, Type type, boolean isStatic,
                                   long offset, Address staticFieldAddress) {
    // "Virtual constructor" based on type
    if (type.isOopType()) {
      return new BasicOopField(db, containingType, name, type,
                               isStatic, offset, staticFieldAddress);
    }

    if (type instanceof CIntegerType) {
      return new BasicCIntegerField(db, containingType, name, type,
                                    isStatic, offset, staticFieldAddress);
    }

    if (type.equals(db.getJBooleanType())) {
      return new BasicJBooleanField(db, containingType, name, type,
                                    isStatic, offset, staticFieldAddress);
    }
    
    if (type.equals(db.getJByteType())) {
      return new BasicJByteField(db, containingType, name, type,
                                 isStatic, offset, staticFieldAddress);
    }
    
    if (type.equals(db.getJCharType())) {
      return new BasicJCharField(db, containingType, name, type,
                                 isStatic, offset, staticFieldAddress);
    }
    
    if (type.equals(db.getJDoubleType())) {
      return new BasicJDoubleField(db, containingType, name, type,
                                   isStatic, offset, staticFieldAddress);
    }
    
    if (type.equals(db.getJFloatType())) {
      return new BasicJFloatField(db, containingType, name, type,
                                  isStatic, offset, staticFieldAddress);
    }
    
    if (type.equals(db.getJIntType())) {
      return new BasicJIntField(db, containingType, name, type,
                                isStatic, offset, staticFieldAddress);
    }
    
    if (type.equals(db.getJLongType())) {
      return new BasicJLongField(db, containingType, name, type,
                                 isStatic, offset, staticFieldAddress);
    }
    
    if (type.equals(db.getJShortType())) {
      return new BasicJShortField(db, containingType, name, type,
                                  isStatic, offset, staticFieldAddress);
    }
    
    // Unknown ("opaque") type. Instantiate ordinary Field.
    return new BasicField(db, containingType, name, type,
                          isStatic, offset, staticFieldAddress);
  }

  // For debugging
  private void dumpMemory(Address addr, int len) {
    int i = 0;
    while (i < len) {
      System.err.print(addr.addOffsetTo(i) + ":");
      for (int j = 0; j < 8 && i < len; i++, j++) {
        String s = Long.toHexString(addr.getCIntegerAt(i, 1, true));
        System.err.print(" 0x");
        for (int k = 0; k < 2 - s.length(); k++) {
          System.err.print("0");
        }
        System.err.print(s);
      }
      System.err.println();
    }
  }
}
