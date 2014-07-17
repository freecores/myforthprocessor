/**
 * @(#)JavaCompiler.java	1.44 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8;
import java.io.*;

import com.sun.tools.javac.v8.util.*;

import com.sun.tools.javac.v8.code.*;

import com.sun.tools.javac.v8.tree.*;

import com.sun.tools.javac.v8.parser.*;

import com.sun.tools.javac.v8.comp.*;

import com.sun.tools.javac.v8.code.Symbol.*;

import com.sun.tools.javac.v8.tree.Tree.*;


/**
 * This class could be the main entry point for GJC when GJC is used as a
 *  component in a larger software system. It provides operations to
 *  construct a new compiler, and to run a new compiler on a set of source
 *  files.
 */
public class JavaCompiler implements ClassReader.SourceCompleter {
    private static final Context.Key compilerKey = new Context.Key();

    /**
     * The current version number as a string.
     */
    public static String version() {
        return System.getProperty("java.version");
    }

    /**
      * The log to be used for error reporting.
      */
    private Log log;

    /**
     * The tree factory module.
     */
    private TreeMaker make;

    /**
     * The class reader.
     */
    private ClassReader reader;

    /**
     * The class writer.
     */
    ClassWriter writer;

    /**
     * The module for the symbol table entry phases.
     */
    private Enter enter;

    /**
     * The module for code generation.
     */
    private Gen gen;

    /**
     * The name table.
     */
    private Name.Table names;

    /**
     * Save the context.
     */
    private Context context;

    /**
     * Construct a new compiler from a log, a symbol table and an options table.
     */
    public JavaCompiler(Context context) {
        super();
        context.put(compilerKey, this);
        this.context = context;
        names = Name.Table.instance(context);
        log = Log.instance(context);
        reader = ClassReader.instance(context);
        make = TreeMaker.instance(context);
        writer = ClassWriter.instance(context);
        enter = Enter.instance(context);
        todo = Todo.instance(context);
        reader.sourceCompleter = this;
        Options options = Options.instance(context);
        verbose = options.get("-verbose") != null;
        sourceOutput = options.get("-s") != null;
        classOutput = options.get("-retrofit") == null;
        printFlat = options.get("-printflat") != null;
        deprecation = options.get("-deprecation") != null;
        warnunchecked = options.get("-warnunchecked") != null;
        attrParseOnly = options.get("-attrparseonly") != null;
        encoding = (String) options.get("-encoding");
        genCrt = options.get("-Xjcov") != null;
    }

    /**
      * Construct a new compiler.
      *  This will create a new symbol table module.
      */
    public static JavaCompiler make(Context context) {
        try {
            Symtab syms = Symtab.instance(context);
        } catch (CompletionFailure ex) {
            Log log = Log.instance(context);
            log.error(Position.NOPOS, ex.getMessage());
            return null;
        }
        return new JavaCompiler(context);
    }

    /**
      * Verbose output.
      */
    public boolean verbose;

    /**
     * Emit plain Java source files rather than class files.
     */
    public boolean sourceOutput;

    /**
     * Generate attributed parse tree only.
     */
    public boolean attrParseOnly;

    /**
     * Emit class files. This switch is always set, except for the first
     *  phase of retrofitting, where signatures are parsed.
     */
    public boolean classOutput;

    /**
     * Debug switch: Emit Java sources after inner class flattening.
     */
    public boolean printFlat;

    /**
     * Give detailed deprecation warnings.
     */
    public boolean deprecation;

    /**
     * Give detailed unchecked warnings.
     */
    public boolean warnunchecked;

    /**
     * Generate the CharacterRangeTable.
     */
    public boolean genCrt;

    /**
     * The encoding to be used for source input.
     */
    public String encoding;

    /**
     * A queue of all as yet unattributed classes.
     */
    private Todo todo;

    /**
     * The set of currently compiled inputfiles, needed to ensure
     *  we don't accidentally overwrite an input file when -s is set.
     *  initialized by `compile'.
     */
    Set inputFiles = Set.make();

    /**
     * The number of errors reported so far.
     */
    public int errorCount() {
        return log.nerrors;
    }

    /**
      * Try to open input stream with given name.
      *  Report an error if this fails.
      *  @param filename   The file name of the input stream to be opened.
      */
    public InputStream openSource(String filename) {
        try {
            File f = new File(filename);
            inputFiles.put(f);
            return new FileInputStream(f);
        } catch (IOException e) {
            log.error(Position.NOPOS, "cant.read.file", filename);
            return null;
        }
    }

    /**
      * Parse contents of input stream.
      *  @param filename     The name of the file from which input stream comes.
      *  @param input        The input stream to be parsed.
      */
    public TopLevel parse(String filename, InputStream input) {
        long msec = System.currentTimeMillis();
        Name prev = log.useSource(names.fromString(filename));
        TopLevel tree = make.TopLevel(null, Tree.emptyList);
        if (input != null) {
            if (verbose) {
                printVerbose("parsing.started", filename);
            }
            try {
                Scanner scanner = new Scanner(context, input, encoding);
                input.close();
                Parser parser = new Parser(context, scanner, keepComments(), genCrt);
                tree = parser.compilationUnit();
                if (verbose) {
                    printVerbose("parsing.done",
                            Long.toString(System.currentTimeMillis() - msec));
                }
            } catch (IOException e) {
                log.error(Position.NOPOS, "error.reading.file", filename,
                        e.toString());
            }
        }
        log.useSource(prev);
        tree.sourcefile = names.fromString(filename);
        return tree;
    }

    protected boolean keepComments() {
        return sourceOutput;
    }

    /**
      * Parse contents of file.
      *  @param filename     The name of the file to be parsed.
      */
    public Tree.TopLevel parse(String filename) {
        return parse(filename, openSource(filename));
    }

    /**
      * Emit plain Java source for a class.
      *  @param env    The attribution environment of the outermost class
      *                containing this class.
      *  @param cdef   The class definition to be printed.
      */
    void printSource(Env env, ClassDef cdef) throws IOException {
        File outFile = writer.outputFile(cdef.sym, ".java");
        if (inputFiles.contains(outFile)) {
            log.error(cdef.pos, "source.cant.overwrite.input.file",
                    outFile.toString());
        } else {
            PrintWriter out = new PrintWriter( new BufferedWriter(
                    new OutputStreamWriter(new FileOutputStream(outFile))));
            try {
                new Pretty(out, true).printUnit(env.toplevel, cdef);
                if (verbose)
                    printVerbose("wrote.file", outFile.getPath());
            }
            finally { out.close();
                    } }
    }

    /**
      * Generate code and emit a class file for a given class
      *  @param env    The attribution environment of the outermost class
      *                containing this class.
      *  @param cdef   The class definition from which code is generated.
      */
    void genCode(Env env, ClassDef cdef) throws IOException {
        try {
            if (gen.genClass(env, cdef))
                writer.writeClass(cdef.sym);
        } catch (ClassWriter.PoolOverflow ex) {
            log.error(cdef.pos, "limit.pool");
        }
        catch (ClassWriter.StringOverflow ex) {
            log.error(cdef.pos, "limit.string.overflow", ex.value.substring(0, 20));
        }
        catch (CompletionFailure ex) {
            log.error(Position.NOPOS, ex.getMessage());
        }
    }

    /**
      * Complete compiling a source file that has been accessed
      *  by the class file reader.
      *  @param c          The class the source file of which needs to be compiled.
      *  @param filename   The name of the source file.
      *  @param f          An input stream that reads the source file.
      */
    public void complete(ClassSymbol c, String filename,
            InputStream f) throws CompletionFailure {
        Tree tree = parse(filename, f);
        enter.complete(List.make(tree), c);
        if (enter.getEnv(c) == null) {
            throw new ClassReader.BadClassFile(c, filename,
                    log.getLocalizedString("file.doesnt.contain.class",
                    c.fullname.toJava()));
        }
    }

    /**
      * Track when the JavaCompiler has been used to compile something.
      */
    private boolean hasBeenUsed = false;

    /**
     * Main method: compile a list of files, return all compiled classes
     *  @param filenames     The names of all files to be compiled.
     */
    public List compile(List filenames) throws Throwable {
        assert ! hasBeenUsed :
        "attempt to reuse JavaCompiler";
        hasBeenUsed = true;
        long msec = System.currentTimeMillis();
        ListBuffer classes = new ListBuffer();
        try {
            ListBuffer trees = new ListBuffer();
            for (List l = filenames; l.nonEmpty(); l = l.tail)
                trees.append(parse((String) l.head));
            List roots = trees.toList();
            if (errorCount() == 0)
                enter.main(roots);
            List rootClasses = null;
            if (sourceOutput) {
                ListBuffer cdefs = new ListBuffer();
                for (List l = roots; l.nonEmpty(); l = l.tail) {
                    for (List defs = ((TopLevel) l.head).defs; defs.nonEmpty();
                            defs = defs.tail) {
                        if (defs.head instanceof ClassDef)
                            cdefs.append((ClassDef) defs.head);
                    }
                }
                rootClasses = cdefs.toList();
            }
            Attr attr = Attr.instance(context);
            this.gen = Gen.instance(context);
            Flow flow = Flow.instance(context);
            TransTypes transTypes = TransTypes.instance(context);
            TransInner transInner = TransInner.instance(context);
            while (todo.nonEmpty()) {
                Env env = (Env) todo.next();
                Tree untranslated = env.tree;
                if (verbose)
                    printVerbose("checking.attribution", env.enclClass.sym.toJava());
                Name prev = log.useSource(env.enclClass.sym.sourcefile);
                attr.attribClass(env.tree.pos, env.enclClass.sym);
                if (attrParseOnly)
                    continue;
                make.at(Position.FIRSTPOS);
                TreeMaker localMake = new TreeMaker(env.toplevel);
                if (errorCount() == 0) {
                    flow.analyzeTree(env.tree, localMake);
                }
                if (errorCount() == 0) {
                    env.tree = transTypes.translateTopLevelClass(env.tree, localMake);
                }
                if (errorCount() == 0) {
                    ClassDef cdef = null;
                    try {
                        if (sourceOutput) {
                            cdef = (ClassDef) env.tree;
                            if (untranslated instanceof ClassDef &&
                                    rootClasses.contains((ClassDef) untranslated)) {
                                printSource(env, cdef);
                            }
                        } else {
                            List cdefs = transInner.translateTopLevelClass(env,
                                    env.tree, localMake);
                            if (errorCount() == 0)
                                for (List l = cdefs;
                                        errorCount() == 0 && l.nonEmpty();
                                        l = l.tail) {
                                    cdef = (ClassDef) l.head;
                                    if (printFlat)
                                        printSource(env, cdef);
                                    else if (classOutput)
                                        genCode(env, cdef);
                                    classes.append(cdef.sym);
                                }
                        }
                    } catch (IOException ex) {
                        log.error(cdef.pos, "class.cant.write",
                                cdef.sym.toJava(), ex.getMessage());
                    }
                }
                log.useSource(prev);
            }
        } catch (Abort ex) {
        }
        Check chk = Check.instance(context);
        if (verbose)
            printVerbose("total", Long.toString(System.currentTimeMillis() - msec));
        if (chk.deprecatedSource != null && !deprecation)
            noteDeprecated(chk.deprecatedSource.toString());
        if (chk.uncheckedSource != null && !warnunchecked)
            noteUnchecked(chk.uncheckedSource.toString());
        int errCount = errorCount();
        if (errCount == 1)
            printCount("error", errCount);
        else
            printCount("error.plural", errCount);
        if (log.nwarnings == 1)
            printCount("warn", log.nwarnings);
        else
            printCount("warn.plural", log.nwarnings);
        return classes.toList();
    }

    /**
      * Close the compiler, flushing the logs
      */
    public void close() {
        log.flush();
        reader.close();
        names.dispose();
    }

    /**
      * Output for "-verbose" option.
      *  @param key The key to look up the correct internationalized string.
      *  @param arg An argument for substitution into the output string.
      */
    private void printVerbose(String key, String arg) {
        Log.printLines(log.noticeWriter,
                log.getLocalizedString("verbose." + key, arg));
    }

    /**
      * Print note that deprecated API's are used.
      */
    private void noteDeprecated(String input) {
        if (input.equals("*"))
            log.note("deprecated.plural");
        else
            log.note("deprecated.filename", input);
        log.note("deprecated.recompile");
    }

    /**
      * Print note that unchecked operations are used.
      */
    void noteUnchecked(String input) {
        if (input.equals("*"))
            log.note("unchecked.plural");
        else
            log.note("unchecked.filename", input);
        log.note("unchecked.recompile");
    }

    /**
      * Print numbers of errors and warnings.
      */
    void printCount(String kind, int count) {
        if (count != 0) {
            Log.printLines(log.errWriter,
                    log.getLocalizedString("count." + kind, Integer.toString(count)));
            log.errWriter.flush();
        }
    }
}
