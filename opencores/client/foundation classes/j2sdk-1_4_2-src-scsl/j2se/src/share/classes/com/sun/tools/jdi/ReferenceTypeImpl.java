/*
 * @(#)ReferenceTypeImpl.java	1.63 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdi;

import com.sun.jdi.*;

import java.util.*;    
import java.lang.ref.SoftReference;

public abstract class ReferenceTypeImpl extends TypeImpl
implements ReferenceType {
    protected long ref;
    private String signature = null;
    private String baseSourceName = null;
    private String baseSourceDir = null;
    private String baseSourcePath = null;
    protected int modifiers = -1;
    private SoftReference fieldsRef = null;
    private SoftReference methodsRef = null;
    private SoftReference sdeRef = null;

    private boolean isClassLoaderCached = false;
    private ClassLoaderReference classLoader = null;
    private ClassObjectReference classObject = null;

    private int status = -1;
    private boolean isPrepared = false;

    /* to mark a SourceFile request that returned a genuine JDWP.Error.ABSENT_INFORMATION */
    private static final String ABSENT_BASE_SOURCE_NAME = "**ABSENT_BASE_SOURCE_NAME**";

    /* to mark when no info available */
    static final SDE NO_SDE_INFO_MARK = new SDE();

    // bits set when initialization was attempted (succeeded or failed)
    private static final int INITIALIZED_OR_FAILED = 
        JDWP.ClassStatus.INITIALIZED | JDWP.ClassStatus.ERROR;

    protected ReferenceTypeImpl(VirtualMachine aVm, long aRef) {
        super(aVm);
        ref = aRef;
    }

    void noticeRedefineClass() {
        //Invalidate information previously fetched and cached.
        //These will be refreshed later on demand.
        baseSourceName = null;
        baseSourcePath = null;
        modifiers = -1;
        fieldsRef = null;
        methodsRef = null;
        sdeRef = null;
    }

    Method getMethodMirror(long ref) {
        if (ref == 0) {
            // obsolete method
            return new ObsoleteMethodImpl(vm, this);
        }
        // Fetch all methods for the class, check performance impact
        // Needs no synchronization now, since methods() returns 
        // unmodifiable local data
        Iterator it = methods().iterator();
        while (it.hasNext()) {
            MethodImpl method = (MethodImpl)it.next();
            if (method.ref() == ref) {
                return method;
            }
        }
        throw new IllegalArgumentException("Invalid method id: " + ref);
    }         

    Field getFieldMirror(long ref) {
        // Fetch all fields for the class, check performance impact
        // Needs no synchronization now, since fields() returns 
        // unmodifiable local data
        Iterator it = fields().iterator();
        while (it.hasNext()) {
            FieldImpl field = (FieldImpl)it.next();
            if (field.ref() == ref) {
                return field;
            }
        }
        throw new IllegalArgumentException("Invalid field id: " + ref);
    }         

    public boolean equals(Object obj) {
        if ((obj != null) && (obj instanceof ReferenceTypeImpl)) {
            ReferenceTypeImpl other = (ReferenceTypeImpl)obj;
            return (ref() == other.ref()) && 
                (vm.equals(other.virtualMachine()));
        } else {
            return false;
        }
    }

    public int hashCode() {
        return(int)ref();
    }

    public int compareTo(Object object) {
        /*
         * Note that it is critical that compareTo() == 0 
         * implies that equals() == true. Otherwise, TreeSet
         * will collapse classes.
         * 
         * (Classes of the same name loaded by different class loaders
         * or in different VMs must not return 0).
         */
        ReferenceTypeImpl other = (ReferenceTypeImpl)object;
        int comp = name().compareTo(other.name());
        if (comp == 0) {
            long rf1 = ref();
            long rf2 = other.ref();
            // optimize for typical case: refs equal and VMs equal
            if (rf1 == rf2) {
                // sequenceNumbers are always positive
                comp = vm.sequenceNumber - 
                 ((VirtualMachineImpl)(other.virtualMachine())).sequenceNumber;
            } else {
                comp = (rf1 < rf2)? -1 : 1;
            }
        }
        return comp;
    }

    public String signature() {
        if (signature == null) {
            // Does not need synchronization, since worst-case
            // static info is fetched twice
            try {
                signature = JDWP.ReferenceType.Signature.
                    process(vm, this).signature;
            } catch (JDWPException exc) {
                throw exc.toJDIException();
            }
        }
        return signature;
    }

    public ClassLoaderReference classLoader() {
        if (!isClassLoaderCached) {
            // Does not need synchronization, since worst-case
            // static info is fetched twice
            try {
                classLoader = (ClassLoaderReference)
                    JDWP.ReferenceType.ClassLoader.
                    process(vm, this).classLoader;
                isClassLoaderCached = true;
            } catch (JDWPException exc) {
                throw exc.toJDIException();
            }
        }
        return classLoader;
    }

    public boolean isPublic() {
        if (modifiers == -1)
            getModifiers();

        return((modifiers & VMModifiers.PUBLIC) > 0);
    }

    public boolean isProtected() {
        if (modifiers == -1)
            getModifiers();

        return((modifiers & VMModifiers.PROTECTED) > 0);
    }

    public boolean isPrivate() {
        if (modifiers == -1)
            getModifiers();

        return((modifiers & VMModifiers.PRIVATE) > 0);
    }

    public boolean isPackagePrivate() {
        return !isPublic() && !isPrivate() && !isProtected();
    }

    public boolean isAbstract() {
        if (modifiers == -1)
            getModifiers();

        return((modifiers & VMModifiers.ABSTRACT) > 0);
    }

    public boolean isFinal() {
        if (modifiers == -1)
            getModifiers();

        return((modifiers & VMModifiers.FINAL) > 0);
    }

    public boolean isStatic() {
        if (modifiers == -1)
            getModifiers();

        return((modifiers & VMModifiers.STATIC) > 0);
    }

    public boolean isPrepared() {
        // This ref type may have been prepared before we were getting
        // events, so get it once.  After that,
        // this status flag is updated through the ClassPrepareEvent,
        // there is no need for the expense of a JDWP query.
        if (status == -1) {
	    updateStatus();
        }
        return isPrepared;
    }

    public boolean isVerified() {
        // Once true, it never resets, so we don't need to update
        if ((status & JDWP.ClassStatus.VERIFIED) == 0) {
            updateStatus();
        }
        return (status & JDWP.ClassStatus.VERIFIED) != 0;
    }

    public boolean isInitialized() {
        // Once initialization succeeds or fails, it never resets, 
        // so we don't need to update
        if ((status & INITIALIZED_OR_FAILED) == 0) {
            updateStatus();
        }
        return (status & JDWP.ClassStatus.INITIALIZED) != 0;
    }

    public boolean failedToInitialize() {
        // Once initialization succeeds or fails, it never resets, 
        // so we don't need to update
        if ((status & INITIALIZED_OR_FAILED) == 0) {
            updateStatus();
        }
        return (status & JDWP.ClassStatus.ERROR) != 0;
    }

    public List fields() {
        List fields = (fieldsRef == null) ? null : (List)fieldsRef.get();
        if (fields == null) {
            JDWP.ReferenceType.Fields.FieldInfo[] jdwpFields;
            try {
                jdwpFields = JDWP.ReferenceType.Fields.
                                       process(vm, this).declared;
            } catch (JDWPException exc) {
                throw exc.toJDIException();
            }
            fields = new ArrayList(jdwpFields.length);
            for (int i=0; i<jdwpFields.length; i++) {
                JDWP.ReferenceType.Fields.FieldInfo fi = jdwpFields[i];

                Field field = new FieldImpl(vm, this, fi.fieldID,
                                            fi.name, fi.signature, 
                                            fi.modBits);
                fields.add(field);
            }   
            fields = Collections.unmodifiableList(fields);
            fieldsRef = new SoftReference(fields);
        }
        return fields;
    }

    abstract List inheritedTypes();

    void addVisibleFields(List visibleList, Map visibleTable, List ambiguousNames) {
        List list = visibleFields();
        Iterator iter = list.iterator();
        while (iter.hasNext()) {
            Field field = (Field)iter.next();
            String name = field.name();
            if (!ambiguousNames.contains(name)) {
                Field duplicate = (Field)visibleTable.get(name);
                if (duplicate == null) {
                    visibleList.add(field);
                    visibleTable.put(name, field);
                } else if (!field.equals(duplicate)) {
                    ambiguousNames.add(name);
                    visibleTable.remove(name);
                    visibleList.remove(duplicate);
                } else {
                    // identical field from two branches; do nothing
                }
            }
        }
    }

    public List visibleFields() {
        /*
         * Maintain two different collections of visible fields. The 
         * list maintains a reasonable order for return. The
         * hash map provides an efficient way to lookup visible fields
         * by name, important for finding hidden or ambiguous fields.
         */
        List visibleList = new ArrayList();
        Map  visibleTable = new HashMap();

        /* Track fields removed from above collection due to ambiguity */
        List ambiguousNames = new ArrayList();

        /* Add inherited, visible fields */
        List types = inheritedTypes();
        Iterator iter = types.iterator();
        while (iter.hasNext()) {
            /*
             * TO DO: Be defensive and check for cyclic interface inheritance
             */
            ReferenceTypeImpl type = (ReferenceTypeImpl)iter.next();
            type.addVisibleFields(visibleList, visibleTable, ambiguousNames);
        }

        /* 
         * Insert fields from this type, removing any inherited fields they
         * hide. 
         */
        List retList = new ArrayList(fields());
        iter = retList.iterator();
        while (iter.hasNext()) {
            Field field = (Field)iter.next();
            Field hidden = (Field)visibleTable.get(field.name());
            if (hidden != null) {
                visibleList.remove(hidden);
            }
        }
        retList.addAll(visibleList);
        return retList;
    }

    void addAllFields(List fieldList, Set typeSet) {
        /* Continue the recursion only if this type is new */
        if (!typeSet.contains(this)) {
            typeSet.add(this);

            /* Add local fields */
            fieldList.addAll(fields());

            /* Add inherited fields */
            List types = inheritedTypes();
            Iterator iter = types.iterator();
            while (iter.hasNext()) {
                ReferenceTypeImpl type = (ReferenceTypeImpl)iter.next();
                type.addAllFields(fieldList, typeSet);
            }
        }
    }
    public List allFields() {
        List fieldList = new ArrayList();
        Set typeSet = new HashSet();
        addAllFields(fieldList, typeSet);
        return fieldList;
    }

    public Field fieldByName(String fieldName) {
        java.util.List searchList;
        Field f;

        searchList = visibleFields();

        for (int i=0; i<searchList.size(); i++) {
            f = (Field)searchList.get(i);

            if (f.name().equals(fieldName)) {
                return f;
            }
        }
        //throw new NoSuchFieldException("Field '" + fieldName + "' not found in " + name());
        return null;
    }

    public List methods() {
        List methods = (methodsRef == null) ? null : (List)methodsRef.get();
        if (methods == null) {
            JDWP.ReferenceType.Methods.MethodInfo[] declared;
            try {
                declared = JDWP.ReferenceType.Methods.
                                       process(vm, this).declared;
            } catch (JDWPException exc) {
                throw exc.toJDIException();
            }
            methods = new ArrayList(declared.length);
            for (int i=0; i<declared.length; i++) {
                JDWP.ReferenceType.Methods.MethodInfo mi = declared[i];

                Method method = MethodImpl.createMethodImpl(vm, this, 
                                            mi.methodID,
                                            mi.name, mi.signature, 
                                            mi.modBits);
                methods.add(method);
            }
            methods = Collections.unmodifiableList(methods);
            methodsRef = new SoftReference(methods);
        }
        return methods;
    }


    /*
     * Utility method used by subclasses to build lists of visible
     * methods.
     */
    void addToMethodMap(Map methodMap, List methodList) {
        Iterator iter = methodList.iterator();
        while (iter.hasNext()) {
            Method method = (Method)iter.next();
            methodMap.put(method.name().concat(method.signature()), method);
        }
    }

    abstract void addVisibleMethods(Map methodMap);

    public List visibleMethods() {
        /*
         * Build a collection of all visible methods. The hash
         * map allows us to do this efficiently by keying on the
         * concatenation of name and signature.
         */
        Map map = new HashMap();
        addVisibleMethods(map);

        /*
         * ... but the hash map destroys order. Methods should be
         * returned in a sensible order, as they are in allMethods().
         * So, start over with allMethods() and use the hash map
         * to filter that ordered collection.
         */
        List list = allMethods();
        list.retainAll(map.values());
        return list;
    }

    abstract public List allMethods();

    public List methodsByName(String name) {
        List methods = visibleMethods();
        ArrayList retList = new ArrayList(methods.size());
        Iterator iter = methods.iterator();
        while (iter.hasNext()) {
            Method candidate = (Method)iter.next();
            if (candidate.name().equals(name)) {
                retList.add(candidate);
            }
        }
        retList.trimToSize();
        return retList;
    }

    public List methodsByName(String name, String signature) {
        List methods = visibleMethods();
        ArrayList retList = new ArrayList(methods.size());
        Iterator iter = methods.iterator();
        while (iter.hasNext()) {
            Method candidate = (Method)iter.next();
            if (candidate.name().equals(name) &&
                candidate.signature().equals(signature)) {
                retList.add(candidate);
            }
        }
        retList.trimToSize();
        return retList;
    }

    List getInterfaces() {
        InterfaceTypeImpl[] intfs;
	try {
	    intfs = JDWP.ReferenceType.Interfaces.
                                         process(vm, this).interfaces;
	} catch (JDWPException exc) {
	    throw exc.toJDIException();
	}
        return Arrays.asList(intfs);
    }

    public List nestedTypes() {
        List all = vm.allClasses();
        List nested = new ArrayList();
        String prefix = name() + '$';
        Iterator iter = all.iterator();
        while (iter.hasNext()) {
            ReferenceType refType = (ReferenceType)iter.next();
            if (refType.name().startsWith(prefix)) {
                nested.add(refType);
            }
        }
        return nested;
    }

    public Value getValue(Field sig) {
        List list = new ArrayList(1);
        list.add(sig);
        Map map = getValues(list);
        return(Value)map.get(sig);
    }


    void validateFieldAccess(Field field) {
        /*
         * Field must be in this object's class, a superclass, or
         * implemented interface
         */
        ReferenceTypeImpl declType = (ReferenceTypeImpl)field.declaringType();
        if (!declType.isAssignableFrom(this)) {
            throw new IllegalArgumentException("Invalid field");
        }
    }

    void validateFieldSet(Field field) {
        validateFieldAccess(field);
        if (field.isFinal()) {
            throw new IllegalArgumentException("Cannot set value of static final field");
        }
    }

    /**
     * Returns a map of field values
     */
    public Map getValues(List theFields) {
        validateMirrors(theFields);

        int size = theFields.size();
        JDWP.ReferenceType.GetValues.Field[] queryFields =
                         new JDWP.ReferenceType.GetValues.Field[size];

        for (int i=0; i<size; i++) {
            FieldImpl field = (FieldImpl)theFields.get(i);

            validateFieldAccess(field);

            // Do more validation specific to ReferenceType field getting
            if (!field.isStatic()) {
                throw new IllegalArgumentException(
                     "Attempt to use non-static field with ReferenceType");
            }
            queryFields[i] = new JDWP.ReferenceType.GetValues.Field(
                                         field.ref());
        }

        Map map = new HashMap(size);

	ValueImpl[] values;
	try {
	    values = JDWP.ReferenceType.GetValues.
                                     process(vm, this, queryFields).values;
	} catch (JDWPException exc) {
	    throw exc.toJDIException();
	}

        if (size != values.length) {
            throw new InternalException(
                         "Wrong number of values returned from target VM");
        }
        for (int i=0; i<size; i++) {
            FieldImpl field = (FieldImpl)theFields.get(i);
            map.put(field, values[i]);
        }

        return map;
    }

    public ClassObjectReference classObject() {
        if (classObject == null) {
            // Are classObjects unique for an Object, or
            // created each time? Is this spec'ed?
            synchronized(this) {
                if (classObject == null) {
                    try {
                        classObject = JDWP.ReferenceType.ClassObject.
                            process(vm, this).classObject;
                    } catch (JDWPException exc) {
                        throw exc.toJDIException();
                    }
                }
            }
        }
        return classObject;
    }

    SDE.Stratum stratum(String stratumID) {
        SDE sde = sourceDebugExtensionInfo();
        if (!sde.isValid()) {
            sde = NO_SDE_INFO_MARK;
        }
        return sde.stratum(stratumID);
    }

    public String sourceName() throws AbsentInformationException {
        return (String)(sourceNames(vm.getDefaultStratum()).get(0));
    }

    public List sourceNames(String stratumID)
                                throws AbsentInformationException {
        SDE.Stratum stratum = stratum(stratumID);
        if (stratum.isJava()) {
            List result = new ArrayList(1);
            result.add(baseSourceName());
            return result;
        }
        return stratum.sourceNames(this);
    }

    public List sourcePaths(String stratumID)
                                throws AbsentInformationException {
        SDE.Stratum stratum = stratum(stratumID);
        if (stratum.isJava()) {
            List result = new ArrayList(1);
            result.add(baseSourceDir() + baseSourceName());
            return result;
        }
        return stratum.sourcePaths(this);
    }

    String baseSourceName() throws AbsentInformationException {
        String bsn = baseSourceName;
        if (bsn == null) {
            // Does not need synchronization, since worst-case
            // static info is fetched twice
            try {
                bsn = JDWP.ReferenceType.SourceFile.
                    process(vm, this).sourceFile;
            } catch (JDWPException exc) {
                if (exc.errorCode() == JDWP.Error.ABSENT_INFORMATION) {
                    bsn = ABSENT_BASE_SOURCE_NAME;
                } else {
                    throw exc.toJDIException();
                }
            }
            baseSourceName = bsn;
        }
        if (bsn == ABSENT_BASE_SOURCE_NAME) {
            throw new AbsentInformationException();
        }
        return bsn;
    }

    String baseSourcePath() throws AbsentInformationException {
        String bsp = baseSourcePath;
        if (bsp == null) {
            bsp = baseSourceDir() + baseSourceName();
            baseSourcePath = bsp;
        }
        return bsp;
    }

    String baseSourceDir() {
        if (baseSourceDir == null) {
            String typeName = name();
            StringBuffer sb = new StringBuffer(typeName.length() + 10);
            int index = 0;
            int nextIndex;

            while ((nextIndex = typeName.indexOf('.', index)) > 0) {
                sb.append(typeName.substring(index, nextIndex));
                sb.append(java.io.File.separatorChar);
                index = nextIndex + 1;
            }
            baseSourceDir = sb.toString();
        }
        return baseSourceDir;
    }

    public String sourceDebugExtension()
                           throws AbsentInformationException {
        if (!vm.canGetSourceDebugExtension()) {
            throw new UnsupportedOperationException();
        }
        SDE sde = sourceDebugExtensionInfo();
        if (sde == NO_SDE_INFO_MARK) {
            throw new AbsentInformationException();
        }
        return sde.sourceDebugExtension;
    }

    private SDE sourceDebugExtensionInfo() {
        if (!vm.canGetSourceDebugExtension()) {
            return NO_SDE_INFO_MARK;
        }
        SDE sde = (sdeRef == null) ?  null : (SDE)sdeRef.get();
        if (sde == null) {
            String extension = null;
            try {
                extension = JDWP.ReferenceType.SourceDebugExtension.
                    process(vm, this).extension;
            } catch (JDWPException exc) {
                if (exc.errorCode() != JDWP.Error.ABSENT_INFORMATION) {
                    sdeRef = new SoftReference(NO_SDE_INFO_MARK);
                    throw exc.toJDIException();
                }
            }
            if (extension == null) {
                sde = NO_SDE_INFO_MARK;
            } else {
                sde = new SDE(extension);
            }
            sdeRef = new SoftReference(sde);
        }
        return sde;
    }

    public List availableStrata() {
        SDE sde = sourceDebugExtensionInfo();
        if (sde.isValid()) {
            return sde.availableStrata();
        } else {
            List strata = new ArrayList();
            strata.add(SDE.BASE_STRATUM_NAME);
            return strata;
        }
    }

    /**
     * Always returns non-null stratumID
     */
    public String defaultStratum() {
        SDE sdei = sourceDebugExtensionInfo();
        if (sdei.isValid()) {
            return sdei.defaultStratumId;
        } else {
            return SDE.BASE_STRATUM_NAME;
        }
    }

    public int modifiers() {
        if (modifiers == -1)
            getModifiers();

        return modifiers;
    }        

    public List allLineLocations()
                            throws AbsentInformationException {
        return allLineLocations(vm.getDefaultStratum(), null);
    }

    public List allLineLocations(String stratumID, String sourceName)
                            throws AbsentInformationException {
        boolean someAbsent = false; // A method that should have info, didn't
        SDE.Stratum stratum = stratum(stratumID);
        List list = new ArrayList();  // location list

        for (Iterator iter = methods().iterator(); iter.hasNext(); ) {
            MethodImpl method = (MethodImpl)iter.next();
            try {
                list.addAll(
                   method.allLineLocations(stratum, sourceName));
            } catch(AbsentInformationException exc) {
                someAbsent = true;
            }
        }

        // If we retrieved no line info, and at least one of the methods
        // should have had some (as determined by an 
        // AbsentInformationException being thrown) then we rethrow
        // the AbsentInformationException.
        if (someAbsent && list.size() == 0) {
            throw new AbsentInformationException();
        }
        return list;
    }

    public List locationsOfLine(int lineNumber)
                           throws AbsentInformationException {
        return locationsOfLine(vm.getDefaultStratum(),
                               null,
                               lineNumber);
    }

    public List locationsOfLine(String stratumID,
                                String sourceName,
                                int lineNumber)
                           throws AbsentInformationException {
        // A method that should have info, didn't
        boolean someAbsent = false; 
        // A method that should have info, did
        boolean somePresent = false; 
        List methods = methods();
        SDE.Stratum stratum = stratum(stratumID);

        List list = new ArrayList();

        Iterator iter = methods.iterator();
        while(iter.hasNext()) {
            MethodImpl method = (MethodImpl)iter.next();
            // eliminate native and abstract to eliminate 
            // false positives
            if (!method.isAbstract() && 
                !method.isNative()) {
                try {
                    list.addAll(
                       method.locationsOfLine(stratum,
                                              sourceName,
                                              lineNumber));
                    somePresent = true;
                } catch(AbsentInformationException exc) {
                    someAbsent = true;
                }
            }
        }
        if (someAbsent && !somePresent) {
            throw new AbsentInformationException();
        }
        return list;
    }


    // Does not need synchronization, since worst-case
    // static info is fetched twice
    void getModifiers() {
        if (modifiers != -1) {
            return;
        }
        try {
            modifiers = JDWP.ReferenceType.Modifiers.
                                  process(vm, this).modBits;
        } catch (JDWPException exc) {
            throw exc.toJDIException();
        }
    }

    void decodeStatus(int status) {
        this.status = status;
        if ((status & JDWP.ClassStatus.PREPARED) != 0) {
            isPrepared = true;
        }
    }

    void updateStatus() {
        try {
            decodeStatus(JDWP.ReferenceType.Status.process(vm, this).status);
        } catch (JDWPException exc) {
            throw exc.toJDIException();
        }
    }

    void markPrepared() {
        isPrepared = true;
    }

    long ref() {
        return ref;
    }

    int indexOf(Method method) {
        // Make sure they're all here - the obsolete method
        // won't be found and so will have index -1
        return methods().indexOf(method);
    }

    int indexOf(Field field) {
        // Make sure they're all here
        return fields().indexOf(field);
    }

    /*
     * Return true if an instance of this type
     * can be assigned to a variable of the given type
     */
    abstract boolean isAssignableTo(ReferenceType type);

    boolean isAssignableFrom(ReferenceType type) {
        return ((ReferenceTypeImpl)type).isAssignableTo(this);
    }

    boolean isAssignableFrom(ObjectReference object) {
        return object == null || 
               isAssignableFrom(object.referenceType());
    }

    void setStatus(int status) {
        decodeStatus(status);
    }

    void setSignature(String signature) {
        this.signature = signature;        
    }

    private static boolean isPrimitiveArray(String signature) {
        int i = signature.lastIndexOf('[');
        /*
         * TO DO: Centralize JNI signature knowledge.
         *
         * Ref:
         *  jdk1.4/doc/guide/jpda/jdi/com/sun/jdi/doc-files/signature.html
         */
        boolean isPA;
        if (i < 0) {
            isPA = false;
        } else {
            char c = signature.charAt(i + 1);
            isPA = (c != 'L');
        }
        return isPA;
    }

    Type findType(String signature) throws ClassNotLoadedException {
        Type type;
        if (signature.length() == 1) {
            /* OTI FIX: Must be a primitive type or the void type */
            char sig = signature.charAt(0);
            if (sig == 'V') {
                type = vm.theVoidType();
            } else {
                type = vm.primitiveTypeMirror((byte)sig);
            }
        } else {
            // Must be a reference type.
            ClassLoaderReferenceImpl loader = 
                       (ClassLoaderReferenceImpl)classLoader();
            if ((loader == null) ||
                (isPrimitiveArray(signature)) //Work around 4450091
                ) {
                // Caller wants type of boot class field
                type = vm.findBootType(signature);
            } else {
                // Caller wants type of non-boot class field
                type = loader.findType(signature);
            }
        }
        return type;
    }

    String loaderString() {
        if (classLoader() != null) {
            return "loaded by " + classLoader().toString();
        } else {
            return "no class loader";
        }
    }
}
