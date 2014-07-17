/*
 * @(#)Unpack.java	1.10 03/01/23
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
 * Read a packed file and unpack it to disk.
 * Usage:  java ...Unpack infile.pack [outdir]
 *
 * @see Pack
 * @author John Rose
 * @version 1.10, 01/23/03
 */

public
class Unpack {
    private final String USAGE = "Unpack [-j] infile.pack [outfile.jar | outdir]";

    int verbose = 0;
    boolean stripDebug = false;   // by default, keep this stuff on output
    boolean stripCompile = false; // by default, keep this stuff on output

    // I/O
    InputStream in = null;
    File outDir = null;
    JarOutputStream jarFileOutPut = null;
    
    public static void main(String av[]) throws IOException {
	new Unpack().run(av);
    }

    public void run(String av[]) throws IOException {
	boolean jarout=false;
	int n = 0;
	Iterator inFileNames = null;
	while (n < av.length) {
	    //System.out.println("av["+n+"]="+av[n]);
	    String a = av[n].intern();
	    if (a == "-v")
		++verbose;
	    else if (!a.startsWith("-") || a.equals("-"))
		break;  // ran into the input file name
	    else if (a.startsWith("-F"))
		processFlag(a.substring(2));
	    else if (a.startsWith("-j"))
		jarout=true;
	    else if (a == "-c")  // strip compilation attrs
		stripCompile = true;
	    else if (a == "-d")  // strip debugging attrs
		stripDebug = true;
	    else
		throw new RuntimeException("Bad flag "+a);
	    n++;
	}
	BandStructure.setVerbose(verbose);
	if (n < av.length) {
	    if (av[n].equals("-")) {
		in = System.in;
	    } else {
		// Open the input file lazily, to avoid null files on error.
		in = new FileInputStream(new File(av[n]));
	    }
	    ++n;
	}
	
	// Process the output directory or jar output.
	if (n < av.length) {
	    if (jarout || av[n].endsWith(".jar")) {
		try {
		    jarFileOutPut = new JarOutputStream(new FileOutputStream(av[n]));
		    jarFileOutPut.setLevel(0); // No compression
		} catch (java.io.IOException ioe) {
		    ioe.printStackTrace();
		}
	    } else {
		outDir = new File(av[n]);
		if (outDir != null && !outDir.exists()) {
		    if (!outDir.mkdir()) {
			throw new IOException("Cannot create: "+outDir);
		    }
		}
	    }
	    ++n;
	}
	
	if (in == null || n != av.length) {
	    System.out.println("Usage:  "+USAGE);
	    return;
	}

	// Here's where the bits are read from disk:
	Package pkg = new Package();
	new PackageReader(pkg, in).read();
	in.close();

	if (stripDebug)      pkg.strip("Debug");
	if (stripCompile)    pkg.strip("Compile");

	// Construct constant pools, etc.
	pkg.reconstructClasses();

	// Now write out the files.
	writeSideFiles(pkg);
	writeClassFiles(pkg);

	if (jarFileOutPut != null) {
	    jarFileOutPut.close();
	}
    }

    void writeClassFiles(Package pkg) throws IOException {
	// Do all the classes
	Package.Class[] allClasses = new Package.Class[pkg.classes.size()];
	pkg.classes.toArray(allClasses);
	//Arrays.sort(allClasses);
	for (int i = 0; i < allClasses.length; i++) {
	    Package.Class c = allClasses[i];
	    File name;
	    if (jarFileOutPut != null) {
		name = c.getFileName();
		JarEntry je = new JarEntry(getJarEntryName(name));
		if (verbose > 0) System.out.println("Writing " + je.getName());
		jarFileOutPut.putNextEntry(je);
		new ClassWriter(c, jarFileOutPut).write();
		jarFileOutPut.flush();
	    } else {
		name = c.getFileName(outDir);
		name.getParentFile().mkdirs();
		OutputStream out = new RenamingOutputStream(name);
		new ClassWriter(c, out).write();
		out.close();
		if (verbose > 0)  System.out.println("wrote "+name);
	    }
	    
	}
    }

    void writeSideFiles(Package pkg) throws IOException {
	// Do all the non-class files
	Package.File[] allFiles = new Package.File[pkg.files.size()];
	pkg.files.toArray(allFiles);
	//Arrays.sort(allFiles);
	for (int i = 0; i < allFiles.length; i++) {
	    Package.File f = allFiles[i];
	    File name;
	    if (jarFileOutPut != null) {
		name = f.getFileName();
		JarEntry je = new JarEntry(getJarEntryName(name));
		if (verbose > 0) System.out.println("Writing " + je.getName());
		jarFileOutPut.putNextEntry(je);
		jarFileOutPut.write(f.contents);
		jarFileOutPut.flush();
	    } else {
		name = f.getFileName(outDir);
		name.getParentFile().mkdirs();
		OutputStream out = new RenamingOutputStream(name);
		out.write(f.contents);
		out.close();
		if (verbose > 0)  System.out.println("wrote "+name);
	    }
	}
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
 
    
    static void assert0(boolean z) {
	if (!z) throw new RuntimeException("assert failed");
    }
    
    private static String getJarEntryName(File f) {
	return f.toString().replace(File.separatorChar,'/');
    }
}






