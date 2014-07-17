/*
 * @(#)MethodImpl.java	1.45 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdi;

import com.sun.jdi.*;

import java.util.List;
import java.util.Iterator;
import java.util.ArrayList;
import java.util.Comparator;

public abstract class MethodImpl extends TypeComponentImpl implements Method {
    private JNITypeParser signatureParser;

    abstract int argSlotCount() throws AbsentInformationException;

    abstract List allLineLocations(SDE.Stratum stratum,
                                   String sourceName)
                           throws AbsentInformationException;

    abstract List locationsOfLine(SDE.Stratum stratum,
                                  String sourceName,
                                  int lineNumber)
                           throws AbsentInformationException;

    MethodImpl(VirtualMachine vm, ReferenceTypeImpl declaringType, 
               long ref,
               String name, String signature, int modifiers) {
        super(vm, declaringType, ref, name, signature,  modifiers);
        signatureParser = new JNITypeParser(signature);
    }

    static MethodImpl createMethodImpl(VirtualMachine vm, 
                                       ReferenceTypeImpl declaringType, 
                                       long ref,
                                       String name, String signature, 
                                       int modifiers) {
        if ((modifiers & 
             (VMModifiers.NATIVE | VMModifiers.ABSTRACT)) != 0) {
            return new NonConcreteMethodImpl(vm, declaringType, ref,
                                             name, signature, modifiers);
        } else {
            return new ConcreteMethodImpl(vm, declaringType, ref,
                                          name, signature, modifiers);
        }
    }

    public boolean equals(Object obj) {
        if ((obj != null) && (obj instanceof MethodImpl)) {
            MethodImpl other = (MethodImpl)obj;
            return (declaringType().equals(other.declaringType())) &&
                   (ref() == other.ref()) &&
                   super.equals(obj);
        } else {
            return false;
        }
    }

    public int hashCode() {
        return (int)ref();
    }

    public final List allLineLocations()
                           throws AbsentInformationException {
        return allLineLocations(vm.getDefaultStratum(), null);
    }

    public List allLineLocations(String stratumID,
                                 String sourceName)
                           throws AbsentInformationException {
        return allLineLocations(declaringType.stratum(stratumID),
                                sourceName);
    }

    public final List locationsOfLine(int lineNumber)
                           throws AbsentInformationException {
        return locationsOfLine(vm.getDefaultStratum(), 
                               null, lineNumber);
    }

    public List locationsOfLine(String stratumID,
                                String sourceName,
                                int lineNumber)
                           throws AbsentInformationException {
        return locationsOfLine(declaringType.stratum(stratumID),
                               sourceName, lineNumber);
    }

    LineInfo codeIndexToLineInfo(SDE.Stratum stratum, 
                                 long codeIndex) {
        if (stratum.isJava()) {
            return new BaseLineInfo(-1, declaringType);
        } else {
            return new StratumLineInfo(stratum.id(), -1,
                                       null, null);
        }
    }

    /**
     * @return a text representation of the declared return type
     * of this method.
     */
    public String returnTypeName() {
        return signatureParser.typeName();
    }

    private String returnSignature() {
        return signatureParser.signature();
    }

    public Type returnType() throws ClassNotLoadedException {
        return findType(returnSignature());
    }

    public Type findType(String signature) throws ClassNotLoadedException {
        ReferenceTypeImpl enclosing = (ReferenceTypeImpl)declaringType(); 
        return enclosing.findType(signature);
    }


    public List argumentTypeNames() {
        return signatureParser.argumentTypeNames();
    }

    public List argumentSignatures() {
        return signatureParser.argumentSignatures();
    }

    Type argumentType(int index) throws ClassNotLoadedException {
        ReferenceTypeImpl enclosing = (ReferenceTypeImpl)declaringType(); 
        String signature = (String)argumentSignatures().get(index);
        return enclosing.findType(signature);
    }

    public List argumentTypes() throws ClassNotLoadedException {
        int size = argumentSignatures().size();
        ArrayList types = new ArrayList(size);
        for (int i = 0; i < size; i++) {
            Type type = argumentType(i);
            types.add(type);
        }

        return types;
    }

    public int compareTo(Object object) {
        Method method = (Method)object;
        ReferenceTypeImpl declaringType = (ReferenceTypeImpl)declaringType();
        int rc = declaringType.compareTo(method.declaringType());
        if (rc == 0) {
            rc = declaringType.indexOf(this) - 
                    declaringType.indexOf(method);
        }
        return rc;
    }
    
    public boolean isAbstract() {
        return isModifierSet(VMModifiers.ABSTRACT);
    }
 
    public boolean isSynchronized() {
        return isModifierSet(VMModifiers.SYNCHRONIZED);
    }

    public boolean isNative() {
        return isModifierSet(VMModifiers.NATIVE);
    }

    public boolean isConstructor() {
        return name().equals("<init>");
    }

    public boolean isStaticInitializer() {
        return name().equals("<clinit>");
    }
                                
    public boolean isObsolete() {
        try {
            return JDWP.Method.IsObsolete.process(vm, 
                                    declaringType, ref).isObsolete;
        } catch (JDWPException exc) {
            throw exc.toJDIException();
        }
    }

    /*
     * A container class for the argument to allow
     * proper type-checking.
     */
    class ArgumentContainer implements ValueContainer {
        int index;

        ArgumentContainer(int index) {
            this.index = index;
        }
        public Type type() throws ClassNotLoadedException {
            return argumentType(index);
        }
        public String typeName(){
            return (String)argumentTypeNames().get(index);
        }
        public String signature() {
            return (String)argumentSignatures().get(index);
        }
        public Type findType(String signature) throws ClassNotLoadedException {
            return MethodImpl.this.findType(signature);
        }
    }

    void prepareArgumentsForInvoke(List arguments) 
                         throws ClassNotLoadedException, InvalidTypeException {

        int size = arguments.size();
        for (int i = 0; i < size; i++) {
            Value value = (Value)arguments.get(i);
            value = ValueImpl.prepareForAssignment(value, 
                                                   new ArgumentContainer(i));
            arguments.set(i, value);
        }
    }

    public String toString() {
        StringBuffer sb = new StringBuffer();
        sb.append(declaringType().name());
        sb.append(".");
        sb.append(name());
        sb.append("(");
        boolean first = true;
        for (Iterator it = argumentTypeNames().iterator(); it.hasNext();) {
            if (!first) {
                sb.append(", ");
            }
            sb.append((String)it.next());
            first = false;
        }
        sb.append(")");
        return sb.toString();
    }
}
