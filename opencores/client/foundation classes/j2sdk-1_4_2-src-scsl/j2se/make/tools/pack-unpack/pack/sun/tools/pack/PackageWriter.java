/*
 * @(#)PackageWriter.java	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.pack;

import java.io.*;
import java.util.*;
import sun.tools.pack.Package.Class;
import sun.tools.pack.Package.InnerClass;
import sun.tools.pack.ConstantPool.*;

/**
 * Writer for a package file.
 * @author John Rose
 * @version 1.8, 01/23/03
 */
class PackageWriter extends BandStructure {
    Package pkg;
    OutputStream finalOut;

    PackageWriter(Package pkg, OutputStream out) throws IOException {
	super(false);
	this.pkg = pkg;
	this.finalOut = out;
    }

    Utf8Entry ensureUtf8Entry(String s) {
	return pkg.cp.ensureUtf8Entry(s);
    }
    Index getCPIndex(byte tag) {
	return pkg.cp.getIndexByTag(tag);
    }

    void write() throws IOException {
	if (verbose > 0) {
	    System.out.println("packBetter,selfCall,initCall,allBands = "+true);
	}
	setBandIndexes();
	boolean ok = false;
	try {
	    writeMagicNumbers();
	    writeConstantPool();
	    writeClasses();
	    writeAttributes();

	    if (verbose > 1)  printCodeHist();

	    writeAllBandsTo(finalOut);
	    ok = true;
	} catch (Exception ee) {
	    if (!ok) {
		System.out.println("*** Error on output: "+ee);
		if (verbose > 0)  ee.printStackTrace();
		// Write partial output only if we are verbose.
		if (verbose > 0)  finalOut.close();
	    }
	}
    }

    void writeMagicNumbers() throws IOException {
	pkg.checkVersion();
	package_magic.putInt32(pkg.magic);
	package_magic.putShort(pkg.package_minor_version);
	package_magic.putShort(pkg.package_major_version);
    }

// Note:  Keeping the data separate in passes (or "bands") allows the
// compressor to issue significantly shorter indexes for repeated data.
// The difference in zipped size is 4%, which is remarkable since the
// unzipped sizes are the same (only the byte order differs).

// After moving similar data into bands, it becomes natural to delta-encode
// each band.  (This is especially useful if we sort the constant pool first.)
// Delta encoding saves an extra 5% in the output size (13% of the CP itself).
// Because a typical delta usees much less data than a byte, the savings after
// zipping is even better:  A zipped delta-encoded package is 8% smaller than
// a zipped non-delta-encoded package.  Thus, in the zipped file, a banded,
// delta-encoded constant pool saves over 11% (of the total file size) compared
// with a zipped unbanded file.

    void writeConstantPool() throws IOException {
	ConstantPool cp = pkg.cp;
	// No more changes, now.
	cp.setNotPermutable();
	cp.setFrozen();

	if (verbose > 0)  System.out.println("writing CP");

	for (int k = 0; k < ConstantPool.TAGS_IN_ORDER.length; k++) {
	    byte  tag     = ConstantPool.TAGS_IN_ORDER[k];
	    Index index   = cp.getIndexByTag(tag);

	    // Write the quantity of each tag.
	    package_header.putUnsigned(index.size());

	    Entry[] cpMap = index.cpMap;
	    String tagName = ConstantPool.tagName(tag);
	    if (verbose > 0)
		System.out.println("Write "+index);

	    switch (tag) {
	    case CONSTANT_Utf8:
		writeUtf8Values(cpMap);
		break;
	    case CONSTANT_Integer:
		for (int i = 0; i < cpMap.length; i++) {
		    NumberEntry e = (NumberEntry) cpMap[i];
		    int x = ((Integer)e.numberValue()).intValue();
		    cp_Int.putInt(x);
		}
		break;
	    case CONSTANT_Float:
		for (int i = 0; i < cpMap.length; i++) {
		    NumberEntry e = (NumberEntry) cpMap[i];
		    float fx = ((Float)e.numberValue()).floatValue();
		    int x = Float.floatToIntBits(fx);
		    cp_Float.putInt(x);
		}
		break;
	    case CONSTANT_Long:
		for (int i = 0; i < cpMap.length; i++) {
		    NumberEntry e = (NumberEntry) cpMap[i];
		    long x = ((Long)e.numberValue()).longValue();
		    cp_Long_hi.putInt((int)(x >>> 32));
		    cp_Long_lo.putInt((int)(x >>> 0));
		}
		break;
	    case CONSTANT_Double:
		for (int i = 0; i < cpMap.length; i++) {
		    NumberEntry e = (NumberEntry) cpMap[i];
		    double dx = ((Double)e.numberValue()).doubleValue();
		    long x = Double.doubleToLongBits(dx);
		    cp_Double_hi.putInt((int)(x >>> 32));
		    cp_Double_lo.putInt((int)(x >>> 0));
		}
		break;
	    case CONSTANT_String:
		for (int i = 0; i < cpMap.length; i++) {
		    StringEntry e = (StringEntry) cpMap[i];
		    cp_String.putRef(e.ref);
		}
		break;
	    case CONSTANT_Class:
		for (int i = 0; i < cpMap.length; i++) {
		    ClassEntry e = (ClassEntry) cpMap[i];
		    cp_Class.putRef(e.ref);
		}
		break;
	    case CONSTANT_Signature:
		for (int i = 0; i < cpMap.length; i++) {
		    SignatureEntry e = (SignatureEntry) cpMap[i];
		    cp_Signature_form.putRef(e.formRef);
		    for (int j = 0; j < e.classRefs.length; j++) {
			cp_Signature_classes.putRef(e.classRefs[j]);
		    }
		}
		break;
	    case CONSTANT_NameandType:
		for (int i = 0; i < cpMap.length; i++) {
		    DescriptorEntry e = (DescriptorEntry) cpMap[i];
		    cp_Descr_name.putRef(e.nameRef);
		    cp_Descr_type.putRef(e.typeRef);
		}
		break;
	    case CONSTANT_Fieldref:
		writeMemberRefs(tag, cpMap, cp_Field_class, cp_Field_desc);
		break;
	    case CONSTANT_Methodref:
		writeMemberRefs(tag, cpMap, cp_Method_class, cp_Method_desc);
		break;
	    case CONSTANT_InterfaceMethodref:
		writeMemberRefs(tag, cpMap, cp_Imethod_class, cp_Imethod_desc);
		break;
	    default:
		if(D)assert0(false);
	    }
	}
    }

    void writeUtf8Values(Entry[] cpMap) throws IOException {
	// Fetch the char arrays, first of all.
	char[][] chars = new char[cpMap.length][];
	for (int i = 0; i < chars.length; i++) {
	    chars[i] = cpMap[i].stringValue().toCharArray();
	}

	// First band:  Write lengths of shared prefixes.
	int[] prefixes = new int[cpMap.length];
	char[] prevChars = {};
	for (int i = 0; i < chars.length; i++) {
	    int prefix = 0;
	    char[] curChars = chars[i];
	    int limit = Math.min(curChars.length, prevChars.length);
	    while (prefix < limit && curChars[prefix] == prevChars[prefix])
		prefix++;
	    prefixes[i] = prefix;
	    cp_Utf8_prefix.putUnsigned(prefix);
	    prevChars = curChars;
	}

	// Second band:  Write lengths of unshared suffixes.
	// Third band:  Write the char values in the unshared suffixes.
	for (int i = 0; i < chars.length; i++) {
	    char[] str = chars[i];
	    int prefix = prefixes[i];
	    int suffix = str.length - prefixes[i];
	    boolean isPacked = false;
	    if (suffix == 0) {
		// Zero suffix length is special flag to indicate
		// separate treatment in cp_Utf8_big bands.
		// This suffix length never occurs naturally,
		// except in the one case of a zero-length string.
		// (If it occurs, it is the first, due to sorting.)
		// The zero length string must, paradoxically, be
		// encoded as a zero-length cp_Utf8_big band.
		// This wastes exactly (& tolerably) one null byte.
		isPacked = true;
		// Do not bother to add an empty "(Utf8_big_0)" band.
	    } else if (Pack.bigStrings && Pack.effort > 1 && suffix > 100) {
		int numWide = 0;
		for (int n = 0; n < suffix; n++) {
		    if (str[prefix+n] > 127) {
			numWide++;
		    }
		}
		if (numWide > 100) {
		    // Try packing the chars with an alternate encoding.
		    int[] cvals = new int[suffix];
		    for (int n = 0; n < suffix; n++) {
			cvals[n] = str[prefix+n];
		    }
		    CodingChooser cc = getCodingChooser();
		    Coding bigRegular = cp_Utf8_big_chars.regularCoding;
		    int[] size = { 0, 0 };
		    Coding special = cc.choose(cvals, bigRegular, size);
		    Coding charRegular = cp_Utf8_chars.regularCoding;
		    if (verbose > 1)
			System.out.println("big string["+i+"] len="+suffix+" #wide="+numWide+" size="+size[0]+"/"+size[1]+" coding "+special);
		    if (special != charRegular) {
			int specialZipSize = size[1];
			int[] normalSize = cc.computeSize(charRegular, cvals);
			int normalZipSize = normalSize[1];
			int minWin = Math.max(5, normalZipSize/1000);
			if (verbose > 1)
			    System.out.println("big string["+i+"] normalSize="+normalSize[0]+"/"+normalSize[1]+" win="+(specialZipSize<normalZipSize-minWin));
			if (specialZipSize < normalZipSize-minWin) {
			    IntBand big = cp_Utf8_big_chars.newIntBand("(Utf8_big_"+i+")");
			    big.initializeValues(cvals);
			    isPacked = true;
			}
		    }
		}
	    }
	    if (isPacked) {
		// Mark packed string with zero-length suffix count.
		// This tells the unpacker to go elsewhere for the suffix bits.
		// Fourth band:  Write unshared suffix with alternate coding.
		cp_Utf8_suffix.putUnsigned(0);
		cp_Utf8_big_length.putUnsigned(suffix);
	    } else {
		if(D)assert0(suffix != 0);  // would be ambiguous
		// Normal string.  Save suffix in third and fourth bands.
		cp_Utf8_suffix.putUnsigned(suffix);
		for (int n = 0; n < suffix; n++) {
		    int ch = str[prefix+n];
		    cp_Utf8_chars.putUnsigned(ch);
		}
	    }
	}
	if (verbose > 0) {
	    int normCharCount = cp_Utf8_chars.length();
	    int packCharCount = cp_Utf8_big_chars.length();
	    int charCount = normCharCount + packCharCount;
	    System.out.println("Utf8string #CHARS="+charCount+" #PACKEDCHARS="+packCharCount);
	}
    }

    void writeMemberRefs(byte tag, Entry[] cpMap, CPRefBand cp_class, CPRefBand cp_desc) throws IOException {
	for (int i = 0; i < cpMap.length; i++) {
	    MemberEntry e = (MemberEntry) cpMap[i];
	    cp_class.putRef(e.classRef);
	    cp_desc.putRef(e.descRef);
	}
    }

    void writeClasses() throws IOException {
	package_header.putUnsigned(pkg.default_class_minor_version);
	package_header.putUnsigned(pkg.default_class_major_version);
	package_header.putUnsigned(pkg.classes.size());
	package_header.putUnsigned(pkg.allInnerClasses.size());
	Class[] classes = new Class[pkg.classes.size()];
	pkg.classes.toArray(classes);
	//Arrays.sort(classes);  // respect order in which caller put classes
	if (verbose > 0)
	    System.out.println("  ...scanning "+classes.length+" classes...");
	int nwritten = 0;
	for (int i = 0; i < classes.length; i++) {
	    // Collect the class body, sans bytecodes.
	    if (verbose > 1)
		System.out.println("Scanning "+classes[i]);
	    writeClass(classes[i]);
	    nwritten++;
	    if (verbose > 0 && (nwritten % 1000) == 0)
		System.out.println("Have scanned "+nwritten+" classes...");
	}

	// InnerClasses are specially treated at the package level.
	InnerClass[] innerClasses = new InnerClass[pkg.allInnerClasses.size()];
	pkg.allInnerClasses.toArray(innerClasses);
	// Note that this puts the InnerClasses attribute into a valid order,
	// where A$B must always occur earlier than A$B$C.  This is an
	// important side-effect of sorting lexically by inner class name.
	Arrays.sort(innerClasses);
	// About 30% of inner classes are anonymous (in rt.jar).
	// About 60% are class members; the rest are named locals.
	// Nearly all have predictable outers and names.
	for (int i = 0; i < innerClasses.length; i++) {
	    InnerClass ic = innerClasses[i];
	    int flags = ic.flags;
	    if(D)assert0((flags & ACC_IC_LONG_FORM) == 0);
	    if (!ic.predictable) {
		flags |= ACC_IC_LONG_FORM;
	    }
	    ic_this_class.putRef(ic.thisClass);
	    ic_flags.putFlags(flags, 0);
	    if (!ic.predictable) {
		ic_outer_class.putRef(ic.outerClass);
		ic_name.putRef(ic.name);
	    }
	}
    }

    void writeClass(Class cls) throws IOException {
	// Ready to write class header.
	int na = cls.attrCount();
	class_flags.putFlags(cls.flags, na);
	class_this.putRef(cls.thisClass);
	class_super.putRef(cls.superClass);
	class_interface_count.putUnsigned(cls.interfaces.length);
	for (int i = 0; i < cls.interfaces.length; i++) {
	    class_interface.putRef(cls.interfaces[i]);
	}

	Class.Field[] fields = cls.getFields();
	class_field_count.putUnsigned(fields.length);
	for (int i = 0; i < fields.length; i++) {
	    writeField(fields[i]);
	}

	Class.Method[] methods = cls.getMethods();
	class_method_count.putUnsigned(methods.length);
	for (int i = 0; i < methods.length; i++) {
	    writeMethod(methods[i]);
	}

	if (na > 0)
	    class_attr_count.putUnsigned(na);
	if (cls.retroVersion != null) {
	    AttrBlock block = new AttrBlock(ensureUtf8Entry(CLASS_FILE_VERSION));
	    block.putSingleRef(cls.retroVersion, CONSTANT_Integer);
	    block.putAttrTo(class_attrs);
	    na--;
	}
	na = writeMiscAttrs(na, cls.isDeprecated, cls.isSynthetic, class_attrs);
	if (cls.sourceFile != null) {
	    AttrBlock block = new AttrBlock(ensureUtf8Entry("SourceFile"));
	    block.putSingleRef(cls.sourceFile, CONSTANT_Utf8);
	    block.putAttrTo(class_attrs);
	    na--;
	}
	if(D)assert0(na == 0);  // correct attribute count
    }

    int writeMiscAttrs(int na, boolean isDeprecated, boolean isSynthetic, AttrBands attr) throws IOException {
	if (isDeprecated) {
	    AttrBlock block = new AttrBlock(ensureUtf8Entry("Deprecated"));
	    block.putAttrTo(attr);
	    na--;
	}
	if (isSynthetic) {
	    AttrBlock block = new AttrBlock(ensureUtf8Entry("Synthetic"));
	    block.putAttrTo(attr);
	    na--;
	}
	return na;
    }

    void writeField(Class.Field m) throws IOException {
	int na = m.attrCount();
	int flags = m.flags;
	if(D)assert0((flags & ACC_CONSTANT_VALUE) == 0);  //unused bit!
	if (m.constantValue != null) {
	    na--;  // do not output ConstantValue as an attribute
	    flags |= ACC_CONSTANT_VALUE;
	}
	field_flags.putFlags(flags, na);
	field_descr.putRef(m.descriptor);
	LiteralEntry cv = m.constantValue;
	if (cv != null) {
	    byte tag = m.descriptor.getLiteralTag();
	    Index ix = pkg.cp.getIndexByTag(tag);
	    field_constant_value.putRef(cv, ix);
	}
	if (na > 0)
	    field_attr_count.putUnsigned(na);
	na = writeMiscAttrs(na, m.isDeprecated, m.isSynthetic, field_attrs);
	if(D)assert0(na == 0);  // correct attribute count
    }

    void writeMethod(Class.Method m) throws IOException {
	int na = m.attrCount();
	int flags = m.flags;
	if(D)assert0((flags & ACC_EXCEPTIONS) == 0);  //unused bit!
	if (m.exceptions.length > 0) {
	    na--;  // do not output Exception as an attribute
	    flags |= ACC_EXCEPTIONS;
	}
	boolean hasCode = Code.flagsRequireCode(flags);
	if (hasCode) {
	    na--;  // do not output Code as an attribute
	    if(D)assert0(m.code != null);
	} else {
	    if(D)assert0(m.code == null);
	}
	method_flags.putFlags(flags, na);
	method_descr.putRef(m.descriptor);
	if (na > 0)
	    method_attr_count.putUnsigned(na);
	na = writeMiscAttrs(na, m.isDeprecated, m.isSynthetic, method_attrs);
	if(D)assert0(na == 0);  // correct attribute count
	if (m.exceptions.length > 0) {
	    method_exception_count.putUnsigned(m.exceptions.length);
	    for (int i = 0; i < m.exceptions.length; i++) {
		method_exceptions.putRef(m.exceptions[i]);
		// Merge w/ code_handler_catch?  Worth << 0.1% (zipped).
	    }
	}
	if (hasCode) {
	    // Output the code block, except for the actual bytecodes
	    writeCodeHeader(m.code);
	    // Output the actual bytecodes.
	    writeCodeBytes(m.code);
	}
    }

    void writeCodeHeader(Code code) throws IOException {
	int na = code.attrCount();
	int sc = (na > 0) ? 0 : shortCodeHeader(code);
	if (verbose > 2) {
	    int siglen = code.getMethod().getArgumentSize();
	    System.out.println("Code sizes info "+code.max_stack+" "+code.max_locals+" "+code.getHandlerCount()+" "+siglen+" "+na+(sc > 0 ? " SHORT="+sc : ""));
	}
	if(D)assert0(sc == (sc & 0xFF));
	code_headers.putInt(sc);
	if (sc == 0) {
	    code_max_stack.putUnsigned(code.max_stack);
	    code_max_locals.putUnsigned(code.max_locals);
	    code_handler_count.putUnsigned(code.getHandlerCount());
	    code_attr_count.putUnsigned(na);  // even if na==0
	} else {
	    if(D)assert0(na == 0);
	    if(D)assert0(code.getHandlerCount() < shortCodeHeader_h_limit);
	}
	int del, mod = code.getLength();
	for (int i = 0; i < code.getHandlerCount(); i++) {
	    int start = code.handler_start[i];
	    int end   = code.handler_end[i];
	    int catsh = code.handler_catch[i];
	    // Encode end as offset from start, and catch as offset from end,
	    // because they are strongly correlated.
	    code_handler_start.putUnsigned(start);
	    del = (end - start);
	    code_handler_end.putUnsigned(del >= 0 ? del : del+mod);
	    del = (catsh - end);
	    code_handler_catch.putUnsigned(del >= 0 ? del : del+mod);
	    code_handler_class.putRef(code.handler_class[i]); // null OK
	}
	if (code.lineNumberTable_pc.length > 0) {
	    AttrBlock block = new AttrBlock(ensureUtf8Entry("LineNumberTable"));
	    block.putUnsigned(code.lineNumberTable_pc.length);
	    for (int i = 0; i < code.lineNumberTable_pc.length; i++) {
		block.putUnsigned(code.lineNumberTable_pc[i]);
		block.putUnsigned(code.lineNumberTable_line[i]);
	    }
	    block.putAttrTo(code_attrs);
	    na--;
	}
	if (code.localVariableTable_start.length > 0) {
	    AttrBlock block = new AttrBlock(ensureUtf8Entry("LocalVariableTable"));
	    block.putUnsigned(code.localVariableTable_start.length);
	    for (int i = 0; i < code.localVariableTable_start.length; i++) {
		int tem;
		block.putUnsigned(tem = code.localVariableTable_start[i]);
		block.putUnsigned(code.localVariableTable_end[i] - tem);
		block.putSingleRef(code.localVariableTable_name[i], CONSTANT_Utf8);
		block.putSingleRef(code.localVariableTable_type[i], CONSTANT_Utf8);
		block.putUnsigned(code.localVariableTable_slot[i]);
	    }
	    block.putAttrTo(code_attrs);
	    na--;
	}
	if(D)assert0(na == 0);
    }

    // Write package attributes.
    void writeAttributes() throws IOException {
	// Precount the attributes:
	int na = pkg.attrCount();
	// Now output the attributes:
	package_header.putUnsigned(na);
	// output one attribute per file
	int numFiles = pkg.files.size();
	if (numFiles > 0) {
	    Package.File[] files = new Package.File[numFiles];
	    pkg.files.toArray(files);
	    Arrays.sort(files);
	    for (int i = 0; i < files.length; i++) {
		package_attrs.name.putRef(files[i].name);
		byte[] x = files[i].contents;
		package_attrs.size.putUnsigned(x.length);
		package_attrs.bits.collectorStream().write(x);
		if (verbose > 1)
		    System.out.println("Wrote "+x.length+" bytes of "+files[i].name.stringValue());
		na--;
	    }
	    if (verbose > 0)
		System.out.println("Wrote "+numFiles+" side files");
	}
	if(D)assert0(na == 0);  // correct attribute count
    }

    // temporary scratch variables for processing code blocks
    private Code                 curCode;
    private Class                curClass;
    private Entry[] curCPMap;
    private void beginCode(Code c) {
	if(D)assert0(curCode == null);
	curCode = c;
	curClass = c.m.thisClass();
	curCPMap = c.getCPMap();
    }
    private void endCode() {
	curCode = null;
	curClass = null;
	curCPMap = null;
    }

    // Return an _invokeinit_op variant, if the instruction matches one,
    // else -1.
    private int initOpVariant(Instruction i, Entry newClass) {
	if (i.getBC() != _invokespecial)  return -1;
	MemberEntry ref = (MemberEntry) i.getCPRef(curCPMap);
	if (ref.descRef.nameRef.stringValue() != "<init>")
	    return -1;
	ClassEntry refClass = ref.classRef;
	if (refClass == curClass.thisClass)
	    return _invokeinit_op+_invokeinit_self_option;
	if (refClass == curClass.superClass)
	    return _invokeinit_op+_invokeinit_super_option;
	if (refClass == newClass)
	    return _invokeinit_op+_invokeinit_new_option;
	return -1;
    }

    // Return a _self_linker_op variant, if the instruction matches one,
    // else -1.
    private int selfOpVariant(Instruction i) {
	int bc = i.getBC();
	if (!(bc >= _first_linker_op && bc <= _last_linker_op))  return -1;
	MemberEntry ref = (MemberEntry) i.getCPRef(curCPMap);
	ClassEntry refClass = ref.classRef;
	int self_bc = _self_linker_op + (bc - _first_linker_op);
	if (refClass == curClass.thisClass)
	    return self_bc;
	if (refClass == curClass.superClass)
	    return self_bc + _self_linker_super_flag;
	return -1;
    }

    void writeCodeBytes(Code code) throws IOException {
	beginCode(code);
	ConstantPool cp = pkg.cp;

	// true if the previous instruction is an aload to absorb
	boolean prevAload = false;

	// class of most recent new; helps compress <init> calls
	Entry newClass = null;

	for (Instruction i = code.instructionAt(0); i != null; i = i.next()) {
	    if (verbose > 3)  System.out.println(i);

	    if (i.isWide()) {
		if (verbose > 1) {
		    System.out.println("_wide opcode in "+code);
		    System.out.println(i);
		}
		bc_codes.putByte(_wide);
		codeHist[_wide]++;
	    }

	    int bc = i.getBC();

	    // Begin "bc_linker" compression.
	    if (bc == _aload_0) {
		// Try to group aload_0 with a following operation.
		Instruction ni = code.instructionAt(i.getNextPC());
		if (selfOpVariant(ni) >= 0) {
		    prevAload = true;
		    continue;
		}
	    }

	    // Test for <init> invocations:
	    int init_bc = initOpVariant(i, newClass);
	    if (init_bc >= 0) {
		if (prevAload) {
		    // get rid of it
		    bc_codes.putByte(_aload_0);
		    codeHist[_aload_0]++;
		    prevAload = false;  //used up
		}
		// Write special bytecode.
		bc_codes.putByte(init_bc);
		codeHist[init_bc]++;
		MemberEntry ref = (MemberEntry) i.getCPRef(curCPMap);
		// Write operand to a separate band.
		int coding = cp.getOverloadingIndex(ref);
		bc_initref.putUnsigned(coding);
		continue;
	    }

	    int self_bc = selfOpVariant(i);
	    if (self_bc >= 0) {
		boolean isField = Instruction.isFieldOp(bc);
		boolean isSuper = (self_bc >= _self_linker_op+_self_linker_super_flag);
		boolean isAload = prevAload;
		prevAload = false;  //used up
		if (isAload)
		    self_bc += _self_linker_aload_flag;
		// Write special bytecode.
		bc_codes.putByte(self_bc);
		codeHist[self_bc]++;
		// Write field or method ref to a separate band.
		MemberEntry ref = (MemberEntry) i.getCPRef(curCPMap);
		CPRefBand bc_which = selfOpRefBand(self_bc);
		Index which_ix = cp.getMemberIndex(ref.tag, ref.classRef);
		bc_which.putRef(ref, which_ix);
		continue;
	    }
	    if(D)assert0(!prevAload);
	    // End "bc_linker" compression.

	    // Normal bytecode.
	    codeHist[bc]++;
	    switch (bc) {
	    case _tableswitch: // apc:  (df, lo, hi, (hi-lo+1)*(label))
	    case _lookupswitch: // apc:  (df, nc, nc*(case, label))
		bc_codes.putByte(bc);
		Instruction.Switch isw = (Instruction.Switch) i;
		// Note that we do not write the alignment bytes.
		int apc = isw.getAlignedPC();
		int npc = isw.getNextPC();
		// write a length specification into the bytecode stream
		int caseCount = isw.getCaseCount();
		int pc = isw.getPC();
		bc_case_count.putUnsigned(caseCount);
		bc_label.putSigned(isw.getDefaultLabel()-pc);
		for (int j = 0; j < caseCount; j++) {
		    bc_label.putSigned(isw.getCaseLabel(j)-pc);
		}
		// Just dump the values to the instruction stream:
		if (bc == _tableswitch) {
		    bc_case_value.putSigned(isw.getCaseValue(0));
		} else {
		    for (int j = 0; j < caseCount; j++) {
			bc_case_value.putSigned(isw.getCaseValue(j));
		    }
		}
		// Done with the switch.
		continue;
	    }

	    int branch = i.getBranchLabel();
	    if (branch >= 0) {
		bc_codes.putByte(bc);
		int offset = branch - i.getPC();
		bc_label.putSigned(offset);
		continue;
	    }
	    Entry ref = i.getCPRef(curCPMap);
	    if (ref != null) {
		if (bc == _new)  newClass = ref;
		if (bc == _ldc)  ldcHist[ref.tag]++;
		CPRefBand bc_which;
		int vbc = bc;
		switch (i.getCPTag()) {
		case CONSTANT_Literal:
		    switch (ref.tag) {
		    case CONSTANT_Integer:
			bc_which = bc_intref;
			switch (bc) {
			case _ldc:    vbc = _ildc; break;
			case _ldc_w:  vbc = _ildc_w; break;
			default:      if(D)assert0(false);
			}
			break;
		    case CONSTANT_Float:
			bc_which = bc_floatref;
			switch (bc) {
			case _ldc:    vbc = _fldc; break;
			case _ldc_w:  vbc = _fldc_w; break;
			default:      if(D)assert0(false);
			}
			break;
		    case CONSTANT_Long:
			bc_which = bc_longref;
			if(D)assert0(bc == _ldc2_w);
			vbc = _lldc2_w;
			break;
		    case CONSTANT_Double:
			bc_which = bc_doubleref;
			if(D)assert0(bc == _ldc2_w);
			vbc = _dldc2_w;
			break;
		    case CONSTANT_String:
			bc_which = bc_stringref;
			switch (bc) {
			case _ldc:    vbc = _aldc; break;
			case _ldc_w:  vbc = _aldc_w; break;
			default:      if(D)assert0(false);
			}
			break;
		    default:
			bc_which = null;
			if(D)assert0(false);
		    }
		    break;
		case CONSTANT_Class:
		    // Use a special shorthand for the current class:
		    if (ref == curClass.thisClass)  ref = null;
		    bc_which = bc_classref; break;
		case CONSTANT_Fieldref:
		    bc_which = bc_fieldref; break;
		case CONSTANT_Methodref:
		    bc_which = bc_methodref; break;
		case CONSTANT_InterfaceMethodref:
		    bc_which = bc_imethodref; break;
		default:
		    bc_which = null;
		    if(D)assert0(false);
		}
		bc_codes.putByte(vbc);
		bc_which.putRef(ref);
		// handle trailing junk
		if (bc == _multianewarray) {
		    if(D)assert0(i.getConstant() == code.getByte(i.getPC()+3));
		    // Just dump the byte into the bipush pile
		    bc_byte.putUnsigned(0xFF & i.getConstant());
		} else if (bc == _invokeinterface) {
		    if(D)assert0(i.getLength() == 5);
		    // Make sure the discarded bytes are sane:
		    if(D)assert0(i.getConstant() == (1+((MemberEntry)ref).descRef.typeRef.computeSize(true)) << 8);
		} else {
		    // Make sure there is nothing else to write.
		    if(D)assert0(i.getLength() == ((bc == _ldc)?2:3));
		}
		continue;
	    }
	    int slot = i.getLocalSlot();
	    if (slot >= 0) {
		bc_codes.putByte(bc);
		bc_local.putUnsigned(slot);
		int con = i.getConstant();
		if (bc == _iinc) {
		    if (!i.isWide()) {
			bc_byte.putUnsigned(0xFF & con);
		    } else {
			bc_short.putUnsigned(0xFFFF & con);
		    }
		} else {
		    if(D)assert0(con == 0);
		}
		continue;
	    }
	    // Generic instruction.  Copy the body.
	    bc_codes.putByte(bc);
	    int pc = i.getPC()+1;
	    int npc = i.getNextPC();
	    if (pc < npc) {
		// Do a few remaining multi-byte instructions.
		switch (bc) {
		case _sipush:
		    bc_short.putUnsigned(0xFFFF & i.getConstant());
		    break;
		case _bipush:
		    bc_byte.putUnsigned(0xFF & i.getConstant());
		    break;
		case _newarray:
		    bc_byte.putUnsigned(0xFF & i.getConstant());
		    break;
		default:
		    if(D)assert0(false);  // that's it
		}
	    }
	}
	bc_codes.putByte(_end_marker);
	bc_codes.elementCountForDebug++;
	codeHist[_end_marker]++;
	endCode();
    }

    int[] codeHist = new int[1<<8];
    int[] ldcHist  = new int[20];
    void printCodeHist() {
	if(D)assert0(verbose > 0);
	String[] hist = new String[codeHist.length];
	int totalBytes = 0;
	for (int bc = 0; bc < codeHist.length; bc++) {
	    totalBytes += codeHist[bc];
	}
	for (int bc = 0; bc < codeHist.length; bc++) {
	    if (codeHist[bc] == 0) { hist[bc] = ""; continue; }
	    String iname = Instruction.byteName(bc);
	    String count = "" + codeHist[bc];
	    count = "         ".substring(count.length()) + count;
	    String pct = "" + (codeHist[bc] * 10000 / totalBytes);
	    while (pct.length() < 4)  pct = "0" + pct;
	    pct = pct.substring(0, pct.length()-2) + "." + pct.substring(pct.length()-2);
	    hist[bc] = count + "  " + pct + "%  " + iname;
	}
	Arrays.sort(hist);
	System.out.println("Bytecode histogram ["+totalBytes+"]");
	for (int i = hist.length; --i >= 0; ) {
	    if (hist[i] == "")  continue;
	    System.out.println(hist[i]);
	}
	for (int tag = 0; tag < ldcHist.length; tag++) {
	    int count = ldcHist[tag];
	    if (count == 0)  continue;
	    System.out.println("ldc "+ConstantPool.tagName(tag)+" "+count);
	}
    }
}
