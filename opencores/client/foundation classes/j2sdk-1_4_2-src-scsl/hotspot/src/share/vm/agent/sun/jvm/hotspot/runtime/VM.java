/*
 * @(#)VM.java	1.23 03/01/23 11:46:07
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.runtime;

import java.util.*;
import sun.jvm.hotspot.code.*;
import sun.jvm.hotspot.c1.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.interpreter.*;
import sun.jvm.hotspot.memory.*;
import sun.jvm.hotspot.oops.*;
import sun.jvm.hotspot.types.*;
import sun.jvm.hotspot.utilities.*;

/** <P> This class encapsulates the global state of the VM; the
    universe, object heap, interpreter, etc. It is a Singleton and
    must be initialized with a call to initialize() before calling
    getVM(). </P>

    <P> Many auxiliary classes (i.e., most of the VMObjects) keep
    needed field offsets in the form of static Field objects. In a
    debugging system, the VM might be shutdown and re-initialized (on
    a differently-configured build, i.e., 32- vs. 64-bit), and all old
    cached state (including fields and field offsets) must be
    flushed. </P>

    <P> An Observer pattern is used to implement the initialization of
    such classes. Each such class, in its static initializer,
    registers an Observer with the VM class via
    VM.registerVMInitializedObserver(). This Observer is guaranteed to
    be notified whenever the VM is initialized (or re-initialized). To
    implement the first-time initialization, the observer is also
    notified when it registers itself with the VM. (For bootstrapping
    reasons, this implies that the constructor of VM can not
    instantiate any such objects, since VM.soleInstance will not have
    been set yet. This is a bootstrapping issue which may have to be
    revisited later.) </P>
*/

public class VM {
  private static VM    soleInstance;
  private static List  vmInitializedObservers = new ArrayList();
  private List         vmResumedObservers   = new ArrayList();
  private List         vmSuspendedObservers = new ArrayList();
  private TypeDataBase db;
  private boolean      isBigEndian;
  /** This is only present if in a debugging system */
  private JVMDebugger  debugger;
  private long         stackBias;
  private long         logAddressSize;
  private Universe     universe;
  private ObjectHeap   heap;
  private SymbolTable  symbols;
  private SystemDictionary dict;
  private Threads      threads;
  private ObjectSynchronizer synchronizer;
  private JNIHandles   handles;
  private Interpreter  interpreter;
  private StubRoutines stubRoutines;
  private SharedInfo   sharedInfo;
  private Bytes        bytes;
  /** Flags indicating whether we are attached to a core, C1, or C2 build */
  private boolean      usingClientCompiler;
  private boolean      usingServerCompiler;
  /** Flag indicating whether UseTLAB is turned on */
  private boolean      useTLAB;
  /** This is only present in a non-core build */
  private CodeCache    codeCache;
  /** This is only present in a C1 build */
  private Runtime1     runtime1;
  /** These constants come from globalDefinitions.hpp */
  private int          invocationEntryBCI;
  private int          invalidOSREntryBCI;
  private ReversePtrs  revPtrs;

  private VM(TypeDataBase db, JVMDebugger debugger, boolean isBigEndian) {
    this.db          = db;
    this.debugger    = debugger;
    this.isBigEndian = isBigEndian;

    // Note that we don't construct universe, heap, threads,
    // interpreter, or stubRoutines here (any more).  The current
    // initialization mechanisms require that the VM be completely set
    // up (i.e., out of its constructor, with soleInstance assigned)
    // before their static initializers are run.

    stackBias    = db.lookupIntConstant("STACK_BIAS").intValue();
    if (db.getAddressSize() == 4) {
      logAddressSize = 2;
    } else if (db.getAddressSize() == 8) {
      logAddressSize = 3;
    } else {
      throw new RuntimeException("Address size " + db.getAddressSize() + " not yet supported");
    }
    invocationEntryBCI = db.lookupIntConstant("InvocationEntryBci").intValue();
    invalidOSREntryBCI = db.lookupIntConstant("InvalidOSREntryBci").intValue();

    // We infer the presence of C1 or C2 from a couple of fields we
    // already have present in the type database
    {
      Type type = db.lookupType("methodOopDesc");
      if (type.getField("_from_compiled_code_entry_point", false, false) == null) {
        // Neither C1 nor C2 is present
        usingClientCompiler = false;
        usingServerCompiler = false;
      } else {
        // Determine whether C2 is present
        if (type.getField("_interpreter_invocation_count", false, false) != null) {
          usingServerCompiler = true;
        } else {
          usingClientCompiler = true;
        }
      }
    }

    useTLAB = (db.lookupIntConstant("UseTLAB").intValue() != 0);
  }

  /** This could be used by a reflective runtime system */
  public static void initialize(TypeDataBase db, boolean isBigEndian) {
    if (soleInstance != null) {
      throw new RuntimeException("Attempt to initialize VM twice");
    }
    soleInstance = new VM(db, null, isBigEndian);
    for (Iterator iter = vmInitializedObservers.iterator(); iter.hasNext(); ) {
      ((Observer) iter.next()).update(null, null);
    }
  }

  /** This is used by the debugging system */
  public static void initialize(TypeDataBase db, JVMDebugger debugger) {
    if (soleInstance != null) {
      throw new RuntimeException("Attempt to initialize VM twice");
    }
    soleInstance = new VM(db, debugger, debugger.getMachineDescription().isBigEndian());
    for (Iterator iter = vmInitializedObservers.iterator(); iter.hasNext(); ) {
      ((Observer) iter.next()).update(null, null);
    }
  }

  /** This is used by the debugging system */
  public static void shutdown() {
    soleInstance = null;
  }

  /** This is used by both the debugger and any runtime system. It is
      the basic mechanism by which classes which mimic underlying VM
      functionality cause themselves to be initialized. The given
      observer will be notified (with arguments (null, null)) when the
      VM is re-initialized, as well as when it registers itself with
      the VM. */
  public static void registerVMInitializedObserver(Observer o) {
    vmInitializedObservers.add(o);
    o.update(null, null);
  }

  /** This is the primary accessor used by both the debugger and any
      potential runtime system */
  public static VM getVM() {
    if (soleInstance == null) {
      throw new RuntimeException("VM.initialize() was not yet called");
    }
    return soleInstance;
  }

  /** This is only used by the debugging system. The given observer
      will be notified if the underlying VM resumes execution. NOTE
      that the given observer is not triggered if the VM is currently
      running and therefore differs in behavior from {@link
      #registerVMInitializedObserver} (because of the possibility of
      race conditions if the observer is added while the VM is being
      suspended or resumed).  */
  public void registerVMResumedObserver(Observer o) {
    vmResumedObservers.add(o);
  }

  /** This is only used by the debugging system. The given observer
      will be notified if the underlying VM suspends execution. NOTE
      that the given observer is not triggered if the VM is currently
      suspended and therefore differs in behavior from {@link
      #registerVMInitializedObserver} (because of the possibility of
      race conditions if the observer is added while the VM is being
      suspended or resumed).  */
  public void registerVMSuspendedObserver(Observer o) {
    vmSuspendedObservers.add(o);
  }

  /** This is only used by the debugging system. Informs all
      registered resumption observers that the VM has been resumed.
      The application is responsible for actually having performed the
      resumption. No OopHandles must be used after this point, as they
      may move in the target address space due to garbage
      collection. */
  public void fireVMResumed() {
    for (Iterator iter = vmResumedObservers.iterator(); iter.hasNext(); ) {
      ((Observer) iter.next()).update(null, null);
    }
  }

  /** This is only used by the debugging system. Informs all
      registered suspension observers that the VM has been suspended.
      The application is responsible for actually having performed the
      suspension. Garbage collection must be forbidden at this point;
      for example, a JPDA-level suspension is not adequate since the
      VM thread may still be running. */
  public void fireVMSuspended() {
    for (Iterator iter = vmSuspendedObservers.iterator(); iter.hasNext(); ) {
      ((Observer) iter.next()).update(null, null);
    }
  }

  /** Returns the OS this VM is running on. Notice that by delegating
      to the debugger we can transparently support remote
      debugging. */
  public String getOS() {
    if (debugger != null) {
      return debugger.getOS();
    }
    return PlatformInfo.getOS();
  }

  /** Returns the CPU this VM is running on. Notice that by delegating
      to the debugger we can transparently support remote
      debugging. */
  public String getCPU() {
    if (debugger != null) {
      return debugger.getCPU();
    }
    return PlatformInfo.getCPU();
  }

  public Type lookupType(String cTypeName) {
    return db.lookupType(cTypeName);
  }

  public Integer lookupIntConstant(String name) {
    return db.lookupIntConstant(name);
  }

  public long getAddressSize() {
    return db.getAddressSize();
  }

  public long getOopSize() {
    return db.getOopSize();
  }

  public long getLogAddressSize() {
    return logAddressSize;
  }

  /** NOTE: this offset is in BYTES in this system! */
  public long getStackBias() {
    return stackBias;
  }

  /** Indicates whether the underlying machine supports the LP64 data
      model. This is needed for conditionalizing code in a few places */
  public boolean isLP64() {
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(isDebugging(), "Debugging system only for now");
    }
    return debugger.getMachineDescription().isLP64();
  }

  /** Indicate whether we should align all objects on doubleword
      boundaries. */
  public boolean getAlignAllObjects() {
    return (lookupIntConstant("ALIGN_ALL_OBJECTS").intValue() != 0);
  }

  /** Indicate whether we should align all double fields on doubleword
      boundaries. */
  public boolean getAlignDoubleFields() {
    return (lookupIntConstant("ALIGN_DOUBLE_FIELDS").intValue() != 0);
  }

  /** Indicate whether we should align elements of double arrays on
      doubleword boundaries. */
  public boolean getAlignDoubleElements() {
    return (lookupIntConstant("ALIGN_DOUBLE_ELEMENTS").intValue() != 0);
  }

  /** Indicate whether we should align all long fields on doubleword
      boundaries. */
  public boolean getAlignLongFields() {
    return (lookupIntConstant("ALIGN_LONG_FIELDS").intValue() != 0);
  }

  /** Indicate whether we should align elements of long arrays on
      doubleword boundaries. */
  public boolean getAlignLongElements() {
    return (lookupIntConstant("ALIGN_LONG_ELEMENTS").intValue() != 0);
  }

  /** Utility routine for getting data structure alignment correct */
  public long alignUp(long size, long alignment) {
    return (size + alignment - 1) & ~(alignment - 1);
  }

  /** Utility routine for getting data structure alignment correct */
  public long alignDown(long size, long alignment) {
    return size & ~(alignment - 1);
  }

  /** Utility routine for building an int from two "unsigned" 16-bit
      shorts */
  public int buildIntFromShorts(short low, short high) {
    return (((int) high) << 16) | (((int) low) & 0xFFFF);
  }

  /** Utility routine for building a long from two "unsigned" 32-bit
      ints in <b>platform-dependent</b> order */
  public long buildLongFromIntsPD(int oneHalf, int otherHalf) {
    if (isBigEndian) {
      return (((long) otherHalf) << 32) | (((long) oneHalf) & 0x00000000FFFFFFFFL);
    } else{
      return (((long) oneHalf) << 32) | (((long) otherHalf) & 0x00000000FFFFFFFFL);
    }
  }

  /** Indicates whether Thread-Local Allocation Buffers are used */
  public boolean getUseTLAB() {
    return useTLAB;
  }

  public TypeDataBase getTypeDataBase() {
    return db;
  }

  public Universe    getUniverse() {
    if (universe == null) {
      universe = new Universe();
    }
    return universe;
  }

  public ObjectHeap  getObjectHeap() {
    if (heap == null) {
      heap = new ObjectHeap(db);
    }
    return heap;
  }

  public SymbolTable getSymbolTable() {
    if (symbols == null) {
      symbols = new SymbolTable();
    }
    return symbols;
  }

  public SystemDictionary getSystemDictionary() {
    if (dict == null) {
      dict = new SystemDictionary();
    }
    return dict;
  }

  public Threads     getThreads() {
    if (threads == null) {
      threads = new Threads();
    }
    return threads;
  }

  public ObjectSynchronizer getObjectSynchronizer() {
    if (synchronizer == null) {
      synchronizer = new ObjectSynchronizer();
    }
    return synchronizer;
  }

  public JNIHandles getJNIHandles() {
    if (handles == null) {
      handles = new JNIHandles();
    }
    return handles;
  }

  public Interpreter getInterpreter() {
    if (interpreter == null) {
      interpreter = new Interpreter();
    }
    return interpreter;
  }

  public StubRoutines getStubRoutines() {
    if (stubRoutines == null) {
      stubRoutines = new StubRoutines();
    }
    return stubRoutines;
  }

  public SharedInfo getSharedInfo() {
    if (sharedInfo == null) {
      sharedInfo = new SharedInfo();
    }
    return sharedInfo;
  }

  public Bytes getBytes() {
    if (bytes == null) {
      bytes = new Bytes(debugger.getMachineDescription());
    }
    return bytes;
  }

  /** Returns true if this is a "core" build, false if either C1 or C2
      is present */
  public boolean isCore() {
    return (!(usingClientCompiler || usingServerCompiler));
  }

  /** Returns true if this is a C1 build, false otherwise */
  public boolean isClientCompiler() {
    return usingClientCompiler;
  }

  /** Returns true if this is a C2 build, false otherwise */
  public boolean isServerCompiler() {
    return usingServerCompiler;
  }

  /** Returns the code cache; should not be used if is core build */
  public CodeCache getCodeCache() {
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(!isCore(), "noncore builds only");
    }
    if (codeCache == null) {
      codeCache = new CodeCache();
    }
    return codeCache;
  }

  /** Should only be called for C1 builds */
  public Runtime1 getRuntime1() {
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(isClientCompiler(), "C1 builds only");
    }
    if (runtime1 == null) {
      runtime1 = new Runtime1();
    }
    return runtime1;
  }

  /** Test to see whether we're in debugging mode (NOTE: this really
      should not be tested by this code; currently only used in
      StackFrameStream) */
  public boolean isDebugging() {
    return (debugger != null);
  }

  /** This is only used by the debugging (i.e., non-runtime) system */
  public JVMDebugger getDebugger() {
    if (debugger == null) {
      throw new RuntimeException("Attempt to use debugger in runtime system");
    }
    return debugger;
  }

  /** Indicates whether a given program counter is in Java code. This
      includes but is not spanned by the interpreter and code cache.
      Only used in the debugging system, for implementing
      JavaThread.currentFrameGuess() on x86. */
  public boolean isJavaPCDbg(Address addr) {
    // FIXME: this is not a complete enough set: must include areas
    // like vtable stubs
    return (getInterpreter().contains(addr) ||
            getCodeCache().contains(addr));
  }

  /** FIXME: figure out where to stick this */
  public int getInvocationEntryBCI() {
    return invocationEntryBCI;
  }

  /** FIXME: figure out where to stick this */
  public int getInvalidOSREntryBCI() {
    return invalidOSREntryBCI;
  }

  // FIXME: figure out where to stick this
  public boolean wizardMode() {
    return true;
  }

  public ReversePtrs getRevPtrs() {
    return revPtrs;
  }

  public void setRevPtrs(ReversePtrs rp) {
    revPtrs = rp;
  }
}
