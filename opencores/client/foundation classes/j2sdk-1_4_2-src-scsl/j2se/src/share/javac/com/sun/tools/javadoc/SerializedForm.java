/**
 * @(#)SerializedForm.java	1.20 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javadoc;
import com.sun.javadoc.*;

import com.sun.tools.javac.v8.code.Flags;

import com.sun.tools.javac.v8.code.Kinds;

import com.sun.tools.javac.v8.code.Scope;

import com.sun.tools.javac.v8.code.Symbol.VarSymbol;

import com.sun.tools.javac.v8.code.Symbol.ClassSymbol;

import com.sun.tools.javac.v8.code.Symbol.MethodSymbol;

import com.sun.tools.javac.v8.util.Name;

import com.sun.tools.javac.v8.util.ListBuffer;


/**
 * The serialized form is the specification of a class' serialization
 * state. <p>
 *
 * It consists of the following information:<p>
 *
 * <pre>
 * 1. Whether class is Serializable or Externalizable.
 * 2. Javadoc for serialization methods.
 *    a. For Serializable, the optional readObject, writeObject,
 *       readResolve and writeReplace.
 *       serialData tag describes, in prose, the sequence and type
 *       of optional data written by writeObject.
 *    b. For Externalizable, writeExternal and readExternal.
 *       serialData tag describes, in prose, the sequence and type
 *       of optional data written by writeExternal.
 * 3. Javadoc for serialization data layout.
 *    a. For Serializable, the name,type and description
 *       of each Serializable fields.
 *    b. For Externalizable, data layout is described by 2(b).
 * </pre>
 *
 * @since JDK1.2
 * @author Joe Fialli
 * @author Neal Gafter (rewrite but not too proud)
 */
class SerializedForm {
    ListBuffer methods = new ListBuffer();
    private final ListBuffer fields = new ListBuffer();
    private boolean definesSerializableFields = false;
    private static final String SERIALIZABLE_FIELDS = "serialPersistentFields";
    private static final String READOBJECT = "readObject";
    private static final String WRITEOBJECT = "writeObject";
    private static final String READRESOLVE = "readResolve";
    private static final String WRITEREPLACE = "writeReplace";

    /**
     * Constructor.
     *
     * Catalog Serializable fields for Serializable class.
     * Catalog serialization methods for Serializable and
     * Externalizable classes.
     */
    SerializedForm(DocEnv env, ClassSymbol def, ClassDocImpl cd) {
        super();
        if (cd.isExternalizable()) {
            String[] readExternalParamArr = {"java.io.ObjectInput"};
            String[] writeExternalParamArr = {"java.io.ObjectOutput"};
            MethodDoc md = cd.findMethod("readExternal", readExternalParamArr);
            if (md != null) {
                methods.append(md);
            }
            md = cd.findMethod("writeExternal", writeExternalParamArr);
            if (md != null) {
                methods.append(md);
                Tag[] tag = md.tags("serialData");
            }
        } else if (cd.isSerializable()) {
            VarSymbol dsf = getDefinedSerializableFields(def);
            if (dsf != null) {
                definesSerializableFields = true;
                FieldDocImpl dsfDoc = env.getFieldDoc(dsf);
                fields.append(dsfDoc);
                mapSerialFieldTagImplsToFieldDocImpls(dsfDoc, env, def);
            } else {
                computeDefaultSerializableFields(env, def, cd);
            }
            addMethodIfExist(env, def, READOBJECT);
            addMethodIfExist(env, def, WRITEOBJECT);
            addMethodIfExist(env, def, READRESOLVE);
            addMethodIfExist(env, def, WRITEREPLACE);
        }
    }

    private VarSymbol getDefinedSerializableFields(ClassSymbol def) {
        Name.Table names = def.name.table;
        for (Scope.Entry e =
                def.members().lookup(names.fromString(SERIALIZABLE_FIELDS));
                e != null; e = e.shadowed) {
            if (e.sym != null && e.sym.kind == Kinds.VAR) {
                VarSymbol f = (VarSymbol) e.sym;
                if ((f.flags() & Flags.STATIC) != 0 &&
                        (f.flags() & Flags.PRIVATE) != 0) {
                    return f;
                }
            }
        }
        return null;
    }

    private void computeDefaultSerializableFields(DocEnv env, ClassSymbol def,
            ClassDocImpl cd) {
        for (Scope.Entry e = def.members().elems; e != null; e = e.sibling) {
            if (e.sym != null && e.sym.kind == Kinds.VAR) {
                VarSymbol f = (VarSymbol) e.sym;
                if ((f.flags() & Flags.STATIC) == 0 &&
                        (f.flags() & Flags.TRANSIENT) == 0) {
                    FieldDocImpl fd = env.getFieldDoc(f);
                    fields.prepend(fd);
                }
            }
        }
    }

    private void addMethodIfExist(DocEnv env, ClassSymbol def, String methodName) {
        Name.Table names = def.name.table;
        for (Scope.Entry e = def.members().lookup(names.fromString(methodName));
                e != null; e = e.shadowed) {
            if (e.sym != null && e.sym.kind == Kinds.MTH &&
                    e.sym.name.toString().equals(methodName)) {
                MethodSymbol md = (MethodSymbol) e.sym;
                if ((md.flags() & Flags.STATIC) == 0) {
                    methods.append(env.getMethodDoc(md));
                }
            }
        }
    }

    private void mapSerialFieldTagImplsToFieldDocImpls(FieldDocImpl spfDoc,
            DocEnv env, ClassSymbol def) {
        Name.Table names = def.name.table;
        SerialFieldTag[] sfTag = spfDoc.serialFieldTags();
        for (int i = 0; i < sfTag.length; i++) {
            Name fieldName = names.fromString(sfTag[i].fieldName());
            for (Scope.Entry e = def.members().lookup(fieldName); e != null;
                    e = e.shadowed) {
                if (e.sym != null && e.sym.kind == Kinds.VAR) {
                    VarSymbol f = (VarSymbol) e.sym;
                    FieldDocImpl fdi = env.getFieldDoc(f);
                    ((SerialFieldTagImpl)(sfTag[i])).mapToFieldDocImpl(fdi);
                    break;
                }
            }
        }
    }

    /**
      * Return serializable fields in class. <p>
      *
      * Returns either a list of default fields documented by serial tag comment or
      *         javadoc comment<p>
      * Or Returns a single FieldDocImpl for serialPersistentField. There is a
      *         serialField tag for each serializable field.<p>
      *
      * @return an array of FieldDocImpl for representing the visible
      *         fields in this class.
      */
    FieldDoc[] fields() {
        return (FieldDoc[]) fields.toArray(new FieldDocImpl[fields.length()]);
    }

    /**
      * Return serialization methods in class.
      *
      * @return an array of MethodDocImpl for serialization methods in this class.
      */
    MethodDoc[] methods() {
        return (MethodDoc[]) methods.toArray(new MethodDoc[methods.length()]);
    }

    /**
      * Returns true if Serializable fields are defined explicitly using
      * member, serialPersistentFields.
      *
      * @see #fields()
      */
    boolean definesSerializableFields() {
        return definesSerializableFields;
    }
}
