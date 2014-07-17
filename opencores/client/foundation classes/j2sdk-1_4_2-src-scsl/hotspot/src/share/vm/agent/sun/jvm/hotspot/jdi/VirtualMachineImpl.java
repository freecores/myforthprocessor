/*
 * @(#)VirtualMachineImpl.java	1.3 02/03/04
 *
 * Copyright 2003 Sun Microsystems, Inc. All Rights Reserved.
 *
 * This software is the proprietary information of Sun Microsystems, Inc.
 * Use is subject to license terms.
 *
 */

package sun.jvm.hotspot.jdi;
import com.sun.jdi.*;
import com.sun.jdi.event.EventQueue;
import com.sun.jdi.request.EventRequestManager;
import sun.jvm.hotspot.HotSpotAgent;
import sun.jvm.hotspot.types.TypeDataBase;
import sun.jvm.hotspot.oops.Klass;
import sun.jvm.hotspot.oops.InstanceKlass;
import sun.jvm.hotspot.oops.ObjArrayKlass;
import sun.jvm.hotspot.oops.Instance;
import sun.jvm.hotspot.runtime.VM;
import sun.jvm.hotspot.runtime.JavaThread;
import sun.jvm.hotspot.memory.SystemDictionary;

import java.util.List;
import java.util.ArrayList;
import java.util.Map;
import java.util.Iterator;
import java.util.Collections;
import java.util.TreeSet;
import java.util.HashMap;
import java.lang.ref.SoftReference;
import java.lang.ref.ReferenceQueue;
import java.lang.ref.Reference;
public class VirtualMachineImpl extends MirrorImpl implements VirtualMachine {
    
    static HotSpotAgent saAgent = new HotSpotAgent();
    static TypeDataBase saTypeDataBase;
    static VM           saVM;
    static SystemDictionary saSystemDictionary;

    VirtualMachineManager vmmgr;

    private final ThreadGroup threadGroupForJDI;
    private final EventQueueImpl eventQueue;
    private final EventRequestManagerImpl eventRequestManager;

    // Per-vm singletons for primitive types and for void.
    // singleton-ness protected by "synchronized(this)".
    private BooleanType theBooleanType;
    private ByteType    theByteType;
    private CharType    theCharType;
    private ShortType   theShortType;
    private IntegerType theIntegerType;
    private LongType    theLongType;
    private FloatType   theFloatType;
    private DoubleType  theDoubleType;

    private VoidType    theVoidType;

    private Map typesByID;
    private TreeSet typesBySignature;
    private boolean retrievedAllTypes = false;
    private ArrayList allThreads;
    private ArrayList topLevelGroups;
    final int sequenceNumber;

    // ObjectReference cache
    // "objectsByID" protected by "synchronized(this)".
    private final Map objectsByID = new HashMap();
    private final ReferenceQueue referenceQueue = new ReferenceQueue();

    int traceFlags = TRACE_NONE;

    static public VirtualMachineImpl createVirtualMachineForCorefile(VirtualMachineManager mgr,
                                                                     String javaExecutableName,
                                                                     String coreFileName,
                                                                     int sequenceNumber)
        throws Exception {
        if (coreFileName == null) {
            throw new Exception("SA VirtualMachineImpl: core filename = null is not yet implemented");
        }
        if (javaExecutableName == null) {
            throw new Exception("SA VirtualMachineImpl: java executable = null is not yet implemented");
        }

        VirtualMachineImpl myvm = new VirtualMachineImpl(mgr, sequenceNumber);
        try {
            saAgent.attach(javaExecutableName, coreFileName);
            saTypeDataBase = saAgent.getTypeDataBase();
            saVM = VM.getVM();
            saSystemDictionary = saVM.getSystemDictionary();
        } catch (Exception ee) {
            saAgent.detach();
            throw ee;
        }
        return myvm;
    }

    static public VirtualMachineImpl createVirtualMachineForPID(VirtualMachineManager mgr,
                                                                int pid,
                                                                int sequenceNumber)
        throws Exception {

        VirtualMachineImpl myvm = new VirtualMachineImpl(mgr, sequenceNumber);
        try {
            saAgent.attach(pid);
            saTypeDataBase = saAgent.getTypeDataBase();
            saVM = VM.getVM();
            saSystemDictionary = saVM.getSystemDictionary();
        } catch (Exception ee) {
            saAgent.detach();
            throw ee;
        }
        return myvm;
    }

    static public VirtualMachineImpl createVirtualMachineForServer(VirtualMachineManager mgr,
                                                                String server,
                                                                int sequenceNumber)
        throws Exception {
        if (server == null) {
            throw new Exception("SA VirtualMachineImpl: DebugServer = null is not yet implemented");
        }

        VirtualMachineImpl myvm = new VirtualMachineImpl(mgr, sequenceNumber);
        try {
            saAgent.attach(server);
            saTypeDataBase = saAgent.getTypeDataBase();
            saVM = VM.getVM();
            saSystemDictionary = saVM.getSystemDictionary();
        } catch (Exception ee) {
            saAgent.detach();
            throw ee;
        }
        return myvm;
    }
   

    VirtualMachineImpl(VirtualMachineManager mgr, int sequenceNumber) 
        throws Exception {
        super(null);  // Can't use super(this)
        vm = this;

        this.sequenceNumber = sequenceNumber;
        this.vmmgr = mgr;

	/* Create ThreadGroup to be used by all threads servicing
	 * this VM.
	 */
	threadGroupForJDI = new ThreadGroup("JDI [" + 
					    this.hashCode() + "]");

        // Create dummy versions of these for the client to talk to.
        eventQueue = new EventQueueImpl(this);
        eventRequestManager = new EventRequestManagerImpl(this);
        
        ((com.sun.tools.jdi.VirtualMachineManagerImpl)mgr).addVirtualMachine(this);
    }

    public boolean equals(Object obj) {
        // Oh boy; big recursion troubles if we don't have this!
        // See MirrorImpl.equals
        return this == obj;
    }

    public List classesByName(String className) {
        String signature = JNITypeParser.typeNameToSignature(className);
        List list;
        if (!retrievedAllTypes) {
            retrieveAllClasses();
        }
        list = findReferenceTypes(signature);
        return Collections.unmodifiableList(list);
    }

    public List allClasses() {
        if (!retrievedAllTypes) {
            retrieveAllClasses();
        }
        ArrayList a;
        synchronized (this) {
            a = new ArrayList(typesBySignature);
        }
        return Collections.unmodifiableList(a);
    }

    private synchronized List findReferenceTypes(String signature) {
        if (typesByID == null) {
            return new ArrayList(0);
        }
        Iterator iter = typesBySignature.iterator();
        List list = new ArrayList();
        while (iter.hasNext()) {
            ReferenceTypeImpl type = (ReferenceTypeImpl)iter.next();
            int comp = signature.compareTo(type.signature());
            if (comp == 0) {
                list.add(type);
/* fix for 4359077 , don't break out. list is no longer sorted
	in the order we think
 */
	    }
        }
        return list;
    }

    private void retrieveAllClasses() {
        
        List saKlasses = saSystemDictionary.allClasses();

        // Hold lock during processing to improve performance
        // and to have safe check/set of retrievedAllTypes
        synchronized (this) {
            if (!retrievedAllTypes) {             
                // Number of classes
                int count = saKlasses.size();
                for (int ii = 0; ii < count; ii++) {
                    Klass kk = (Klass)saKlasses.get(ii);
                    ReferenceTypeImpl type = referenceType(kk);
                }
                retrievedAllTypes = true;
            }
        }
    }

    ReferenceTypeImpl referenceType(Klass kk) {
        ReferenceTypeImpl retType = null;
        synchronized (this) {
            if (typesByID != null) {
                retType = (ReferenceTypeImpl)typesByID.get(new Long(kk.hashCode()));
            }
            if (retType == null) {
                retType = addReferenceType(kk);
            }
        }
        return retType;
    }

    private void initReferenceTypes() {
        typesByID = new HashMap(300);
        typesBySignature = new TreeSet();
    }


    private synchronized ReferenceTypeImpl addReferenceType(Klass kk) {
        if (typesByID == null) {
            initReferenceTypes();
        }
        ReferenceTypeImpl newRefType;
        if (kk.isInterface()) {
            newRefType = new InterfaceTypeImpl(this, (InstanceKlass)kk);
        } else if (kk.isObjArray()) {
            newRefType = new ArrayTypeImpl(this, (ObjArrayKlass)kk); //fixme jjh:  Is this right? See comment in ArrayTypeImpl ctor
        } else {
            // it must be a class
            newRefType = new ClassTypeImpl(this, (InstanceKlass)kk);
        }
        
        /* 
         * If a signature was specified, make sure to set it ASAP, to 
         * prevent any needless JDWP command to retrieve it. (for example,
         * typesBySignature.add needs the signature, to maintain proper
         * ordering.
         */
        String signature = kk.signature();

        if (signature != null) {
            newRefType.setSignature(signature);
        }

        typesByID.put(new Long(kk.hashCode()), newRefType);
        typesBySignature.add(newRefType);

        return newRefType;
    }

    ThreadGroup threadGroupForJDI() {
	return threadGroupForJDI;
    }

    public void redefineClasses(Map classToBytes) {
        throw new UnsupportedOperationException("VirtualMachineImpl.redefineClasses()");
    }

    private List getAllThreads() {
        if (allThreads == null) {
            allThreads = new ArrayList(10);  // Might be enough, might not be
            for (sun.jvm.hotspot.runtime.JavaThread thread = 
                     VM.getVM().getThreads().first(); thread != null; 
                     thread = thread.next()) {
                ThreadReferenceImpl myThread = objectMirror(thread, JDWP.Tag.THREAD);
                
                allThreads.add(myThread);
            }
        }
        return allThreads;
    }

    public List allThreads() { //fixme jjh
        return Collections.unmodifiableList(getAllThreads());
    }

    public void suspend() {
        throw new UnsupportedOperationException("VirtualMachineImpl.suspend()");
    }

    public void resume() {
        throw new UnsupportedOperationException("VirtualMachineImpl.resume()");
    }

    public List topLevelThreadGroups() { //fixme jjh
        // The doc for ThreadGroup says that The top-level thread group
        // is the only thread group whose parent is null.  This means there is
        // only one top level thread group.  There will be a thread in this
        // group so we will just find a thread whose threadgroup has no parent
        // and that will be it.

        if (topLevelGroups == null) {
            topLevelGroups = new ArrayList(1);  
            Iterator myIt = getAllThreads().iterator();
            while (myIt.hasNext()) {
                ThreadReferenceImpl myThread = (ThreadReferenceImpl)myIt.next();
                ThreadGroupReference myGroup = myThread.threadGroup();
                ThreadGroupReference myParent = myGroup.parent();
                if (myGroup.parent() == null) {
                    topLevelGroups.add(myGroup);
                    break;
                }
            }
        }
        return  Collections.unmodifiableList(topLevelGroups);
    }

    public EventQueue eventQueue() {
        return eventQueue;
    }

    public EventRequestManager eventRequestManager() {
        return eventRequestManager;
    }

    public BooleanValue mirrorOf(boolean value) {
        return new BooleanValueImpl(this,value);
    }

    public ByteValue mirrorOf(byte value) {
        return new ByteValueImpl(this,value);
    }

    public CharValue mirrorOf(char value) {
        return new CharValueImpl(this,value);
    }

    public ShortValue mirrorOf(short value) {
        return new ShortValueImpl(this,value);
    }

    public IntegerValue mirrorOf(int value) {
        return new IntegerValueImpl(this,value);
    }

    public LongValue mirrorOf(long value) {
        return new LongValueImpl(this,value);
    }

    public FloatValue mirrorOf(float value) {
        return new FloatValueImpl(this,value);
    }

    public DoubleValue mirrorOf(double value) {
        return new DoubleValueImpl(this,value);
    }

    public StringReference mirrorOf(String value) { //fixme jjh
        return null;
    }

    public Process process() { //fixme jjh
        return null;
    }

    public void dispose() {
        saAgent.detach();
    }

    public void exit(int exitCode) {
        saAgent.detach();
    }

    public boolean canWatchFieldModification() {
        return false;
    }

    public boolean canWatchFieldAccess() {
        return false;
    }

    public boolean canGetBytecodes() {
        return true;
    }

    public boolean canGetSyntheticAttribute() {
        return true;
    }

    public boolean canGetOwnedMonitorInfo() {
        return true;
    }

    public boolean canGetCurrentContendedMonitor() {
        return false;
    }

    public boolean canGetMonitorInfo() {
        return true;
    }

    public boolean canUseInstanceFilters() {
        return false;
    }

    public boolean canRedefineClasses() {
        return false;
    }

    public boolean canAddMethod() {
        return false;
    }

    public boolean canUnrestrictedlyRedefineClasses() {
        return false;
    }

    public boolean canPopFrames() {
        return false;
    }

    public boolean canGetSourceDebugExtension() {
        return false;
    }

    public boolean canRequestVMDeathEvent() {
        return false;
    }

    public void setDefaultStratum(String stratum) { //fixme jjh
    }

    public String getDefaultStratum() { //fixme jjh
        return null;
    }

    public String description() { //fixme jjh
        return null;
    }

    public String version() { //fixme jjh
        return null;
    }

    public String name() { //fixme jjh
        return null;
    }

    void printTrace(String string) {
        System.err.println("[JDI: " + string + "]");
    }

    // from interface Mirror
    public VirtualMachine virtualMachine() {
        return this;
    }

    //    public String toString() { //fixme jjh
    //        return null;
    //}

    public void setDebugTraceMode(int traceFlags) { //fixme jjh
    }

    Type findBootType(String signature) throws ClassNotLoadedException {
        List types = allClasses();
        Iterator iter = types.iterator();
        while (iter.hasNext()) {
            ReferenceType type = (ReferenceType)iter.next();
            if ((type.classLoader() == null) && 
                (type.signature().equals(signature))) {
                return type;
            } 
        }
        JNITypeParser parser = new JNITypeParser(signature);
        throw new ClassNotLoadedException(parser.typeName(),
                                         "Type " + parser.typeName() + " not loaded");
    }

    BooleanType theBooleanType() {
        if (theBooleanType == null) {
            synchronized(this) {
                if (theBooleanType == null) {
                    theBooleanType = new BooleanTypeImpl(this);
                }
            }
        }
        return theBooleanType;
    }

    ByteType theByteType() {
        if (theByteType == null) {
            synchronized(this) {
                if (theByteType == null) {
                    theByteType = new ByteTypeImpl(this);
                }
            }
        }
        return theByteType;
    }

    CharType theCharType() {
        if (theCharType == null) {
            synchronized(this) {
                if (theCharType == null) {
                    theCharType = new CharTypeImpl(this);
                }
            }
        }
        return theCharType;
    }

    ShortType theShortType() {
        if (theShortType == null) {
            synchronized(this) {
                if (theShortType == null) {
                    theShortType = new ShortTypeImpl(this);
                }
            }
        }
        return theShortType;
    }

    IntegerType theIntegerType() {
        if (theIntegerType == null) {
            synchronized(this) {
                if (theIntegerType == null) {
                    theIntegerType = new IntegerTypeImpl(this);
                }
            }
        }
        return theIntegerType;
    }

    LongType theLongType() {
        if (theLongType == null) {
            synchronized(this) {
                if (theLongType == null) {
                    theLongType = new LongTypeImpl(this);
                }
            }
        }
        return theLongType;
    }

    FloatType theFloatType() {
        if (theFloatType == null) {
            synchronized(this) {
                if (theFloatType == null) {
                    theFloatType = new FloatTypeImpl(this);
                }
            }
        }
        return theFloatType;
    }

    DoubleType theDoubleType() {
        if (theDoubleType == null) {
            synchronized(this) {
                if (theDoubleType == null) {
                    theDoubleType = new DoubleTypeImpl(this);
                }
            }
        }
        return theDoubleType;
    }

    VoidType theVoidType() {
        if (theVoidType == null) {
            synchronized(this) {
                if (theVoidType == null) {
                    theVoidType = new VoidTypeImpl(this);
                }
            }
        }
        return theVoidType;
    }

    PrimitiveType primitiveTypeMirror(byte tag) {
        switch (tag) {
            case JDWP.Tag.BOOLEAN:
                return theBooleanType();
            case JDWP.Tag.BYTE:
                return theByteType();
            case JDWP.Tag.CHAR:
                return theCharType();
            case JDWP.Tag.SHORT:
                return theShortType();
            case JDWP.Tag.INT:
                return theIntegerType();
            case JDWP.Tag.LONG:
                return theLongType();
            case JDWP.Tag.FLOAT:
                return theFloatType();
            case JDWP.Tag.DOUBLE:
                return theDoubleType();
            default:
                throw new IllegalArgumentException("Unrecognized primitive tag " + tag);
        }
    }

    private void processQueue() {
        Reference ref;
        //if ((traceFlags & TRACE_OBJREFS) != 0) {
        //    printTrace("Checking for softly reachable objects");
        //}
        while ((ref = referenceQueue.poll()) != null) {
            SoftObjectReference softRef = (SoftObjectReference)ref;
            removeObjectMirror(softRef);
        }
    }

    synchronized ThreadReferenceImpl objectMirror(JavaThread key, int tag) {

        // Handle any queue elements that are not strongly reachable 
        processQueue();

        if (key == null) {
            return null;
        }
        ObjectReferenceImpl object = null;

        /*
         * Attempt to retrieve an existing object object reference 
         */
        SoftObjectReference ref = (SoftObjectReference)objectsByID.get(key);
        if (ref != null) {
            object = ref.object();
        }

        /*
         * If the object wasn't in the table, or it's soft reference was
         * cleared, create a new instance.
         */
        if (object == null) {
            if (tag != JDWP.Tag.THREAD) {
                System.out.println("Error: VMI.java: It should be a Thread!"); // fixme jjh
                return null;
            }
            ThreadReferenceImpl thread = new ThreadReferenceImpl(vm, key);
            object = thread;
            ref = new SoftObjectReference(key, object, referenceQueue);

            /*
             * If there was no previous entry in the table, we add one here
             * If the previous entry was cleared, we replace it here.
             */
            objectsByID.put(key, ref);
            if ((traceFlags & TRACE_OBJREFS) != 0) {
                printTrace("Creating new " +
                           object.getClass().getName() + " (id = " + key+ ")");
            }
        } else {
            ref.incrementCount();
        }

        return (ThreadReferenceImpl)object;
    }


    synchronized ObjectReferenceImpl objectMirror(Instance key, int tag) {

        // Handle any queue elements that are not strongly reachable 
        processQueue();

        if (key == null) {
            return null;
        }
        ObjectReferenceImpl object = null;

        /*
         * Attempt to retrieve an existing object object reference 
         */
        SoftObjectReference ref = (SoftObjectReference)objectsByID.get(key);
        if (ref != null) {
            object = ref.object();
        }

        /*
         * If the object wasn't in the table, or it's soft reference was
         * cleared, create a new instance.
         */
        if (object == null) {
            switch (tag) {
                case JDWP.Tag.OBJECT:
                    object = new ObjectReferenceImpl(vm, key);
                    break;
                case JDWP.Tag.STRING:
                    object = new StringReferenceImpl(vm, key);
                    break;
                case JDWP.Tag.ARRAY:
                    object = new ArrayReferenceImpl(vm, key);
                    break;
                case JDWP.Tag.THREAD:
                    System.out.println("Error: VMI: it should not be a JDWP.Tag.THREAD"); //fixme jjh
                    break;
                case JDWP.Tag.THREAD_GROUP:
                    object = new ThreadGroupReferenceImpl(vm, key);
                    break;
                case JDWP.Tag.CLASS_LOADER:
                    object = new ClassLoaderReferenceImpl(vm, key);
                    break;
                case JDWP.Tag.CLASS_OBJECT:
                    object = new ClassObjectReferenceImpl(vm, key);
                    break;
                default:
                    throw new IllegalArgumentException("Invalid object tag: " + tag);
            }
            ref = new SoftObjectReference(key, object, referenceQueue);

            /*
             * If there was no previous entry in the table, we add one here
             * If the previous entry was cleared, we replace it here.
             */
            objectsByID.put(key, ref);
            if ((traceFlags & TRACE_OBJREFS) != 0) {
                printTrace("Creating new " +
                           object.getClass().getName() + " (id = " + key+ ")");
            }
        } else {
            ref.incrementCount();
        }

        return object;
    }

    synchronized void removeObjectMirror(SoftObjectReference ref) {
        /*
         * This will remove the soft reference if it has not been 
         * replaced in the cache.
         */
        objectsByID.remove(ref.key());
    }

    ObjectReferenceImpl objectMirror(Instance id) {
        return objectMirror(id, JDWP.Tag.OBJECT);
    }

    StringReferenceImpl stringMirror(Instance id) {
        return (StringReferenceImpl)objectMirror(id, JDWP.Tag.STRING);
    }

    ArrayReferenceImpl arrayMirror(Instance id) {
       return (ArrayReferenceImpl)objectMirror(id, JDWP.Tag.ARRAY);
    }

    ThreadReferenceImpl threadMirror(JavaThread id) {
        return (ThreadReferenceImpl)objectMirror(id, JDWP.Tag.THREAD);
    }

    ThreadGroupReferenceImpl threadGroupMirror(Instance id) {
        return (ThreadGroupReferenceImpl)objectMirror(id, 
                                                      JDWP.Tag.THREAD_GROUP);
    }

    ClassLoaderReferenceImpl classLoaderMirror(Instance id) {
        return (ClassLoaderReferenceImpl)objectMirror(id, 
                                                      JDWP.Tag.CLASS_LOADER);
    }

    ClassObjectReferenceImpl classObjectMirror(Instance id) {
        return (ClassObjectReferenceImpl)objectMirror(id, 
                                                      JDWP.Tag.CLASS_OBJECT);
    }

    // Use of soft refs and caching stuff here has to be re-examined.
    //  It might not make sense for JDI - SA.
    static private class SoftObjectReference extends SoftReference {
       int count;
       Object key;

       SoftObjectReference(Object key, ObjectReferenceImpl mirror, 
                           ReferenceQueue queue) {
           super(mirror, queue);
           this.count = 1;
           this.key = key;
       }

       int count() {
           return count;
       }

       void incrementCount() {
           count++;
       }

       Object key() {
           return key;
       }

       ObjectReferenceImpl object() {
           return (ObjectReferenceImpl)get();
       }
   }
}




            
