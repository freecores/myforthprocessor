/*
 * @(#)Code.java	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.pack;

import java.io.*;
import java.util.*;
import sun.tools.pack.Package.Class;
import java.lang.reflect.Modifier;

/**
 * Represents a chunk of bytecodes.
 * @author John Rose
 * @version 1.5, 01/23/03
 */
class Code implements Constants {
    public static int verbose = 0;

    Class.Method m;

    public Code(Class.Method m) {
	this.m = m;
    }

    public Class.Method getMethod() {
	return m;
    }
    public Class thisClass() {
	return m.thisClass();
    }
    public Package getPackage() {
	return m.thisClass().getPackage();
    }

    public ConstantPool.Entry[] getCPMap() {
	return m.getCPMap();
    }

    static private final ConstantPool.Entry[] noRefs = ConstantPool.noRefs;

    // The following fields are used directly by the ClassReader, etc.
    int max_stack;
    int max_locals;

    ConstantPool.Entry handler_class[] = noRefs;
    int handler_start[] = noInts;
    int handler_end[] = noInts;
    int handler_catch[] = noInts;

    byte[] bytes;

    int getLength() { return bytes.length; }

    int[] lineNumberTable_pc = noInts;
    int[] lineNumberTable_line = noInts;

    int[] localVariableTable_start = noInts;
    int[] localVariableTable_end = noInts;
    ConstantPool.Entry[] localVariableTable_name = noRefs;
    ConstantPool.Entry[] localVariableTable_type = noRefs;
    int[] localVariableTable_slot = noInts;

    int getHandlerCount() {
	if(D)assert0(handler_class.length == handler_start.length);
	if(D)assert0(handler_class.length == handler_end.length);
	if(D)assert0(handler_class.length == handler_catch.length);
	return handler_class.length;
    }
    void setHandlerCount(int h) {
	if (h > 0) {
	    handler_class = new ConstantPool.Entry[h];
	    handler_start = new int[h];
	    handler_end   = new int[h];
	    handler_catch = new int[h];
	    // caller must fill these in ASAP
	}
    }

    public int attrCount() {
	int na = 0;
	if (lineNumberTable_pc.length > 0)         na++;
	if (localVariableTable_start.length > 0)   na++;
	return na;
    }

    boolean strip(String what) {
	if (what == "Debug") {
	    lineNumberTable_pc = noInts;
	    lineNumberTable_line = noInts;
	    localVariableTable_start = noInts;
	    localVariableTable_end = noInts;
	    localVariableTable_name = noRefs;
	    localVariableTable_type = noRefs;
	}
	return false;
    }
    void countRefs() {
	if (verbose > 2)
	    System.out.println("Reference scan "+this);
	Class cls = thisClass();
	Package pkg = cls.getPackage();
	for (int i = 0; i < handler_class.length; i++) {
	    cls.countRef(handler_class[i]);
	}
	if (lineNumberTable_pc.length > 0) {
	    // %%% Hardwire this attribute.
	    if (false) // this attribute should be hardwired
		cls.maybeCountRef(pkg.getRefString("LineNumberTable"));
	    else
		cls.countRef(pkg.getRefString("LineNumberTable"));
	}
	if (localVariableTable_name.length > 0) {
	    // %%% Hardwire this attribute.
	    if (false) // this attribute should be hardwired
		cls.maybeCountRef(pkg.getRefString("LocalVariableTable"));
	    else
		cls.countRef(pkg.getRefString("LocalVariableTable"));
	}
	for (int i = 0; i < localVariableTable_name.length; i++) {
	    cls.countRef(localVariableTable_name[i]);
	    cls.countRef(localVariableTable_type[i]);
	}
	countBytecodeRefs();
    }
    protected void countBytecodeRefs() { // see override in PackageReader
	Class cls = thisClass();
	ConstantPool.Entry[] cpMap = getCPMap();
	for (Instruction i = instructionAt(0); i != null; i = i.next()) {
	    if (verbose > 4)
		System.out.println(i);
	    int cpref = i.getCPIndex();
	    if (cpref >= 0) {
		cls.countRef(cpMap[cpref]);
	    }
	}
    }

    Instruction instructionAt(int pc) {
	return Instruction.at(bytes, pc);
    }

    static boolean flagsRequireCode(int flags) {
	// A method's flags force it to have a Code attribute,
	// if the flags are neither native nor abstract.
	return (flags & (Modifier.NATIVE | Modifier.ABSTRACT)) == 0;
    }

    public String toString() {
	return m+".Code";
    }

    /// Fetching values from my own array.
    public int getInt(int pc)    { return Instruction.getInt(bytes, pc); }
    public int getShort(int pc)  { return Instruction.getShort(bytes, pc); }
    public int getByte(int pc)   { return Instruction.getByte(bytes, pc); }
    void setInt(int pc, int x)   { Instruction.setInt(bytes, pc, x); }
    void setShort(int pc, int x) { Instruction.setShort(bytes, pc, x); }
    void setByte(int pc, int x)  { Instruction.setByte(bytes, pc, x); }

    static void assert0(boolean z) {
	if (!z) throw new RuntimeException("assert failed");
    }
}
