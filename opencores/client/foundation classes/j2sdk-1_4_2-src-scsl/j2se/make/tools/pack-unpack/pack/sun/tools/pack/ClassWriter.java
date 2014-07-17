/*
 * @(#)ClassWriter.java	1.5 03/01/23
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
 * Writer for a class file that is incorporated into a package.
 * @author John Rose
 * @version 1.1, 10/09/01
 */
class ClassWriter implements Constants {
    public static int verbose = 0;

    Package pkg;
    Class cls;
    DataOutputStream out;

    ClassWriter(Class cls, OutputStream out) throws IOException {
	this.pkg = cls.getPackage();
	this.cls = cls;
	this.out = new DataOutputStream(new BufferedOutputStream(out));
    }

    private void writeShort(int x) throws IOException {
	out.writeShort(x);
    }

    private void writeInt(int x) throws IOException {
	out.writeInt(x);
    }

    /** Write a 2-byte int representing a CP entry. */
    private void writeRef(Entry e) throws IOException {
	int i = (e == null) ? 0 : cls.getCPIndex().indexOf(e);
	writeShort(i);
    }

    /** Write a 1-byte int representing a CP entry. */
    private void writeNarrowRef(Entry e) throws IOException {
	int i = (e == null) ? 0 : cls.getCPIndex().indexOf(e);
	if(D)assert0(i <= 255);
	out.write(i);
    }

    private void writeRefString(String x) throws IOException {
	writeRef(pkg.cp.ensureUtf8Entry(x));
    }

    ByteArrayOutputStream bufPool;  // one-element pool of reusable buffers
    ByteArrayOutputStream getBuf() {
	if (bufPool != null) {
	    ByteArrayOutputStream res = bufPool;
	    bufPool = null;
	    return res;
	} else {
	    return new ByteArrayOutputStream(1 << 12);
	}
    }
    void putBuf(ByteArrayOutputStream buf) {
	buf.reset();
	bufPool = buf;
    }

    private class Attribute {
	DataOutputStream       savedOut = out;
	ByteArrayOutputStream  buf      = getBuf();
	Utf8Entry              attrName;

	Attribute(String attrName) throws IOException {
	    this.attrName = pkg.cp.ensureUtf8Entry(attrName);
	    writeRef(this.attrName);
	    out = new DataOutputStream(buf);
	}
	void close() throws IOException {
	    out = savedOut;
	    if (verbose > 2)
		System.out.println("Attribute "+attrName.stringValue()+" ["+buf.size()+"]");
	    writeInt(buf.size());
	    buf.writeTo(out);
	    putBuf(buf);
	}
    }

    void write() throws IOException {
	boolean ok = false;
	try {
	    if (verbose > 1)  System.out.println("...writing "+cls);
	    writeMagicNumbers();
	    writeConstantPool();
	    writeHeader();
	    writeMembers(false);  // fields
	    writeMembers(true);   // methods
	    writeClassAttributes();
	    /* Closing here will cause all the underlying
	       streams to close, Causing the jar stream
	       to close prematurely, instead we just flush.
	       out.close();
	     */
	    out.flush();
	    cls.cpIndex = null;  // decache for happier GC
	    ok = true;
	} finally {
	    if (!ok) {
		System.out.println("*** Error on output of "+cls);
	    }
	}
    }

    void writeMagicNumbers() throws IOException {
	writeInt(cls.magic);
	writeShort(cls.minor_version);
	writeShort(cls.major_version);
    }

    void writeConstantPool() throws IOException {
	Entry[] cpMap = cls.cpMap;
	writeShort(cpMap.length);
	for (int i = 0; i < cpMap.length; i++) {
	    Entry e = cpMap[i];
	    if(D)assert0((e == null) == (i == 0 || cpMap[i-1] != null && cpMap[i-1].isDoubleWord()));
	    if (e == null)  continue;
	    byte tag = e.getTag();
	    if (verbose > 2)  System.out.println("   CP["+i+"] = "+e);
	    out.write((e.tag == CONSTANT_Signature) ? CONSTANT_Utf8 : e.tag);
	    switch (tag) {
		case CONSTANT_Utf8:
		case CONSTANT_Signature:
		    out.writeUTF(e.stringValue());
		    break;
		case CONSTANT_Integer:
		    out.writeInt(((NumberEntry)e).numberValue().intValue());
		    break;
		case CONSTANT_Float:
		    out.writeFloat(((NumberEntry)e).numberValue().floatValue());
		    break;
		case CONSTANT_Long:
		    out.writeLong(((NumberEntry)e).numberValue().longValue());
		    break;
		case CONSTANT_Double:
		    out.writeDouble(((NumberEntry)e).numberValue().doubleValue());
		    break;
		case CONSTANT_Class:
		case CONSTANT_String:
		    writeRef(e.getRef(0));
		    break;
		case CONSTANT_Fieldref:
		case CONSTANT_Methodref:
		case CONSTANT_InterfaceMethodref:
		case CONSTANT_NameandType:
		    writeRef(e.getRef(0));
		    writeRef(e.getRef(1));
		    break;
		default:
		    throw new IOException("Bad constant pool tag "+tag);
	    }
	}
    }
    
    void writeHeader() throws IOException {
	writeShort(cls.flags);
	writeRef(cls.thisClass);
	writeRef(cls.superClass);
	writeShort(cls.interfaces.length);
	for (int i = 0; i < cls.interfaces.length; i++) {
	    writeRef(cls.interfaces[i]);
	}
    }

    void writeMembers(boolean doMethods) throws IOException {
	Class.Member[] mems;
	if (!doMethods)
	    mems = cls.getFields();
	else
	    mems = cls.getMethods();
	writeShort(mems.length);
	for (int i = 0; i < mems.length; i++) {
	    writeMember(mems[i], doMethods);
	}
    }

    void writeMember(Class.Member m, boolean doMethod) throws IOException {
	if (verbose > 2)  System.out.println("writeMember "+m);
	writeShort(m.flags);
	writeRef(m.getDescriptor().nameRef);
	writeRef(m.getDescriptor().typeRef);

	// Write attributes:
	int na = m.attrCount();
	writeShort(na);
	if (m.isDeprecated) {
	    new Attribute("Deprecated").close();
	    na--;
	}
	if (m.isSynthetic) {
	    new Attribute("Synthetic").close();
	    na--;
	}
	if (!doMethod) {
	    Class.Field mf = (Class.Field) m;
	    if (mf.constantValue != null) {
		Attribute a = new Attribute("ConstantValue");
		writeRef(mf.constantValue);
		a.close();
		na--;
	    }
	} else {
	    Class.Method mm = (Class.Method) m;
	    if (mm.exceptions.length > 0) {
		Attribute a = new Attribute("Exceptions");
		int ne = mm.exceptions.length;
		writeShort(ne);
		for (int i = 0; i < ne; i++) {
		    writeRef(mm.exceptions[i]);
		}
		a.close();
		na--;
	    }
	    if (mm.code != null) {
		Attribute a = new Attribute("Code");
		writeCode(mm.code);
		a.close();
		na--;
	    }
	}
	if(D)assert0(na == 0);  // correct attr count
    }

    void writeCode(Code code) throws IOException {
	writeShort(code.max_stack);
	writeShort(code.max_locals);
	writeInt(code.bytes.length);
	out.write(code.bytes);
	int nh = code.getHandlerCount();
	writeShort(nh);
	for (int i = 0; i < nh; i++) {
	     writeShort(code.handler_start[i]);
	     writeShort(code.handler_end[i]);
	     writeShort(code.handler_catch[i]);
	     writeRef(code.handler_class[i]);
	}
	writeCodeAttributes(code);
    }

    void writeCodeAttributes(Code code) throws IOException {
	int na = code.attrCount();
	writeShort(na);
	if (code.lineNumberTable_pc.length > 0) {
	    Attribute a = new Attribute("LineNumberTable");
	    int nl = code.lineNumberTable_pc.length;
	    writeShort(nl);
	    for (int i = 0; i < nl; i++) {
		writeShort(code.lineNumberTable_pc[i]);
		writeShort(code.lineNumberTable_line[i]);
	    }
	    a.close();
	    na--;
	}
	if (code.localVariableTable_start.length > 0) {
	    Attribute a = new Attribute("LocalVariableTable");
	    int nl = code.localVariableTable_start.length;
	    writeShort(nl);
	    for (int i = 0; i < nl; i++) {
		int start = code.localVariableTable_start[i];
		int end   = code.localVariableTable_end[i];
		int span  = end - start;
		writeShort(start);
		writeShort(span);
		writeRef(code.localVariableTable_type[i]);
		writeShort(code.localVariableTable_slot[i]);
	    }
	    a.close();
	    na--;
	}
	if(D)assert0(na == 0);  // correct attr count
    }

    void writeClassAttributes() throws IOException {
	int na = cls.attrCount();
	writeShort(na);
	if(D)assert0(cls.retroVersion == null);  // must strip retroVersion!
	if (cls.isDeprecated) {
	    new Attribute("Deprecated").close();
	    na--;
	}
	if (cls.isSynthetic) {
	    new Attribute("Synthetic").close();
	    na--;
	}
	if (cls.sourceFile != null) {
	    Attribute a = new Attribute("SourceFile");
	    writeRef(cls.sourceFile);
	    a.close();
	    na--;
	}
	if (cls.innerClasses.size() > 0) {
	    Attribute a = new Attribute("InnerClasses");
	    InnerClass[] ics = new InnerClass[cls.innerClasses.size()];
	    cls.innerClasses.toArray(ics);
	    Arrays.sort(ics);
	    writeShort(ics.length);
	    for (int i = 0; i < ics.length; i++) {
		InnerClass ic = ics[i];
		writeRef(ic.thisClass);
		writeRef(ic.outerClass);
		writeRef(ic.name);
		writeShort(ic.flags);
	    }
	    a.close();
	    na--;
	}
	if(D)assert0(na == 0);  // correct attr count
    }

    static void assert0(boolean z) {
	if (!z) throw new RuntimeException("assert failed");
    }
}
