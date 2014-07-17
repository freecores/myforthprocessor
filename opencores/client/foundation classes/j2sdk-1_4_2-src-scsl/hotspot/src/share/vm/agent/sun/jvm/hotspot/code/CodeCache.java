/*
 * @(#)CodeCache.java	1.8 03/01/23 11:23:33
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.code;

import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.memory.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;
import sun.jvm.hotspot.utilities.*;

public class CodeCache {
  private static AddressField       heapField;
  private static VirtualConstructor virtualConstructor;

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) {
    Type type = db.lookupType("CodeCache");
    
    heapField = type.getAddressField("_heap");

    virtualConstructor = new VirtualConstructor(db);
    // Add mappings for all possible CodeBlob subclasses
    virtualConstructor.addMapping("BufferBlob", BufferBlob.class);
    virtualConstructor.addMapping("nmethod", NMethod.class);
    virtualConstructor.addMapping("RuntimeStub", RuntimeStub.class);
    virtualConstructor.addMapping("SafepointBlob", SafepointBlob.class);
    if (VM.getVM().isServerCompiler()) {
      virtualConstructor.addMapping("C2IAdapter", C2IAdapter.class);
      virtualConstructor.addMapping("DeoptimizationBlob", DeoptimizationBlob.class);
      virtualConstructor.addMapping("ExceptionBlob", ExceptionBlob.class);
      virtualConstructor.addMapping("I2CAdapter", I2CAdapter.class);
      virtualConstructor.addMapping("OSRAdapter", OSRAdapter.class);
      virtualConstructor.addMapping("UncommonTrapBlob", UncommonTrapBlob.class);
    }
  }
  
  public CodeCache() {
  }

  public boolean contains(Address p) {
    return getHeap().contains(p);
  }

  /** When VM.getVM().isDebugging() returns true, this behaves like
      findBlobUnsafe */
  public CodeBlob findBlob(Address start) {
    CodeBlob result = findBlobUnsafe(start);
    if (result == null) return null;
    if (VM.getVM().isDebugging()) {
      return result;
    }
    // We could potientially look up non_entrant methods
    // NOTE: this is effectively a "guarantee", and is slightly different from the one in the VM
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(!(result.isZombie() || result.isLockedByVM()), "unsafe access to zombie method");    
    }
    return result;  
  }

  public CodeBlob findBlobUnsafe(Address start) {
    CodeBlob result = null;

    try {
      result = (CodeBlob) virtualConstructor.instantiateWrapperFor(getHeap().findStart(start));
    }
    catch (WrongTypeException e) {
      Address cbAddr = null;
      try {
        cbAddr = getHeap().findStart(start);
      }
      catch (Exception findEx) {
        findEx.printStackTrace();
      }

      String message = "Couldn't deduce type of CodeBlob ";
      if (cbAddr != null) {
        message = message + "@" + cbAddr + " ";
      }
      message = message + "for PC=" + start;

      throw new RuntimeException(message);
    }
    if (result == null) return null;
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(result.blobContains(start), "found wrong CodeBlob");
    }
    return result;  
  }

  public NMethod findNMethod(Address start) {
    CodeBlob cb = findBlob(start);
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(cb == null || cb.isNMethod(), "did not find an nmethod");
    }
    return (NMethod) cb;
  }

  public NMethod findNMethodUnsafe(Address start) {
    CodeBlob cb = findBlobUnsafe(start);
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(cb == null || cb.isNMethod(), "did not find an nmethod");
    }
    return (NMethod) cb;
  }

  /** Routine for instantiating appropriately-typed wrapper for a
      CodeBlob. Used by CodeCache, Runtime1, etc. */
  public CodeBlob createCodeBlobWrapper(Address codeBlobAddr) {
    try {
      return (CodeBlob) virtualConstructor.instantiateWrapperFor(codeBlobAddr);
    }
    catch (Exception e) {
      String message = "Unable to deduce type of CodeBlob from address " + codeBlobAddr +
                       " (expected type nmethod, RuntimeStub, ";
      if (VM.getVM().isClientCompiler()) {
        message = message + " or ";
      }
      message = message + "SafepointBlob";
      if (VM.getVM().isServerCompiler()) {
        message = message + ", C2IAdapter, I2CAdapter, DeoptimizationBlob, or ExceptionBlob";
      }
      message = message + ")";
      throw new RuntimeException(message);
    }
  }

  //--------------------------------------------------------------------------------
  // Internals only below this point
  //

  private CodeHeap getHeap() {
    return (CodeHeap) VMObjectFactory.newObject(CodeHeap.class, heapField.getValue());
  }
}
