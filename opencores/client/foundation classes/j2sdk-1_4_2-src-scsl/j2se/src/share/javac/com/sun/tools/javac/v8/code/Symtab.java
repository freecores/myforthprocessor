/**
 * @(#)Symtab.java	1.27 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8.code;
import com.sun.tools.javac.v8.util.*;

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
     * The context key for the symbol table.
     */
    private static final Context.Key symtabKey = new Context.Key();

    /**
     * Get the symbol table instance.
     */
    public static Symtab instance(Context context) {
        Symtab instance = (Symtab) context.get(symtabKey);
        if (instance == null)
            instance = new Symtab(context);
        return instance;
    }
    private final Name.Table names;
    private final ClassReader reader;

    /**
     * A symbol for the root package.
     */
    public final PackageSymbol rootPackage;

    /**
     * A symbol for the empty package.
     */
    public final PackageSymbol emptyPackage;

    /**
     * A symbol that stands for a missing symbol.
     */
    public final TypeSymbol noSymbol;

    /**
     * The error symbol.
     */
    public final ClassSymbol errSymbol;

    /**
     * Builtin types.
     */
    public final Type byteType;
    public final Type charType;
    public final Type shortType;
    public final Type intType;
    public final Type longType;
    public final Type floatType;
    public final Type doubleType;
    public final Type booleanType;
    public final Type voidType;
    public final Type botType;
    public final Type errType;

    /**
     * A value for the unknown type.
     */
    public final Type unknownType;

    /**
     * The builtin type of all arrays.
     */
    public final ClassSymbol arrayClass;

    /**
     * The builtin type of all methods.
     */
    public final ClassSymbol methodClass;

    /**
     * Predefined types.
     */
    public final Type objectType;
    public final Type classType;
    public final Type classLoaderType;
    public final Type stringType;
    public final Type stringBufferType;
    public final Type cloneableType;
    public final Type serializableType;
    public final Type throwableType;
    public final Type errorType;
    public final Type exceptionType;
    public final Type runtimeExceptionType;
    public final Type classNotFoundExceptionType;
    public final Type noClassDefFoundErrorType;
    public final Type assertionErrorType;

    /**
     * The symbol representing the length field of an array.
     */
    public final VarSymbol lengthVar;

    /**
     * Predefined constants.
     */
    public final VarSymbol nullConst;
    public final VarSymbol trueConst;
    public final VarSymbol falseConst;

    /**
     * The null check operator.
     */
    public final OperatorSymbol nullcheck;

    /**
     * The predefined type that belongs to a tag.
     */
    public final Type[] typeOfTag = new Type[TypeTags.TypeTagCount];

    /**
     * The name of the class that belongs to a basix type tag.
     */
    public final Name[] boxedName = new Name[TypeTags.TypeTagCount];

    /**
     * A hashtable containing the encountered top-level and member classes,
     *  indexed by flat names. The table does not contain local classes.
     *  It should be updated from the outside to reflect classes defined
     *  by compiled source files.
     */
    public final Hashtable classes = Hashtable.make();

    /**
     * A hashtable containing the encountered packages.
     *  the table should be updated from outside to reflect packages defined
     *  by compiled source files.
     */
    public final Hashtable packages = Hashtable.make();

    public void initType(Type type, ClassSymbol c) {
        type.tsym = c;
        typeOfTag[type.tag] = type;
    }

    public void initType(Type type, String name) {
        initType(type,
                new ClassSymbol(PUBLIC, names.fromString(name), type, rootPackage));
    }

    public void initType(Type type, String name, String bname) {
        initType(type, name);
        boxedName[type.tag] = names.fromString("java.lang." + bname);
    }

    /**
      * The class symbol that owns all predefined symbols.
      */
    public final ClassSymbol predefClass;

    /**
     * Enter a constant into symbol table.
     *  @param name   The constant's name.
     *  @param type   The constant's type.
     */
    private VarSymbol enterConstant(String name, Type type) {
        VarSymbol c = new VarSymbol(PUBLIC | STATIC | FINAL, names.fromString(name),
                type, predefClass);
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
        predefClass.members().enter( new OperatorSymbol(names.fromString(name),
                new MethodType(List.make(left, right), res, Type.emptyList,
                methodClass), opcode, predefClass));
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
    private OperatorSymbol enterUnop(String name, Type arg, Type res, int opcode) {
        OperatorSymbol sym = new OperatorSymbol(names.fromString(name),
                new MethodType(List.make(arg), res, Type.emptyList,
                methodClass), opcode, predefClass);
        predefClass.members().enter(sym);
        return sym;
    }

    /**
      * Enter a class into symbol table.
      *  @param    The name of the class.
      */
    private Type enterClass(String s) {
        return reader.enterClass(names.fromString(s)).type;
    }

    /**
      * Constructor; enters all predefined identifiers and operators
      *  into symbol table.
      */
    private Symtab(Context context) throws CompletionFailure {
        super();
        context.put(symtabKey, this);
        names = Name.Table.instance(context);
        byteType = new Type(TypeTags.BYTE, null);
        charType = new Type(TypeTags.CHAR, null);
        shortType = new Type(TypeTags.SHORT, null);
        intType = new Type(TypeTags.INT, null);
        longType = new Type(TypeTags.LONG, null);
        floatType = new Type(TypeTags.FLOAT, null);
        doubleType = new Type(TypeTags.DOUBLE, null);
        booleanType = new Type(TypeTags.BOOLEAN, null);
        voidType = new Type(TypeTags.VOID, null);
        botType = new Type(TypeTags.BOT, null);
        unknownType = new Type(TypeTags.UNKNOWN, null) {


                          public boolean isSameType(Type that) {
                              return true;
                          }

                          public boolean isSubType(Type that) {
                              return false;
                          }

                          public boolean isSuperType(Type that) {
                              return true;
                          }
                      };
        rootPackage = new PackageSymbol(names.empty, null);
        emptyPackage = new PackageSymbol(names.emptyPackage, rootPackage);
        noSymbol = new TypeSymbol(0, names.empty, Type.noType, rootPackage);
        noSymbol.kind = Kinds.NIL;
        errSymbol = new ClassSymbol(PUBLIC | STATIC, names.any, null, rootPackage);
        errType = new ErrorType(errSymbol);
        initType(byteType, "byte", "Byte");
        initType(shortType, "short", "Short");
        initType(charType, "char", "Character");
        initType(intType, "int", "Integer");
        initType(longType, "long", "Long");
        initType(floatType, "float", "Float");
        initType(doubleType, "double", "Double");
        initType(booleanType, "boolean", "Boolean");
        initType(voidType, "void", "Void");
        initType(botType, "<nulltype>");
        initType(errType, errSymbol);
        initType(unknownType, "<any?>");
        arrayClass = new ClassSymbol(PUBLIC, names.Array, noSymbol);
        methodClass = new ClassSymbol(PUBLIC, names.Method, noSymbol);
        predefClass = new ClassSymbol(PUBLIC, names.empty, rootPackage);
        Scope scope = new Scope(predefClass);
        predefClass.members_field = scope;
        scope.enter(byteType.tsym);
        scope.enter(shortType.tsym);
        scope.enter(charType.tsym);
        scope.enter(intType.tsym);
        scope.enter(longType.tsym);
        scope.enter(floatType.tsym);
        scope.enter(doubleType.tsym);
        scope.enter(booleanType.tsym);
        scope.enter(errType.tsym);
        classes.put(predefClass.fullname, predefClass);
        reader = ClassReader.instance(context);
        reader.init(this);
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
        ClassType arrayClassType = (ClassType) arrayClass.type;
        arrayClassType.supertype_field = objectType;
        arrayClassType.interfaces_field = List.make(cloneableType, serializableType);
        arrayClass.members_field = new Scope(arrayClass);
        lengthVar = new VarSymbol(PUBLIC | FINAL, names.length, intType, arrayClass);
        arrayClass.members().enter(lengthVar);
        Symbol cloneMethod = new MethodSymbol(PUBLIC, names.clone,
                new MethodType(Type.emptyList, objectType, Type.emptyList,
                methodClass), arrayClass);
        arrayClass.members().enter(cloneMethod);
        nullConst = enterConstant("null", botType);
        trueConst = enterConstant("true", booleanType.constType(new Integer(1)));
        falseConst = enterConstant("false", booleanType.constType(new Integer(0)));
        enterUnop("+", intType, intType, nop);
        enterUnop("+", longType, longType, nop);
        enterUnop("+", floatType, floatType, nop);
        enterUnop("+", doubleType, doubleType, nop);
        enterUnop("-", intType, intType, ineg);
        enterUnop("-", longType, longType, lneg);
        enterUnop("-", floatType, floatType, fneg);
        enterUnop("-", doubleType, doubleType, dneg);
        enterUnop("~", intType, intType, ixor);
        enterUnop("~", longType, longType, lxor);
        enterUnop("++", byteType, byteType, iadd);
        enterUnop("++", shortType, shortType, iadd);
        enterUnop("++", charType, charType, iadd);
        enterUnop("++", intType, intType, iadd);
        enterUnop("++", longType, longType, ladd);
        enterUnop("++", floatType, floatType, fadd);
        enterUnop("++", doubleType, doubleType, dadd);
        enterUnop("--", byteType, byteType, isub);
        enterUnop("--", shortType, shortType, isub);
        enterUnop("--", charType, charType, isub);
        enterUnop("--", intType, intType, isub);
        enterUnop("--", longType, longType, lsub);
        enterUnop("--", floatType, floatType, fsub);
        enterUnop("--", doubleType, doubleType, dsub);
        enterUnop("!", booleanType, booleanType, bool_not);
        nullcheck = enterUnop("<*nullchk*>", objectType, objectType, nullchk);
        enterBinop("+", stringType, stringType, stringType, string_add);
        enterBinop("+", stringType, intType, stringType, string_add);
        enterBinop("+", stringType, longType, stringType, string_add);
        enterBinop("+", stringType, floatType, stringType, string_add);
        enterBinop("+", stringType, doubleType, stringType, string_add);
        enterBinop("+", stringType, booleanType, stringType, string_add);
        enterBinop("+", stringType, objectType, stringType, string_add);
        enterBinop("+", stringType, botType, stringType, string_add);
        enterBinop("+", intType, stringType, stringType, string_add);
        enterBinop("+", longType, stringType, stringType, string_add);
        enterBinop("+", floatType, stringType, stringType, string_add);
        enterBinop("+", doubleType, stringType, stringType, string_add);
        enterBinop("+", booleanType, stringType, stringType, string_add);
        enterBinop("+", objectType, stringType, stringType, string_add);
        enterBinop("+", botType, stringType, stringType, string_add);
        enterBinop("+", intType, intType, intType, iadd);
        enterBinop("+", longType, longType, longType, ladd);
        enterBinop("+", floatType, floatType, floatType, fadd);
        enterBinop("+", doubleType, doubleType, doubleType, dadd);
        enterBinop("+", botType, botType, botType, error);
        enterBinop("+", botType, intType, botType, error);
        enterBinop("+", botType, longType, botType, error);
        enterBinop("+", botType, floatType, botType, error);
        enterBinop("+", botType, doubleType, botType, error);
        enterBinop("+", botType, booleanType, botType, error);
        enterBinop("+", botType, objectType, botType, error);
        enterBinop("+", intType, botType, botType, error);
        enterBinop("+", longType, botType, botType, error);
        enterBinop("+", floatType, botType, botType, error);
        enterBinop("+", doubleType, botType, botType, error);
        enterBinop("+", booleanType, botType, botType, error);
        enterBinop("+", objectType, botType, botType, error);
        enterBinop("-", intType, intType, intType, isub);
        enterBinop("-", longType, longType, longType, lsub);
        enterBinop("-", floatType, floatType, floatType, fsub);
        enterBinop("-", doubleType, doubleType, doubleType, dsub);
        enterBinop("*", intType, intType, intType, imul);
        enterBinop("*", longType, longType, longType, lmul);
        enterBinop("*", floatType, floatType, floatType, fmul);
        enterBinop("*", doubleType, doubleType, doubleType, dmul);
        enterBinop("/", intType, intType, intType, idiv);
        enterBinop("/", longType, longType, longType, ldiv);
        enterBinop("/", floatType, floatType, floatType, fdiv);
        enterBinop("/", doubleType, doubleType, doubleType, ddiv);
        enterBinop("%", intType, intType, intType, imod);
        enterBinop("%", longType, longType, longType, lmod);
        enterBinop("%", floatType, floatType, floatType, fmod);
        enterBinop("%", doubleType, doubleType, doubleType, dmod);
        enterBinop("&", booleanType, booleanType, booleanType, iand);
        enterBinop("&", intType, intType, intType, iand);
        enterBinop("&", longType, longType, longType, land);
        enterBinop("|", booleanType, booleanType, booleanType, ior);
        enterBinop("|", intType, intType, intType, ior);
        enterBinop("|", longType, longType, longType, lor);
        enterBinop("^", booleanType, booleanType, booleanType, ixor);
        enterBinop("^", intType, intType, intType, ixor);
        enterBinop("^", longType, longType, longType, lxor);
        enterBinop("<<", intType, intType, intType, ishl);
        enterBinop("<<", longType, intType, longType, lshl);
        enterBinop("<<", intType, longType, intType, ishll);
        enterBinop("<<", longType, longType, longType, lshll);
        enterBinop(">>", intType, intType, intType, ishr);
        enterBinop(">>", longType, intType, longType, lshr);
        enterBinop(">>", intType, longType, intType, ishrl);
        enterBinop(">>", longType, longType, longType, lshrl);
        enterBinop(">>>", intType, intType, intType, iushr);
        enterBinop(">>>", longType, intType, longType, lushr);
        enterBinop(">>>", intType, longType, intType, iushrl);
        enterBinop(">>>", longType, longType, longType, lushrl);
        enterBinop("<", intType, intType, booleanType, if_icmplt)
        ;
        enterBinop("<", longType, longType, booleanType, lcmp, iflt);
        enterBinop("<", floatType, floatType, booleanType, fcmpg, iflt);
        enterBinop("<", doubleType, doubleType, booleanType, dcmpg, iflt);
        enterBinop(">", intType, intType, booleanType, if_icmpgt)
        ;
        enterBinop(">", longType, longType, booleanType, lcmp, ifgt);
        enterBinop(">", floatType, floatType, booleanType, fcmpl, ifgt);
        enterBinop(">", doubleType, doubleType, booleanType, dcmpl, ifgt);
        enterBinop("<=", intType, intType, booleanType, if_icmple)
        ;
        enterBinop("<=", longType, longType, booleanType, lcmp, ifle);
        enterBinop("<=", floatType, floatType, booleanType, fcmpg, ifle);
        enterBinop("<=", doubleType, doubleType, booleanType, dcmpg, ifle);
        enterBinop(">=", intType, intType, booleanType, if_icmpge)
        ;
        enterBinop(">=", longType, longType, booleanType, lcmp, ifge);
        enterBinop(">=", floatType, floatType, booleanType, fcmpl, ifge);
        enterBinop(">=", doubleType, doubleType, booleanType, dcmpl, ifge);
        enterBinop("==", intType, intType, booleanType, if_icmpeq)
        ;
        enterBinop("==", longType, longType, booleanType, lcmp, ifeq);
        enterBinop("==", floatType, floatType, booleanType, fcmpl, ifeq);
        enterBinop("==", doubleType, doubleType, booleanType, dcmpl, ifeq);
        enterBinop("==", booleanType, booleanType, booleanType, if_icmpeq)
        ;
        enterBinop("==", objectType, objectType, booleanType, if_acmpeq)
        ;
        enterBinop("!=", intType, intType, booleanType, if_icmpne)
        ;
        enterBinop("!=", longType, longType, booleanType, lcmp, ifne);
        enterBinop("!=", floatType, floatType, booleanType, fcmpl, ifne);
        enterBinop("!=", doubleType, doubleType, booleanType, dcmpl, ifne);
        enterBinop("!=", booleanType, booleanType, booleanType, if_icmpne)
        ;
        enterBinop("!=", objectType, objectType, booleanType, if_acmpne)
        ;
        enterBinop("&&", booleanType, booleanType, booleanType, bool_and);
        enterBinop("||", booleanType, booleanType, booleanType, bool_or);
    }
}
