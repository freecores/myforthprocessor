/**
 * @(#)ClassWriter.java	1.48 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8.code;
import com.sun.tools.javac.v8.util.*;

import java.io.*;

import com.sun.tools.javac.v8.code.Symbol.*;

import com.sun.tools.javac.v8.code.Type.*;


/**
 * This class provides operations to map an internal symbol table graph
 *  rooted in a ClassSymbol into a classfile.
 */
public class ClassWriter extends ClassFile implements Flags, Kinds, TypeTags {
    private static final Context.Key classWriterKey = new Context.Key();
    private final Symtab syms;

    /**
     * Can be reassigned from outside: the output directory.
     *  If outDir = null, files are written into same directory as the sources
     *  the were generated from.
     */
    public File outDir = null;

    /**
     * Switch: verbose output.
     */
    private boolean verbose;

    /**
     * Switch: scrable private names.
     */
    private boolean scramble;

    /**
     * Switch: scrable private names.
     */
    private boolean scrambleAll;

    /**
     * Switch: retrofit mode.
     */
    private boolean retrofit;

    /**
     * Switch: emit source file attribute.
     */
    private boolean emitSourceFile;

    /**
     * Switch: generate CharacterRangeTable attribute.
     */
    private boolean genCrt;

    /**
     * Target class version.
     */
    private Target target;

    /**
     * The initial sizes of the data and constant pool buffers.
     *  sizes are increased when buffers get full.
     */
    static final int DATA_BUF_SIZE = 65520;
    static final int POOL_BUF_SIZE = 131056;

    /**
     * An output buffer for member info.
     */
    ByteBuffer databuf = new ByteBuffer(DATA_BUF_SIZE);

    /**
     * An output buffer for the constant pool.
     */
    ByteBuffer poolbuf = new ByteBuffer(POOL_BUF_SIZE);

    /**
     * An output buffer for type signatures.
     */
    ByteBuffer sigbuf = new ByteBuffer();

    /**
     * The constant pool.
     */
    Pool pool;

    /**
     * The inner classes to be written, as a set.
     */
    Set innerClasses;

    /**
     * The inner classes to be written, as a queue where
     *  enclosing classes come first.
     */
    ListBuffer innerClassesQueue;

    /**
     * The log to use for verbose output.
     */
    private final Log log;

    /**
     * The name table.
     */
    private final Name.Table names;

    /**
     * Get the ClassWriter instance for this context.
     */
    public static ClassWriter instance(Context context) {
        ClassWriter instance = (ClassWriter) context.get(classWriterKey);
        if (instance == null)
            instance = new ClassWriter(context);
        return instance;
    }

    /**
      * Construct a class writer, given an options table.
      */
    private ClassWriter(Context context) {
        super();
        context.put(classWriterKey, this);
        log = Log.instance(context);
        names = Name.Table.instance(context);
        syms = Symtab.instance(context);
        Options options = Options.instance(context);
        target = Target.instance(context);
        verbose = options.get("-verbose") != null;
        scramble = options.get("-scramble") != null;
        scrambleAll = options.get("-scrambleAll") != null;
        retrofit = options.get("-retrofit") != null;
        genCrt = options.get("-Xjcov") != null;
        emitSourceFile =
                options.get("-g:") == null || options.get("-g:source") != null;
        String od = (String) options.get("-d");
        if (od != null)
            outDir = new File(od);
        String dumpModFlags = (String) options.get("dumpmodifiers");
        dumpClassModifiers =
                (dumpModFlags != null && dumpModFlags.indexOf('c') != -1);
        dumpFieldModifiers =
                (dumpModFlags != null && dumpModFlags.indexOf('f') != -1);
        dumpInnerClassModifiers =
                (dumpModFlags != null && dumpModFlags.indexOf('i') != -1);
        dumpMethodModifiers =
                (dumpModFlags != null && dumpModFlags.indexOf('m') != -1);
    }

    /**
      * Value of option 'dumpmodifiers' is a string
      *  indicating which modifiers should be dumped for debugging:
      *    'c' -- classes
      *    'f' -- fields
      *    'i' -- innerclass attributes
      *    'm' -- methods
      *  For example, to dump everything:
      *    javac -XDdumpmodifiers=cifm MyProg.java
      */
    private final boolean dumpClassModifiers;
    private final boolean dumpFieldModifiers;
    private final boolean dumpInnerClassModifiers;
    private final boolean dumpMethodModifiers;

    /**
     * Return flags as a string, separated by " ".
     */
    public static String flagNames(long flags) {
        StringBuffer sbuf = new StringBuffer();
        int i = 0;
        long f = flags & StandardFlags;
        while (f != 0) {
            if ((f & 1) != 0)
                sbuf.append(" " + flagName[i]);
            f = f >> 1;
            i++;
        }
        return sbuf.toString();
    }
    private static final String[] flagName = {"PUBLIC", "PRIVATE", "PROTECTED", "STATIC",
    "FINAL", "SUPER", "VOLATILE", "TRANSIENT", "NATIVE", "INTERFACE", "ABSTRACT",
    "STRICTFP"};

    /**
     * Write a character into given byte buffer;
     *  byte buffer will not be grown.
     */
    void putChar(ByteBuffer buf, int op, int x) {
        buf.elems[op] = (byte)((x >> 8) & 255);
        buf.elems[op + 1] = (byte)((x) & 255);
    }

    /**
      * Write an integer into given byte buffer;
      *  byte buffer will not be grown.
      */
    void putInt(ByteBuffer buf, int adr, int x) {
        buf.elems[adr] = (byte)((x >> 24) & 255);
        buf.elems[adr + 1] = (byte)((x >> 16) & 255);
        buf.elems[adr + 2] = (byte)((x >> 8) & 255);
        buf.elems[adr + 3] = (byte)((x) & 255);
    }

    /**
      * Assemble signature of given type in string buffer.
      */
    void assembleSig(Type type) {
        switch (type.tag) {
        case BYTE:
            sigbuf.appendByte('B');
            break;

        case SHORT:
            sigbuf.appendByte('S');
            break;

        case CHAR:
            sigbuf.appendByte('C');
            break;

        case INT:
            sigbuf.appendByte('I');
            break;

        case LONG:
            sigbuf.appendByte('J');
            break;

        case FLOAT:
            sigbuf.appendByte('F');
            break;

        case DOUBLE:
            sigbuf.appendByte('D');
            break;

        case BOOLEAN:
            sigbuf.appendByte('Z');
            break;

        case VOID:
            sigbuf.appendByte('V');
            break;

        case CLASS:
            ClassType ct = (ClassType) type;
            ClassSymbol c = (ClassSymbol) ct.tsym;
            enterInner(c);
            if (ct.outer().allparams().nonEmpty()) {
                assembleSig(ct.outer());
                sigbuf.appendByte('.');
            }
            sigbuf.appendByte('L');
            sigbuf.appendBytes(externalize(c.flatname));
            sigbuf.appendByte(';');
            break;

        case ARRAY:
            ArrayType at = (ArrayType) type;
            sigbuf.appendByte('[');
            assembleSig(at.elemtype);
            break;

        case METHOD:
            MethodType mt = (MethodType) type;
            sigbuf.appendByte('(');
            assembleSig(mt.argtypes);
            sigbuf.appendByte(')');
            assembleSig(mt.restype);
            break;

        default:
            assert false :
            "typeSig" + type.tag;

        }
    }

    void assembleSig(List types) {
        for (List ts = types; ts.nonEmpty(); ts = ts.tail)
            assembleSig((Type) ts.head);
    }

    /**
      * Return signature of given type
      */
    Name typeSig(Type type) {
        assert sigbuf.length == 0;
        assembleSig(type);
        Name n = sigbuf.toName(names);
        sigbuf.reset();
        return n;
    }

    /**
      * Given a type t, return the extended class name of its erasure in
      *  external representation.
      */
    public Name xClassName(Type t) {
        if (t.tag == CLASS) {
            return names.fromUtf(externalize(t.tsym.flatName()));
        } else if (t.tag == ARRAY) {
            return typeSig(t.erasure());
        } else {
            throw new AssertionError("xClassName");
        }
    }

    /**
      * Thrown when the constant pool is over full.
      */
    public static class PoolOverflow extends Exception {

        public PoolOverflow() {
            super();
        }
    }

    public static class StringOverflow extends Exception {
        public final String value;

        public StringOverflow(String s) {
            super();
            value = s;
        }
    }

    /**
      * Write constant pool to pool buffer.
      *  Note: during writing, constant pool
      *  might grow since some parts of constants still need to be entered.
      */
    void writePool(Pool pool) throws PoolOverflow, StringOverflow {
        int poolCountIdx = poolbuf.length;
        poolbuf.appendChar(0);
        int i = 1;
        while (i < pool.pp) {
            Object value = pool.pool[i];
            assert value != null;
            if (value instanceof Pool.Method)
                value = ((Pool.Method) value).m;
            else if (value instanceof Pool.Variable)
                value = ((Pool.Variable) value).v;
            if (value instanceof MethodSymbol) {
                MethodSymbol m = (MethodSymbol) value;
                poolbuf.appendByte((m.owner.flags() & INTERFACE) != 0 ?
                        CONSTANT_InterfaceMethodref : CONSTANT_Methodref);
                poolbuf.appendChar(pool.put(m.owner));
                poolbuf.appendChar(pool.put(nameType(m)));
            } else if (value instanceof VarSymbol) {
                VarSymbol v = (VarSymbol) value;
                poolbuf.appendByte(CONSTANT_Fieldref);
                poolbuf.appendChar(pool.put(v.owner));
                poolbuf.appendChar(pool.put(nameType(v)));
            } else if (value instanceof Name) {
                poolbuf.appendByte(CONSTANT_Utf8);
                byte[] bs = ((Name) value).toUtf();
                poolbuf.appendChar(bs.length);
                poolbuf.appendBytes(bs, 0, bs.length);
                if (bs.length > Pool.MAX_STRING_LENGTH)
                    throw new StringOverflow(value.toString());
            } else if (value instanceof ClassSymbol) {
                ClassSymbol c = (ClassSymbol) value;
                if (c.owner.kind == TYP)
                    pool.put(c.owner);
                poolbuf.appendByte(CONSTANT_Class);
                if (c.type.tag == ARRAY) {
                    poolbuf.appendChar(pool.put(typeSig(c.type)));
                } else {
                    poolbuf.appendChar(
                            pool.put(names.fromUtf(externalize(c.flatname))));
                    enterInner(c);
                }
            } else if (value instanceof NameAndType) {
                NameAndType nt = (NameAndType) value;
                poolbuf.appendByte(CONSTANT_NameandType);
                poolbuf.appendChar(pool.put(nt.name));
                poolbuf.appendChar(pool.put(typeSig(nt.type)));
            } else if (value instanceof Integer) {
                poolbuf.appendByte(CONSTANT_Integer);
                poolbuf.appendInt(((Integer) value).intValue());
            } else if (value instanceof Long) {
                poolbuf.appendByte(CONSTANT_Long);
                poolbuf.appendLong(((Long) value).longValue());
                i++;
            } else if (value instanceof Float) {
                poolbuf.appendByte(CONSTANT_Float);
                poolbuf.appendFloat(((Float) value).floatValue());
            } else if (value instanceof Double) {
                poolbuf.appendByte(CONSTANT_Double);
                poolbuf.appendDouble(((Double) value).doubleValue());
                i++;
            } else if (value instanceof String) {
                poolbuf.appendByte(CONSTANT_String);
                poolbuf.appendChar(pool.put(names.fromString((String) value)));
            } else if (value instanceof Type) {
                poolbuf.appendByte(CONSTANT_Class);
                poolbuf.appendChar(pool.put(xClassName((Type) value)));
            } else {
                assert false :
                "writePool " + value;
            }
            i++;
        }
        if (pool.pp > Pool.MAX_ENTRIES)
            throw new PoolOverflow();
        putChar(poolbuf, poolCountIdx, pool.pp);
    }

    /**
      * Given a field, return its name.
      */
    Name fieldName(Symbol sym) {
        if (scramble && (sym.flags() & PRIVATE) != 0 || scrambleAll &&
                (sym.flags() & (PROTECTED | PUBLIC)) == 0)
            return names.fromString("_$" + sym.name.index);
        else
            return sym.name;
    }

    /**
      * Given a symbol, return its name-and-type.
      */
    NameAndType nameType(Symbol sym) {
        return new NameAndType(fieldName(sym),
                retrofit ? sym.erasure() : sym.externalType());
    }

    /**
      * Write header for an attribute to data buffer and return
      *  position past attribute length index.
      */
    int writeAttr(Name attrName) {
        databuf.appendChar(pool.put(attrName));
        databuf.appendInt(0);
        return databuf.length;
    }

    /**
      * Fill in attribute length.
      */
    void endAttr(int index) {
        putInt(databuf, index - 4, databuf.length - index);
    }

    /**
      * Leave space for attribute count and return index for
      *  number of attributes field.
      */
    int beginAttrs() {
        databuf.appendChar(0);
        return databuf.length;
    }

    /**
      * Fill in number of attributes.
      */
    void endAttrs(int index, int count) {
        putChar(databuf, index - 2, count);
    }

    /**
      * Write flag attributes; return number of attributes written.
      */
    int writeFlagAttrs(long flags) {
        int acount = 0;
        if ((flags & DEPRECATED) != 0) {
            int alenIdx = writeAttr(names.Deprecated);
            endAttr(alenIdx);
            acount++;
        }
        if ((flags & SYNTHETIC) != 0) {
            int alenIdx = writeAttr(names.Synthetic);
            endAttr(alenIdx);
            acount++;
        }
        return acount;
    }

    /**
      * Write member (field or method) attributes;
      *  return number of attributes written.
      */
    int writeMemberAttrs(Symbol sym) {
        int acount = writeFlagAttrs(sym.flags());
        return acount;
    }

    /**
      * Enter an inner class into the `innerClasses' set/queue.
      */
    void enterInner(ClassSymbol c) {
        assert (c.flags() & COMPOUND) == 0;
        if (c.type.tag != CLASS)
            return;
        if (pool != null && c.owner.kind != PCK &&
                (innerClasses == null || !innerClasses.contains(c))) {
            if (c.owner.kind == TYP)
                enterInner((ClassSymbol) c.owner);
            pool.put(c);
            pool.put(c.name);
            if (innerClasses == null) {
                innerClasses = Set.make();
                innerClassesQueue = new ListBuffer();
                pool.put(names.InnerClasses);
            }
            innerClasses.put(c);
            innerClassesQueue.append(c);
        }
    }

    /**
      * Write "inner classes" attribute.
      */
    void writeInnerClasses() {
        int alenIdx = writeAttr(names.InnerClasses);
        databuf.appendChar(innerClassesQueue.length());
        for (List l = innerClassesQueue.toList(); l.nonEmpty(); l = l.tail) {
            ClassSymbol inner = (ClassSymbol) l.head;
            if (dumpInnerClassModifiers) {
                log.errWriter.println("INNERCLASS  " + inner.name);
                log.errWriter.println("---" + flagNames(inner.flags_field));
            }
            databuf.appendChar(pool.get(inner));
            databuf.appendChar(inner.owner.kind == TYP ? pool.get(inner.owner) : 0);
            databuf.appendChar(inner.name.len != 0 ? pool.get(inner.name) : 0);
            databuf.appendChar((int) inner.flags_field);
        }
        endAttr(alenIdx);
    }

    /**
      * Write field symbol, entering all references into constant pool.
      */
    void writeField(VarSymbol v) {
        databuf.appendChar((int) v.flags());
        if (dumpFieldModifiers) {
            log.errWriter.println("FIELD  " + fieldName(v));
            log.errWriter.println("---" + flagNames(v.flags()));
        }
        databuf.appendChar(pool.put(fieldName(v)));
        databuf.appendChar(pool.put(typeSig(v.erasure())));
        int acountIdx = beginAttrs();
        int acount = 0;
        if (v.constValue != null) {
            int alenIdx = writeAttr(names.ConstantValue);
            databuf.appendChar(pool.put(v.constValue));
            endAttr(alenIdx);
            acount++;
        }
        acount += writeMemberAttrs(v);
        endAttrs(acountIdx, acount);
    }

    /**
      * Write method symbol, entering all references into constant pool.
      */
    void writeMethod(MethodSymbol m) {
        databuf.appendChar((int) m.flags());
        if (dumpMethodModifiers) {
            log.errWriter.println("METHOD  " + fieldName(m));
            log.errWriter.println("---" + flagNames(m.flags()));
        }
        databuf.appendChar(pool.put(fieldName(m)));
        databuf.appendChar(pool.put(typeSig(m.externalType())));
        int acountIdx = beginAttrs();
        int acount = 0;
        if (m.code != null) {
            int alenIdx = writeAttr(names.Code);
            writeCode(m.code);
            m.code = null;
            endAttr(alenIdx);
            acount++;
        }
        List thrown = m.erasure().thrown();
        if (thrown.nonEmpty()) {
            int alenIdx = writeAttr(names.Exceptions);
            databuf.appendChar(thrown.length());
            for (List l = thrown; l.nonEmpty(); l = l.tail)
                databuf.appendChar(pool.put(((Type) l.head).tsym));
            endAttr(alenIdx);
            acount++;
        }
        acount += writeMemberAttrs(m);
        endAttrs(acountIdx, acount);
    }

    /**
      * Write code attribute of method.
      */
    void writeCode(Code code) {
        databuf.appendChar(code.max_stack);
        databuf.appendChar(code.max_locals);
        databuf.appendInt(code.cp);
        databuf.appendBytes(code.code, 0, code.cp);
        databuf.appendChar(code.catchInfo.length());
        for (List l = code.catchInfo.toList(); l.nonEmpty(); l = l.tail) {
            for (int i = 0; i < ((char[]) l.head).length; i++)
                databuf.appendChar(((char[]) l.head)[i]);
        }
        int acountIdx = beginAttrs();
        int acount = 0;
        if (code.lineInfo.nonEmpty()) {
            int alenIdx = writeAttr(names.LineNumberTable);
            databuf.appendChar(code.lineInfo.length());
            for (List l = code.lineInfo.reverse(); l.nonEmpty(); l = l.tail)
                for (int i = 0; i < ((char[]) l.head).length; i++)
                    databuf.appendChar(((char[]) l.head)[i]);
            endAttr(alenIdx);
            acount++;
        }
        if (genCrt && (code.crt != null)) {
            CRTable crt = code.crt;
            int alenIdx = writeAttr(names.CharacterRangeTable);
            int crtIdx = beginAttrs();
            int crtEntries = crt.writeCRT(databuf);
            endAttrs(crtIdx, crtEntries);
            endAttr(alenIdx);
            acount++;
        }
        if (code.varBufferSize > 0) {
            int alenIdx = writeAttr(names.LocalVariableTable);
            int nvars = code.varBufferSize;
            databuf.appendChar(nvars);
            for (int i = 0; i < nvars; i++) {
                Code.LocalVar var = code.varBuffer[i];
                assert var.start_pc >= 0;
                assert var.start_pc <= code.cp;
                databuf.appendChar(var.start_pc);
                assert var.length >= 0;
                assert (var.start_pc + var.length) <= code.cp;
                databuf.appendChar(var.length);
                VarSymbol sym = var.var;
                databuf.appendChar(pool.put(sym.name));
                databuf.appendChar(pool.put(typeSig(sym.erasure())));
                databuf.appendChar(var.reg);
            }
            endAttr(alenIdx);
            acount++;
        }
        endAttrs(acountIdx, acount);
    }

    void writeFields(Scope.Entry e) {
        List vars = List.make();
        for (Scope.Entry i = e; i != null; i = i.sibling) {
            if (i.sym.kind == VAR)
                vars = vars.prepend((VarSymbol) i.sym);
        }
        while (vars.nonEmpty()) {
            writeField((VarSymbol) vars.head);
            vars = vars.tail;
        }
    }

    void writeMethods(Scope.Entry e) {
        List methods = List.make();
        for (Scope.Entry i = e; i != null; i = i.sibling) {
            if (i.sym.kind == MTH)
                methods = methods.prepend((MethodSymbol) i.sym);
        }
        while (methods.nonEmpty()) {
            writeMethod((MethodSymbol) methods.head);
            methods = methods.tail;
        }
    }

    /**
      * Emit a class file for a given class.
      *  @param c      The class from which a class file is generated.
      */
    public void writeClass(ClassSymbol c) throws IOException, PoolOverflow,
    StringOverflow {
        File outFile = outputFile(c, ".class");
        OutputStream out = new FileOutputStream(outFile);
        try {
            writeClassFile(out, c);
            if (verbose)
                log.errWriter.println( log.getLocalizedString("verbose.wrote.file",
                        outFile.getPath()));
            out.close();
            out = null;
        }
        finally { if (out != null) {
                      out.close();
                      outFile.delete();
                      out = null;
                  }
                } }

    /**
      * Write class `c' to outstream `out'.
      */
    public void writeClassFile(OutputStream out,
            ClassSymbol c) throws IOException, PoolOverflow, StringOverflow {
        assert (c.flags() & COMPOUND) == 0;
        databuf.reset();
        poolbuf.reset();
        sigbuf.reset();
        pool = c.pool;
        innerClasses = null;
        innerClassesQueue = null;
        Type supertype = c.type.supertype();
        List interfaces = c.type.interfaces();
        long flags = c.flags();
        if ((flags & PROTECTED) != 0)
            flags |= PUBLIC;
        flags = flags & ClassFlags & ~STRICTFP;
        if ((flags & INTERFACE) == 0)
            flags |= ACC_SUPER;
        if (dumpClassModifiers) {
            log.errWriter.println();
            log.errWriter.println("CLASSFILE  " + c.fullName());
            log.errWriter.println("---" + flagNames(flags));
        }
        databuf.appendChar((int) flags);
        databuf.appendChar(pool.put(c));
        databuf.appendChar(supertype.tag == CLASS ? pool.put(supertype.tsym) : 0);
        databuf.appendChar(interfaces.length());
        for (List l = interfaces; l.nonEmpty(); l = l.tail)
            databuf.appendChar(pool.put(((Type) l.head).tsym));
        int fieldsCount = 0;
        int methodsCount = 0;
        for (Scope.Entry e = c.members().elems; e != null; e = e.sibling) {
            switch (e.sym.kind) {
            case VAR:
                fieldsCount++;
                break;

            case MTH:
                methodsCount++;
                break;

            case TYP:
                enterInner((ClassSymbol) e.sym);
                break;

            default:
                assert false;

            }
        }
        databuf.appendChar(fieldsCount);
        writeFields(c.members().elems);
        databuf.appendChar(methodsCount);
        writeMethods(c.members().elems);
        int acountIdx = beginAttrs();
        int acount = 0;
        if (c.sourcefile != null && emitSourceFile) {
            int alenIdx = writeAttr(names.SourceFile);
            String filename = c.sourcefile.toString();
            int sepIdx = filename.lastIndexOf(File.separatorChar);
            int slashIdx = filename.lastIndexOf('/');
            if (slashIdx > sepIdx)
                sepIdx = slashIdx;
            if (sepIdx >= 0)
                filename = filename.substring(sepIdx + 1);
            databuf.appendChar(c.pool.put(names.fromString(filename)));
            endAttr(alenIdx);
            acount++;
        }
        if (genCrt) {
            int alenIdx = writeAttr(names.SourceID);
            databuf.appendChar( c.pool.put(
                    names.fromString(Long.toString(getLastModified(c.sourcefile)))));
            endAttr(alenIdx);
            acount++;
            alenIdx = writeAttr(names.CompilationID);
            databuf.appendChar( c.pool.put(
                    names.fromString(Long.toString(System.currentTimeMillis()))));
            endAttr(alenIdx);
            acount++;
        }
        acount += writeFlagAttrs(c.flags());
        poolbuf.appendInt(JAVA_MAGIC);
        poolbuf.appendChar(target.minorVersion);
        poolbuf.appendChar(target.majorVersion);
        writePool(c.pool);
        if (innerClasses != null) {
            writeInnerClasses();
            acount++;
        }
        endAttrs(acountIdx, acount);
        poolbuf.appendBytes(databuf.elems, 0, databuf.length);
        out.write(poolbuf.elems, 0, poolbuf.length);
        pool = c.pool = null;
    }

    /**
      * Get modification date of file entry with name filename
      */
    long getLastModified(Name filename) {
        long mod = 0;
        File file = new File(filename.toString());
        try {
            mod = file.lastModified();
        } catch (SecurityException e) {
            throw new AssertionError(
                    "CRT: couldn\'t get source file modification date: " +
                    e.getMessage());
        }
        return mod;
    }

    /**
      * Create output file with given extension for given class.
      */
    public File outputFile(ClassSymbol c, String extension) throws IOException {
        if (outDir == null) {
            String filename = Convert.shortName(c.flatname) + extension;
            if (c.sourcefile == null)
                return new File(filename);
            String sourcedir = new File(c.sourcefile.toString()).getParent();
            if (sourcedir == null)
                return new File(filename);
            else
                return new File(sourcedir, filename);
        } else {
            return outputFile(outDir, c.flatname.toString(), extension);
        }
    }

    /**
      * open file given by a fully qualified name from root directory `outdir'.
      *  create intermediate directories if they don't exist already
      */
    File outputFile(File outdir, String name, String extension) throws IOException {
        int start = 0;
        int end = name.indexOf('.');
        while (end >= start) {
            outdir = new File(outdir, name.substring(start, end));
            if (!outdir.exists())
                outdir.mkdir();
            start = end + 1;
            end = name.indexOf('.', start);
        }
        return new File(outdir, name.substring(start) + extension);
    }
}
