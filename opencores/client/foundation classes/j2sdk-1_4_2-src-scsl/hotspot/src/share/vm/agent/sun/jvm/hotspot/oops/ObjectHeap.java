/*
 * @(#)ObjectHeap.java	1.17 03/01/23 11:43:38
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

//
// The ObjectHeap is an abstraction over all generations in the VM
// It gives access to all present objects and classes.
//

package sun.jvm.hotspot.oops;

import java.util.*;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.memory.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;
import sun.jvm.hotspot.utilities.*;

public class ObjectHeap {

  private OopHandle              symbolKlassHandle;
  private OopHandle              methodKlassHandle;
  private OopHandle              methodDataKlassHandle;
  private OopHandle              constantPoolKlassHandle;
  private OopHandle              constantPoolCacheKlassHandle;
  private OopHandle              klassKlassHandle;
  private OopHandle              instanceKlassKlassHandle;
  private OopHandle              typeArrayKlassKlassHandle;
  private OopHandle              objArrayKlassKlassHandle;
  private OopHandle              boolArrayKlassHandle;
  private OopHandle              byteArrayKlassHandle;
  private OopHandle              charArrayKlassHandle;
  private OopHandle              intArrayKlassHandle;
  private OopHandle              shortArrayKlassHandle;
  private OopHandle              longArrayKlassHandle;
  private OopHandle              singleArrayKlassHandle;
  private OopHandle              doubleArrayKlassHandle;
  private OopHandle              arrayKlassKlassHandle;
  private OopHandle              compiledICHolderKlassHandle;

  private SymbolKlass            symbolKlassObj;
  private MethodKlass            methodKlassObj;
  private MethodDataKlass        methodDataKlassObj;
  private ConstantPoolKlass      constantPoolKlassObj; 
  private ConstantPoolCacheKlass constantPoolCacheKlassObj;
  private KlassKlass             klassKlassObj;
  private InstanceKlassKlass     instanceKlassKlassObj;
  private TypeArrayKlassKlass    typeArrayKlassKlassObj;
  private ObjArrayKlassKlass     objArrayKlassKlassObj;
  private TypeArrayKlass         boolArrayKlassObj;
  private TypeArrayKlass         byteArrayKlassObj;
  private TypeArrayKlass         charArrayKlassObj;
  private TypeArrayKlass         intArrayKlassObj;
  private TypeArrayKlass         shortArrayKlassObj;
  private TypeArrayKlass         longArrayKlassObj;
  private TypeArrayKlass         singleArrayKlassObj;
  private TypeArrayKlass         doubleArrayKlassObj;
  private ArrayKlassKlass        arrayKlassKlassObj;
  private CompiledICHolderKlass  compiledICHolderKlassObj;
  private Method                 finalizerRegisterMethod;

  public void initialize(TypeDataBase db) throws WrongTypeException {
    // Lookup the roots in the object hierarchy.
    Type universeType = db.lookupType("Universe");

    symbolKlassHandle         = universeType.getOopField("_symbolKlassObj").getValue();
    symbolKlassObj            = new SymbolKlass(symbolKlassHandle, this);

    methodKlassHandle         = universeType.getOopField("_methodKlassObj").getValue();
    methodKlassObj            = new MethodKlass(methodKlassHandle, this);

    constantPoolKlassHandle   = universeType.getOopField("_constantPoolKlassObj").getValue();
    constantPoolKlassObj      = new ConstantPoolKlass(constantPoolKlassHandle, this);

    constantPoolCacheKlassHandle = universeType.getOopField("_constantPoolCacheKlassObj").getValue();
    constantPoolCacheKlassObj = new ConstantPoolCacheKlass(constantPoolCacheKlassHandle, this);

    klassKlassHandle          = universeType.getOopField("_klassKlassObj").getValue();
    klassKlassObj             = new KlassKlass(klassKlassHandle, this);

    arrayKlassKlassHandle     = universeType.getOopField("_arrayKlassKlassObj").getValue();
    arrayKlassKlassObj        = new ArrayKlassKlass(arrayKlassKlassHandle, this);

    instanceKlassKlassHandle  = universeType.getOopField("_instanceKlassKlassObj").getValue();
    instanceKlassKlassObj     = new InstanceKlassKlass(instanceKlassKlassHandle, this);

    typeArrayKlassKlassHandle = universeType.getOopField("_typeArrayKlassKlassObj").getValue();
    typeArrayKlassKlassObj    = new TypeArrayKlassKlass(typeArrayKlassKlassHandle, this);

    objArrayKlassKlassHandle  = universeType.getOopField("_objArrayKlassKlassObj").getValue();
    objArrayKlassKlassObj     = new ObjArrayKlassKlass(objArrayKlassKlassHandle, this);

    boolArrayKlassHandle      = universeType.getOopField("_boolArrayKlassObj").getValue();
    boolArrayKlassObj         = new TypeArrayKlass(boolArrayKlassHandle, this);

    byteArrayKlassHandle      = universeType.getOopField("_byteArrayKlassObj").getValue();
    byteArrayKlassObj         = new TypeArrayKlass(byteArrayKlassHandle, this);

    charArrayKlassHandle      = universeType.getOopField("_charArrayKlassObj").getValue();
    charArrayKlassObj         = new TypeArrayKlass(charArrayKlassHandle, this);

    intArrayKlassHandle       = universeType.getOopField("_intArrayKlassObj").getValue();
    intArrayKlassObj          = new TypeArrayKlass(intArrayKlassHandle, this);

    shortArrayKlassHandle     = universeType.getOopField("_shortArrayKlassObj").getValue();
    shortArrayKlassObj        = new TypeArrayKlass(shortArrayKlassHandle, this);

    longArrayKlassHandle      = universeType.getOopField("_longArrayKlassObj").getValue();
    longArrayKlassObj         = new TypeArrayKlass(longArrayKlassHandle, this);

    singleArrayKlassHandle    = universeType.getOopField("_singleArrayKlassObj").getValue();
    singleArrayKlassObj       = new TypeArrayKlass(singleArrayKlassHandle, this);

    doubleArrayKlassHandle    = universeType.getOopField("_doubleArrayKlassObj").getValue();
    doubleArrayKlassObj       = new TypeArrayKlass(doubleArrayKlassHandle, this);

    if (!VM.getVM().isCore()) {
      methodDataKlassHandle   = universeType.getOopField("_methodDataKlassObj").getValue();
      methodDataKlassObj      = new MethodDataKlass(methodDataKlassHandle, this);

      compiledICHolderKlassHandle = universeType.getOopField("_compiledICHolderKlassObj").getValue();
      compiledICHolderKlassObj= new CompiledICHolderKlass(compiledICHolderKlassHandle ,this);
    }

    OopHandle handle          = universeType.getOopField("_finalizer_register_method").getValue();
    finalizerRegisterMethod   = new Method(handle, this);
  }

  public ObjectHeap(TypeDataBase db) throws WrongTypeException {
    // Get commonly used sizes of basic types
    oopSize     = db.getOopSize();
    byteSize    = db.getJByteType().getSize();
    charSize    = db.getJCharType().getSize();
    booleanSize = db.getJBooleanType().getSize();
    intSize     = db.getJIntType().getSize();
    shortSize   = db.getJShortType().getSize();
    longSize    = db.getJLongType().getSize();
    floatSize   = db.getJFloatType().getSize();
    doubleSize  = db.getJDoubleType().getSize();

    initialize(db);
  }

  /** Comparison operation for oops, either or both of which may be null */
  public boolean equal(Oop o1, Oop o2) {
    if (o1 != null) return o1.equals(o2);
    return (o2 == null);
  }

  // Cached sizes of basic types
  private long oopSize;
  private long byteSize;
  private long charSize;
  private long booleanSize;
  private long intSize;
  private long shortSize;
  private long longSize;
  private long floatSize;
  private long doubleSize;

  long getOopSize()     { return oopSize;     }
  long getByteSize()    { return byteSize;    }
  long getCharSize()    { return charSize;    }
  long getBooleanSize() { return booleanSize; }
  long getIntSize()     { return intSize;     }
  long getShortSize()   { return shortSize;   }
  long getLongSize()    { return longSize;    }
  long getFloatSize()   { return floatSize;   }
  long getDoubleSize()  { return doubleSize;  }

  // Accessors for well-known system classes (from Universe)
  public SymbolKlass            getSymbolKlassObj()            { return symbolKlassObj; }
  public MethodKlass            getMethodKlassObj()            { return methodKlassObj; }
  public MethodDataKlass        getMethodDataKlassObj()        { return methodDataKlassObj; }
  public ConstantPoolKlass      getConstantPoolKlassObj()      { return constantPoolKlassObj; } 
  public ConstantPoolCacheKlass getConstantPoolCacheKlassObj() { return constantPoolCacheKlassObj; } 
  public KlassKlass             getKlassKlassObj()             { return klassKlassObj; } 
  public ArrayKlassKlass        getArrayKlassKlassObj()        { return arrayKlassKlassObj; } 
  public InstanceKlassKlass     getInstanceKlassKlassObj()     { return instanceKlassKlassObj; }
  public ObjArrayKlassKlass     getObjArrayKlassKlassObj()     { return objArrayKlassKlassObj; }
  public TypeArrayKlassKlass    getTypeArrayKlassKlassObj()    { return typeArrayKlassKlassObj; }
  public TypeArrayKlass         getBoolArrayKlassObj()         { return boolArrayKlassObj; }
  public TypeArrayKlass         getByteArrayKlassObj()         { return byteArrayKlassObj; }
  public TypeArrayKlass         getCharArrayKlassObj()         { return charArrayKlassObj; }
  public TypeArrayKlass         getIntArrayKlassObj()          { return intArrayKlassObj; }
  public TypeArrayKlass         getShortArrayKlassObj()        { return shortArrayKlassObj; }
  public TypeArrayKlass         getLongArrayKlassObj()         { return longArrayKlassObj; }
  public TypeArrayKlass         getSingleArrayKlassObj()       { return singleArrayKlassObj; }
  public TypeArrayKlass         getDoubleArrayKlassObj()       { return doubleArrayKlassObj; }
  public CompiledICHolderKlass  getCompiledICHolderKlassObj()  {
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(!VM.getVM().isCore(), "must not be called for core build");
    }
    return compiledICHolderKlassObj;
  }
  public Method                 getFinalizerRegisterMethod()   { return finalizerRegisterMethod; }

  /** Takes a BasicType and returns the corresponding primitive array
      klass */
  public Klass typeArrayKlassObj(int t) {
    if (t == BasicType.getTBoolean()) return getBoolArrayKlassObj();
    if (t == BasicType.getTChar())    return getCharArrayKlassObj();
    if (t == BasicType.getTFloat())   return getSingleArrayKlassObj();
    if (t == BasicType.getTDouble())  return getDoubleArrayKlassObj();
    if (t == BasicType.getTByte())    return getByteArrayKlassObj();
    if (t == BasicType.getTShort())   return getShortArrayKlassObj();
    if (t == BasicType.getTInt())     return getIntArrayKlassObj();
    if (t == BasicType.getTLong())    return getLongArrayKlassObj();
    throw new RuntimeException("Illegal basic type " + t);
  }

  /** The base heap iteration mechanism */
  public void iterate(HeapVisitor visitor) {
    List liveRegions = collectLiveRegions();

    // Summarize size
    long totalSize = 0;
    for (int i = 0; i < liveRegions.size(); i += 2) {
      Address bottom = (Address) liveRegions.get(i);
      Address top    = (Address) liveRegions.get(i+1);
      totalSize += top.minus(bottom);
    }
    visitor.prologue(totalSize);

    for (int i = 0; i < liveRegions.size(); i += 2) {
      Address bottom = (Address) liveRegions.get(i);
      Address top    = (Address) liveRegions.get(i+1);

      try {
        // Traverses the space from bottom to top
        OopHandle handle = bottom.addOffsetToAsOopHandle(0);
        while (handle.lessThan(top)) {
          Oop obj = newOop(handle);
          visitor.doObj(obj);
          handle = handle.addOffsetToAsOopHandle(obj.getObjectSize());
        }
      }
      catch (AddressException e) {
        // This is okay at the top of these regions
      }
      catch (UnknownOopException e) {
        // This is okay at the top of these regions
      }
    }

    visitor.epilogue();
  }

  /** This routine can be used to iterate through the heap at an
      extremely low level (stepping word-by-word) to provide the
      ability to do very low-level debugging */
  public void iterateRaw(RawHeapVisitor visitor) {
    List liveRegions = collectLiveRegions();

    // Summarize size
    long totalSize = 0;
    for (int i = 0; i < liveRegions.size(); i += 2) {
      Address bottom = (Address) liveRegions.get(i);
      Address top    = (Address) liveRegions.get(i+1);
      totalSize += top.minus(bottom);
    }
    visitor.prologue(totalSize);

    for (int i = 0; i < liveRegions.size(); i += 2) {
      Address bottom = (Address) liveRegions.get(i);
      Address top    = (Address) liveRegions.get(i+1);

      // Traverses the space from bottom to top
      while (bottom.lessThan(top)) {
        visitor.visitAddress(bottom);
        bottom = bottom.addOffsetTo(VM.getVM().getAddressSize());
      }
    }

    visitor.epilogue();
  }

  // Iterates through only the perm generation for the purpose of
  // finding static fields for liveness analysis
  public void iteratePerm(HeapVisitor visitor) {
    GenCollectedHeap heap = (GenCollectedHeap) VM.getVM().getUniverse().heap();
    Generation gen = heap.permGen();
    if (Assert.ASSERTS_ENABLED) {
      Assert.that((gen instanceof CompactingPermGenGen), "Expected CompactingPermGenGen perm gen, got " +
                  gen.getClass().getName());
    }
    CompactingPermGenGen perm = (CompactingPermGenGen) gen;
    ContiguousSpace space = perm.theSpace();
    Address bottom = space.bottom();
    Address top    = space.top();
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(bottom != null, "PermGen's bottom should not be null");
      Assert.that(top    != null, "PermGen's top should not be null");
    }

    try {
      // Traverses the space from bottom to top
      OopHandle handle = bottom.addOffsetToAsOopHandle(0);
      while (handle.lessThan(top)) {
        Oop obj = newOop(handle);
        visitor.doObj(obj);
        handle = handle.addOffsetToAsOopHandle(obj.getObjectSize());
      }
    }
    catch (AddressException e) {
      // This is okay at the top of these regions
    }
    catch (UnknownOopException e) {
      // This is okay at the top of these regions
    }
  }

  // Creates an instance from the Oop hierarchy based based on the handle
  public Oop newOop(OopHandle handle) {
    // The only known way to detect the right type of an oop is 
    // traversing the class chain until a well-known klass is recognized.
    // A more direct solution would require the klasses to expose
    // the C++ vtbl structure.

    // Handle the null reference
    if (handle == null) return null;

    // First check if handle is one of the root objects
    if (handle.equals(methodKlassHandle))              return getMethodKlassObj();
    if (handle.equals(symbolKlassHandle))              return getSymbolKlassObj();
    if (handle.equals(constantPoolKlassHandle))        return getConstantPoolKlassObj();
    if (handle.equals(constantPoolCacheKlassHandle))   return getConstantPoolCacheKlassObj();
    if (handle.equals(instanceKlassKlassHandle))       return getInstanceKlassKlassObj();
    if (handle.equals(objArrayKlassKlassHandle))       return getObjArrayKlassKlassObj();
    if (handle.equals(klassKlassHandle))               return getKlassKlassObj();
    if (handle.equals(arrayKlassKlassHandle))          return getArrayKlassKlassObj();
    if (handle.equals(typeArrayKlassKlassHandle))      return getTypeArrayKlassKlassObj();
    if (handle.equals(boolArrayKlassHandle))           return getBoolArrayKlassObj();
    if (handle.equals(byteArrayKlassHandle))           return getByteArrayKlassObj();
    if (handle.equals(charArrayKlassHandle))           return getCharArrayKlassObj();
    if (handle.equals(intArrayKlassHandle))            return getIntArrayKlassObj();
    if (handle.equals(shortArrayKlassHandle))          return getShortArrayKlassObj();
    if (handle.equals(longArrayKlassHandle))           return getLongArrayKlassObj();
    if (handle.equals(singleArrayKlassHandle))         return getSingleArrayKlassObj();
    if (handle.equals(doubleArrayKlassHandle))         return getDoubleArrayKlassObj();
    if (!VM.getVM().isCore()) {
      if (handle.equals(compiledICHolderKlassHandle))  return getCompiledICHolderKlassObj();
      if (handle.equals(methodDataKlassHandle))        return getMethodDataKlassObj();
    }

    // Then check if obj.klass() is one of the root objects
    OopHandle klass = Oop.getKlassForOopHandle(handle);
    if (klass != null) {
      if (klass.equals(methodKlassHandle))              return new Method(handle, this);
      if (klass.equals(symbolKlassHandle))              return new Symbol(handle, this);
      if (klass.equals(constantPoolKlassHandle))        return new ConstantPool(handle, this);
      if (klass.equals(constantPoolCacheKlassHandle))   return new ConstantPoolCache(handle, this);
      if (!VM.getVM().isCore()) {
        if (klass.equals(compiledICHolderKlassHandle))  return new CompiledICHolder(handle, this);
        if (klass.equals(methodDataKlassHandle))        return new MethodData(handle, this);
      }
      if (klass.equals(instanceKlassKlassHandle))       return new InstanceKlass(handle, this);
      if (klass.equals(objArrayKlassKlassHandle))       return new ObjArrayKlass(handle, this);
      if (klass.equals(typeArrayKlassKlassHandle))      return new TypeArrayKlass(handle, this);

      // Lastly check if obj.klass().klass() is on of the root objects
      OopHandle klassKlass = Oop.getKlassForOopHandle(klass);
      if (klassKlass != null) {
        if (klassKlass.equals(instanceKlassKlassHandle))    return new Instance(handle, this);
        if (klassKlass.equals(objArrayKlassKlassHandle))    return new ObjArray(handle, this);
        if (klassKlass.equals(typeArrayKlassKlassHandle))   return new TypeArray(handle, this);
      }
    }
    
    System.err.println("Unknown oop at " + handle);
    System.err.println("Oop's klass is " + klass);

    throw new UnknownOopException();
  }

  // Print all objects in the object heap
  public void print() {
    HeapPrinter printer = new HeapPrinter(System.out);
    iterate(printer);
  }

  //---------------------------------------------------------------------------
  // Internals only below this point
  //

  // Returns a List<Address> where the addresses come in pairs. These
  // designate the live regions of the heap.
  private List collectLiveRegions() {
    // We want to iterate through all live portions of the heap, but
    // do not want to abort the heap traversal prematurely if we find
    // a problem (like an allocated but uninitialized object at the
    // top of a generation). To do this we enumerate all generations'
    // bottom and top regions, and factor in TLABs if necessary.

    // List<Address>. Addresses come in pairs.
    List liveRegions = new ArrayList();

    GenCollectedHeap heap = (GenCollectedHeap) VM.getVM().getUniverse().heap();

    // Run through all generations, obtaining bottom-top pairs.
    for (int i = 0; i < heap.nGens(); i++) {
      Generation gen = heap.getGen(i);
      if (gen.kind() == GenerationName.DEF_NEW) {
        DefNewGeneration def = (DefNewGeneration) gen;
        // Add Eden's bottom and either top or proper top
        EdenSpace eden = def.eden();
        if (Assert.ASSERTS_ENABLED) {
          Assert.that(eden.bottom() != null, "Eden's bottom should not be null");
        }
        liveRegions.add(eden.bottom());
	if (Assert.ASSERTS_ENABLED) {
          Assert.that(eden.top() != null, "Eden's top should not be null");
	}
	liveRegions.add(eden.top());
        // Add from-space but not to-space
        ContiguousSpace from = def.from();
        if (Assert.ASSERTS_ENABLED) {
          Assert.that(from.bottom() != null, "From's bottom should not be null");
          Assert.that(from.top()    != null, "From's top should not be null");
        }
        liveRegions.add(from.bottom());
        liveRegions.add(from.top());
      } else {
        // OneContigSpaceCardGeneration
        OneContigSpaceCardGeneration card = (OneContigSpaceCardGeneration) gen;
        ContiguousSpace space = card.theSpace();
        if (Assert.ASSERTS_ENABLED) {
          Assert.that(space.bottom() != null, "Space " + i + "'s bottom should not be null");
          Assert.that(space.top()    != null, "Space " + i + "'s top should not be null");
        }
        liveRegions.add(space.bottom());
        liveRegions.add(space.top());
      }
    }

    {
      // Special-case perm gen
      Generation gen = heap.permGen();
      if (Assert.ASSERTS_ENABLED) {
        Assert.that((gen instanceof CompactingPermGenGen), "Expected CompactingPermGenGen perm gen, got " +
                    gen.getClass().getName());
      }
      CompactingPermGenGen perm = (CompactingPermGenGen) gen;
      ContiguousSpace space = perm.theSpace();
      if (Assert.ASSERTS_ENABLED) {
        Assert.that(space.bottom() != null, "PermGen's bottom should not be null");
        Assert.that(space.top()    != null, "PermGen's top should not be null");
      }
      liveRegions.add(space.bottom());
      liveRegions.add(space.top());
    }

    // If UseTLAB is enabled, snip out regions associated with TLABs'
    // dead regions. Note that TLABs can be present in any generation.

    // FIXME: consider adding fewer boundaries to live region list.
    // Theoretically only need to stop at TLAB's top and resume at its
    // end.

    if (VM.getVM().getUseTLAB()) {
      for (JavaThread thread = VM.getVM().getThreads().first(); thread != null; thread = thread.next()) {
        if (thread.isJavaThread()) {
          ThreadLocalAllocBuffer tlab = thread.tlab();
          if (tlab.start() != null) {
            if ((tlab.top() == null) || (tlab.end() == null)) {
              System.err.print("Warning: skipping invalid TLAB for thread ");
              thread.printThreadIDOn(System.err);
              System.err.println();
            } else {
              // Go from:
              //  - below start() to start()
              //  - start() to top()
              //  - end() and above
              liveRegions.add(tlab.start());
              liveRegions.add(tlab.start());
              liveRegions.add(tlab.top());
              liveRegions.add(tlab.end());
            }
          }
        }
      }
    }

    // Now sort live regions
    Collections.sort(liveRegions, new Comparator() {
        public int compare(Object o1, Object o2) {
          Address a1 = (Address) o1;
          Address a2 = (Address) o2;
          if (AddressOps.lt(a1, a2)) {
            return -1;
          } else if (AddressOps.gt(a1, a2)) {
            return 1;
          }
          return 0;
        }
      });
    
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(liveRegions.size() % 2 == 0, "Must have even number of region boundaries");
    }
    
    return liveRegions;
  }
}
