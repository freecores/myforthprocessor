/**
 * @(#)ClassReader.java	1.55 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8.code;
import com.sun.tools.javac.v8.util.*;

import java.io.*;

import java.util.zip.*;

import java.util.StringTokenizer;

import com.sun.tools.javac.v8.code.Type.*;

import com.sun.tools.javac.v8.code.Symbol.*;

import com.sun.tools.javac.v8.code.Symtab;


/**
 * This class provides operations to read a classfile into an internal
 *  representation. The internal representation is anchored in a
 *  ClassSymbol which contains in its scope symbol representations
 *  for all other definitions in the classfile. Top-level Classes themselves
 *  appear as members of the scopes of PackageSymbols.
 */
public class ClassReader extends ClassFile implements Completer, Flags, Kinds,
TypeTags {

    /**
     * The context key for the class reader.
     */
    private static final Context.Key classReaderKey = new Context.Key();

    /**
     * The string used as separator in class paths.
     */
    public static final String pathSep = System.getProperty("path.separator");

    /**
     * Switch: verbose output.
     */
    public boolean verbose;

    /**
     * Switch: check class file for correct minor version, unrecognized
     *  attributes.
     */
    public boolean checkClassFile;

    /**
     * Switch: read constant pool and code sections. This switch is initially
     *  set to false but can be turned on from outside.
     */
    public boolean readAllOfClassFile = false;

    /**
     * The log to use for verbose output
     */
    private final Log log;

    /**
     * The symbol table.
     */
    private final Symtab syms;

    /**
     * The name table.
     */
    protected final Name.Table names;

    /**
     * Force a completion failure on this name
     */
    final Name completionFailureName;

    /**
     * Can be reassigned from outside:
     *  the class path to be searched for user classes.
     */
    public String classPath;

    /**
     * The class path to be searched for system classes.
     */
    public String bootClassPath;

    /**
     * The path to be searched for sources
     */
    public String sourceClassPath;

    /**
     * Can be reassigned from outside:
     *  the completer to be used for ".java" files. If this remains unassigned
     *  ".java" files will not be loaded.
     */
    public SourceCompleter sourceCompleter = null;

    /**
     * A hashtable containing the encountered top-level and member classes,
     *  indexed by flat names. The table does not contain local classes.
     */
    private Hashtable classes;

    /**
     * A hashtable containing the encountered packages.
     */
    private Hashtable packages;

    /**
     * The current scope where type variables are entered.
     */
    Scope typevars = new Scope(null);

    /**
     * The path name of the class file currently being read.
     */
    String currentClassFileName = null;

    /**
     * The class or method currently being read.
     */
    Symbol currentOwner = null;

    /**
     * The buffer containing the currently read class file.
     */
    byte[] buf = new byte[65520];

    /**
     * The current input pointer.
     */
    int bp;

    /**
     * The objects of the constant pool.
     */
    Object[] poolObj;

    /**
     * For every constant pool entry, an index into buf where the
     *  defining section of the entry is found.
     */
    int[] poolIdx;

    /**
     * Get the ClassReader instance for this invocation.
     */
    public static ClassReader instance(Context context) {
        ClassReader instance = (ClassReader) context.get(classReaderKey);
        if (instance == null)
            instance = new ClassReader(context, true);
        return instance;
    }

    /**
      * Initialize classes and packages, treating this as the definitive classreader.
      */
    public void init(Symtab syms) {
        init(syms, true);
    }

    /**
      * Initialize classes and packages, optionally treating this as
      *  the definitive classreader.
      */
    private void init(Symtab syms, boolean definitive) {
        if (classes != null)
            return;
        if (definitive) {
            assert packages == null || packages == syms.packages;
            packages = syms.packages;
            assert classes == null || classes == syms.classes;
            classes = syms.classes;
        } else {
            packages = new Hashtable();
            classes = new Hashtable();
        }
        packages.put(syms.rootPackage.fullname, syms.rootPackage);
        syms.rootPackage.completer = this;
        packages.put(syms.emptyPackage.fullname, syms.emptyPackage);
        syms.emptyPackage.completer = this;
    }

    /**
      * Construct a new class reader, optionally treated as the
      *  definitive classreader for this invocation.
      */
    protected ClassReader(Context context, boolean definitive) {
        super();
        if (definitive)
            context.put(classReaderKey, this);
        names = Name.Table.instance(context);
        syms = Symtab.instance(context);
        init(syms, definitive);
        log = Log.instance(context);
        Options options = Options.instance(context);
        verbose = options.get("-verbose") != null;
        checkClassFile = options.get("-checkclassfile") != null;
        setClassPaths(options);
        completionFailureName = (options.get("failcomplete") != null) ?
                names.fromString((String) options.get("failcomplete")) : null;
    }

    /**
      * Set classPath, bootClassPath, and sourceClassPath.
      */
    private void setClassPaths(Options options) {
        String cp = (String) options.get("-classpath");
        if (cp == null)
            cp = System.getProperty("env.class.path");
        if (cp == null && System.getProperty("application.home") == null)
            cp = System.getProperty("java.class.path");
        if (cp == null)
            cp = ".";
        classPath = terminate(cp);
        String bp = (String) options.get("-bootclasspath");
        if (bp == null) {
            bp = System.getProperty("sun.boot.class.path");
            if (bp == null)
                bp = System.getProperty("java.class.path");
            if (bp == null)
                bp = ".";
        }
        String bpp = (String) options.get("-Xbootclasspath/p:");
        if (bpp != null)
            bp = bpp + System.getProperty("path.separator") + bp;
        bootClassPath = terminate(bp);
        String ed = (String) options.get("-extdirs");
        if (ed == null)
            ed = System.getProperty("java.ext.dirs");
        if (ed == null)
            ed = "";
        String extdirs = terminate(ed);
        int i = 0;
        int edlen = extdirs.length();
        while (i < edlen) {
            int end = extdirs.indexOf(pathSep, i);
            String extdir = extdirs.substring(i, end);
            addArchives(extdir);
            i = end + 1;
        }
        String sp = (String) options.get("-sourcepath");
        if (sp != null) {
            sourceClassPath = terminate(sp);
        } else {
            sourceClassPath = null;
        }
    }

    /**
      * Add all archives in `extdir' to boot class path
      */
    private void addArchives(String extdir) {
        String[] archives = new File(extdir).list();
        for (int i = 0; archives != null && i < archives.length; i++) {
            if (archives[i].endsWith(".jar")) {
                String prefix = (extdir.endsWith(File.separator)) ? extdir :
                        (extdir + File.separator);
                bootClassPath = bootClassPath + prefix + archives[i] + pathSep;
            }
        }
    }

    /**
      * Add path separator to string if it does not end in one already
      */
    private String terminate(String s) {
        return (s.endsWith(pathSep)) ? s : s + pathSep;
    }

    /**
      * Add member to class unless it is synthetic
      */
    private void enterMember(ClassSymbol c, Symbol sym) {
        if ((sym.flags_field & SYNTHETIC) == 0)
            c.members_field.enter(sym);
    }

    /**
      * Error Diagnoses
      */
    public static class BadClassFile extends CompletionFailure {

        /**
         * @param msg A localized message.
         */
        public BadClassFile(ClassSymbol c, String cname, String msg) {
            super(c, Log.getLocalizedString("bad.class.file.header", cname, msg));
        }
    }

    public BadClassFile badClassFile(String key) {
        return new BadClassFile(currentOwner.enclClass(), currentClassFileName,
                Log.getLocalizedString(key));
    }

    public BadClassFile badClassFile(String key, String arg0) {
        return new BadClassFile(currentOwner.enclClass(), currentClassFileName,
                Log.getLocalizedString(key, arg0));
    }

    public BadClassFile badClassFile(String key, String arg0, String arg1) {
        return new BadClassFile(currentOwner.enclClass(), currentClassFileName,
                Log.getLocalizedString(key, arg0, arg1));
    }

    public BadClassFile badClassFile(String key, String arg0, String arg1,
            String arg2) {
        return new BadClassFile(currentOwner.enclClass(), currentClassFileName,
                Log.getLocalizedString(key, arg0, arg1, arg2));
    }

    public BadClassFile badClassFile(String key, String arg0, String arg1,
            String arg2, String arg3) {
        return new BadClassFile(currentOwner.enclClass(), currentClassFileName,
                Log.getLocalizedString(key, arg0, arg1, arg2, arg3));
    }

    /**
      * Read a character.
      */
    char nextChar() {
        return (char)(((buf[bp++] & 255)<< 8) + (buf[bp++] & 255));
    }

    /**
      * Read an integer.
      */
    int nextInt() {
        return ((buf[bp++] & 255)<< 24) + ((buf[bp++] & 255)<< 16) +
                ((buf[bp++] & 255)<< 8) + (buf[bp++] & 255);
    }

    /**
      * Extract a character at position bp from buf.
      */
    char getChar(int bp) {
        return (char)(((buf[bp] & 255)<< 8) + (buf[bp + 1] & 255));
    }

    /**
      * Extract an integer at position bp from buf.
      */
    int getInt(int bp) {
        return ((buf[bp] & 255)<< 24) + ((buf[bp + 1] & 255)<< 16) +
                ((buf[bp + 2] & 255)<< 8) + (buf[bp + 3] & 255);
    }

    /**
      * Extract a long integer at position bp from buf.
      */
    long getLong(int bp) {
        DataInputStream bufin =
                new DataInputStream(new ByteArrayInputStream(buf, bp, 8));
        try {
            return bufin.readLong();
        } catch (IOException e) {
            throw new AssertionError();
        }
    }

    /**
      * Extract a float at position bp from buf.
      */
    float getFloat(int bp) {
        DataInputStream bufin =
                new DataInputStream(new ByteArrayInputStream(buf, bp, 4));
        try {
            return bufin.readFloat();
        } catch (IOException e) {
            throw new AssertionError();
        }
    }

    /**
      * Extract a double at position bp from buf.
      */
    double getDouble(int bp) {
        DataInputStream bufin =
                new DataInputStream(new ByteArrayInputStream(buf, bp, 8));
        try {
            return bufin.readDouble();
        } catch (IOException e) {
            throw new AssertionError();
        }
    }

    /**
      * Index all constant pool entries, writing their start addresses into
      *  poolIdx.
      */
    void indexPool() {
        poolIdx = new int[nextChar()];
        poolObj = new Object[poolIdx.length];
        int i = 1;
        while (i < poolIdx.length) {
            poolIdx[i++] = bp;
            byte tag = buf[bp++];
            switch (tag) {
            case CONSTANT_Utf8:

            case CONSTANT_Unicode:
                {
                    int len = nextChar();
                    bp = bp + len;
                    break;
                }

            case CONSTANT_Class:

            case CONSTANT_String:
                bp = bp + 2;
                break;

            case CONSTANT_Fieldref:

            case CONSTANT_Methodref:

            case CONSTANT_InterfaceMethodref:

            case CONSTANT_NameandType:

            case CONSTANT_Integer:

            case CONSTANT_Float:
                bp = bp + 4;
                break;

            case CONSTANT_Long:

            case CONSTANT_Double:
                bp = bp + 8;
                i++;
                break;

            default:
                throw badClassFile("bad.const.pool.tag.at", Byte.toString(tag),
                        Integer.toString(bp - 1));

            }
        }
    }

    /**
      * Read constant pool entry at start address i, use pool as a cache.
      */
    Object readPool(int i) {
        Object result = poolObj[i];
        if (result != null)
            return result;
        int index = poolIdx[i];
        if (index == 0)
            return null;
        byte tag = buf[index];
        switch (tag) {
        case CONSTANT_Utf8:
            poolObj[i] = names.fromUtf(buf, index + 3, getChar(index + 1));
            break;

        case CONSTANT_Unicode:
            throw badClassFile("unicode.str.not.supported");

        case CONSTANT_Class:
            poolObj[i] = readClassOrType(getChar(index + 1));
            break;

        case CONSTANT_String:
            poolObj[i] = readName(getChar(index + 1)).toString();
            break;

        case CONSTANT_Fieldref:
            {
                ClassSymbol owner = readClassSymbol(getChar(index + 1));
                NameAndType nt = (NameAndType) readPool(getChar(index + 3));
                poolObj[i] = new VarSymbol(0, nt.name, nt.type, owner);
                break;
            }

        case CONSTANT_Methodref:

        case CONSTANT_InterfaceMethodref:
            {
                ClassSymbol owner = readClassSymbol(getChar(index + 1));
                NameAndType nt = (NameAndType) readPool(getChar(index + 3));
                poolObj[i] = new MethodSymbol(0, nt.name, nt.type, owner);
                break;
            }

        case CONSTANT_NameandType:
            poolObj[i] = new NameAndType(readName(getChar(index + 1)),
                    readType(getChar(index + 3)));
            break;

        case CONSTANT_Integer:
            poolObj[i] = new Integer(getInt(index + 1));
            break;

        case CONSTANT_Float:
            poolObj[i] = new Float(getFloat(index + 1));
            break;

        case CONSTANT_Long:
            poolObj[i] = new Long(getLong(index + 1));
            break;

        case CONSTANT_Double:
            poolObj[i] = new Double(getDouble(index + 1));
            break;

        default:
            throw badClassFile("bad.const.pool.tag", Byte.toString(tag));

        }
        return poolObj[i];
    }

    /**
      * Read signature and convert to type.
      */
    Type readType(int i) {
        int index = poolIdx[i];
        return sigToType(buf, index + 3, getChar(index + 1));
    }

    /**
      * If name is an array type or class signature, return the
      *  corresponding type; otherwise return a ClassSymbol with given name.
      */
    Object readClassOrType(int i) {
        int index = poolIdx[i];
        int len = getChar(index + 1);
        int start = index + 3;
        return (buf[start] == '[' || buf[start + len - 1] == ';') ?
                (Object) sigToType(buf, start, len) :
                (Object) enterClass(names.fromUtf(internalize(buf, start, len)));
    }

    /**
      * Read class entry.
      */
    ClassSymbol readClassSymbol(int i) {
        return (ClassSymbol)(readPool(i));
    }

    /**
      * Read name.
      */
    Name readName(int i) {
        return (Name)(readPool(i));
    }

    /**
      * The unread portion of the currently read type is
      *  signature[sigp..siglimit-1].
      */
    byte[] signature;
    int sigp;
    int siglimit;
    boolean sigEnterPhase = false;

    /**
     * Convert signature to type, where signature is a name.
     */
    Type sigToType(Name sig) {
        return sig == null ? null : sigToType(sig.table.names, sig.index, sig.len);
    }

    /**
      * Convert signature to type, where signature is a byte array segment.
      */
    Type sigToType(byte[] sig, int offset, int len) {
        signature = sig;
        sigp = offset;
        siglimit = offset + len;
        return sigToType();
    }

    /**
      * Convert signature to type, where signature is implicit.
      */
    Type sigToType() {
        switch ((char) signature[sigp]) {
        case 'B':
            sigp++;
            return syms.byteType;

        case 'C':
            sigp++;
            return syms.charType;

        case 'D':
            sigp++;
            return syms.doubleType;

        case 'F':
            sigp++;
            return syms.floatType;

        case 'I':
            sigp++;
            return syms.intType;

        case 'J':
            sigp++;
            return syms.longType;

        case 'L':
            Type t = classSigToType(Type.noType);
            while (sigp < siglimit && signature[sigp] == '.') {
                sigp++;
                t = classSigToType(t);
            }
            return t;

        case 'S':
            sigp++;
            return syms.shortType;

        case 'V':
            sigp++;
            return syms.voidType;

        case 'Z':
            sigp++;
            return syms.booleanType;

        case '[':
            sigp++;
            while ('0' <= signature[sigp] && signature[sigp] <= '9')
                sigp++;
            return new ArrayType(sigToType(), syms.arrayClass);

        case '(':
            List argtypes = sigToTypes(')');
            Type restype = sigToType();
            ListBuffer thrown = new ListBuffer();
            while (signature[sigp] == '^') {
                sigp++;
                thrown.append(sigToType());
            }
            return new MethodType(argtypes, restype, thrown.toList(),
                    syms.methodClass);

        default:
            throw badClassFile("bad.signature",
                    Convert.utf2string(signature, sigp, 10));

        }
    }

    /**
      * Convert class signature to type, where signature is implicit.
      *  @param outer      The outer type encountered so far. This is meaningful
      *                    only if outer is parameterized. In that case we know
      *                    that we have to store it in the new type's
      *			  outer_field since a parameterized prefix is legal
      *			  only for non-static inner classes.
      */
    Type classSigToType(Type outer) {
        if (signature[sigp] == 'L') {
            sigp++;
            int start = sigp;
            while (signature[sigp] != ';' && signature[sigp] != '<')
                sigp++;
            ClassType t = (ClassType) enterClass(
                    names.fromUtf(internalize(signature, start, sigp - start))).type;
            if (signature[sigp] == '<')
                t = new ClassType(t.outer_field, sigToTypes('>'), t.tsym);
            else if (t.typarams().nonEmpty())
                t = (ClassType) t.erasure();
            if (outer.isParameterized())
                t = new ClassType(outer, t.typarams(), t.tsym);
            sigp++;
            return t;
        } else {
            throw badClassFile("bad.class.signature",
                    Convert.utf2string(signature, sigp, 10));
        }
    }

    /**
      * Convert (implicit) signature to list of types
      *  until `terminator' is encountered.
      */
    List sigToTypes(char terminator) {
        sigp++;
        ListBuffer types = new ListBuffer();
        while (signature[sigp] != terminator)
            types.append(sigToType());
        sigp++;
        return types.toList();
    }

    /**
      * Report unrecognized attribute.
      */
    void unrecogized(Name attrName) {
        if (checkClassFile)
            printCCF("ccf.unrecognized.attribute", attrName.toJava());
    }

    /**
      * Read member attribute.
      */
    void readMemberAttr(Symbol sym, Name attrName, int attrLen) {
        if (attrName == names.ConstantValue) {
            Object v = readPool(nextChar());
            if ((sym.flags() & FINAL) != 0)
                ((VarSymbol) sym).constValue = v;
        } else if (attrName == names.Code) {
            if (readAllOfClassFile)
                ((MethodSymbol) sym).code = readCode(sym);
            else
                bp = bp + attrLen;
        } else if (attrName == names.Exceptions) {
            int nexceptions = nextChar();
            ListBuffer thrown = new ListBuffer();
            for (int j = 0; j < nexceptions; j++)
                thrown.append(readClassSymbol(nextChar()).type);
            if (sym.type.thrown().isEmpty())
                sym.type.asMethodType().thrown = thrown.toList();
        } else if (attrName == names.Synthetic) {
            sym.flags_field |= SYNTHETIC;
        } else if (attrName == names.Deprecated) {
            sym.flags_field |= DEPRECATED;
        } else {
            unrecogized(attrName);
            bp = bp + attrLen;
        }
    }

    /**
      * Read member attributes.
      */
    void readMemberAttrs(Symbol sym) {
        char ac = nextChar();
        for (int i = 0; i < ac; i++) {
            Name attrName = readName(nextChar());
            int attrLen = nextInt();
            readMemberAttr(sym, attrName, attrLen);
        }
    }

    /**
      * Read class attribute.
      */
    void readClassAttr(ClassSymbol c, Name attrName, int attrLen) {
        if (attrName == names.SourceFile) {
            c.sourcefile = readName(nextChar());
        } else if (attrName == names.InnerClasses) {
            readInnerClasses(c);
        } else {
            readMemberAttr(c, attrName, attrLen);
        }
    }

    /**
      * Read class attributes.
      */
    void readClassAttrs(ClassSymbol c) {
        char ac = nextChar();
        for (int i = 0; i < ac; i++) {
            Name attrName = readName(nextChar());
            int attrLen = nextInt();
            readClassAttr(c, attrName, attrLen);
        }
    }

    /**
      * Read code block.
      */
    Code readCode(Symbol owner) {
        return null;
    }

    /**
      * Read a field.
      */
    VarSymbol readField() {
        int flags = nextChar();
        Name name = readName(nextChar());
        Type type = readType(nextChar());
        VarSymbol v = new VarSymbol(flags, name, type, currentOwner);
        readMemberAttrs(v);
        return v;
    }

    /**
      * Read a method.
      */
    MethodSymbol readMethod() {
        int flags = nextChar();
        Name name = readName(nextChar());
        Type type = readType(nextChar());
        if (name == names.init && currentOwner.hasOuterInstance()) {
            type = new MethodType(type.argtypes().tail, type.restype(),
                    type.thrown(), syms.methodClass);
        }
        MethodSymbol m = new MethodSymbol(flags, name, type, currentOwner);
        Symbol prevOwner = currentOwner;
        currentOwner = m;
        readMemberAttrs(m);
        currentOwner = prevOwner;
        return m;
    }

    /**
      * Skip a field or method
      */
    void skipMember() {
        bp = bp + 6;
        char ac = nextChar();
        for (int i = 0; i < ac; i++) {
            bp = bp + 2;
            int attrLen = nextInt();
            bp = bp + attrLen;
        }
    }

    /**
      * Enter type variables of this classtype and all enclosing ones in
      *  `typevars'.
      */
    void enterTypevars(Type t) {
        if (t.outer().tag == CLASS)
            enterTypevars(t.outer());
        for (List xs = t.typarams(); xs.nonEmpty(); xs = xs.tail)
            typevars.enter(((Type) xs.head).tsym);
    }

    /**
      * Read contents of a given class symbol `c'. Both external and internal
      *  versions of an inner class are read.
      */
    void readClass(ClassSymbol c) {
        ClassType ct = (ClassType) c.type;
        c.members_field = new Scope(c);
        typevars = typevars.dup();
        if (ct.outer().tag == CLASS)
            enterTypevars(ct.outer());
        int flags = nextChar();
        if (c.owner.kind == PCK)
            c.flags_field = flags;
        ClassSymbol self = readClassSymbol(nextChar());
        if (c != self)
            throw badClassFile("class.file.wrong.class", self.flatname.toJava());
        int startbp = bp;
        nextChar();
        char interfaceCount = nextChar();
        bp += interfaceCount * 2;
        char fieldCount = nextChar();
        for (int i = 0; i < fieldCount; i++)
            skipMember();
        char methodCount = nextChar();
        for (int i = 0; i < methodCount; i++)
            skipMember();
        readClassAttrs(c);
        if (readAllOfClassFile) {
            for (int i = 1; i < poolObj.length; i++)
                readPool(i);
            c.pool = new Pool(poolObj.length, poolObj);
        }
        bp = startbp;
        int n = nextChar();
        ct.supertype_field = (n == 0) ? Type.noType : readClassSymbol(n).type;
        n = nextChar();
        ListBuffer is = new ListBuffer();
        for (int i = 0; i < n; i++)
            is.append(readClassSymbol(nextChar()).type);
        ct.interfaces_field = is.toList();
        if (fieldCount != nextChar())
            assert false;
        for (int i = 0; i < fieldCount; i++)
            enterMember(c, readField());
        if (methodCount != nextChar())
            assert false;
        for (int i = 0; i < methodCount; i++)
            enterMember(c, readMethod());
        typevars = typevars.leave();
    }

    /**
      * Read inner class info. For each inner/outer pair allocate a
      *  member class.
      */
    void readInnerClasses(ClassSymbol c) {
        int n = nextChar();
        for (int i = 0; i < n; i++) {
            nextChar();
            ClassSymbol outer = readClassSymbol(nextChar());
            Name name = readName(nextChar());
            if (name == null)
                name = names.empty;
            int flags = nextChar();
            if (outer != null) {
                if (name == names.empty)
                    name = names.one;
                ClassSymbol member = enterClass(name, outer);
                if ((flags & STATIC) == 0) {
                    ((ClassType) member.type).outer_field = outer.type;
                }
                if (c == outer) {
                    member.flags_field = flags;
                    enterMember(c, member);
                }
            }
        }
    }

    /**
      * Read a class file.
      */
    void readClassFile(ClassSymbol c) throws IOException {
        int magic = nextInt();
        if (magic != JAVA_MAGIC)
            throw badClassFile("illegal.start.of.class.file");
        int minorVersion = nextChar();
        int majorVersion = nextChar();
        if (majorVersion > Target.MAX().majorVersion ||
                majorVersion * 1000 + minorVersion <
                Target.MIN().majorVersion * 1000 + Target.MIN().minorVersion) {
            throw badClassFile("wrong.version", Integer.toString(majorVersion),
                    Integer.toString(minorVersion),
                    Integer.toString(Target.MAX().majorVersion),
                    Integer.toString(Target.MAX().minorVersion));
        } else if (checkClassFile && majorVersion == Target.MAX().majorVersion &&
                minorVersion > Target.MAX().minorVersion) {
            printCCF("found.later.version", Integer.toString(minorVersion));
        }
        indexPool();
        readClass(c);
    }

    /**
      * Is this the name of a zip file?
      */
    boolean isZip(String name) {
        return new File(name).isFile();
    }

    /**
      * An archive consists of a zipfile and a list of zip entries in
      *  that file.
      */
    static class Archive {
        ZipFile zdir;
        List entries;

        Archive(ZipFile zdir, List entries) {
            super();
            this.zdir = zdir;
            this.entries = entries;
        }
    }

    /**
      * A directory of zip files already opened.
      */
    Hashtable archives = Hashtable.make();

    /**
     * Open a new zip file directory.
     */
    Archive openArchive(String dirname) throws IOException {
        Archive archive = (ClassReader.Archive) archives.get(dirname);
        if (archive == null) {
            ZipFile zdir = new ZipFile(dirname);
            ListBuffer entries = new ListBuffer();
            for (java.util.Enumeration e = zdir.entries(); e.hasMoreElements();) {
                entries.append((ZipEntry) e.nextElement());
            }
            archive = new Archive(zdir, entries.toList());
            archives.put(dirname, archive);
        }
        return archive;
    }

    /**
      * Close the ClassReader, releasing resources.
      */
    public void close() {
        for (List dirnames = archives.keys(); dirnames.nonEmpty();
                dirnames = dirnames.tail) {
            Archive a = (ClassReader.Archive) archives.remove(dirnames.head);
            try {
                a.zdir.close();
            } catch (IOException e) {
            }
        }
    }

    /**
      * Define a new class given its name and owner.
      */
    public ClassSymbol defineClass(Name name, Symbol owner) {
        ClassSymbol c = new ClassSymbol(0, name, owner);
        c.completer = this;
        return c;
    }

    /**
      * Create a new toplevel or member class symbol with given name
      *  and owner and enter in `classes' unless already there.
      */
    public ClassSymbol enterClass(Name name, TypeSymbol owner) {
        Name flatname = TypeSymbol.formFlatName(name, owner);
        ClassSymbol c = (ClassSymbol) classes.get(flatname);
        if (c == null) {
            c = defineClass(name, owner);
            classes.put(flatname, c);
        } else if ((c.name != name || c.owner != owner) && owner.kind == TYP) {
            c.name = name;
            c.owner = owner;
            c.fullname = ClassSymbol.formFullName(name, owner);
        }
        return c;
    }

    /**
      * Create a new member or toplevel class symbol with given flat name
      *  and enter in `classes' unless already there.
      */
    public ClassSymbol enterClass(Name flatname) {
        ClassSymbol c = (ClassSymbol) classes.get(flatname);
        if (c == null) {
            Name packageName = Convert.packagePart(flatname);
            if (packageName == names.empty)
                packageName = names.emptyPackage;
            c = defineClass(Convert.shortName(flatname), enterPackage(packageName));
            classes.put(flatname, c);
        }
        return c;
    }

    /**
      * Completion for classes to be loaded. Before a class is loaded
      *  we make sure its enclosing class (if any) is loaded.
      */
    public void complete(Symbol sym) throws CompletionFailure {
        if (sym.kind == TYP) {
            ClassSymbol c = (ClassSymbol) sym;
            c.members_field = new Scope.ErrorScope(c);
            c.owner.complete();
            fillIn(c);
        } else if (sym.kind == PCK) {
            PackageSymbol p = (PackageSymbol) sym;
            fillIn(p);
        }
    }

    /**
      * We can only read a single class file at a time; this
      * flag keeps track of when we are currently reading a class
      * file.
      */
    private boolean filling = false;

    /**
     * Fill in definition of class `c' from corresponding class or
     *  source file.
     */
    private void fillIn(ClassSymbol c) {
        if (completionFailureName == c.fullname) {
            throw new CompletionFailure(c, "user-selected completion failure by class name");
        }
        currentOwner = c;
        FileEntry classfile = c.classfile;
        if (classfile != null) {
            try {
                assert ! filling;
                InputStream s = classfile.open();
                currentClassFileName = classfile.getPath();
                if (verbose) {
                    printVerbose("loading", currentClassFileName);
                }
                if (classfile.getName().endsWith(".class")) {
                    filling = true;
                    int size = (int) classfile.length();
                    if (buf.length < size)
                        buf = new byte[size];
                    int n = 0;
                    while (n < size)
                        n = n + s.read(buf, n, size - n);
                    s.close();
                    bp = 0;
                    readClassFile(c);
                } else {
                    sourceCompleter.complete(c, currentClassFileName, s);
                }
                return;
            } catch (IOException ex) {
                throw badClassFile("unable.to.access.file", ex.getMessage());
            }
            finally { filling = false;
                } } else {
            String fn = externalizeFileName(c.flatname);
            throw newCompletionFailure(c,
                    Log.getLocalizedString("dot.class.not.found", fn));
        }
    }

    /**
      * Static factory for CompletionFailure objects.
      *  In practice, only one can be used at a time, so we share one
      *  to reduce the expense of allocating new exception objects.
      */
    private CompletionFailure newCompletionFailure(ClassSymbol c, String localized) {
        CompletionFailure result = cachedCompletionFailure;
        result.sym = c;
        result.errmsg = localized;
        return result;
    }
    private CompletionFailure cachedCompletionFailure =
            new CompletionFailure(null, null);

    /**
     * Load a toplevel class with given fully qualified name
     *  The class is entered into `classes' only if load was successful.
     */
    public ClassSymbol loadClass(Name flatname) throws CompletionFailure {
        boolean absent = classes.get(flatname) == null;
        ClassSymbol c = enterClass(flatname);
        if (c.members_field == null && c.completer != null) {
            try {
                c.complete();
            } catch (CompletionFailure ex) {
                if (absent)
                    classes.remove(flatname);
                throw ex;
            }
        }
        return c;
    }

    /**
      * Check to see if a package exists, given its fully qualified name.
      */
    public boolean packageExists(Name fullname) {
        return enterPackage(fullname).exists();
    }

    /**
      * Make a package, given its fully qualified name.
      */
    public PackageSymbol enterPackage(Name fullname) {
        PackageSymbol p = (PackageSymbol) packages.get(fullname);
        if (p == null) {
            assert fullname.length() != 0 :
            "rootPackage missing!";
            p = new PackageSymbol(Convert.shortName(fullname),
                    enterPackage(Convert.packagePart(fullname)));
            p.completer = this;
            packages.put(fullname, p);
        }
        return p;
    }

    /**
      * Make a package, given its unqualified name and enclosing package.
      */
    public PackageSymbol enterPackage(Name name, PackageSymbol owner) {
        return enterPackage(TypeSymbol.formFullName(name, owner));
    }

    /**
      * Include class corresponding to given class file in package,
      *  unless (1) we already have one the same kind (.class or .java), or
      *         (2) we have one of the other kind, and the given class file
      *             is older.
      */
    private void includeClassFile(PackageSymbol p, FileEntry file) {
        if ((p.flags_field & EXISTS) == 0)
            for (Symbol q = p; q != null && q.kind == PCK; q = q.owner)
                q.flags_field |= EXISTS;
        String filename = file.getName();
        int seen;
        int extlen;
        if (filename.endsWith(".class")) {
            seen = CLASS_SEEN;
            extlen = 6;
        } else {
            seen = SOURCE_SEEN;
            extlen = 5;
        }
        Name classname =
                names.fromString(filename.substring(0, filename.length() - extlen));
        ClassSymbol c = (ClassSymbol) p.members_field.lookup(classname).sym;
        if (c == null) {
            c = enterClass(classname, p);
            c.classfile = file;
            if (c.owner == p)
                p.members_field.enter(c);
        } else if (c.classfile != null && (c.flags_field & seen) == 0) {
            if ((c.flags_field & (CLASS_SEEN | SOURCE_SEEN)) != 0) {
                long fdate = file.lastModified();
                long cdate = c.classfile.lastModified();
                if (fdate >= 0 && cdate >= 0 && fdate > cdate) {
                    c.classfile = file;
                }
            }
        }
        c.flags_field |= seen;
    }

    /**
      * Insert all files in subdirectory `name' of `pathname'
      *  which end in one of the extensions in `extensions' into package.
      */
    private void list(String pathname, String name, String[] extensions,
            PackageSymbol p) {
        try {
            if (isZip(pathname)) {
                Archive archive = openArchive(pathname);
                if (name.length() != 0) {
                    name = name.replace('\\', '/');
                    if (!name.endsWith("/"))
                        name = name + "/";
                }
                int namelen = name.length();
                for (List l = archive.entries; l.nonEmpty(); l = l.tail) {
                    ZipEntry entry = (ZipEntry) l.head;
                    String ename = entry.getName();
                    if (ename.startsWith(name)) {
                        if (endsWith(ename, extensions)) {
                            String suffix = ename.substring(namelen);
                            if (suffix.length() > 0 && suffix.indexOf('/') < 0) {
                                includeClassFile(p,
                                        new FileEntry.Zipped(suffix,
                                        archive.zdir, entry));
                            }
                        } else
                            extraZipFileActions(p, ename, name, pathname);
                    }
                }
            } else {
                File f = name.length() != 0 ? new File(pathname, name) :
                        new File(pathname);
                String[] names = f.list();
                if (names != null && caseMapCheck(f, name)) {
                    for (int i = 0; i < names.length; i++) {
                        String fname = names[i];
                        if (isValidFile(fname, extensions)) {
                            includeClassFile(p,
                                    new FileEntry.Regular(fname, new File(f, fname)));
                        } else
                            extraFileActions(p, fname, f);
                    }
                }
            }
        } catch (IOException ex) {
        }
    }

    private boolean endsWith(String s, String[] extensions) {
        for (int i = 0; i < extensions.length; i++)
            if (s.endsWith(extensions[i]))
                return true;
        return false;
    }

    private boolean isValidFile(String s, String[] extensions) {
        for (int i = 0; i < extensions.length; i++) {
            String extension = extensions[i];
            if (s.endsWith(extension) && isJavaIdentifier(
                    s.substring(0, s.length() - extension.length())))
                return true;
        }
        return false;
    }

    private boolean isJavaIdentifier(String s) {
        if (s.length() < 1)
            return false;
        if (!Character.isJavaIdentifierStart(s.charAt(0)))
            return false;
        for (int j = 1; j < s.length(); j++)
            if (!Character.isJavaIdentifierPart(s.charAt(j)))
                return false;
        return true;
    }
    static final boolean fileSystemIsCaseSensitive = File.separatorChar == '/';

    /**
     * Hack to make Windows case sensitive. Test whether given path
     *  ends in a string of characters with the same case as given name.
     *  Ignore file separators in both path and name.
     */
    private boolean caseMapCheck(File f, String name) throws IOException {
        if (fileSystemIsCaseSensitive)
            return true;
        String path = f.getCanonicalPath();
        char[] pcs = path.toCharArray();
        char[] ncs = name.toCharArray();
        int i = pcs.length - 1;
        int j = ncs.length - 1;
        while (i >= 0 && j >= 0) {
            while (i >= 0 && pcs[i] == File.separatorChar)
                i--;
            while (j >= 0 && ncs[j] == File.separatorChar)
                j--;
            if (i >= 0 && j >= 0) {
                if (pcs[i] != ncs[j])
                    return false;
                i--;
                j--;
            }
        }
        return j < 0;
    }

    /**
      * These are used to support javadoc
      */
    protected void extraZipFileActions(PackageSymbol pack, String zipEntryName,
            String classPathName, String zipName) {
    }

    protected void extraFileActions(PackageSymbol pack, String fileName,
            File fileDir) {
    }

    /**
      * Insert all files in subdirectory `name' somewhere on
      *  class path `path' which end in one of the extensions in `extensions'
      *  into package.
      */
    private void listAll(String path, String name, String[] extensions,
            PackageSymbol p) {
        int i = 0;
        int plen = path.length();
        while (i < plen) {
            int end = path.indexOf(pathSep, i);
            String pathname = path.substring(i, end);
            list(pathname, name, extensions, p);
            i = end + 1;
        }
    }
    static final String[] classOnly = {".class"};
    static final String[] javaOnly = {".java"};
    static final String[] classOrJava = {".class", ".java"};

    /**
     * Load directory of package into members scope.
     */
    private void fillIn(PackageSymbol p) {
        if (p.members_field == null)
            p.members_field = new Scope(p);
        Name packageName = p.fullname;
        if (packageName == names.emptyPackage)
            packageName = names.empty;
        String dirname = externalizeFileName(packageName);
        listAll(bootClassPath, dirname, classOnly, p);
        if (sourceCompleter != null && sourceClassPath == null) {
            listAll(classPath, dirname, classOrJava, p);
        } else {
            listAll(classPath, dirname, classOnly, p);
            if (sourceCompleter != null) {
                listAll(sourceClassPath, dirname, javaOnly, p);
            }
        }
    }

    /**
      * Output for "-verbose" option.
      *  @param key The key to look up the correct internationalized string.
      *  @param arg An argument for substitution into the output string.
      */
    private void printVerbose(String key, String arg) {
        Log.printLines(log.noticeWriter,
                Log.getLocalizedString("verbose." + key, arg));
    }

    /**
      * Output for "-checkclassfile" option.
      *  @param key The key to look up the correct internationalized string.
      *  @param arg An argument for substitution into the output string.
      */
    private void printCCF(String key, String arg) {
        Log.printLines(log.noticeWriter,
                Log.getLocalizedString("verbose." + key, arg));
    }

    public interface SourceCompleter {

        void complete(ClassSymbol sym, String filename,
                InputStream f) throws CompletionFailure;
            }
        }
