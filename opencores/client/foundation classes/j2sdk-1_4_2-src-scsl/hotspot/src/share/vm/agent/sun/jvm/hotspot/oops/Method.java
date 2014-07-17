/*
 * @(#)Method.java	1.21 03/01/23 11:43:15
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.oops;

import java.io.*;
import java.util.*;
import sun.jvm.hotspot.code.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.interpreter.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;
import sun.jvm.hotspot.utilities.*;

// A Method represents a Java method

public class Method extends Oop {
  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) throws WrongTypeException {
    Type type                  = db.lookupType("methodOopDesc");
    constants                  = new OopField(type.getOopField("_constants"), 0);
    exceptionTable             = new OopField(type.getOopField("_exception_table"), 0);
    methodSize                 = new CIntField(type.getCIntegerField("_method_size"), 0);
    maxStack                   = new CIntField(type.getCIntegerField("_max_stack"), 0);
    maxLocals                  = new CIntField(type.getCIntegerField("_max_locals"), 0);
    sizeOfParameters           = new CIntField(type.getCIntegerField("_size_of_parameters"), 0);
    nameIndex                  = new CIntField(type.getCIntegerField("_name_index"), 0);
    signatureIndex             = new CIntField(type.getCIntegerField("_signature_index"), 0);
    accessFlags                = new CIntField(type.getCIntegerField("_access_flags"), 0);
    codeSize                   = new CIntField(type.getCIntegerField("_code_size"), 0);
    code                       = type.getAddressField("_code");
    vtableIndex                = new CIntField(type.getCIntegerField("_vtable_index"), 0);
    if (!VM.getVM().isCore()) {
      invocationCounter        = new CIntField(type.getCIntegerField("_invocation_counter"), 0);
    }
    bytecodeOffset = type.getSize();

    type                       = db.lookupType("CheckedExceptionElement");
    checkedExceptionElementSize = type.getSize();

    type                       = db.lookupType("LocalVariableTableElement");
    localVariableTableElementSize = type.getSize();
    

    /*
    interpreterEntry           = type.getAddressField("_interpreter_entry");
    fromCompiledCodeEntryPoint = type.getAddressField("_from_compiled_code_entry_point");
    
    */
  }

  Method(OopHandle handle, ObjectHeap heap) {
    super(handle, heap);
  }

  public boolean isMethod()            { return true; }

  // Fields
  private static OopField  constants;
  private static OopField  exceptionTable;
  private static CIntField methodSize;
  private static CIntField maxStack;
  private static CIntField maxLocals;
  private static CIntField sizeOfParameters;
  private static CIntField nameIndex;
  private static CIntField signatureIndex;
  private static CIntField accessFlags;
  private static CIntField codeSize;
  private static CIntField vtableIndex;
  private static CIntField invocationCounter;
  private static long      bytecodeOffset;
  private static long      checkedExceptionElementSize;
  private static long      localVariableTableElementSize;

  
  private static AddressField       code;

  /*
  private static AddressCField       interpreterEntry;
  private static AddressCField       fromCompiledCodeEntryPoint;
  */

  // Accessors for declared fields
  public ConstantPool getConstants()                  { return (ConstantPool) constants.getValue(this);         }
  public TypeArray    getExceptionTable()             { return (TypeArray)    exceptionTable.getValue(this);    }
  /** WARNING: this is in words, not useful in this system; use getObjectSize() instead */
  public long         getMethodSize()                 { return                methodSize.getValue(this);        }
  public long         getMaxStack()                   { return                maxStack.getValue(this);          }
  public long         getMaxLocals()                  { return                maxLocals.getValue(this);         }
  public long         getSizeOfParameters()           { return                sizeOfParameters.getValue(this);  }
  public long         getNameIndex()                  { return                nameIndex.getValue(this);         }  
  public long         getSignatureIndex()             { return                signatureIndex.getValue(this);    }
  public long         getAccessFlags()                { return                accessFlags.getValue(this);       }
  public long         getCodeSize()                   { return                codeSize.getValue(this);          }
  public long         getVtableIndex()                { return                vtableIndex.getValue(this);       }
  public long         getInvocationCounter()          {
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(!VM.getVM().isCore(), "must not be used in core build");
    }
    return invocationCounter.getValue(this);
  }

  // get associated compiled native method, if available, else return null.
  public NMethod getNativeMethod() {
    Address addr = code.getValue(getHandle());
    return (NMethod) VMObjectFactory.newObject(NMethod.class, addr); 
  }

  // Convenience routine
  public AccessFlags getAccessFlagsObj() {
    return new AccessFlags(getAccessFlags());
  }

  /** Get a bytecode or breakpoint at the given bci */
  public int getBytecodeOrBPAt(int bci) {
    return getHandle().getJByteAt(bytecodeOffset + bci) & 0xFF;
  }

  /** Fetch the original non-breakpoint bytecode at the specified
      bci. It is required that there is currently a bytecode at this
      bci. */
  public int getOrigBytecodeAt(int bci) {
    BreakpointInfo bp = ((InstanceKlass) getMethodHolder()).getBreakpoints();
    for (; bp != null; bp = bp.getNext()) {
      if (bp.match(this, bci)) {
        return bp.getOrigBytecode();
      }
    }
    System.err.println("Requested bci " + bci);
    for (; bp != null; bp = bp.getNext()) {
      System.err.println("Breakpoint at bci " + bp.getBCI() + ", bytecode " +
                         bp.getOrigBytecode());
    }
    Assert.that(false, "Should not reach here");
    return -1; // not reached
  }

  public byte getBytecodeByteArg(int bci) {
    return (byte) getBytecodeOrBPAt(bci);
  }

  /** Fetches a 16-bit big-endian ("Java ordered") value from the
      bytecode stream */
  public short getBytecodeShortArg(int bci) {
    int hi = getBytecodeOrBPAt(bci);
    int lo = getBytecodeOrBPAt(bci + 1);
    return (short) ((hi << 8) | lo);
  }

  /** Fetches a 32-bit big-endian ("Java ordered") value from the
      bytecode stream */
  public int getBytecodeIntArg(int bci) {
    int b4 = getBytecodeOrBPAt(bci);
    int b3 = getBytecodeOrBPAt(bci + 1);
    int b2 = getBytecodeOrBPAt(bci + 2);
    int b1 = getBytecodeOrBPAt(bci + 3);
    
    return (b4 << 24) | (b3 << 16) | (b2 << 8) | b1;
  }

  public byte[] getByteCode()
  {
     byte[] bc = new byte[ (int) getCodeSize() ];
     for( int i=0; i < bc.length; i++ )
     {
        long offs = bytecodeOffset + i;
        bc[i] = getHandle().getJByteAt( offs );
     }
     return bc;
  }

  /*
  public Address      getCode()                       { return codeField.getValue(this); }
  public Address      getInterpreterEntry()           { return interpreterEntryField.getValue(this); }
  public Address      getFromCompiledCodeEntryPoint() { return fromCompiledCodeEntryPointField.getValue(this); }
  */
  // Accessors
  public Symbol  getName()          { return (Symbol) getConstants().getObjAt(getNameIndex());         }
  public Symbol  getSignature()     { return (Symbol) getConstants().getObjAt(getSignatureIndex());    }

  // Method holder (the Klass holding this method)
  public Klass   getMethodHolder()  { return getConstants().getPoolHolder();                           }

  // Access flags
  public boolean isPublic()         { return getAccessFlagsObj().isPublic();                           }
  public boolean isPrivate()        { return getAccessFlagsObj().isPrivate();                          }
  public boolean isProtected()      { return getAccessFlagsObj().isProtected();                        }
  public boolean isPackagePrivate() { AccessFlags af = getAccessFlagsObj();
                                      return (!af.isPublic() && !af.isPrivate() && !af.isProtected()); }
  public boolean isStatic()         { return getAccessFlagsObj().isStatic();                           }
  public boolean isFinal()          { return getAccessFlagsObj().isFinal();                            }
  public boolean isSynchronized()   { return getAccessFlagsObj().isSynchronized();                     }
  public boolean isNative()         { return getAccessFlagsObj().isNative();                           }
  public boolean isAbstract()       { return getAccessFlagsObj().isAbstract();                         }
  public boolean isStrict()         { return getAccessFlagsObj().isStrict();                           }
  public boolean isSynthetic()      { return getAccessFlagsObj().isSynthetic();                        }

  public OopMapCacheEntry getMaskFor(int bci) {
    OopMapCacheEntry entry = new OopMapCacheEntry();
    entry.fill(this, bci);
    return entry;
  }

  public long getObjectSize() {
    return getMethodSize() * getHeap().getOopSize();
  }

  public void printValueOn(PrintStream tty) {
    tty.print("Method " + getName().asString() + getSignature().asString() + "@" + getHandle());
  }

  public void iterateFields(OopVisitor visitor, boolean doVMFields) {
    super.iterateFields(visitor, doVMFields);
    if (doVMFields) {
      visitor.doOop(constants, true);
      visitor.doOop(exceptionTable, true);
      visitor.doCInt(methodSize, true);
      visitor.doCInt(maxStack, true);
      visitor.doCInt(maxLocals, true);
      visitor.doCInt(sizeOfParameters, true);
      visitor.doCInt(nameIndex, true);
      visitor.doCInt(signatureIndex, true);
    }
  }

  public boolean hasLineNumberTable() {
    return getAccessFlagsObj().hasLineNumberTable();
  }

  public int getLineNumberFromBCI(int bci) {
    if (!VM.getVM().isCore()) {
      if (bci == DebugInformationRecorder.SYNCHRONIZATION_ENTRY_BCI) bci = 0;
    }
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(bci == 0 || 0 <= bci && bci < getCodeSize(), "illegal bci");
    }
    int bestBCI  =  0;
    int bestLine = -1;
    if (hasLineNumberTable()) {
      // The line numbers are a short array of 2-tuples [start_pc, line_number].
      // Not necessarily sorted and not necessarily one-to-one.
      CompressedLineNumberReadStream stream =
        new CompressedLineNumberReadStream(getHandle(), (int) offsetOfCompressedLineNumberTable());
      while (stream.readPair()) {
        if (stream.bci() == bci) {
          // perfect match
          return stream.line();
        } else {
          // update best_bci/line
          if (stream.bci() < bci && stream.bci() >= bestBCI) {
            bestBCI  = stream.bci();
            bestLine = stream.line();
          }
        }
      }
    }
    return bestLine;
  }

  public LineNumberTableElement[] getLineNumberTable() {
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(hasLineNumberTable(),
                  "should only be called if table is present");
    }
    int len = getLineNumberTableLength();
    CompressedLineNumberReadStream stream =
      new CompressedLineNumberReadStream(getHandle(), (int) offsetOfCompressedLineNumberTable());
    LineNumberTableElement[] ret = new LineNumberTableElement[len];
    for (int idx = 0; idx < len; idx++) {
      stream.readPair();
      ret[idx] = new LineNumberTableElement(stream.bci(), stream.line());
    }
    return ret;
  }

  public boolean hasLocalVariableTable() {
    return getAccessFlagsObj().hasLocalVariableTable();
  }
  
  /** Should only be called if table is present */
  public LocalVariableTableElement[] getLocalVariableTable() {
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(hasLocalVariableTable(), "should only be called if table is present");
    }
    LocalVariableTableElement[] ret = new LocalVariableTableElement[getLocalVariableTableLength()];
    long offset = offsetOfLocalVariableTable();
    for (int i = 0; i < ret.length; i++) {
      ret[i] = new LocalVariableTableElement(getHandle(), offset);
      offset += localVariableTableElementSize;
    }
    return ret;
  }

  public Symbol getLocalVariableName(int bci, int slot) {
    if (! hasLocalVariableTable()) {
       return null;
    }

    LocalVariableTableElement[] locals = getLocalVariableTable();
    for (int l = 0; l < locals.length; l++) {
       LocalVariableTableElement local = locals[l];
       if ((bci >= local.getStartBCI()) && 
          (bci < (local.getStartBCI() + local.getLength())) &&
          slot == local.getSlot()) {
          return getConstants().getSymbolAt(local.getNameCPIndex());
       }
    }

    return null;
  }

  public boolean hasCheckedExceptions() {
    return getAccessFlagsObj().hasCheckedExceptions();
  }

  /** Should only be called if table is present */
  public CheckedExceptionElement[] getCheckedExceptions() {
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(hasCheckedExceptions(), "should only be called if table is present");
    }
    CheckedExceptionElement[] ret = new CheckedExceptionElement[getCheckedExceptionsLength()];
    long offset = offsetOfCheckedExceptions();
    for (int i = 0; i < ret.length; i++) {
      ret[i] = new CheckedExceptionElement(getHandle(), offset);
      offset += checkedExceptionElementSize;
    }
    return ret;
  }

  /** Returns name and signature in external form for debugging
      purposes */
  public String externalNameAndSignature() {
    final StringBuffer buf = new StringBuffer();
    buf.append(getMethodHolder().getName().asString());
    buf.append(".");
    buf.append(getName().asString());
    buf.append("(");
    new SignatureConverter(getSignature(), buf).iterateParameters();
    buf.append(")");
    return buf.toString().replace('/', '.');
  }

  //---------------------------------------------------------------------------
  // Internals only below this point
  //

  // Offset of end of code
  private long offsetOfCodeEnd() {
    return bytecodeOffset + getCodeSize();
  }

  // Offset of start of compressed line number table (see methodOop.hpp)
  private long offsetOfCompressedLineNumberTable() {
    return offsetOfCodeEnd() + (isNative() ? 2 * VM.getVM().getAddressSize() : 0);
  }

  // Offset of last short in methodOop
  private long offsetOfLastU2Element() {
    return getObjectSize() - 2;
  }

  private long offsetOfCheckedExceptionsLength() {
    return offsetOfLastU2Element();
  }

  private int getCheckedExceptionsLength() {
    if (hasCheckedExceptions()) {
      return (int) getHandle().getCIntegerAt(offsetOfCheckedExceptionsLength(), 2, true);
    } else {
      return 0;
    }
  }

  // Offset of start of checked exceptions
  private long offsetOfCheckedExceptions() {
    long offset = offsetOfCheckedExceptionsLength();
    long length = getCheckedExceptionsLength();
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(length > 0, "should only be called if table is present");
    }
    offset -= length * checkedExceptionElementSize;
    return offset;
  }

  private int getLineNumberTableLength() {
    int len = 0;
    if (hasLineNumberTable()) {
      CompressedLineNumberReadStream stream =
        new CompressedLineNumberReadStream(getHandle(), (int) offsetOfCompressedLineNumberTable());
      while (stream.readPair()) {
        len += 1;
      }
    }
    return len;
  }

  private int getLocalVariableTableLength() {
    if (hasLocalVariableTable()) {
      return (int) getHandle().getCIntegerAt(offsetOfLocalVariableTableLength(), 2, true);
    } else {
      return 0;
    }
  }

  // Offset of local variable table length
  private long offsetOfLocalVariableTableLength() {
    AccessFlags flags = getAccessFlagsObj();
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(flags.hasLocalVariableTable(), "should only be called if table is present");
    }
    if (flags.hasCheckedExceptions()) {
      return offsetOfCheckedExceptions() - 2;
    } else {
      return offsetOfLastU2Element();
    }
  }

  private long offsetOfLocalVariableTable() {
    long offset = offsetOfLocalVariableTableLength();
    long length = getLocalVariableTableLength();
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(length > 0, "should only be called if table is present");
    }
    offset -= length * localVariableTableElementSize;
    return offset;
  }
}
