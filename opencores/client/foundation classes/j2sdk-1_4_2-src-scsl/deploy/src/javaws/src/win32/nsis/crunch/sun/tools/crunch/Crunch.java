/*
 * @(#)Crunch.java	1.3 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.tools.crunch;

/**
 * Program to transform a JAR file into a compact form.
 * 
 * @author Graham Hamilton
 */

import java.io.*;
import java.util.*;
import java.util.zip.*;

public class Crunch {

    public final static int CRUNCH_MAGIC = 0x4b474832;
    public final static int CRUNCH_VERSION = 19;

    private static String lastFileDir = "";

    private static String mapFileName(String name) {
	int ix = name.lastIndexOf("/");
	String prefix = name.substring(0,ix+1);
	String result = "";
	String tail = name.substring(ix+1);

	if (prefix.equals(lastFileDir)) {
	    return tail;
	}
	for (;;) {
	    if (prefix.startsWith(lastFileDir)) {
		break;
	    }
	    ix = lastFileDir.lastIndexOf("/", lastFileDir.length()-2);
	    if (ix >= 0) {
	        lastFileDir = lastFileDir.substring(0,ix+1);
	    } else {
		lastFileDir = "";
	    }
	    result = result + "../";
	}

	prefix = prefix.substring(lastFileDir.length());
	result = result + prefix + tail;
	lastFileDir = lastFileDir + prefix;

	return result;
    }

    /**
     * Write the filename.
     */
    private static void writeFilename(DataOutputStream out, String fname) 
						throws IOException {

	if (fname.length() > 255) {
	    throw new Error("filename too long: " + fname);
	}
	out.writeByte(fname.length());
	for (int i = 0; i < fname.length(); i++) {
	    short ch = (short)fname.charAt(i);
	    if (ch <= 0 || ch >= 128) {
	        throw new Error("Non-ascii character in writeFilename " + fname);
	    }
	    out.writeByte(ch);
	}
    }

    // We do a warmup pass to read all the constants into the 
    // global constant pool.
    private static void warmup(String fname) throws IOException {
	FileInputStream fin = new FileInputStream(fname);
	BufferedInputStream bin = new BufferedInputStream(fin);
	ZipInputStream zin = new ZipInputStream(bin);

	ByteArrayOutputStream byteOut = new ByteArrayOutputStream(32000);

	for (;;) {
	    ZipEntry entry = zin.getNextEntry();
	    if (entry == null) {
		break;
	    }
	    String name = entry.getName();

	    if (name.endsWith(".class")) {
		// Read the classfile.
	        ClassFile cf = ClassFile.read(name, zin);

		// Write the crunched classfile into the byte array
		// and then discard it.
		byteOut.reset();
	        DataOutputStream dout = new DataOutputStream(byteOut);
	        cf.write(dout);
		dout.close();
	    }
	}
	zin.close();
    }

    private static void doit(String fname) throws IOException {
	FileInputStream fin = new FileInputStream(fname);
	BufferedInputStream bin = new BufferedInputStream(fin);
	ZipInputStream zin = new ZipInputStream(bin);
	int fileCount = 0;
        int totalFileNameLength = 0;
	int randomDataCount = 0;

 	// // Do a warmup pass to read and process all the classfiles
	// // so we get all the constants ibnto the global pool.
	// System.err.println("Doing warmup pass");
	// warmup(fname);

	// We write the crunched classfiles to a temporary file.
	// After we have processed the entire zip we write the
	// constant pools to the output file and then copy the
	// teporary file to the end of the output file.
	String tname = "crunch.tmp";
	String oname = "crunch.out";
	if (fname.endsWith(".jar")) {
	    tname = fname.substring(0, fname.length()-4) + ".tmp";
	    oname = fname.substring(0, fname.length()-4) + ".out";
	}
	FileOutputStream fout = new FileOutputStream(tname);
	DataOutputStream out = new DataOutputStream(new BufferedOutputStream(fout));
	ByteArrayOutputStream byteOut = new ByteArrayOutputStream(32000);
	byte buff[] = new byte[8096];

	System.err.println("Processing class files");

	for (;;) {
	    ZipEntry entry = zin.getNextEntry();
	    if (entry == null) {
		break;
	    }
	    String name = entry.getName();

	    if (name.endsWith("/")) {
		// Ignore directories.
		continue;
	    }
	    // System.out.println("    " + name);
	    fileCount++;

	    // Remap the filename, relative to the last directory.
	    name = mapFileName(name);

	    // Get the processed file into a byte array.
	    if (name.endsWith(".class")) {

		// Read the classfile.
	        ClassFile cf = ClassFile.read(name, zin);

		// Write the crunched classfile into the byte array.
	        DataOutputStream dout = new DataOutputStream(byteOut);
	        cf.write(dout);
		dout.flush();

	        // Write the (positive) length to our output stream.
	        byte data[] = byteOut.toByteArray();
	        out.writeInt(data.length);

	 	// Write the filename without the ".class" part
		name = name.substring(0, name.length()-6);
		writeFilename(out, name);
	        totalFileNameLength += 1 + name.length();

	        // Write the byte array to our output stream.
	        out.write(data);
	        byteOut.reset();
	    } else {
		// Simple data file.

		for (;;) {
		    int rc = zin.read(buff);
		    if (rc <= 0) {
			break;
		    }
		    byteOut.write(buff, 0, rc);
		    randomDataCount += rc;
		}

		// Write a negative length to indicate it is data.
	        byte data[] = byteOut.toByteArray();
	        out.writeInt(-data.length);

	 	// Write the filename
		writeFilename(out, name);
	        totalFileNameLength += 1 + name.length();

	        // Write the byte array to our output stream.
	        out.write(data);
	        byteOut.reset();
	    }

	}
	out.close();
	zin.close();

	System.out.println("================================================================");
	ClassFile.summarize();
	System.out.println("Total files = " + fileCount);
	System.out.println("Total non-classfile data = " + randomDataCount);
	System.out.println("Total file name length = " + totalFileNameLength);
	System.out.println("================================================================");

	System.err.println("Writing global pool");

	// Write the global tables to the final output file.
	fout = new FileOutputStream(oname);
	out = new DataOutputStream(new BufferedOutputStream(fout));
	out.writeInt(CRUNCH_MAGIC);
	out.writeInt(CRUNCH_VERSION);
	out.writeInt(fileCount);
	out.writeInt(0);	// Will hold global tables length;
	GlobalTables.write(out);
	out.flush();
	fout.close();

	// Update the output file header with the pool length
	int headerLength = (4*4) + (5*2);
	int poolLength = (int)((new File(oname)).length() - headerLength);
	System.out.println("Actual constant pool length = " + poolLength);
	RandomAccessFile raf = new RandomAccessFile(oname, "rw");
	raf.seek(12);
	raf.writeInt(poolLength);
	// and seek to the end:
	raf.seek(headerLength + poolLength);

	// Copy the temporary file to the end of the final output file.
	fin = new FileInputStream(tname);
	for (;;) {
	    int rc = fin.read(buff);
	    if (rc <= 0) {
		break;
	    }
	    raf.write(buff, 0, rc);
	}
	raf.close();
	fin.close();
	(new File(tname)).delete();

	System.out.println("=================================================================");
	File f = new File(fname);
	System.out.println("Input Jar file size = " + f.length() + " bytes");
	f = new File(oname);
	System.out.println("Output Crunch file size = " + f.length() + " bytes");
	System.out.println("=================================================================");
    }

    public static void main(String argv[]) {
	try {
	    for (int i = 0; i < argv.length; i++) {
	        String fname = argv[i];
	        doit(fname);
	    }
	} catch (Throwable th) {
	    System.out.println("Caught: " + th);
	    th.printStackTrace();
	}
    }
}
