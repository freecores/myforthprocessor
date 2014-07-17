/*
 * @(#)InstanceKlass.java	1.13 03/01/23 11:42:51
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.oops;

import java.io.*;
import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.memory.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;
import sun.jvm.hotspot.utilities.*;

// An InstanceKlass is the VM level representation of a Java class.

public class InstanceKlass extends Klass {
  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }
  
  private static synchronized void initialize(TypeDataBase db) throws WrongTypeException {
    Type type            = db.lookupType("instanceKlass");
    arrayKlasses         = new OopField(type.getOopField("_array_klasses"), Oop.getHeaderSize());
    methods              = new OopField(type.getOopField("_methods"), Oop.getHeaderSize());
    methodOrdering       = new OopField(type.getOopField("_method_ordering"), Oop.getHeaderSize());
    localInterfaces      = new OopField(type.getOopField("_local_interfaces"), Oop.getHeaderSize());
    transitiveInterfaces = new OopField(type.getOopField("_transitive_interfaces"), Oop.getHeaderSize());
    nofImplementors      = new CIntField(type.getCIntegerField("_nof_implementors"), Oop.getHeaderSize());
    implementor          = new OopField(type.getOopField("_implementor"), Oop.getHeaderSize());
    fields               = new OopField(type.getOopField("_fields"), Oop.getHeaderSize());
    constants            = new OopField(type.getOopField("_constants"), Oop.getHeaderSize());
    classLoader          = new OopField(type.getOopField("_class_loader"), Oop.getHeaderSize());
    protectionDomain     = new OopField(type.getOopField("_protection_domain"), Oop.getHeaderSize());
    signers              = new OopField(type.getOopField("_signers"), Oop.getHeaderSize());
    sourceFileName       = new OopField(type.getOopField("_source_file_name"), Oop.getHeaderSize());
    innerClasses         = new OopField(type.getOopField("_inner_classes"), Oop.getHeaderSize());
    nonstaticFieldSize   = new CIntField(type.getCIntegerField("_nonstatic_field_size"), Oop.getHeaderSize());
    staticFieldSize      = new CIntField(type.getCIntegerField("_static_field_size"), Oop.getHeaderSize());
    staticOopFieldSize   = new CIntField(type.getCIntegerField("_static_oop_field_size"), Oop.getHeaderSize());
    nonstaticOopMapSize  = new CIntField(type.getCIntegerField("_nonstatic_oop_map_size"), Oop.getHeaderSize());
    isMarkedDependent    = new IntField(type.getJIntField("_is_marked_dependent"), Oop.getHeaderSize());
    initState            = new CIntField(type.getCIntegerField("_init_state"), Oop.getHeaderSize());
    vtableLen            = new CIntField(type.getCIntegerField("_vtable_len"), Oop.getHeaderSize());
    itableLen            = new CIntField(type.getCIntegerField("_itable_len"), Oop.getHeaderSize());
    breakpoints          = type.getAddressField("_breakpoints");
    headerSize           = alignObjectOffset(Oop.getHeaderSize() + type.getSize());
  }

  InstanceKlass(OopHandle handle, ObjectHeap heap) {
    super(handle, heap);
  }

  private static OopField  arrayKlasses;
  private static OopField  methods;
  private static OopField  methodOrdering;
  private static OopField  localInterfaces;
  private static OopField  transitiveInterfaces;
  private static CIntField nofImplementors;
  private static OopField  implementor;
  private static OopField  fields;
  private static OopField  constants;
  private static OopField  classLoader;
  private static OopField  protectionDomain;
  private static OopField  signers;
  private static OopField  sourceFileName;
  private static OopField  innerClasses;
  private static CIntField nonstaticFieldSize;
  private static CIntField staticFieldSize;
  private static CIntField staticOopFieldSize;
  private static CIntField nonstaticOopMapSize;
  private static IntField  isMarkedDependent;
  private static CIntField initState;
  private static CIntField vtableLen;
  private static CIntField itableLen;
  private static AddressField breakpoints;

  // Byteside of the header
  private static long headerSize;

  public static long getHeaderSize() { return headerSize; }

  // Accessors for declared fields
  public Klass     getArrayKlasses()        { return (Klass)        arrayKlasses.getValue(this); }
  public ObjArray  getMethods()             { return (ObjArray)     methods.getValue(this); }
  public TypeArray getMethodOrdering()      { return (TypeArray)    methodOrdering.getValue(this); }
  public ObjArray  getLocalInterfaces()     { return (ObjArray)     localInterfaces.getValue(this); }
  public ObjArray  getTransitiveInterfaces() { return (ObjArray)     transitiveInterfaces.getValue(this); }
  public long      nofImplementors()        { return                nofImplementors.getValue(this); }
  public Klass     getImplementor()         { return (Klass)        implementor.getValue(this); }
  public TypeArray getFields()              { return (TypeArray)    fields.getValue(this); }
  public ConstantPool getConstants()        { return (ConstantPool) constants.getValue(this); }
  public Oop       getClassLoader()         { return                classLoader.getValue(this); }
  public Oop       getProtectionDomain()    { return                protectionDomain.getValue(this); }
  public ObjArray  getSigners()             { return (ObjArray)     signers.getValue(this); }
  public Symbol    getSourceFileName()      { return (Symbol)       sourceFileName.getValue(this); }
  public TypeArray getInnerClasses()        { return (TypeArray)    innerClasses.getValue(this); }
  public long      getNonstaticFieldSize()  { return                nonstaticFieldSize.getValue(this); }
  public long      getStaticFieldSize()     { return                staticFieldSize.getValue(this); }
  public long      getStaticOopFieldSize()  { return                staticOopFieldSize.getValue(this); }
  public long      getNonstaticOopMapSize() { return                nonstaticOopMapSize.getValue(this); }
  public int       getIsMarkedDependent()   { return                isMarkedDependent.getValue(this); }
  public long      getInitState()           { return                initState.getValue(this); }
  public long      getVtableLen()           { return                vtableLen.getValue(this); }
  public long      getItableLen()           { return                itableLen.getValue(this); }

  public void printValueOn(PrintStream tty) {
    tty.print("InstanceKlass for " + getName().asString());
  }

  public void iterateFields(OopVisitor visitor, boolean doVMFields) {
    super.iterateFields(visitor, doVMFields);
    if (doVMFields) {
      visitor.doOop(arrayKlasses, true);
      visitor.doOop(methods, true);
      visitor.doOop(methodOrdering, true);
      visitor.doOop(localInterfaces, true);
      visitor.doOop(transitiveInterfaces, true);
      visitor.doCInt(nofImplementors, true);
      visitor.doOop(implementor, true);
      visitor.doOop(fields, true);
      visitor.doOop(constants, true);
      visitor.doOop(classLoader, true);
      visitor.doOop(protectionDomain, true);
      visitor.doOop(signers, true);
      visitor.doOop(sourceFileName, true);
      visitor.doOop(innerClasses, true);
      visitor.doCInt(nonstaticFieldSize, true);
      visitor.doCInt(staticFieldSize, true);
      visitor.doCInt(staticOopFieldSize, true);
      visitor.doCInt(nonstaticOopMapSize, true);
      visitor.doInt(isMarkedDependent, true);
      visitor.doCInt(initState, true);
      visitor.doCInt(vtableLen, true);
      visitor.doCInt(itableLen, true);
    }

    TypeArray fields = getFields();
    int length = (int) fields.getLength();
    for (int index = 0; index < length; index += 6) {
      short accessFlags    = fields.getShortAt(index + 0);
      short signatureIndex = fields.getShortAt(index + 2);
            
      FieldType   type   = new FieldType((Symbol) getConstants().getObjAt(signatureIndex));
      AccessFlags access = new AccessFlags(accessFlags);
      if (access.isStatic()) {
        visitField(visitor, type, index);
      }
    }
  }

  // FIXME: read these constants from the VM
  public static final int ACCESS_FLAGS_OFFSET = 0;
  public static final int NAME_INDEX_OFFSET = 1;
  public static final int SIGNATURE_INDEX_OFFSET = 2;
  public static final int INITVAL_INDEX_OFFSET = 3;
  public static final int LOW_OFFSET = 4;
  public static final int HIGH_OFFSET = 5;
  public static final int NEXT_OFFSET = 6;

  public void iterateNonStaticFields(OopVisitor visitor) {
    if (getSuper() != null) {
      ((InstanceKlass) getSuper()).iterateNonStaticFields(visitor);
    }    
    TypeArray fields = getFields();

    int length = (int) fields.getLength();
    for (int index = 0; index < length; index += NEXT_OFFSET) {
      short accessFlags    = fields.getShortAt(index + ACCESS_FLAGS_OFFSET);
      short signatureIndex = fields.getShortAt(index + SIGNATURE_INDEX_OFFSET);
            
      FieldType   type   = new FieldType((Symbol) getConstants().getObjAt(signatureIndex));
      AccessFlags access = new AccessFlags(accessFlags);
      if (!access.isStatic()) {
        visitField(visitor, type, index);
      }
    }
  }

  /** Field access by name. */
  public Field findLocalField(Symbol name, Symbol sig) {
    TypeArray fields = getFields();
    int n = (int) fields.getLength();
    ConstantPool cp = getConstants();
    for (int i = 0; i < n; i += NEXT_OFFSET) {
      int nameIndex = fields.getShortAt(i + NAME_INDEX_OFFSET);
      int sigIndex  = fields.getShortAt(i + SIGNATURE_INDEX_OFFSET);
      Symbol f_name = cp.getSymbolAt(nameIndex);
      Symbol f_sig  = cp.getSymbolAt(sigIndex);
      if (name.equals(f_name) && sig.equals(f_sig)) {
        return newField(i);
      }
    }

    return null;
  }

  /** Find field in direct superinterfaces. */
  public Field findInterfaceField(Symbol name, Symbol sig) {
    ObjArray interfaces = getLocalInterfaces();
    int n = (int) interfaces.getLength();
    for (int i = 0; i < n; i++) {
      InstanceKlass intf1 = (InstanceKlass) interfaces.getObjAt(i);
      if (Assert.ASSERTS_ENABLED) {
        Assert.that(intf1.isInterface(), "just checking type");
      }
      // search for field in current interface
      Field f = intf1.findLocalField(name, sig);
      if (f != null) {
        if (Assert.ASSERTS_ENABLED) {
          Assert.that(f.getAccessFlagsObj().isStatic(), "interface field must be static");
        }
        return f;
      }
      // search for field in direct superinterfaces
      f = intf1.findInterfaceField(name, sig);
      if (f != null) return f;
    }
    // otherwise field lookup fails
    return null;
  }

  /** Find field according to JVM spec 5.4.3.2, returns the klass in
      which the field is defined. */
  public Field findField(Symbol name, Symbol sig) {
    // search order according to newest JVM spec (5.4.3.2, p.167).
    // 1) search for field in current klass
    Field f = findLocalField(name, sig);
    if (f != null) return f;

    // 2) search for field recursively in direct superinterfaces
    f = findInterfaceField(name, sig);
    if (f != null) return f;

    // 3) apply field lookup recursively if superclass exists
    InstanceKlass supr = (InstanceKlass) getSuper();
    if (supr != null) return supr.findField(name, sig);

    // 4) otherwise field lookup fails
    return null;
  }

  /** Find field according to JVM spec 5.4.3.2, returns the klass in
      which the field is defined (convenience routine) */
  public Field findField(String name, String sig) {
    SymbolTable symbols = VM.getVM().getSymbolTable();
    Symbol nameSym = symbols.probe(name);
    Symbol sigSym  = symbols.probe(sig);
    if (nameSym == null || sigSym == null) {
      return null;
    }
    return findField(nameSym, sigSym);
  }

  /** Find field according to JVM spec 5.4.3.2, returns the klass in
      which the field is defined (retained only for backward
      compatibility with jdbx) */
  public Field findFieldDbg(String name, String sig) {
    return findField(name, sig);
  }

  /** Get field by its index in the fields array. Only designed for
      use in a debugging system. */
  public Field getFieldByIndex(int fieldArrayIndex) {
    return newField(fieldArrayIndex);
  }

    //fixme jjh:  To cache lists of SA objects or not?
    // Currently, the following methods don't cache.  Move the
    // List xxx local vars out to be ivars to make them cache,
    // but then beware of sync issues.
    //

    /** Return a List of SA Fields for the fields declared in this class.
        Inherited fields are not included.
        Return an empty list if there are no fields declared in this class.
        Only designed for use in a debugging system. */
    public List getImmediateFields() {
        // A list of Fields for each field declared in this class/interface,
        // not including inherited fields.
        List immediateFields = null;
        if (immediateFields == null) {
            TypeArray fields = getFields();
            
            int length = (int) fields.getLength();
            immediateFields = new ArrayList(length / NEXT_OFFSET);
            for (int index = 0; index < length; index += NEXT_OFFSET) {
                immediateFields.add(getFieldByIndex(index));
            }
        }
        return immediateFields;
    }

    /** Return a List of SA Fields for all the java fields in this class,
        including all inherited fields.  This includes hidden
        fields.  Thus the returned list can contain fields with
        the same name.
        Return an empty list if there are no fields.
        Only designed for use in a debugging system. */
    public List getAllFields() {
        // Contains a Field for each field in this class, including immediate
        // fields and inherited fields.
        List allFields = null;

        if (allFields == null) {
            allFields = getImmediateFields();
            
            // transitiveInterfaces contains all interfaces implemented
            // by this class and its superclass chain with no duplicates.
            
            ObjArray interfaces = getTransitiveInterfaces();
            int n = (int) interfaces.getLength();
            for (int i = 0; i < n; i++) {
                InstanceKlass intf1 = (InstanceKlass) interfaces.getObjAt(i);
                if (Assert.ASSERTS_ENABLED) {
                    Assert.that(intf1.isInterface(), "just checking type");
                }
                allFields.addAll(intf1.getImmediateFields());
            }
            
            // Get all fields in the superclass, recursively.  But, don't
            // include fields in interfaces implemented by superclasses;
            // we already have all those.
            if (!isInterface()) {
                InstanceKlass supr;
                if /*fixme jj: while*/ ( (supr = (InstanceKlass) getSuper()) != null) {
                    allFields.addAll(supr.getImmediateFields());
                }
            }
        }
        return allFields;
    }

    
    /** Return a List of SA Methods declared directly in this class/interface.
        Return an empty list if there are none, or if this isn't a class/
        interface.
    */
    public List getImmediateMethods() {
        // Contains a Method for each method declared in this class/interface
        // not including inherited methods.
        List immediateMethods = null;
      if (immediateMethods == null) {
        ObjArray methods = getMethods();
        int length = (int)methods.getLength();
        immediateMethods = new ArrayList(length);

        for (int index = 0; index < length; index ++) {
          immediateMethods.add(methods.getObjAt(index));
        }
      }
      return immediateMethods;
    }

    /** Return a List of SA Methods declared directly in this class/interface
        or its superclasses or in its implemented methods.  EG, if
        method gus is declared in an implemented interface and in two superclasses,
        all three gus's will appear.
        Return an empty list if there are none, or if this isn't a class/
        interface.
    */
    public List getAllMethods() { //fixme jjh:  I don't think this is used anymore
        // Contains a Method for each method declared or inherited in this
        // class/interface.
        List allMethods = null;

        if (allMethods != null) {
            return allMethods;
        }
        
        // Get methods actually declared in this class.
        allMethods = getImmediateMethods();

        // Get methods declared in all interfaces implemented by this
        // class and its supers.
        ObjArray interfaces = getTransitiveInterfaces();
        int n = (int) interfaces.getLength();
        for (int i = 0; i < n; i++) {
            InstanceKlass intf1 = (InstanceKlass) interfaces.getObjAt(i);
            if (Assert.ASSERTS_ENABLED) {
                Assert.that(intf1.isInterface(), "just checking type");
            }
            allMethods.addAll(intf1.getImmediateMethods());
        }
        
        // Add methods declared in each superclass.
        if (!isInterface()) {
            InstanceKlass supr;
            if /*fixme: jj: while*/ ( (supr = (InstanceKlass) getSuper()) != null) {
                allMethods.addAll(supr.getImmediateMethods());
            }
        }
        return allMethods;
    }

    /** Return a List containing an SA InstanceKlass for each
        interface named in this class's 'implements' clause.
    */
    public List getDirectImplementedInterfaces() {
        // Contains an InstanceKlass for each interface in this classes
        // 'implements' clause.
        List directImplementedInterfaces = null;
        if (directImplementedInterfaces == null) {
            ObjArray interfaces = getLocalInterfaces();
            int length = (int) interfaces.getLength();
            directImplementedInterfaces = new ArrayList(length);
            
            for (int index = 0; index < length; index ++) {
                directImplementedInterfaces.add(interfaces.getObjAt(index));
            }
        }
        return directImplementedInterfaces;
    }


  public long getObjectSize() {
    long bodySize =    alignObjectOffset(getVtableLen() * getHeap().getOopSize())
                     + alignObjectOffset(getItableLen() * getHeap().getOopSize())
                     + (getStaticFieldSize() + getNonstaticOopMapSize()) * getHeap().getOopSize();
    return alignObjectSize(headerSize + bodySize);
  }

  Klass arrayKlassImpl(InstanceKlass thisOop, boolean orNull, int n) {
    // FIXME: in reflective system this would need to change to
    // actually allocate
    if (thisOop.getArrayKlasses() == null) { return null; }
    ObjArrayKlass oak = (ObjArrayKlass) thisOop.getArrayKlasses();
    if (orNull) {
      return oak.arrayKlassOrNull(n);
    }
    return oak.arrayKlass(n);
  }

  public Klass arrayKlassImpl(boolean orNull) {
    return arrayKlassImpl(orNull, 1);
  }
  
  public String baseSourceName() {
     return getSourceFileName().asString();
  }

  public String signature() {
      return "L" + super.signature() + ";";
  }

  /** Convenience routine taking Strings; lookup is done in
      SymbolTable. */
  public Method findMethod(String name, String sig) {
    SymbolTable syms = VM.getVM().getSymbolTable();
    Symbol nameSym = syms.probe(name);
    Symbol sigSym  = syms.probe(sig);
    if (nameSym == null || sigSym == null) {
      return null;
    }
    return findMethod(nameSym, sigSym);
  }

  /** Find method in vtable. */
  public Method findMethod(Symbol name, Symbol sig) {
    return findMethod(getMethods(), name, sig);
  }
  
  /** Breakpoint support (see methods on methodOop for details) */
  public BreakpointInfo getBreakpoints() {
    Address addr = getHandle().getAddressAt(Oop.getHeaderSize() + breakpoints.getOffset());
    return (BreakpointInfo) VMObjectFactory.newObject(BreakpointInfo.class, addr);
  }

  //----------------------------------------------------------------------
  // Internals only below this point
  //

  private void visitField(OopVisitor visitor, FieldType type, int index) {
    Field f = newField(index);
    if (type.isOop()) {
      visitor.doOop((OopField) f, false);
      return;
    }
    if (type.isByte()) {
      visitor.doByte((ByteField) f, false);
      return;
    }
    if (type.isChar()) {
      visitor.doChar((CharField) f, false);
      return;
    }
    if (type.isDouble()) {
      visitor.doDouble((DoubleField) f, false);
      return;
    }
    if (type.isFloat()) {
      visitor.doFloat((FloatField) f, false);
      return;
    }
    if (type.isInt()) {
      visitor.doInt((IntField) f, false);
      return;
    }
    if (type.isLong()) {
      visitor.doLong((LongField) f, false);
      return;
    }
    if (type.isShort()) {
      visitor.doShort((ShortField) f, false);
      return;
    }
    if (type.isBoolean()) {
      visitor.doBoolean((BooleanField) f, false);
      return;
    }
  }

  // Creates new field from index in fields TypeArray
  private Field newField(int index) {
    TypeArray fields = getFields();
    short signatureIndex = fields.getShortAt(index + SIGNATURE_INDEX_OFFSET);
    FieldType type = new FieldType((Symbol) getConstants().getObjAt(signatureIndex));
    if (type.isOop()) {
      return new OopField(this, index);
    }
    if (type.isByte()) {
      return new ByteField(this, index);
    }
    if (type.isChar()) {
      return new CharField(this, index);
    }
    if (type.isDouble()) {
      return new DoubleField(this, index);
    }
    if (type.isFloat()) {
      return new FloatField(this, index);
    }
    if (type.isInt()) {
      return new IntField(this, index);
    }
    if (type.isLong()) {
      return new LongField(this, index);
    }
    if (type.isShort()) {
      return new ShortField(this, index);
    }
    if (type.isBoolean()) {
      return new BooleanField(this, index);
    }
    throw new RuntimeException("Illegal field type at index " + index);
  }

  private static Method findMethod(ObjArray methods, Symbol name, Symbol signature) {
    int len = (int) methods.getLength();
    // methods are sorted, so do binary search
    int l = 0;
    int h = len - 1;
    while (l <= h) {
      int mid = (l + h) >> 1;
      Method m = (Method) methods.getObjAt(mid);
      int res = m.getName().fastCompare(name);
      if (res == 0) {
        // found matching name; do linear search to find matching signature
        // first, quick check for common case 
        if (m.getSignature().equals(signature)) return m;
        // search downwards through overloaded methods
        int i;
        for (i = mid - 1; i >= l; i--) {
          Method m1 = (Method) methods.getObjAt(i);
          if (!m1.getName().equals(name)) break;
          if (m1.getSignature().equals(signature)) return m1;
        }
        // search upwards
        for (i = mid + 1; i <= h; i++) {
          Method m1 = (Method) methods.getObjAt(i);
          if (!m1.getName().equals(name)) break;
          if (m1.getSignature().equals(signature)) return m1;
        }
        // not found
        if (Assert.ASSERTS_ENABLED) {
          int index = linearSearch(methods, name, signature);
          if (index != -1) {
            throw new DebuggerException("binary search bug: should have found entry " + index);
          }
        }
        return null;
      } else if (res < 0) {
        l = mid + 1;
      } else {
        h = mid - 1;
      }
    }
    if (Assert.ASSERTS_ENABLED) {
      int index = linearSearch(methods, name, signature);
      if (index != -1) {
        throw new DebuggerException("binary search bug: should have found entry " + index);
      }
    }
    return null;
  }

  private static int linearSearch(ObjArray methods, Symbol name, Symbol signature) {
    int len = (int) methods.getLength();
    for (int index = 0; index < len; index++) {
      Method m = (Method) methods.getObjAt(index);
      if (m.getSignature().equals(signature) && m.getName().equals(name)) {
        return index;
      }
    }
    return -1;
  }
}
