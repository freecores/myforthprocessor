/*
 * @(#)FieldImpl.java	1.3 02/03/04
 *
 * Copyright 2003 Sun Microsystems, Inc. All Rights Reserved.
 *
 * This software is the proprietary information of Sun Microsystems, Inc.
 * Use is subject to license terms.
 *
 */

package sun.jvm.hotspot.jdi;

import com.sun.jdi.*;
import sun.jvm.hotspot.oops.InstanceKlass;
import sun.jvm.hotspot.oops.Symbol;
import sun.jvm.hotspot.oops.FieldIdentifier;

import java.util.List;
import java.util.Iterator;
import java.util.ArrayList;
import java.util.Comparator;

public class FieldImpl extends TypeComponentImpl implements Field {
    private JNITypeParser signatureParser;
    private sun.jvm.hotspot.oops.Field saField;

    FieldImpl( VirtualMachine vm, ReferenceTypeImpl declaringType,
               sun.jvm.hotspot.oops.Field saField) {
        super(vm, declaringType);
        this.saField = saField;
        getParser();
    }

    private void getParser() {
        if (signatureParser == null) {
            Symbol sig1 = saField.getSignature();
            signature = sig1.asString();
            signatureParser = new JNITypeParser(signature);
        }
    }

    Object ref() {
        return saField;
    }

    public boolean equals(Object obj) {
        if ((obj != null) && (obj instanceof MethodImpl)) {
            FieldImpl other = (FieldImpl)obj;
            return (declaringType().equals(other.declaringType())) &&
                (ref() == other.ref()) &&
                super.equals(obj);
        } else {
            return false;
        }
    }

    public boolean isTransient() {
        return saField.isTransient();
    }

    public boolean isVolatile() {
        return saField.isVolatile();
    }
  
    public Type type() throws ClassNotLoadedException { //fixme jjh: saField.getFieldType returns an SA FieldType instead of a Type.
        // So, we do it just like JDI does by searching the enclosing type.
        return findType(signature());
    }

    public String typeName() { //fixme jjh: jpda version creates redundant JNITypeParsers
        getParser();
        return signatureParser.typeName();
    }

    // From interface Comparable
    public int compareTo(Object object) {
        Field field = (Field)object;
        ReferenceTypeImpl declaringType = (ReferenceTypeImpl)declaringType();
        int rc = declaringType.compareTo(field.declaringType());
        if (rc == 0) {
            rc = declaringType.indexOf(this) - 
                declaringType.indexOf(field);
        }
        return rc;
    }

    // from interface Mirror
    public String toString() {
        StringBuffer buf = new StringBuffer();

        buf.append(declaringType().name());
        buf.append('.');
        buf.append(name());
        return buf.toString();
    }

    public String name() {
        FieldIdentifier myName =  saField.getID();
        return myName.getName();
    }

    // From interface Accessible
    public int modifiers() {
        return (int)saField.getAccessFlags(); //fixme jjh: is this correct; no long vs. int issue?
    }

    public boolean isPackagePrivate() {
        return saField.isPackagePrivate();
    }

    public boolean isPrivate() {
        return saField.isPrivate();
    }

    public boolean isProtected() {
        return saField.isProtected();
    }

    public boolean isPublic() {
        return saField.isPublic();
    }


    public boolean isStatic() {
        return saField.isStatic();
    }

    public boolean isFinal() {
        return saField.isFinal();
    }

    public boolean isSynthetic() {
        return saField.isSynthetic();
    }

    public int hashCode() { //fixme jjh; needed in SA
        return saField.hashCode();
    }


    private Type findType(String signature) throws ClassNotLoadedException {
        ReferenceTypeImpl enclosing = (ReferenceTypeImpl)declaringType(); 
        return enclosing.findType(signature);
    }

}
