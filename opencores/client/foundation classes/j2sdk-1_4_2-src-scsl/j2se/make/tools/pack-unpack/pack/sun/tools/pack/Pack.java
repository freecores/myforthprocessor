/*
 * @(#)Pack.java	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.pack;

import java.io.*;
import java.util.*;
import java.lang.reflect.*;
import java.util.zip.*;
import java.util.jar.*;

import sun.tools.io.RenamingOutputStream;

/**
 * Read a jar file and output a package file.
 * Usage:  java ...Pack [-@] [-19CDO] [-F<flag>] outfile infile...
 *
 * The package format uses these techniques:
 *  - Segmented constant pool, with separate numberings for each tag.
 *  - Strongly-typed constant pool references.  (Even ldc & ConstantValue.)
 *  - Specialized sub-numberings (e.g., <init> method w/in each class).
 *  - Global use of variable-length integers.
 *  - Ad hoc removal of various kinds of redundant data.
 *  - Collocation (by "bands") of similar data (numbers, strings, files, etc.)
 *  - Frequent delta encoding of collocated integers.
 *  - Sorting of collocated data (makes for frequent small positive deltas).
 *  - Trie storage of Utf8 data.
 *  - Specialized packing of long, unusual strings.
 *  - Substring sharing within Utf8 entries which serve as signatures.
 *  - Class members are represented as single nameandType CP entries.
 *  - Pseudo-opcodes for frequent operations (invoke method on self, etc.).
 *  - Dynamically varying coding of band data, to match band statistics.
 * @author John Rose
 * @version 1.6, 01/23/03
 */

public
class Pack {
    private final String USAGE = "Pack [-@] [-19CDO] [-F<flag>] outfile infile...";

    // If >0, print summary comments (e.g., constant pool info).
    // If >1, print unit comments (e.g., processing of classes).
    // If >2, print many comments (e.g., processing of members).
    // If >3, print tons of comments (e.g., processing of references).
    int verbose = 0;

    // filtering options:
    public boolean keepDebug = false;     // keep SourceFile, LineNumber, LocalVariables; sort members
    public boolean keepCompile = false;   // keep Exceptions, Deprecated, Synthetic
    public boolean stripExceptions = false; // do not keep exceptions (throws)
    public boolean stripInnerClasses = false; // do not keep InnerClasses info
    public boolean stripConstants = false; // do not keep constants
    public boolean keepClassOrder = false; // do not reorder *.class files
    public static boolean debugBands = false;
    public static boolean dumpBands = false;
    public static boolean bigStrings = true; // worth 1% in zip
    public static boolean varyCodings = true; // new format improvement
    public static boolean useCodingChooserWindow = false; // negligible gain
    public static int effort = 5; /* amount of compression work in [1..9] */
    // changing effort from 5 to 9 quadruples packing time, takes 0.5% from final zip

    public static final String PACK_PROP_FILENAME = "META-INF/pack.properties";
    // I/O
    OutputStream out = null;
    List inFiles = new ArrayList();
    private class InFile {
	final String name;
	final JarFile jf;
	final JarEntry je;
	final File f;
	InFile(String name) {
	    this.name = name;
	    this.f = new File(name);
	    this.jf = null;
	    this.je = null;
	    inFiles.add(this);
	}
	InFile(JarFile jf, JarEntry je) {
	    this.name = je.getName();
	    this.f = null;
	    this.jf = jf;
	    this.je = je;
	    inFiles.add(this);
	}
	InputStream getInputStream() throws IOException {
	    if (jf != null)
		return jf.getInputStream(je);
	    else
		return new FileInputStream(f);
	}
    }

    ArrayList inputOrder = new ArrayList();

    public static void main(String av[]) throws IOException {
	new Pack().run(av);
    }

    public void run(String av[]) throws IOException {
	int n = 0;
	Iterator inFileNames = null;
	while (n < av.length) {
	    String a = av[n].intern();
	    if (a == "-v")
		++verbose;
	    else if (a == "-@")
		inFileNames = oneFilePerLine(new InputStreamReader(System.in));
	    else if (!a.startsWith("-") || a == "-")
		break;  // ran into the output file name
	    else if (a.startsWith("-F"))
		processFlag(av[n].substring(2));
	    else if (a.length() == 2 && Character.isDigit(a.charAt(1)))
		// -9 is shorthand for -Feffort=9
		processFlag("effort="+a.substring(1));
	    else if (a == "-C")
		processFlag("+keepCompile");
	    else if (a == "-D")
		processFlag("+keepDebug");
	    else if (a == "-O")
		processFlag("+keepClassOrder");
	    else
		throw new RuntimeException("Bad flag "+av[n]);
	    n++;
	}
	BandStructure.setVerbose(verbose);
	if (n < av.length) {
	    if (av[n].equals("-")) {
		out = System.out;
		System.setOut(System.err);
	    } else {
		// Open the output file lazily, to avoid null files on error.
		out = new RenamingOutputStream(new File(av[n]));
	    }
	    ++n;
	}
	if (n < av.length) {
	    if (inFileNames != null)
		throw new RuntimeException("cannot specify both -@ and file arguments");
	    inFileNames = Arrays.asList(av).subList(n, av.length).iterator();
	}
	if (out == null || inFileNames == null) {
	    System.out.println("Usage:  "+USAGE);
	    return;
	}

	// Read the files.
	while (inFileNames.hasNext()) {
	    String arg = (String) inFileNames.next();
	    if (arg.endsWith(".jar")) {
		scanJar(arg);
	    } else if (arg.indexOf(".pack") > 0) {
		throw new RuntimeException("package inputs NYI: "+arg);
	    } else {
		new InFile(arg);
	    }
	}

	if (verbose > 0)
	    System.out.println("Reading " + inFiles.size() + " files...");

	// Here's where the bits go:
	Package pkg = new Package();

	int nread = 0;
	for (Iterator i = inFiles.iterator(); i.hasNext(); ) {
	    InFile inFile = (InFile)i.next();
	    String name      = inFile.name;
	    InputStream strm = inFile.getInputStream();
	    if (verbose > 1)
		System.out.println("Reading " + name);
	    if (name.endsWith(".class")) {
		readClass(pkg, name, strm);
	    } else if (name.endsWith(".zip") || name.endsWith(".jar")) {
		throw new RuntimeException("no nested archives, please: "+name);
	    } else if (name.indexOf(".pack") > 0) {
		//pkg.readPackage(f);
		throw new RuntimeException("package inputs NYI: "+name);
	    } else {
		readFile(pkg, name, strm);
	    }
	    strm.close();  // tidy up
	    nread++;
	    if (verbose > 0 && (nread % 1000) == 0)
		System.out.println("Have read "+nread+" files...");
	}
	//We create a null file for now.TODO add Pack properties
	//which we may choose to transmit to the unpack program.
	File propFile = File.createTempFile("pack",".tmp");
	FileInputStream f = new FileInputStream(propFile);
	readFile(pkg,PACK_PROP_FILENAME,f);
	flushPackage(pkg);
	f.close(); propFile.delete();
	out.close();
    }

    void readClass(Package pkg, String fname, InputStream in) throws IOException {
	Package.Class cls = pkg.new Class(fname);
	inputOrder.add(cls);
	in = new BufferedInputStream(in);
	new ClassReader(cls, in).read();
	in.close();
	pkg.addClass(cls);
    }

    void readFile(Package pkg, String fname, InputStream in) throws IOException {
	Package.File file = pkg.new File(fname);
	inputOrder.add(file);
	file.contents = BandStructure.readAll(in);
	in.close();
	pkg.addFile(file);
    }

    /** Write all information in the current package to the output stream. */
    void flushPackage(Package pkg) throws IOException {
	if (!keepClassOrder) Collections.sort(pkg.classes); // 0.7% better
	if (!keepDebug)      pkg.strip("Debug");
	if (!keepCompile)    pkg.strip("Compile");
	if (stripConstants)  pkg.strip("Constant");
	if (stripExceptions) pkg.strip("Exceptions");
	if (stripInnerClasses) pkg.strip("InnerClasses");
	pkg.reconstructConstantPool();
	new PackageWriter(pkg, out).write();
	out.close();
    }

    public void processFlag(String fname) {
	boolean isInt = false;
	int value = -1;
	if (fname.startsWith("+")) { value = 1; fname = fname.substring(1); }
	if (fname.startsWith("-")) { value = 0; fname = fname.substring(1); }
	int eqpos = fname.indexOf("=");
	if (value == -1 && eqpos > 0) {
	    isInt = true;
	    value = Integer.parseInt(fname.substring(eqpos+1));
	    fname = fname.substring(0, eqpos);
	}
	try {
	    Field f = Pack.class.getField(fname);
	    if (isInt)
		f.setInt(this, value);
	    else
		f.setBoolean(this, value != 0);
	} catch (Exception ee) {
	    throw new RuntimeException(ee.toString());
	}
    }

    private void scanJar(String name) throws IOException {
	JarFile jf = new JarFile(name);
	// Collect jar entries.  We will later process them in a canonical order.
	for (Enumeration e = jf.entries(); e.hasMoreElements(); ) {
	    JarEntry je = (JarEntry) e.nextElement();
	    //Ignore the file from the jar we will write a new one later.
	    if (je.getName().compareTo(PACK_PROP_FILENAME) == 0) continue;
	    if (je.isDirectory())  continue;  // totally ignore directories
	    new InFile(jf, je);
	}
    }

    /* Produce a stream of lines from a file. */
    static Iterator oneFilePerLine(final Reader files) {
	return new Iterator() {
	    BufferedReader in = new BufferedReader(files);
	    String line = readLine();  // prime the pump
	    String readLine() {
		try {
		    return in.readLine();
		} catch (IOException io) {
		    return null;
		}
	    }
	    public boolean hasNext() {
		return line != null;
	    }
	    public Object next() {
		String res = line;
		if (res == null)  throw new NoSuchElementException();
		line = readLine();
		return res;
	    }
	    public void remove() { throw new UnsupportedOperationException(); }
	};
    }

    static void assert0(boolean z) {
	if (!z) throw new RuntimeException("assert failed");
    }
}

