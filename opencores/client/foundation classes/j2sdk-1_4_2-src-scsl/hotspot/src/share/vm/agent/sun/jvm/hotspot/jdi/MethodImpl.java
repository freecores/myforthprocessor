/*
 * @(#)MethodImpl.java	1.3 02/03/04
 *
 * Copyright 2003 Sun Microsystems, Inc. All Rights Reserved.
 *
 * This software is the proprietary information of Sun Microsystems, Inc.
 * Use is subject to license terms.
 *
 */

package sun.jvm.hotspot.jdi;

import com.sun.jdi.*;
import sun.jvm.hotspot.oops.Symbol;

import java.util.List;
import java.util.Iterator;
import java.util.ArrayList;
import java.util.Comparator;
import java.lang.ref.SoftReference;
import java.util.Collections;

public class MethodImpl extends TypeComponentImpl implements Method {
    private JNITypeParser signatureParser;
    private sun.jvm.hotspot.oops.Method saMethod;
    private int argSlotCount = -1;
    private SoftReference variablesRef = null;
    private Location location = null;

    static MethodImpl createMethodImpl(VirtualMachine vm, ReferenceTypeImpl declaringType,
                                       sun.jvm.hotspot.oops.Method saMethod) {
        // Someday might have to add concrete and non-concrete subclasses.
        if (saMethod.isNative() || saMethod.isAbstract()) {
            return new NonConcreteMethodImpl(vm, declaringType, saMethod);
        }
        return new MethodImpl(vm, declaringType, saMethod);
    }

    MethodImpl(VirtualMachine vm, ReferenceTypeImpl declaringType,
               sun.jvm.hotspot.oops.Method saMethod ) {
        super(vm, declaringType);
        this.saMethod = saMethod;
        getParser();
        //System.out.println("jj: MethodImpl: " + this + ", hash = " + hashCode);
    }

    private JNITypeParser getParser() {
        if (signatureParser == null) {
            Symbol sig1 = saMethod.getSignature();
            signature = sig1.asString();
            signatureParser = new JNITypeParser(signature);
        }
        return signatureParser;
    }

    // Object ref() {
    sun.jvm.hotspot.oops.Method ref() {
        return saMethod;
    }

    int argSlotCount() throws AbsentInformationException {
        if (argSlotCount == -1) {
            getVariables();
        }
        return argSlotCount;
    }

    public String returnTypeName() {
        return getParser().typeName();
    }

    public Type returnType() throws ClassNotLoadedException {
        return findType(getParser().signature());
    }

    private Type findType(String signature) throws ClassNotLoadedException { 
        ReferenceTypeImpl enclosing = (ReferenceTypeImpl)declaringType(); 
        return enclosing.findType(signature);
    }

    public List argumentTypeNames() {
        return getParser().argumentTypeNames();
    }

    private List argumentSignatures() {
        return getParser().argumentSignatures();
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

    public boolean isAbstract() {
        return saMethod.isAbstract();
    }

    public boolean isSynchronized() {
        return saMethod.isSynchronized();
    }

    public boolean isNative() {
        return saMethod.isNative();
    }

    public boolean isConstructor() { //fixme jjh; need this in SA
        return false;
    }

    public boolean isStaticInitializer() { //fixme jjh; need this in SA
        return false;
    }

    public boolean isObsolete() { //fixme jjh; need this in SA
        return false;
    }

    public List allLineLocations() throws AbsentInformationException { //fixme jjh; need this in SA
        return null;
    }

    public List allLineLocations(String stratum, String sourceName) // fixme jjh; need this in SA
        throws AbsentInformationException {
        return null;
    }

    public List locationsOfLine(int lineNumber) throws AbsentInformationException {  //fixme jjh; need this in SA
        return null;
    }

    public List locationsOfLine(String stratum, String sourceName,  //fixme jjh; need this in SA
                         int lineNumber) throws AbsentInformationException {
        return null;
    }

    LineInfo codeIndexToLineInfo(SDE.Stratum stratum, //jjh fixme stratum ignored
                                 long codeIndex) {
        int lineNum = saMethod.getLineNumberFromBCI((int)codeIndex);
        return new BaseLineInfo(lineNum, declaringType);
    }

    public Location locationOfCodeIndex(long codeIndex) { //fixme jjh; need this in SA
        return null;
    }

    public List variables() throws AbsentInformationException {
        return getVariables();
    }

    public List variablesByName(String name) throws AbsentInformationException {
        List variables = getVariables();
        
        List retList = new ArrayList(2);
        Iterator iter = variables.iterator();
        while(iter.hasNext()) {
            LocalVariable variable = (LocalVariable)iter.next();
            if (variable.name().equals(name)) {
                retList.add(variable);
            }
        }
        return retList;
    }

    public List arguments() throws AbsentInformationException {
        List variables = getVariables();
        
        List retList = new ArrayList(variables.size());
        Iterator iter = variables.iterator();
        while(iter.hasNext()) {
            LocalVariable variable = (LocalVariable)iter.next();
            if (variable.isArgument()) {
                retList.add(variable);
            }
        }
        return retList;
    }

    public byte[] bytecodes() {
        return saMethod.getByteCode();
    }

    public Location location() {
        if (location == null) {
            // This is odd; what is the Location of a Method?
            // A StackFrame can have a location, but a Method?
            // I guess it must be the Location for bci 0.
            location = new LocationImpl(virtualMachine(), this, 0);
        }
        return location;
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

    // From interface Comparable
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

    // from interface Mirror
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

    public String name() {
        Symbol myName = saMethod.getName();
        return myName.asString();
    }

    public int modifiers() {
        return (int)saMethod.getAccessFlags(); //fixme jjh: is this correct; no long vs. int issue?
    }

    public boolean isPackagePrivate() {
        return saMethod.isPackagePrivate();
    }

    public boolean isPrivate() {
        return saMethod.isPrivate();
    }

    public boolean isProtected() {
        return saMethod.isProtected();
    }

    public boolean isPublic() {
        return saMethod.isPublic();
    }

    public boolean isStatic() {
        return saMethod.isStatic();
    }

    public boolean isSynthetic() {
        return saMethod.isStatic();
    }

    public boolean isFinal() {
        return saMethod.isFinal();
    }

    public int hashCode() {
        return saMethod.hashCode();
    }

    private List getVariables() throws AbsentInformationException { //fixme jjh
        List variables = (variablesRef == null) ? null :
                                     (List)variablesRef.get();
        if (variables != null) {
            return variables;
        }

        VariableTable vartab = null;

        // Fill in vartab from SA info.  fixme jjh

        // Get thenumber of slots used by argument variables
        argSlotCount = vartab.argCnt;
        int count = vartab.slots.length;
        variables = new ArrayList(count);
        for (int i=0; i<count; i++) {
            VariableTable.SlotInfo si = vartab.slots[i];

            /*
             * Skip "this*" entries because they are never real
             * variables from the JLS perspective.
             */
            if (!si.name.startsWith("this$") && !si.name.equals("this")) {
                Location scopeStart = new LocationImpl(virtualMachine(),
                                                       this, si.codeIndex);
                Location scopeEnd = 
                    new LocationImpl(virtualMachine(), this,
                                     si.codeIndex + si.length - 1);
                LocalVariable variable = 
                    new LocalVariableImpl(virtualMachine(), this, 
                                          si.slot, scopeStart, scopeEnd,
                                          si.name, si.signature);   
                // Add to the variable list
                variables.add(variable);
            }
        }

        variables = Collections.unmodifiableList(variables);
        variablesRef = new SoftReference(variables);
        return variables;
    }

}

class VariableTable {
    /**
     * The number of words in the frame used by arguments. 
     * Eight-byte arguments use two words; all others use one. 
     */
    int argCnt = -1;
    
    /**
     * The number of variables.
     */
    SlotInfo[] slots = null;
    
    class SlotInfo {

        /**
         * First code index at which the variable is visible (unsigned). 
         * Used in conjunction with <code>length</code>. 
         * The variable can be get or set only when the current 
         * <code>codeIndex</code> <= current frame code index < <code>codeIndex + length</code> 
         */
        long codeIndex;
        
        /**
         * The variable's name.
         */
        String name;
        
        /**
         * The variable type's JNI signature.
         */
        String signature;
        
        /**
         * Unsigned value used in conjunction with <code>codeIndex</code>. 
         * The variable can be get or set only when the current 
         * <code>codeIndex</code> <= current frame code index < <code>code index + length</code> 
         */
        int length;
        
        /**
         * The local variable's index in its frame
         */
        int slot;
    }
}
