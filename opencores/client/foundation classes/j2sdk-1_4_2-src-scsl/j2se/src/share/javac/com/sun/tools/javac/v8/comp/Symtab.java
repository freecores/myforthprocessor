/**
 * @(#)Symtab.java	1.18 01/08/17
 *
 * Copyright 2003 Sun Microsystems, Inc. All Rights Reserved.
 *
 * This software is the proprietary information of Sun Microsystems, Inc.
 * Use is subject to license terms.
 *
 */
package com.sun.tools.javac.v8.comp;
import com.sun.tools.javac.v8.util.*;

import com.sun.tools.javac.v8.code.*;

import com.sun.tools.javac.v8.code.Symbol.*;

import com.sun.tools.javac.v8.code.Type.*;


/**
 * A class that defines all predefined constants and operators
 *  as well as special classes such as java.lang.Object, which need
 *  to be known to the compiler. All symbols are held in instance
 *  fields. This makes it possible to work in multiple concurrent
 *  projects, which might use different class files for library classes.
 */
public class Symtab implements Flags, ByteCodes {

    /**
     * The current class reader.
     */
    public ClassReader reader;

    /**
     * The current class writer.
     */
    public ClassWriter writer;

    /**
     * Predefined types.
     */
    public Type objectType;
    public Type classType;
    public Type classLoaderType;
    public Type stringType;
    public Type stringBufferType;
    public Type cloneableType;
    public Type serializableType;
    public Type throwableType;
    public Type errorType;
    public Type exceptionType;
    public Type runtimeExceptionType;
    public Type classNotFoundExceptionType;
    public Type noClassDefFoundErrorType;
    public Type assertionErrorType;

    /**
     * The symbol representing the length field of an array.
     */
    public VarSymbol lengthVar;

    /**
     * Predefined constants.
     */
    public VarSymbol nullConst;
    public VarSymbol trueConst;
    public VarSymbol falseConst;

    /**
     * The class symbol that owns all predefined symbols.
     */
    public ClassSymbol predefClass;

    /**
     * Enter a constant into symbol table.
     *  @param name   The constant's name.
     *  @param type   The constant's type.
     */
    private VarSymbol enterConstant(String name, Type type) {
        VarSymbol c = new VarSymbol(PUBLIC | STATIC | FINAL,
                StaticName.fromString(name), type, predefClass);
        c.constValue = type.constValue;
        predefClass.members().enter(c);
        return c;
    }

    /**
      * Enter a binary operation into symbol table.
      *  @param name     The name of the operator.
      *  @param left     The type of the left operand.
      *  @param right    The type of the left operand.
      *  @param res      The operation's result type.
      *  @param opcode   The operation's bytecode instruction.
      */
    private void enterBinop(String name, Type left, Type right, Type res,
            int opcode) {
        predefClass.members().enter( new OperatorSymbol(StaticName.fromString(name),
                new MethodType(List.make(left, right), res, Type.emptyList),
                opcode, predefClass));
    }

    /**
      * Enter a binary operation, as above but with two opcodes,
      *  which get encoded as (opcode1 << ByteCodeTags.preShift) + opcode2.
      *  @param opcode1     First opcode.
      *  @param opcode2     Second opcode.
      */
    private void enterBinop(String name, Type left, Type right, Type res,
            int opcode1, int opcode2) {
        enterBinop(name, left, right, res, (opcode1 << ByteCodes.preShift) | opcode2);
    }

    /**
      * Enter a unary operation into symbol table.
      *  @param name     The name of the operator.
      *  @param arg      The type of the operand.
      *  @param res      The operation's result type.
      *  @param opcode   The operation's bytecode instruction.
      */
    private void enterUnop(String name, Type arg, Type res, int opcode) {
        predefClass.members().enter( new OperatorSymbol(StaticName.fromString(name),
                new MethodType(List.make(arg), res, Type.emptyList), opcode,
                predefClass));
    }

    /**
      * Enter a class into symbol table.
      *  @param    The name of the class.
      */
    private Type enterClass(String s) {
        return reader.enterClass(StaticName.fromString(s)).type;
    }

    /**
      * Constructor; enters all predefined identifiers and operators
      *  into symbol table.
      */
    public Symtab(ClassReader reader, ClassWriter writer) throws CompletionFailure {
        super();
        predefClass = new ClassSymbol(PUBLIC, Names.empty, Symbol.rootPackage);
        Scope scope = new Scope(predefClass);
        predefClass.members_field = scope;
        this.reader = reader;
        this.writer = writer;
        reader.classes.put(predefClass.fullname, predefClass);
        scope.enter(Type.byteType.tsym);
        scope.enter(Type.shortType.tsym);
        scope.enter(Type.charType.tsym);
        scope.enter(Type.intType.tsym);
        scope.enter(Type.longType.tsym);
        scope.enter(Type.floatType.tsym);
        scope.enter(Type.doubleType.tsym);
        scope.enter(Type.booleanType.tsym);
        scope.enter(Type.errType.tsym);
        objectType = enterClass("java.lang.Object");
        classType = enterClass("java.lang.Class");
        stringType = enterClass("java.lang.String");
        stringBufferType = enterClass("java.lang.StringBuffer");
        cloneableType = enterClass("java.lang.Cloneable");
        throwableType = enterClass("java.lang.Throwable");
        serializableType = enterClass("java.io.Serializable");
        errorType = enterClass("java.lang.Error");
        exceptionType = enterClass("java.lang.Exception");
        runtimeExceptionType = enterClass("java.lang.RuntimeException");
        classNotFoundExceptionType = enterClass("java.lang.ClassNotFoundException");
        noClassDefFoundErrorType = enterClass("java.lang.NoClassDefFoundError");
        assertionErrorType = enterClass("java.lang.AssertionError");
        classLoaderType = enterClass("java.lang.ClassLoader");
        ClassType arrayClassType = (ClassType) ArrayType.arrayClass.type;
        arrayClassType.supertype_field = objectType;
        arrayClassType.interfaces_field = List.make(cloneableType, serializableType);
        ArrayType.arrayClass.members_field = new Scope(ArrayType.arrayClass);
        lengthVar = new VarSymbol(PUBLIC | FINAL, StaticName.fromString("length"),
                Type.intType, ArrayType.arrayClass);
        ArrayType.arrayClass.members().enter(lengthVar);
        Symbol cloneMethod = new MethodSymbol(PUBLIC, StaticName.fromString("clone"),
                new MethodType(Type.emptyList, objectType, Type.emptyList),
                objectType.tsym);
        ArrayType.arrayClass.members().enter(cloneMethod);
        nullConst = enterConstant("null", Type.botType);
        trueConst = enterConstant("true", Type.booleanType.constType(new Integer(1)));
        falseConst =
                enterConstant("false", Type.booleanType.constType(new Integer(0)));
        enterUnop("+", Type.intType, Type.intType, nop);
        enterUnop("+", Type.longType, Type.longType, nop);
        enterUnop("+", Type.floatType, Type.floatType, nop);
        enterUnop("+", Type.doubleType, Type.doubleType, nop);
        enterUnop("-", Type.intType, Type.intType, ineg);
        enterUnop("-", Type.longType, Type.longType, lneg);
        enterUnop("-", Type.floatType, Type.floatType, fneg);
        enterUnop("-", Type.doubleType, Type.doubleType, dneg);
        enterUnop("~", Type.intType, Type.intType, ixor);
        enterUnop("~", Type.longType, Type.longType, lxor);
        enterUnop("++", Type.byteType, Type.byteType, iadd);
        enterUnop("++", Type.shortType, Type.shortType, iadd);
        enterUnop("++", Type.charType, Type.charType, iadd);
        enterUnop("++", Type.intType, Type.intType, iadd);
        enterUnop("++", Type.longType, Type.longType, ladd);
        enterUnop("++", Type.floatType, Type.floatType, fadd);
        enterUnop("++", Type.doubleType, Type.doubleType, dadd);
        enterUnop("--", Type.byteType, Type.byteType, isub);
        enterUnop("--", Type.shortType, Type.shortType, isub);
        enterUnop("--", Type.charType, Type.charType, isub);
        enterUnop("--", Type.intType, Type.intType, isub);
        enterUnop("--", Type.longType, Type.longType, lsub);
        enterUnop("--", Type.floatType, Type.floatType, fsub);
        enterUnop("--", Type.doubleType, Type.doubleType, dsub);
        enterUnop("!", Type.booleanType, Type.booleanType, bool_not);
        enterUnop("<*nullchk*>", objectType, objectType, nullchk);
        enterBinop("+", stringType, stringType, stringType, string_add);
        enterBinop("+", stringType, Type.intType, stringType, string_add);
        enterBinop("+", stringType, Type.longType, stringType, string_add);
        enterBinop("+", stringType, Type.floatType, stringType, string_add);
        enterBinop("+", stringType, Type.doubleType, stringType, string_add);
        enterBinop("+", stringType, Type.booleanType, stringType, string_add);
        enterBinop("+", stringType, objectType, stringType, string_add);
        enterBinop("+", Type.intType, stringType, stringType, string_add);
        enterBinop("+", Type.longType, stringType, stringType, string_add);
        enterBinop("+", Type.floatType, stringType, stringType, string_add);
        enterBinop("+", Type.doubleType, stringType, stringType, string_add);
        enterBinop("+", Type.booleanType, stringType, stringType, string_add);
        enterBinop("+", objectType, stringType, stringType, string_add);
        enterBinop("+", Type.intType, Type.intType, Type.intType, iadd);
        enterBinop("+", Type.longType, Type.longType, Type.longType, ladd);
        enterBinop("+", Type.floatType, Type.floatType, Type.floatType, fadd);
        enterBinop("+", Type.doubleType, Type.doubleType, Type.doubleType, dadd);
        enterBinop("-", Type.intType, Type.intType, Type.intType, isub);
        enterBinop("-", Type.longType, Type.longType, Type.longType, lsub);
        enterBinop("-", Type.floatType, Type.floatType, Type.floatType, fsub);
        enterBinop("-", Type.doubleType, Type.doubleType, Type.doubleType, dsub);
        enterBinop("*", Type.intType, Type.intType, Type.intType, imul);
        enterBinop("*", Type.longType, Type.longType, Type.longType, lmul);
        enterBinop("*", Type.floatType, Type.floatType, Type.floatType, fmul);
        enterBinop("*", Type.doubleType, Type.doubleType, Type.doubleType, dmul);
        enterBinop("/", Type.intType, Type.intType, Type.intType, idiv);
        enterBinop("/", Type.longType, Type.longType, Type.longType, ldiv);
        enterBinop("/", Type.floatType, Type.floatType, Type.floatType, fdiv);
        enterBinop("/", Type.doubleType, Type.doubleType, Type.doubleType, ddiv);
        enterBinop("%", Type.intType, Type.intType, Type.intType, imod);
        enterBinop("%", Type.longType, Type.longType, Type.longType, lmod);
        enterBinop("%", Type.floatType, Type.floatType, Type.floatType, fmod);
        enterBinop("%", Type.doubleType, Type.doubleType, Type.doubleType, dmod);
        enterBinop("&", Type.booleanType, Type.booleanType, Type.booleanType, iand);
        enterBinop("&", Type.intType, Type.intType, Type.intType, iand);
        enterBinop("&", Type.longType, Type.longType, Type.longType, land);
        enterBinop("|", Type.booleanType, Type.booleanType, Type.booleanType, ior);
        enterBinop("|", Type.intType, Type.intType, Type.intType, ior);
        enterBinop("|", Type.longType, Type.longType, Type.longType, lor);
        enterBinop("^", Type.booleanType, Type.booleanType, Type.booleanType, ixor);
        enterBinop("^", Type.intType, Type.intType, Type.intType, ixor);
        enterBinop("^", Type.longType, Type.longType, Type.longType, lxor);
        enterBinop("<<", Type.intType, Type.intType, Type.intType, ishl);
        enterBinop("<<", Type.longType, Type.intType, Type.longType, lshl);
        enterBinop("<<", Type.intType, Type.longType, Type.intType, ishll);
        enterBinop("<<", Type.longType, Type.longType, Type.longType, lshll);
        enterBinop(">>", Type.intType, Type.intType, Type.intType, ishr);
        enterBinop(">>", Type.longType, Type.intType, Type.longType, lshr);
        enterBinop(">>", Type.intType, Type.longType, Type.intType, ishrl);
        enterBinop(">>", Type.longType, Type.longType, Type.longType, lshrl);
        enterBinop(">>>", Type.intType, Type.intType, Type.intType, iushr);
        enterBinop(">>>", Type.longType, Type.intType, Type.longType, lushr);
        enterBinop(">>>", Type.intType, Type.longType, Type.intType, iushrl);
        enterBinop(">>>", Type.longType, Type.longType, Type.longType, lushrl);
        enterBinop("<", Type.intType, Type.intType, Type.booleanType, if_icmplt)
        ;
        enterBinop("<", Type.longType, Type.longType, Type.booleanType, lcmp, iflt);
        enterBinop("<", Type.floatType, Type.floatType, Type.booleanType, fcmpg,
                iflt);
        enterBinop("<", Type.doubleType, Type.doubleType, Type.booleanType,
                dcmpg, iflt);
        enterBinop(">", Type.intType, Type.intType, Type.booleanType, if_icmpgt)
        ;
        enterBinop(">", Type.longType, Type.longType, Type.booleanType, lcmp, ifgt);
        enterBinop(">", Type.floatType, Type.floatType, Type.booleanType, fcmpl,
                ifgt);
        enterBinop(">", Type.doubleType, Type.doubleType, Type.booleanType,
                dcmpl, ifgt);
        enterBinop("<=", Type.intType, Type.intType, Type.booleanType, if_icmple)
        ;
        enterBinop("<=", Type.longType, Type.longType, Type.booleanType, lcmp, ifle);
        enterBinop("<=", Type.floatType, Type.floatType, Type.booleanType, fcmpg,
                ifle);
        enterBinop("<=", Type.doubleType, Type.doubleType, Type.booleanType,
                dcmpg, ifle);
        enterBinop(">=", Type.intType, Type.intType, Type.booleanType, if_icmpge)
        ;
        enterBinop(">=", Type.longType, Type.longType, Type.booleanType, lcmp, ifge);
        enterBinop(">=", Type.floatType, Type.floatType, Type.booleanType, fcmpl,
                ifge);
        enterBinop(">=", Type.doubleType, Type.doubleType, Type.booleanType,
                dcmpl, ifge);
        enterBinop("==", Type.intType, Type.intType, Type.booleanType, if_icmpeq)
        ;
        enterBinop("==", Type.longType, Type.longType, Type.booleanType, lcmp, ifeq);
        enterBinop("==", Type.floatType, Type.floatType, Type.booleanType, fcmpl,
                ifeq);
        enterBinop("==", Type.doubleType, Type.doubleType, Type.booleanType,
                dcmpl, ifeq);
        enterBinop("==", Type.booleanType, Type.booleanType, Type.booleanType,
                if_icmpeq)
        ;
        enterBinop("==", objectType, objectType, Type.booleanType, if_acmpeq)
        ;
        enterBinop("!=", Type.intType, Type.intType, Type.booleanType, if_icmpne)
        ;
        enterBinop("!=", Type.longType, Type.longType, Type.booleanType, lcmp, ifne);
        enterBinop("!=", Type.floatType, Type.floatType, Type.booleanType, fcmpl,
                ifne);
        enterBinop("!=", Type.doubleType, Type.doubleType, Type.booleanType,
                dcmpl, ifne);
        enterBinop("!=", Type.booleanType, Type.booleanType, Type.booleanType,
                if_icmpne)
        ;
        enterBinop("!=", objectType, objectType, Type.booleanType, if_acmpne)
        ;
        enterBinop("&&", Type.booleanType, Type.booleanType, Type.booleanType,
                bool_and);
        enterBinop("||", Type.booleanType, Type.booleanType, Type.booleanType,
                bool_or);
    }
}
