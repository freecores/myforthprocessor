/*
 * @(#)ClassReader.java	1.4 03/01/23
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
 * Reader for a class file that is being incorporated into a package.
 * @author John Rose
 * @version 1.2, 02/11/02
 */
class ClassReader implements Constants {
    public static int verbose = 0;

    Package pkg;
    Class cls;
    byte[] bytes;
    DataInputStream in;

    ClassReader(Class cls, InputStream in) throws IOException {
	this.pkg = cls.getPackage();
	this.cls = cls;
	this.bytes = BandStructure.readAll(in);
	this.in = new DataInputStream(new ByteArrayInputStream(bytes));
    }

    private int inPos() throws IOException {
	return bytes.length - in.available();
    }

    private void skip(int n) throws IOException {
	long skipped = 0;
	while (skipped < n) {
	    long j = in.skip(n - skipped);
	    if(D)assert0(j > 0);
	    skipped += j;
	}
	if(D)assert0(skipped == n);
    }

    private int readUnsignedShort() throws IOException {
	return in.readUnsignedShort();
    }

    private int readInt() throws IOException {
	return in.readInt();
    }

    /** Read a 2-byte int, and return the <em>global</em> CP entry for it. */
    private Entry readRef() throws IOException {
	int i = in.readUnsignedShort();
	return i == 0 ? null : cls.cpMap[i];
    }

    private Entry readRef(byte tag) throws IOException {
	Entry e = readRef();
	if(D)assert0((e == null ? cls.cpMap[0] : e).assertTagMatches(tag));
	return e;
    }

    private Entry readRefOrNull(byte tag) throws IOException {
	return readRef((byte)(tag | CONSTANT_NullOKMask));
    }

    private Utf8Entry readUtf8Ref() throws IOException {
	return (Utf8Entry) readRef(CONSTANT_Utf8);
    }

    private ClassEntry readClassRef() throws IOException {
	return (ClassEntry) readRef(CONSTANT_Class);
    }

    private ClassEntry readClassRefOrNull() throws IOException {
	return (ClassEntry) readRefOrNull(CONSTANT_Class);
    }

    private SignatureEntry readSignatureRef() throws IOException {
	Entry e = readRef();
	if (e.getTag() == CONSTANT_Utf8) {
	    return pkg.cp.ensureSignatureEntry(e.stringValue());
	}
	return (SignatureEntry) readRef(CONSTANT_Signature);
    }

    void read() throws IOException {
	boolean ok = false;
	try {
	    readMagicNumbers();
	    readConstantPool();
	    readHeader();
	    readMembers(false);  // fields
	    readMembers(true);   // methods
	    readClassAttributes();
	    if(D)assert0(in.available() == 0);
	    ok = true;
	} finally {
	    if (!ok) {
		System.out.println("*** Error at input offset "+inPos());
	    }
	}
    }

    void readMagicNumbers() throws IOException {
	cls.magic = in.readInt();
	if (cls.magic != JAVA_MAGIC)
	    throw new RuntimeException("Bad magic number in class file "+Integer.toHexString(cls.magic));
	cls.minor_version = readUnsignedShort();
	cls.major_version = readUnsignedShort();
	String bad = checkVersion(cls.major_version, cls.minor_version);
	if (bad != null) {
	    throw new RuntimeException
		("classfile version too "+bad+": "+
		 cls.major_version+"."+cls.minor_version+" in "+cls.classFile);
	}
    }

    private String checkVersion(int major_version, int minor_version) {
	if (major_version < JAVA_MIN_MAJOR_VERSION ||
	    major_version == JAVA_MIN_MAJOR_VERSION &&
	    minor_version < JAVA_MIN_MINOR_VERSION) {
	    return "small";
	}
	if (major_version > JAVA_MAX_MAJOR_VERSION ||
	    major_version == JAVA_MAX_MAJOR_VERSION &&
	    minor_version > JAVA_MAX_MINOR_VERSION) {
	    return "large";
	}
	return null;  // OK
    }

    void readConstantPool() throws IOException {
	ConstantPool.Classic cp = new ConstantPool.Classic();
	cp.setNotPermutable();

	int length = in.readUnsignedShort();
	//System.err.println("reading CP, length="+length);

	Entry[] cpMap = new Entry[length];
	cpMap[0] = cp.getNull();
	for (int i = 1; i < length; i++) {
	    //System.err.println("reading CP elt, i="+i);
	    byte tag = in.readByte();
	    Entry e0 = cpMap[i];
	    if(D)assert0(e0 == null || e0.getTag() == tag);
	    Utf8Entry ref, ref2;
	    ClassEntry cref;
	    DescriptorEntry dref;
	    int refn;
	    switch (tag) {
		case CONSTANT_Utf8:
		    {
			Utf8Entry e = (Utf8Entry) e0;
			if (e == null)  e0 = e = cp.new Utf8Entry();
			e.setValue(in.readUTF());
		    }
		    break;
		case CONSTANT_Integer:
		case CONSTANT_Float:
		case CONSTANT_Long:
		case CONSTANT_Double:
		    {
			NumberEntry e = (NumberEntry) e0;
			if (e == null)  e0 = e = cp.new NumberEntry(tag);
			Number value = null;
			switch (tag) {
			case CONSTANT_Integer:
			    value = new Integer(in.readInt()); break;
			case CONSTANT_Float:
			    value = new Float(in.readFloat()); break;
			case CONSTANT_Long:
			    value = new Long(in.readLong()); break;
			case CONSTANT_Double:
			    value = new Double(in.readDouble()); break;
			}
			e.setValue(value);
			if (e.isDoubleWord()) {
			    if(D)assert0(cpMap[i] == null || cpMap[i] == e0);
			    cpMap[i] = e0;
			    if(D)assert0(cpMap[i] != null);
			    e0 = cp.new EmptyEntry(e);
			    ++i;
			}
		    }
		    break;

		// just read the refs; do not attempt to resolve while reading
		case CONSTANT_Class:
		case CONSTANT_String:
		    refn = in.readUnsignedShort();
		    ref = (Utf8Entry) cpMap[refn];
		    if (ref == null)  cpMap[refn] = ref = cp.new Utf8Entry();
		    switch (tag) {
		    case CONSTANT_Class:
			{
			    ClassEntry e = (ClassEntry) e0;
			    if (e == null)  e0 = e = cp.new ClassEntry();
			    e.setRef(ref);
			}
			break;
		    case CONSTANT_String:
			{
			    StringEntry e = (StringEntry) e0;
			    if (e == null)  e0 = e = cp.new StringEntry();
			    e.setRef(ref);
			}
		    }
		    break;
		case CONSTANT_Fieldref:
		case CONSTANT_Methodref:
		case CONSTANT_InterfaceMethodref:
		    {
			MemberEntry e = (MemberEntry) e0;
			if (e == null)  e0 = e = cp.new MemberEntry(tag);
			refn = in.readUnsignedShort();
			cref = (ClassEntry) cpMap[refn];
			if (cref == null)  cpMap[refn] = cref = cp.new ClassEntry();
			refn = in.readUnsignedShort();
			dref = (DescriptorEntry) cpMap[refn];
			if (dref == null)  cpMap[refn] = dref = cp.new DescriptorEntry();
			e.setRefs(cref, dref);
		    }
		    break;
		case CONSTANT_NameandType:
		    {
			DescriptorEntry e = (DescriptorEntry) e0;
			if (e == null)  e0 = e = cp.new DescriptorEntry();
			refn = in.readUnsignedShort();
			ref = (Utf8Entry) cpMap[refn];
			if (ref == null)  cpMap[refn] = ref = cp.new Utf8Entry();
			refn = in.readUnsignedShort();
			ref2 = (Utf8Entry) cpMap[refn];
			if (ref2 == null)  cpMap[refn] = ref2 = cp.new Utf8Entry();
			e.setRefs(ref, ref2);
		    }
		    break;
		default:
		    throw new IOException("Bad constant pool tag "+tag);
	    }
	    if(D)assert0(cpMap[i] == null || cpMap[i] == e0);
	    cpMap[i] = e0;
	    if(D)assert0(cpMap[i] != null);
	}

	// Plug all the new entries into the cp.
	// This resolves all values.
	cp.setAllEntries(cpMap);

	// With all the values in hand, throw away cp, keeping only a cpMap.
	cls.cpMap = pkg.cp.mapEntriesFrom(cp.getIndex());
    }
    
    void readHeader() throws IOException {
	cls.flags = readUnsignedShort();
	cls.thisClass = readClassRef();
	cls.superClass = readClassRefOrNull();
	int ni = readUnsignedShort();
	cls.interfaces = new ClassEntry[ni];
	for (int i = 0; i < ni; i++) {
	    cls.interfaces[i] = readClassRef();
	}
    }

    void readMembers(boolean doMethods) throws IOException {
	int nm = readUnsignedShort();
	for (int i = 0; i < nm; i++) {
	    readMember(doMethods);
	}
    }

    void readMember(boolean doMethod) throws IOException {
	int    mflags = readUnsignedShort();
	String mname  = readUtf8Ref().stringValue();
	String mtype  = readSignatureRef().stringValue();
	Class.Member m;
	if (!doMethod)
	    m = cls.new Field(mflags, pkg.cp.ensureDescriptorEntry(mname, mtype));
	else
	    m = cls.new Method(mflags, pkg.cp.ensureDescriptorEntry(mname, mtype));
	int na = readUnsignedShort();
	for (int i = 0; i < na; i++) {
	    String name = readUtf8Ref().stringValue();
	    int length = readInt();
	    if (name == "Deprecated") {
		if(D)assert0(length == 0);
		m.isDeprecated = true;
	    } else if (name == "Synthetic") {
		if(D)assert0(length == 0);
		m.isSynthetic = true;
	    } else if (name == "ConstantValue" && !doMethod) {
		if(D)assert0(length == 2);
		((Class.Field)m).constantValue = (LiteralEntry) readRef();
	    } else if (name == "Exceptions" && doMethod) {
		Class.Method mm = (Class.Method)m;
		int ne = readUnsignedShort();
		if(D)assert0(length == 2 * (1+ne));
		mm.exceptions = new ClassEntry[ne];
		for (int j = 0; j < ne; j++) {
		    mm.exceptions[j] = readClassRef();
		}
	    } else if (name == "Code" && doMethod) {
		Class.Method mm = (Class.Method)m;
		Code code = new Code(mm);
		mm.code = code;
		int checkl = 0;
		code.max_stack = readUnsignedShort();
		code.max_locals = readUnsignedShort();
		checkl += 2 * 2;
		byte[] bytes = new byte[readInt()];
		in.readFully(bytes);
		checkl += 4 + bytes.length;
		code.bytes = bytes;
		int nh = readUnsignedShort();
		if (nh > 0) {
		    code.handler_start = new int[nh];
		    code.handler_end   = new int[nh];
		    code.handler_catch = new int[nh];
		    code.handler_class = new Entry[nh];
		    for (int j = 0; j < nh; j++) {
			code.handler_start[j] = readUnsignedShort();
			code.handler_end[j]   = readUnsignedShort();
			code.handler_catch[j] = readUnsignedShort();
			code.handler_class[j] = readClassRefOrNull();
		    }
		}
		checkl += 2 * (1 + 4*nh);
		checkl += readCodeAttributes(code);
		if(D)assert0(checkl == length);
	    } else {
		// Skip the attribute.
		System.out.println(m+": skipping "+length+" bytes of "+name);
		skip(length);
	    }
	}
    }

    int readCodeAttributes(Code code) throws IOException {
	int checkl = 0;
	int na = readUnsignedShort();
	checkl += 2;
	for (int j = 0; j < na; j++) {
	    String name = readUtf8Ref().stringValue();
	    int length = readInt();
	    checkl += 2 + 4 + length;
	    if (name == "LineNumberTable") {
		int nl = readUnsignedShort();
		if(D)assert0(length == 2 * (1+2*nl));
		if (nl > 0) {
		    code.lineNumberTable_pc = new int[nl];
		    code.lineNumberTable_line = new int[nl];
		}
		for (int i = 0; i < nl; i++) {
		    code.lineNumberTable_pc[i] = readUnsignedShort();
		    code.lineNumberTable_line[i] = readUnsignedShort();
		}
	    } else if (name == "LocalVariableTable") {
		int nv = readUnsignedShort();
		if(D)assert0(length == 2 * (1+2*nv));
		if (nv > 0) {
		    code.localVariableTable_start = new int[nv];
		    code.localVariableTable_end = new int[nv];
		    code.localVariableTable_name = new Entry[nv];
		    code.localVariableTable_type = new Entry[nv];
		    code.localVariableTable_slot = new int[nv];
		}
		for (int i = 0; i < nv; i++) {
		    int start = readUnsignedShort();
		    int span  = readUnsignedShort();
		    code.localVariableTable_start[i] = start;
		    code.localVariableTable_end[i] = start + span;
		    code.localVariableTable_name[i] = readUtf8Ref();
		    code.localVariableTable_type[i] = readSignatureRef();
		    code.localVariableTable_slot[i] = readUnsignedShort();
		}
	    } else {
		// Skip the attribute.
		System.out.println(code.m+".Code: skipping "+length+" bytes of "+name);
		skip(length);
	    }
	}
	return checkl;
    }

    void readClassAttributes() throws IOException {
	int na = readUnsignedShort();
	for (int i = 0; i < na; i++) {
	    String name = readUtf8Ref().stringValue();
	    int length = readInt();
	    if (name == "Deprecated") {
		if(D)assert0(length == 0);
		cls.isDeprecated = true;
	    } else if (name == "Synthetic") {
		if(D)assert0(length == 0);
		cls.isSynthetic = true;
	    } else if (name == "SourceFile") {
		if(D)assert0(length == 2);
		cls.sourceFile = readUtf8Ref();
	    } else if (name == "InnerClasses") {
		int nc = readUnsignedShort();
		if(D)assert0(length == 2 * (1+4*nc));
		for (int j = 0; j < nc; j++) {
		    InnerClass ic =
			new InnerClass(readClassRef(),
				       readClassRefOrNull(),
				       (Utf8Entry)readRefOrNull(CONSTANT_Utf8),
				       readUnsignedShort());
		    cls.innerClasses.add(ic);
		    // (Later, ic is transferred to the pkg.)
		}
	    } else {
		// Skip the attribute.
		System.out.println(cls+": skipping "+length+" bytes of "+name);
		in.skip(length);
	    }
	}
    }

    static void assert0(boolean z) {
	if (!z) throw new RuntimeException("assert failed");
    }
}
