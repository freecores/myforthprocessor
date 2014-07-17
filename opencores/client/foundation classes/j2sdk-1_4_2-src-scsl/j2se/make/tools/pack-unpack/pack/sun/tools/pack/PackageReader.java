/*
 * @(#)PackageReader.java	1.9 03/01/23
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
 * Reader for a package file.
 *
 * @see PackageWriter
 * @author John Rose
 * @version 1.9, 01/23/03
 */
class PackageReader extends BandStructure {
    Package pkg;
    byte[] bytes;
    ByteArrayInputStream in;

    PackageReader(Package pkg, InputStream in) throws IOException {
	super(true);
	this.pkg = pkg;
	this.bytes = BandStructure.readAll(in);
	if (verbose > 0)
	    System.out.println("Read "+bytes.length+" bytes of package.");
	this.in = new ByteArrayInputStream(bytes);
    }

    void read() throws IOException {
	boolean ok = false;
	try {
	    readMagicNumbers();
	    readPackageHeader();
	    readConstantPool();
	    readClasses();
	    readCodeHeaders();
	    class_bands.doneDisbursing();
	    readAttributes();  // attrs are after headers & before BCs
	    attr_bands.doneDisbursing();
	    readByteCodes();
	    bc_bands.doneDisbursing();
	    all_bands.doneDisbursing();
	    ok = true;
	} catch (Exception ee) {
	    System.out.println("*** Error on input: "+ee);
	    if (verbose > 0)  ee.printStackTrace();
	    if (ee instanceof IOException)  throw (IOException)ee;
	    if (ee instanceof RuntimeException)  throw (RuntimeException)ee;
	    if(D)assert0(false);
	}
    }

    int[] tagCount = new int[CONSTANT_Limit];
    int numClasses;
    int numInnerClasses;
    int numPackageAttrs;

    void readMagicNumbers() throws IOException {
	package_magic.expectLength(4+2+2);  // magic, minor, major
	package_magic.readFrom(in);

	// read and check magic numbers:
	pkg.magic = package_magic.getInt32();
	pkg.package_minor_version = package_magic.getShort();
	pkg.package_major_version = package_magic.getShort();
	pkg.checkVersion();
	package_magic.doneDisbursing();
    }

    void readPackageHeader() throws IOException {
	int phlen = ConstantPool.TAGS_IN_ORDER.length; // getIndexByTag.size
	// #minver, #majver, #classes, #innerClasses, #package_attrs
	phlen += 5;
	package_header.expectLength(phlen);
	package_header.readFrom(in);
	// now we are ready to use the data:

	// size the constant pool:
	for (int k = 0; k < ConstantPool.TAGS_IN_ORDER.length; k++) {
	    byte tag = ConstantPool.TAGS_IN_ORDER[k];
	    int  len = package_header.getUnsigned();
	    tagCount[tag] = len;
	    switch (tag) {
	    case CONSTANT_Utf8:
		cp_Utf8_prefix.expectLength(len);
		cp_Utf8_suffix.expectLength(len);
		break;
	    case CONSTANT_Integer:
		cp_Int.expectLength(len);
		break;
	    case CONSTANT_Float:
		cp_Float.expectLength(len);
		break;
	    case CONSTANT_Long:
		cp_Long_hi.expectLength(len);
		cp_Long_lo.expectLength(len);
		break;
	    case CONSTANT_Double:
		cp_Double_hi.expectLength(len);
		cp_Double_lo.expectLength(len);
		break;
	    case CONSTANT_Class:
		cp_Class.expectLength(len);
		break;
	    case CONSTANT_String:
		cp_String.expectLength(len);
		break;
	    case CONSTANT_Fieldref:
		cp_Field_class.expectLength(len);
		cp_Field_desc.expectLength(len);
		break;
	    case CONSTANT_Methodref:
		cp_Method_class.expectLength(len);
		cp_Method_desc.expectLength(len);
		break;
	    case CONSTANT_InterfaceMethodref:
		cp_Imethod_class.expectLength(len);
		cp_Imethod_desc.expectLength(len);
		break;
	    case CONSTANT_NameandType:
		cp_Descr_name.expectLength(len);
		cp_Descr_type.expectLength(len);
		break;
	    case CONSTANT_Signature:
		cp_Signature_form.expectLength(len);
		cp_Signature_classes.expectLength(0);  // more later
		break;
	    default:
		if(D)assert0(false);
	    }
	}

	pkg.default_class_minor_version = package_header.getUnsigned();
	pkg.default_class_major_version = package_header.getUnsigned();
	numClasses = package_header.getUnsigned();
	numInnerClasses = package_header.getUnsigned();
	numPackageAttrs = package_header.getUnsigned();
	class_flags.expectLength(numClasses);
	class_this.expectLength(numClasses);
	class_super.expectLength(numClasses);
	class_interface_count.expectLength(numClasses);
	class_field_count.expectLength(numClasses);
	class_method_count.expectLength(numClasses);
	ic_this_class.expectLength(numInnerClasses);
	ic_flags.expectLength(numInnerClasses);
	package_header.doneDisbursing();
    }

    Index getCPIndex(byte tag) {
	return pkg.cp.getIndexByTag(tag);
    }
    void initCPIndex(byte tag, Entry[] cpMap) {
	if (verbose > 3) {
	    for (int i = 0; i < cpMap.length; i++) {
		System.out.println("cp.add "+cpMap[i]);
	    }
	}
	Index index = pkg.cp.makeIndexByTag(tag, cpMap);
	if (verbose > 1)  System.out.println("Read "+index);
    }

    void readConstantPool() throws IOException {
	ConstantPool cp = pkg.cp;

	if (verbose > 0)  System.out.println("reading CP");

	Index ix, ix2;
	Entry ref, ref2;

	for (int k = 0; k < ConstantPool.TAGS_IN_ORDER.length; k++) {
	    byte tag = ConstantPool.TAGS_IN_ORDER[k];
	    int  len = tagCount[tag];

	    String tagName = ConstantPool.tagName(tag);

	    // Read the quantity of each tag.
	    if (verbose > 0)
		System.out.println("Reading "+len+" "+tagName+" entries...");
	    Entry[] cpMap = new Entry[len];

	    switch (tag) {
	    case CONSTANT_Utf8:
		readUtf8Values(cpMap);
		break;
	    case CONSTANT_Integer:
		cp_Int.readFrom(in);
		for (int i = 0; i < cpMap.length; i++) {
		    int x = cp_Int.getInt();  // coding handles signs OK
		    cpMap[i] = cp.new NumberEntry(new Integer(x));
		}
		cp_Int.doneDisbursing();
		break;
	    case CONSTANT_Float:
		cp_Float.readFrom(in);
		for (int i = 0; i < cpMap.length; i++) {
		    int x = cp_Float.getInt();
		    float fx = Float.intBitsToFloat(x);
		    cpMap[i] = cp.new NumberEntry(new Float(fx));
		}
		cp_Float.doneDisbursing();
		break;
	    case CONSTANT_Long:
		cp_Long_hi.readFrom(in);
		cp_Long_lo.readFrom(in);
		for (int i = 0; i < cpMap.length; i++) {
		    long hi = cp_Long_hi.getInt();
		    long lo = cp_Long_lo.getInt();
		    long x = (hi << 32) + ((lo << 32) >>> 32);
		    cpMap[i] = cp.new NumberEntry(new Long(x));
		}
		cp_Long_hi.doneDisbursing();
		cp_Long_lo.doneDisbursing();
		break;
	    case CONSTANT_Double:
		cp_Double_hi.readFrom(in);
		cp_Double_lo.readFrom(in);
		for (int i = 0; i < cpMap.length; i++) {
		    long hi = cp_Double_hi.getInt();
		    long lo = cp_Double_lo.getInt();
		    long x = (hi << 32) + ((lo << 32) >>> 32);
		    double dx = Double.longBitsToDouble(x);
		    cpMap[i] = cp.new NumberEntry(new Double(dx));
		}
		cp_Double_hi.doneDisbursing();
		cp_Double_lo.doneDisbursing();
		break;
	    case CONSTANT_String:
		cp_String.readFrom(in);
		ix = getCPIndex(CONSTANT_Utf8);
		for (int i = 0; i < cpMap.length; i++) {
		    cpMap[i] = cp.new StringEntry(cp_String.getRef(ix));
		}
		cp_String.doneDisbursing();
		break;
	    case CONSTANT_Class:
		cp_Class.readFrom(in);
		ix = getCPIndex(CONSTANT_Utf8);
		for (int i = 0; i < cpMap.length; i++) {
		    cpMap[i] = cp.new ClassEntry(cp_Class.getRef(ix));
		}
		cp_Class.doneDisbursing();
		break;
	    case CONSTANT_Signature:
		{
		    Utf8Entry[] forms = new Utf8Entry[len];
		    int[] numClasses = new int[len];
		    cp_Signature_form.readFrom(in);
		    ix = getCPIndex(CONSTANT_Utf8);
		    for (int i = 0; i < forms.length; i++) {
			Utf8Entry formRef = (Utf8Entry) cp_Signature_form.getRef(ix);
			forms[i] = formRef;
			int nc = ConstantPool.countClassParts(formRef);
			numClasses[i] = nc;
			cp_Signature_classes.expectMoreLength(nc);
		    }
		    cp_Signature_form.doneDisbursing();
		    cp_Signature_classes.readFrom(in);
		    ix2 = getCPIndex(CONSTANT_Class);
		    for (int i = 0; i < cpMap.length; i++) {
			Utf8Entry formRef = forms[i];
			int nc = numClasses[i];
			ClassEntry[] classRefs = new ClassEntry[nc];
			for (int j = 0; j < classRefs.length; j++) {
			    classRefs[j] = (ClassEntry) cp_Signature_classes.getRef(ix2);
			}
			cpMap[i] = cp.new SignatureEntry(formRef, classRefs);
		    }
		    cp_Signature_classes.doneDisbursing();
		}
		break;
	    case CONSTANT_NameandType:
		cp_Descr_name.readFrom(in);
		ix = getCPIndex(CONSTANT_Utf8);
		cp_Descr_type.readFrom(in);
		ix2 = getCPIndex(CONSTANT_Signature);
		for (int i = 0; i < cpMap.length; i++) {
		    ref = cp_Descr_name.getRef(ix);
		    ref2 = cp_Descr_type.getRef(ix2);
		    cpMap[i] = cp.new DescriptorEntry(ref, ref2);
		}
		cp_Descr_name.doneDisbursing();
		cp_Descr_type.doneDisbursing();
		break;
	    case CONSTANT_Fieldref:
		readMemberRefs(tag, cpMap, cp_Field_class, cp_Field_desc);
		break;
	    case CONSTANT_Methodref:
		readMemberRefs(tag, cpMap, cp_Method_class, cp_Method_desc);
		break;
	    case CONSTANT_InterfaceMethodref:
		readMemberRefs(tag, cpMap, cp_Imethod_class, cp_Imethod_desc);
		break;
	    default:
		if(D)assert0(false);
	    }

	    initCPIndex(tag, cpMap);
	}

	cp_bands.doneDisbursing();

	setBandIndexes();
    }

    void readUtf8Values(Entry[] cpMap) throws IOException {
	ConstantPool cp = pkg.cp;

	int len = cpMap.length;

	// First band:  Read lengths of shared prefixes.
	cp_Utf8_prefix.readFrom(in);
	int[] prefixes = cp_Utf8_prefix.getUnsignedArray();
	cp_Utf8_prefix.doneDisbursing();

	// Second band:  Read lengths of unshared suffixes:
	cp_Utf8_suffix.readFrom(in);
	int[] suffixes = cp_Utf8_suffix.getUnsignedArray();
	cp_Utf8_suffix.doneDisbursing();

	for (int i = 0; i < len; i++) {
	    cp_Utf8_chars.expectMoreLength(suffixes[i]);
	}

	char[][] suffixChars = new char[len][];

	// Third band:  Read the char values in the unshared suffixes:
	// %%%% Shouldn't buffer this data through cp_Utf8_chars!
	cp_Utf8_chars.readFrom(in);
	for (int i = 0; i < len; i++) {
	    int suffix = suffixes[i];
	    if (suffix == 0) {
		// chars are packed in cp_Utf8_big_chars
		cp_Utf8_big_length.expectMoreLength(1);
		continue;
	    }
	    suffixChars[i] = new char[suffix];
	    for (int j = 0; j < suffix; j++) {
		int ch = cp_Utf8_chars.getUnsigned();
		if(D)assert0(ch == (char)ch);
		suffixChars[i][j] = (char)ch;
	    }
	}
	cp_Utf8_chars.doneDisbursing();

	// Fourth band:  Go back and size the specially packed strings.
	int maxChars = 0;
	cp_Utf8_big_length.readFrom(in);
	for (int i = 0; i < len; i++) {
	    int suffix = suffixes[i];
	    if(D)assert0((suffixChars[i] == null) == (suffix == 0));
	    if (suffix == 0) {
		suffixes[i] = suffix = cp_Utf8_big_length.getUnsigned();
	    }
	    if (maxChars < prefixes[i] + suffixes[i])
		maxChars = prefixes[i] + suffixes[i];
	}
	cp_Utf8_big_length.doneDisbursing();
	char[] buf = new char[maxChars];

	// Fifth band(s):  Get the specially packed characters.
	for (int i = 0; i < len; i++) {
	    if (suffixChars[i] != null)  continue;  // already input
	    int suffix = suffixes[i];
	    suffixChars[i] = new char[suffix];
	    IntBand packed = cp_Utf8_big_chars.newIntBand("(Utf8_big_"+i+")");
	    packed.expectLength(suffix);
	    packed.readFrom(in);
	    for (int j = 0; j < suffix; j++) {
		int ch = packed.getUnsigned();
		if(D)assert0(ch == (char)ch);
		suffixChars[i][j] = (char)ch;
	    }
	    packed.doneDisbursing();
	}
	cp_Utf8_big_chars.doneDisbursing();

	// Finally, sew together all the prefixes and suffixes.
	for (int i = 0; i < len; i++) {
	    int prefix = prefixes[i];
	    int suffix = suffixes[i];

	    // by induction, the buffer is already filled with the prefix
	    System.arraycopy(suffixChars[i], 0, buf, prefix, suffix);

	    cpMap[i] = cp.new Utf8Entry(new String(buf, 0, prefix+suffix));
	}

    }

    void readMemberRefs(byte tag, Entry[] cpMap, CPRefBand cp_class, CPRefBand cp_desc) throws IOException {
	ConstantPool cp = pkg.cp;

	cp_class.readFrom(in);
	Index ix = getCPIndex(CONSTANT_Class);
	cp_desc.readFrom(in);
	Index ix2 = getCPIndex(CONSTANT_NameandType);
	for (int i = 0; i < cpMap.length; i++) {
	    Entry ref = cp_class.getRef(ix);
	    Entry ref2 = cp_desc.getRef(ix2);
	    cpMap[i] = cp.new MemberEntry(tag, ref, ref2);
	}
	cp_class.doneDisbursing();
	cp_desc.doneDisbursing();
    }

    void readClasses() throws IOException {
	Class[] classes = new Class[numClasses];
	if (verbose > 0)
	    System.out.println("  ...building "+classes.length+" classes...");
	class_flags.readFrom(in);
	class_this.readFrom(in);
	class_super.readFrom(in);
	class_interface_count.readFrom(in);
	for (int i = 0; i < classes.length; i++) {
	    int          flags      = class_flags.getFlags();
	    boolean      hasAttrs   = (flags & ACC_HAS_ATTRIBUTES) != 0;
	    flags &= ~ACC_HAS_ATTRIBUTES;
	    ClassEntry   thisClass  = (ClassEntry) class_this.getRef();
	    ClassEntry   superClass = (ClassEntry) class_super.getRef();
	    ClassEntry[] interfaces = new ClassEntry[class_interface_count.getUnsigned()];
	    // fill in the interfaces in a moment
	    class_interface.expectMoreLength(interfaces.length);
	    classes[i] = new PRClass(flags, thisClass, superClass, interfaces);
	    if (hasAttrs)  class_attrs.expectClient(classes[i]);
	}
	class_flags.doneDisbursing();
	class_this.doneDisbursing();
	class_super.doneDisbursing();
	class_interface_count.doneDisbursing();
	// Read the class_interface band.
	class_interface.readFrom(in);
	for (int i = 0; i < classes.length; i++) {
	    ClassEntry[] interfaces = classes[i].interfaces;
	    for (int j = 0; j < interfaces.length; j++) {
		interfaces[j] = (ClassEntry) class_interface.getRef();
	    }
	}
	class_interface.doneDisbursing();
	readInnerClasses();
	readMembers(classes);
    }

    void readInnerClasses() throws IOException {
	ic_this_class.readFrom(in);
	ic_flags.readFrom(in);
	int[] allFlags = ic_flags.getFlagsArray();
	ic_flags.doneDisbursing();
	for (int i = 0; i < allFlags.length; i++) {
	    int flags = allFlags[i];
	    boolean longForm = (flags & ACC_IC_LONG_FORM) != 0;
	    if (longForm) {
		ic_outer_class.expectMoreLength(1);
		ic_name.expectMoreLength(1);
	    }
	}
	ic_outer_class.readFrom(in);
	ic_name.readFrom(in);
	for (int i = 0; i < allFlags.length; i++) {
	    int flags = allFlags[i];
	    boolean longForm = (flags & ACC_IC_LONG_FORM) != 0;
	    flags &= ~ACC_IC_LONG_FORM;
	    ClassEntry thisClass = (ClassEntry) ic_this_class.getRef();
	    ClassEntry outerClass;
	    Utf8Entry  thisName;
	    if (longForm) {
		outerClass = (ClassEntry) ic_outer_class.getRef();
		thisName   = (Utf8Entry)  ic_name.getRef();
	    } else {
		String n = thisClass.stringValue();
		String[] parse = pkg.parseInnerClassName(n);
		if(D)assert0(parse != null);
		String pkgOuter = parse[0];
		//String number = parse[1];
		String name     = parse[2];
		if (pkgOuter == null)
		    outerClass = null;
		else
		    outerClass = pkg.cp.ensureClassEntry(pkgOuter);
		if (name == null)
		    thisName   = null;
		else
		    thisName   = pkg.cp.ensureUtf8Entry(name);
	    }
	    InnerClass ic =
		new InnerClass(thisClass, outerClass, thisName, flags);
	    if(D)assert0(longForm || ic.predictable);
	    pkg.addInnerClass(ic);
	}
	ic_this_class.doneDisbursing();
	ic_outer_class.doneDisbursing();
	ic_name.doneDisbursing();
    }

    class PRClass extends Class {
	PRClass(int flags, ClassEntry thisClass, ClassEntry superClass, ClassEntry[] interfaces) {
	    pkg.super(flags, thisClass, superClass, interfaces);
	}

	// append this stuff to Classes created by PackageWriter:
	{ super.cpRefs  = new HashSet(); }
	HashSet ldcRefs = new HashSet();

	void maybeCountRef(Entry ref) {
	    if (verbose > 2)  System.out.println("maybeCountRef "+ref);
	    countRef(ref);
	}

	void noteRef(Entry ref) {
	    if(D)assert0(ref != null);
	    cpRefs.add(ref);
	}

	void noteNarrowRef(Entry ref) {
	    noteRef(ref);
	    ldcRefs.add(ref);
	    if(D)assert0(ldcRefs.size() <= 255);  // not too many ldcs
	}

	void reconstruct() {
	    if (verbose > 1)  System.out.println("postProcess "+this);
	    // check retroVersion, etc.
	    setMagicNumbers();
	    retroVersion = null;  // Do not use this any more.

	    // look for constant pool entries outside of the bytecodes:
	    countRefs();

	    // flesh out the local constant pool
	    getPackage().cp.addReferencedEntries(cpRefs, true);

	    // Decide on an InnerClasses attribute, if any:
	    // Put out a customized version for each class.
	    this.innerClasses.clear();
	    HashSet allICs = pkg.allInnerClasses;
	    boolean changed = true;
	    while (changed) {
		changed = false;
		// We must iterate to a fixpoint, because adding A$B$C
		// to the mix requires A$B to be considered also.
		// The number of iterations is bounded by the deepest
		// IC nesting (not much more than 2).
		for (Iterator i = allICs.iterator(); i.hasNext(); ) {
		    InnerClass ic = (InnerClass) i.next();
		    if (cpRefs.contains(ic.thisClass) ||
			// Relevant if the class is used somewhere inside me.
			ic.outerClass == this.thisClass
			// Relevant if the class is contained in me:
			   ) {
			// Add every relevant class to the IC attribute:
			if (this.innerClasses.add(ic)) {
			    // incrementally append to cpRefs, also:
			    ic.countRefs(this);
			    changed = true;
			}
		    }
		}
	    }
	    if (this.innerClasses.size() > 0) {
		// flesh out the local constant pool, again
		countRef(pkg.getRefString("InnerClasses"));
		getPackage().cp.addReferencedEntries(cpRefs, true);
	    }

	    // construct a local constant pool
	    int numDoubles = 0;
	    for (Iterator i = cpRefs.iterator(); i.hasNext(); ) {
		Entry e = (Entry) i.next();
		if (e.isDoubleWord())  numDoubles++;
	    }
	    Entry[] cpMap = new Entry[1+numDoubles+cpRefs.size()];
	    int fillp = 1;
	    if(D)assert0(cpRefs.containsAll(ldcRefs));
	    for (Iterator i = ldcRefs.iterator(); i.hasNext(); ) {
		Entry e = (Entry) i.next();
		cpMap[fillp++] = e;
	    }
	    if(D)assert0(fillp == 1+ldcRefs.size());
	    for (Iterator i = cpRefs.iterator(); i.hasNext(); ) {
		Entry e = (Entry) i.next();
		if (ldcRefs.contains(e))  continue;
		cpMap[fillp++] = e;
	    }
	    if(D)assert0(fillp == 1+cpRefs.size());
	    int narrowLimit = 1+ldcRefs.size();
	    Arrays.sort(cpMap, 1, narrowLimit);
	    Arrays.sort(cpMap, narrowLimit, fillp);

	    // Now repack backwards, introducing null elements.
	    int revp = cpMap.length;
	    for (int i = fillp; --i >= 1; ) {
		Entry e = cpMap[i];
		if (e.isDoubleWord())
		    cpMap[--revp] = null;
		cpMap[--revp] = e;
	    }
	    if(D)assert0(revp == 1);  // do not process the initial null

	    // record the local cp:
	    this.cpMap = cpMap;
	    if (verbose > 1)
		System.out.println("postProcess CP="+(verbose > 2 ? getCPIndex().dumpString() : getCPIndex().toString()));

	    // rewrite bytecode refs to the constant pool:
	    Method[] methods = getMethods();
	    for (int i = 0; i < methods.length; i++) {
		PRCode code = (PRCode) methods[i].code;
		if (code != null) {
		    code.insertIndexesIntoBytecodes();
		    code.adjustBCOffsets();
		}
	    }
	}
    }

    class PRCode extends Code {
	PRCode(Class.Method m) { super(m); }

	// append this stuff to Codes created by PackageWriter:
	byte[] codeOps;
	ArrayList fixups = new ArrayList();

	class Fixup {
	    final int pc;
	    final Entry ref;
	    Fixup(int pc, Entry ref) {
		this.pc = pc; this.ref = ref;
		fixups.add(this);
	    }
	}

	void noteRef(int pc, Entry ref) {
	    if(D)assert0(bytes[pc] != _ldc);
	    new Fixup(pc, ref);
	}
	void noteNarrowRef(int pc, Entry ref) {
	    if(D)assert0(bytes[pc] == _ldc);
	    new Fixup(pc, ref);
	    ((PRClass)thisClass()).ldcRefs.add(ref);
	}

	protected void countBytecodeRefs() { // override
	    if (false)  super.countBytecodeRefs();
	    if (verbose > 2)  System.out.println("countBytecodeRefs: #fixups="+fixups.size());
	    // use the fixups, only:
	    Class cls = thisClass();
	    for (Iterator i = fixups.iterator(); i.hasNext(); ) {
		Fixup f = (Fixup) i.next();
		if (verbose > 3)  System.out.println("BC fixup.ref "+f.ref);
		cls.countRef(f.ref);
	    }
	}

	// Rewrite the bytecodes.
	void insertIndexesIntoBytecodes() {
	    Index cpIndex = thisClass().getCPIndex();
	    for (Iterator i = fixups.iterator(); i.hasNext(); ) {
		Fixup f = (Fixup) i.next();
		int pc = f.pc;
		int op = getByte(pc);
		int nn = cpIndex.indexOf(f.ref);
		if(D)assert0(Instruction.isCPRefOp(op));
		if (op == _ldc) {
		    if(D)assert0(nn < 256);
		    setByte(pc+1, nn);
		} else {
		    if(D)assert0(Instruction.opLength(op) >= 3);
		    setShort(pc+1, nn);
		}
		if(D)assert0(instructionAt(pc).getCPRef(cpIndex.cpMap) == f.ref);
	    }
	}

	// Adjust BC offsets that may depend on overall code size.
	void adjustBCOffsets() {
	    int mod = this.getLength();
	    for (int j = 0; j < this.getHandlerCount(); j++) {
		if(D) assert0(this.handler_start[j] < mod);
		this.handler_end[j] %= mod;
		this.handler_catch[j] %= mod;
	    }
	}
    }

    PRCode[] allCodes;

    void readMembers(Class[] classes) throws IOException {
	int nc = classes.length;
	class_field_count.readFrom(in);
	class_method_count.readFrom(in);
	int[] numFields = class_field_count.getUnsignedArray();
	class_field_count.doneDisbursing();
	for (int i = 0; i < numFields.length; i++) {
	    field_flags.expectMoreLength(numFields[i]);
	    field_descr.expectMoreLength(numFields[i]);
	}
	int[] numMethods = class_method_count.getUnsignedArray();
	class_method_count.doneDisbursing();
	for (int i = 0; i < numMethods.length; i++) {
	    method_flags.expectMoreLength(numMethods[i]);
	    method_descr.expectMoreLength(numMethods[i]);
	}
	field_flags.readFrom(in);
	method_flags.readFrom(in);
	field_descr.readFrom(in);
	method_descr.readFrom(in);
	// list of clients for field_constant_value:
	ArrayList conFields = new ArrayList();
	// list of clients for method_exceptions:
	ArrayList excMethods = new ArrayList();
	// list of clients for the code attribute:
	ArrayList codeMethods = new ArrayList();
	for (int i = 0; i < classes.length; i++) {
	    Class c = classes[i];
	    int nf = numFields[i];
	    int nm = numMethods[i];
	    for (int j = 0; j < nf; j++) {
		int     flags    = field_flags.getFlags();
		boolean hasAttrs = (flags & ACC_HAS_ATTRIBUTES) != 0;
		flags &= ~ACC_HAS_ATTRIBUTES;
		boolean hasCon   = (flags & ACC_CONSTANT_VALUE) != 0;
		flags &= ~ACC_CONSTANT_VALUE;
		Class.Field f = c.new Field(flags, (DescriptorEntry)
					    field_descr.getRef());
		if (hasAttrs)  field_attrs.expectClient(f);
		if (hasCon) {
		    field_constant_value.expectMoreLength(1);
		    conFields.add(f);
		}
	    }
	    for (int j = 0; j < nm; j++) {
		int     flags    = method_flags.getFlags();
		boolean hasAttrs = (flags & ACC_HAS_ATTRIBUTES) != 0;
		flags &= ~ACC_HAS_ATTRIBUTES;
		boolean hasExc   = (flags & ACC_EXCEPTIONS) != 0;
		flags &= ~ACC_EXCEPTIONS;
		boolean hasCode  = Code.flagsRequireCode(flags);
		Class.Method m = c.new Method(flags, (DescriptorEntry)
					      method_descr.getRef());
		if (hasAttrs)  method_attrs.expectClient(m);
		if (hasCode)   codeMethods.add(m);
		if (hasExc) {
		    method_exception_count.expectMoreLength(1);
		    excMethods.add(m);
		}
	    }
	}
	field_flags.doneDisbursing();
	field_descr.doneDisbursing();
	method_flags.doneDisbursing();
	method_descr.doneDisbursing();
	// Read field ConstantValue attributes:
	field_constant_value.readFrom(in);
	for (Iterator i = conFields.iterator(); i.hasNext(); ) {
	    Class.Field f = (Class.Field) i.next();
	    byte tag = f.descriptor.getLiteralTag();
	    Index ix = getCPIndex(tag);
	    f.constantValue = (LiteralEntry) field_constant_value.getRef(ix);
	}
	field_constant_value.doneDisbursing();
	// Read method Exceptions attributes:
	method_exception_count.readFrom(in);
	for (Iterator i = excMethods.iterator(); i.hasNext(); ) {
	    Class.Method m = (Class.Method) i.next();
	    int ne = method_exception_count.getUnsigned();
	    if(D)assert0(ne > 0);
	    m.exceptions = new ClassEntry[ne];
	    method_exceptions.expectMoreLength(ne);
	}
	method_exception_count.doneDisbursing();
	method_exceptions.readFrom(in);
	for (Iterator i = excMethods.iterator(); i.hasNext(); ) {
	    Class.Method m = (Class.Method) i.next();
	    int ne = m.exceptions.length;
	    for (int j = 0; j < ne; j++) {
		m.exceptions[j] = (ClassEntry) method_exceptions.getRef();
	    }
	}
	method_exceptions.doneDisbursing();
	// Get ready to read code attributes:
	allCodes = new PRCode[codeMethods.size()];
	int fillp = 0;
	for (Iterator i = codeMethods.iterator(); i.hasNext(); ) {
	    Class.Method m = (Class.Method) i.next();
	    PRCode code = new PRCode(m);
	    m.code = code;
	    allCodes[fillp++] = code;
	}
    }

    void readCodeHeaders() throws IOException {
	code_headers.expectLength(allCodes.length);
	code_headers.readFrom(in);
	ArrayList longCodes = new ArrayList();
	for (int i = 0; i < allCodes.length; i++) {
	    Code c = allCodes[i];
	    int sc = code_headers.getUnsigned();
	    if(D)assert0(sc == (sc & 0xFF));
	    if (verbose > 2)
		System.out.println("codeHeader "+c+" = "+sc);
	    if (sc == 0) {
		code_max_stack.expectMoreLength(1);
		code_max_locals.expectMoreLength(1);
		code_handler_count.expectMoreLength(1);
		code_attrs.expectClient(c);
		longCodes.add(c);
		continue;
	    }
	    // Short code header is the usual case:
	    c.max_stack  = shortCodeHeader_max_stack(sc);
	    c.max_locals = shortCodeHeader_max_locals(sc, c.getMethod());
	    int nh = shortCodeHeader_handler_count(sc);
	    c.setHandlerCount(nh);
	    if(D)assert0(shortCodeHeader(c) == sc);
	}
	code_headers.doneDisbursing();
	// Do the long headers now.
	code_max_stack.readFrom(in);
	code_max_locals.readFrom(in);
	code_handler_count.readFrom(in);
	for (Iterator i = longCodes.iterator(); i.hasNext(); ) {
	    Code c = (Code) i.next();
	    c.max_stack  = code_max_stack.getUnsigned();
	    c.max_locals = code_max_locals.getUnsigned();
	    int nh = code_handler_count.getUnsigned();
	    c.setHandlerCount(nh);
	}
	code_max_stack.doneDisbursing();
	code_max_locals.doneDisbursing();
	code_handler_count.doneDisbursing();
	// Read handler specifications.
	int nh = 0;
	for (int i = 0; i < allCodes.length; i++) {
	    Code c = allCodes[i];
	    nh += c.getHandlerCount();
	}
	ValueBand[] code_handler_bands = {
	    code_handler_start,
	    code_handler_end,
	    code_handler_catch,
	    code_handler_class
	};
	for (int i = 0; i < code_handler_bands.length; i++) {
	    code_handler_bands[i].expectLength(nh);
	    code_handler_bands[i].readFrom(in);
	}
	for (int i = 0; i < allCodes.length; i++) {
	    Code c = allCodes[i];
	    int sum;
	    for (int j = 0; j < c.getHandlerCount(); j++) {
		c.handler_class[j] = code_handler_class.getRef();  // null OK
		sum = code_handler_start.getUnsigned();
		c.handler_start[j] = sum;
		sum += code_handler_end.getUnsigned();
		c.handler_end[j]   = sum;
		sum += code_handler_catch.getUnsigned();
		c.handler_catch[j] = sum;
		// Note:  Sums may overflow code length,
		// in order to express negative offsets via
		// unsigned numbers.  These are fixed up
		// later in postprocessing, in adjustBCOffsets().
	    }
	}
	for (int i = 0; i < code_handler_bands.length; i++) {
	    code_handler_bands[i].doneDisbursing();
	}
    }

    // Read package attributes.
    void readAttributes() throws IOException {
	// xxx_attrs.expectClient() calls have set up the counts for these:
	code_attrs.readCountsFrom(in);
	method_attrs.readCountsFrom(in);
	field_attrs.readCountsFrom(in);
	class_attrs.readCountsFrom(in);
	package_attrs.expectClient(pkg);
	package_attrs.setCounts(new int[]{ numPackageAttrs });

	code_attrs.readNamesFrom(in);
	method_attrs.readNamesFrom(in);
	field_attrs.readNamesFrom(in);
	class_attrs.readNamesFrom(in);
	package_attrs.readNamesFrom(in);

	code_attrs.readSizesFrom(in);
	method_attrs.readSizesFrom(in);
	field_attrs.readSizesFrom(in);
	class_attrs.readSizesFrom(in);
	package_attrs.readSizesFrom(in);

	// Got the raw bits.  Now parse the attributes.
	code_attrs.readBitsFrom(in);
	readCodeAttrs();
	code_attrs.doneDisbursing();

	method_attrs.readBitsFrom(in);
	readMemberAttrs(method_attrs);
	method_attrs.doneDisbursing();

	field_attrs.readBitsFrom(in);
	readMemberAttrs(field_attrs);
	field_attrs.doneDisbursing();

	class_attrs.readBitsFrom(in);
	readClassAttrs();
	class_attrs.doneDisbursing();

	package_attrs.readBitsFrom(in);
	readPackageAttrs();
	package_attrs.doneDisbursing();
    }

    void readCodeAttrs() throws IOException {
	while (code_attrs.hasNextBlock()) {
	    AttrBlock b = code_attrs.nextBlock();
	    String name = b.name();
	    Code   code = (Code) b.client;
	    if (name == "LineNumberTable") {
		int nl = b.getUnsigned();
		if (nl > 0) {
		    code.lineNumberTable_pc = new int[nl];
		    code.lineNumberTable_line = new int[nl];
		}
		for (int i = 0; i < nl; i++) {
		    code.lineNumberTable_pc[i] = b.getUnsigned();
		    code.lineNumberTable_line[i] = b.getUnsigned();
		}
	    } else if (name == "LocalVariableTable") {
		int nv = b.getUnsigned();
		if (nv > 0) {
		    code.localVariableTable_start = new int[nv];
		    code.localVariableTable_end = new int[nv];
		    code.localVariableTable_name = new Entry[nv];
		    code.localVariableTable_type = new Entry[nv];
		    code.localVariableTable_slot = new int[nv];
		}
		for (int i = 0; i < nv; i++) {
		    int start = b.getUnsigned();
		    int span  = b.getUnsigned();
		    code.localVariableTable_start[i] = start;
		    code.localVariableTable_end[i] = start + span;
		    code.localVariableTable_name[i] = (Utf8Entry) b.getSingleRef(CONSTANT_Utf8);
		    code.localVariableTable_type[i] = b.getSingleRef(CONSTANT_Signature);
		    code.localVariableTable_slot[i] = b.getUnsigned();
		}
	    } else {
		b.skip();
	    }
	}
    }

    void readMemberAttrs(AttrBands attrs) throws IOException {
	while (attrs.hasNextBlock()) {
	    AttrBlock b = attrs.nextBlock();
	    String name = b.name();
	    Class.Member m = (Class.Member) b.client;
	    if (name == "Deprecated") {
		m.isDeprecated = true;
	    } else if (name == "Synthetic") {
		m.isSynthetic = true;
	    } else {
		if(D)assert0(!(name == "Exceptions" && m instanceof Class.Method));
		b.skip();
	    }
	}
    }

    void readClassAttrs() throws IOException {
	while (class_attrs.hasNextBlock()) {
	    AttrBlock b = class_attrs.nextBlock();
	    String name = b.name();
	    Class c = (Class) b.client;
	    if (name == "Deprecated") {
		c.isDeprecated = true;
	    } else if (name == "Synthetic") {
		c.isSynthetic = true;
	    } else if (name == "SourceFile") {
		c.sourceFile = (Utf8Entry) b.getSingleRef(CONSTANT_Utf8);
	    } else if (name == CLASS_FILE_VERSION) {
		c.retroVersion = (NumberEntry) b.getSingleRef(CONSTANT_Integer);
	    } else {
		b.skip();
	    }
	}
    }

    void readPackageAttrs() throws IOException {
	while (package_attrs.hasNextBlock()) {
	    AttrBlock b = package_attrs.nextBlock();
	    String name = b.name();
	    if (pkg.isFileName(name)) {
		// It's a side file.
		if (verbose > 1)
		    System.out.println("Reading "+b.length()+" bytes of "+name);
		Package.File file = pkg.new File(name);
		file.contents = b.getBytes();
		pkg.addFile(file);
	    } else {
		System.out.println("Skipping unrecognized package attribute "+name);
	    }
	}
    }

    void readByteCodes() throws IOException {
	bc_codes.elementCountForDebug = allCodes.length;
	bc_codes.setInputStreamFrom(in);
	readByteCodeOps();  // reads from bc_codes and bc_case_count
	bc_codes.doneDisbursing();

	IntBand[] bc_ints = { bc_case_value,
			      bc_byte, bc_short,
			      bc_local, bc_label };
	for (int i = 0; i < bc_ints.length; i++) {
	    IntBand bc_which = bc_ints[i];
	    bc_which.readFrom(in);
	}

	CPRefBand[] bc_refs = { bc_intref, bc_floatref,
				bc_longref, bc_doubleref, bc_stringref,
				bc_classref, bc_fieldref,
				bc_methodref, bc_imethodref,
				bc_thismethod, bc_supermethod,
				bc_thisfield, bc_superfield };
	for (int i = 0; i < bc_refs.length; i++) {
	    CPRefBand bc_which = bc_refs[i];
	    bc_which.readFrom(in);
	}

	bc_initref.readFrom(in);

	expandByteCodeOps();

	// Done fetching values from operand bands:
	Band[] operand_bands_for_debug = {
	    bc_case_value,
	    bc_byte, bc_short,
	    bc_local, bc_label,
	    bc_intref, bc_floatref,
	    bc_longref, bc_doubleref, bc_stringref,
	    bc_classref, bc_fieldref,
	    bc_methodref, bc_imethodref,
	    bc_thismethod, bc_supermethod,
	    bc_thisfield, bc_superfield,
	    bc_initref
	};
	for (int i = 0; i < operand_bands_for_debug.length; i++) {
	    operand_bands_for_debug[i].doneDisbursing();
	}
    }

    // scratch buffer for collecting code bytes (in two passes):
    byte[] buf = new byte[CODE_SIZE_LIMIT+1];
    // side array of switch instruction lengths:
    int[] allSwitchLengths;

    private void readByteCodeOps() throws IOException {
	ArrayList allSwitchTypes = new ArrayList();
	for (int k = 0; k < allCodes.length; k++) {
	    PRCode c = allCodes[k];
	scanOneMethod:
	    for (int i = 0; ; i++) {
		int bc = bc_codes.getByte();
		buf[i] = (byte)bc;
		boolean isWide = false;
		if (bc == _wide) {
		    bc = bc_codes.getByte();
		    buf[++i] = (byte)bc;
		    isWide = true;
		}
		if(D)assert0(bc == (0xFF & bc));
		// Adjust expectations of various band sizes.
		switch (bc) {
		case _tableswitch:
		case _lookupswitch:
		    bc_case_count.expectMoreLength(1);
		    allSwitchTypes.add(bc == _tableswitch ? Boolean.TRUE : Boolean.FALSE);
		    break;
		case _iinc:
		    {
			bc_local.expectMoreLength(1);
			IntBand bc_which = isWide ? bc_short : bc_byte;
			bc_which.expectMoreLength(1);
		    }
		    break;
		case _sipush:
		    bc_short.expectMoreLength(1);
		    break;
		case _bipush:
		    bc_byte.expectMoreLength(1);
		    break;
		case _newarray:
		    bc_byte.expectMoreLength(1);
		    break;
		case _multianewarray:
		    if(D)assert0(getCPRefOpBand(bc) == bc_classref);
		    bc_classref.expectMoreLength(1);
		    bc_byte.expectMoreLength(1);
		    break;
		default:
		    if (Instruction.isInvokeInitOp(bc)) {
			bc_initref.expectMoreLength(1);
			break;
		    }
		    if (Instruction.isSelfLinkerOp(bc)) {
			CPRefBand bc_which = selfOpRefBand(bc);
			bc_which.expectMoreLength(1);
			break;
		    }
		    if (Instruction.isBranchOp(bc)) {
			bc_label.expectMoreLength(1);
			break;
		    }
		    if (Instruction.isCPRefOp(bc)) {
			CPRefBand bc_which = getCPRefOpBand(bc);
			bc_which.expectMoreLength(1);
			if(D)assert0(bc != _multianewarray);  // handled elsewhere
			break;
		    }
		    if (Instruction.isLocalSlotOp(bc)) {
			bc_local.expectMoreLength(1);
			break;
		    }
		    break;
		case _end_marker:
		    {
			// Transfer from buf to a more permanent place:
			int opCount = i;
			c.codeOps = new byte[opCount];
			System.arraycopy(buf, 0, c.codeOps, 0, opCount);
			break scanOneMethod;
		    }
		}
	    }
	}

	// To size instruction bands correctly, we need info on switches:
	bc_case_count.readFrom(in);
	allSwitchLengths = bc_case_count.getUnsignedArray();
	bc_case_count.doneDisbursing();
	if(D)assert0(allSwitchLengths.length == allSwitchTypes.size());
	for (int i = 0; i < allSwitchLengths.length; i++) {
	    int caseCount = allSwitchLengths[i];
	    int bc = allSwitchTypes.get(i) == Boolean.TRUE ? _tableswitch: _lookupswitch;
	    bc_label.expectMoreLength(1+caseCount); // default label + cases
	    bc_case_value.expectMoreLength(bc == _tableswitch ? 1 : caseCount);
	}
    }

    private void expandByteCodeOps() throws IOException {
	int nsw = 0;  // index into allSwitchLengths
	for (int k = 0; k < allCodes.length; k++) {
	    PRCode code = allCodes[k];
	    if(D)assert0(code.bytes == null);
	    code.bytes = buf;  // just for a minute
	    byte[] codeOps = code.codeOps;

	    Class curClass = (Class) code.thisClass();

	    ClassEntry thisClass  = curClass.thisClass;
	    ClassEntry superClass = curClass.superClass;
	    ClassEntry newClass   = null;  // class of last _new opcode

	    int pc = 0;  // fill pointer in buf; actual bytecode PC
	    boolean doAssert = false;
	    if(D)assert0(doAssert = true);
	    if (doAssert) { for (int i = 0; i < buf.length; i++)  buf[i] = 0; }
	    for (int i = 0; i < codeOps.length; i++) {
		int bc = Instruction.getByte(codeOps, i);
		int curPC = pc;
		boolean isWide = false;
		if (bc == _wide) {
		    buf[pc++] = (byte) bc;
		    bc = Instruction.getByte(codeOps, ++i);
		    isWide = true;
		}
		switch (bc) {
		case _tableswitch: // apc:  (df, lo, hi, (hi-lo+1)*(label))
		case _lookupswitch: // apc:  (df, nc, nc*(case, label))
		    {
			int caseCount = allSwitchLengths[nsw++];
			int switchPC = pc;
			buf[pc++] = (byte) bc;
			int apc = Instruction.Switch.alignPC(pc);
			while (pc < apc)  buf[pc++] = 0;
			int df = bc_label.getSigned();
			Instruction.setInt(buf, apc+0, df);
			if (bc == _tableswitch) {
			    int lo = bc_case_value.getSigned();
			    int hi = lo + caseCount-1;
			    Instruction.setInt(buf, apc+4, lo);
			    Instruction.setInt(buf, apc+8, hi);
			    pc = apc+12;
			    for (int j = 0; j < caseCount; j++) {
				int lVal = bc_label.getSigned();
				//int cVal = lo + j;
				Instruction.setInt(buf, pc, lVal);
				pc += 4;
			    }
			} else {
			    Instruction.setInt(buf, apc+4, caseCount);
			    pc = apc+8;
			    for (int j = 0; j < caseCount; j++) {
				int cVal  = bc_case_value.getSigned();
				int lVal = bc_label.getSigned();
				Instruction.setInt(buf, pc+0, cVal);
				Instruction.setInt(buf, pc+4, lVal);
				pc += 8;
			    }
			}
			continue;
		    }
		case _iinc:
		    {
			buf[pc++] = (byte) bc;
			int local = bc_local.getUnsigned();
			int delta = (isWide ? bc_short : bc_byte).getSigned();
			if (isWide) {
			    Instruction.setShort(buf, pc, local); pc += 2;
			    Instruction.setShort(buf, pc, delta); pc += 2;
			} else {
			    buf[pc++] = (byte)local;
			    buf[pc++] = (byte)delta;
			}
			continue;
		    }
		case _sipush:
		    {
			int val = bc_short.getSigned();
			buf[pc++] = (byte) bc;
			Instruction.setShort(buf, pc, val); pc += 2;
			continue;
		    }
		case _bipush:
		case _newarray:
		    {
			int val = bc_byte.getSigned();
			buf[pc++] = (byte) bc;
			buf[pc++] = (byte) val;
			continue;
		    }
		default:
		    if (Instruction.isInvokeInitOp(bc)) {
			int idx = (bc - _invokeinit_op);
			int origBC = _invokespecial;
			ClassEntry classRef;
			switch (idx) {
			case _invokeinit_self_option:
			    classRef = thisClass; break;
			case _invokeinit_super_option:
			    classRef = superClass; break;
			default:
			    if(D)assert0(idx == _invokeinit_new_option);
			    classRef = newClass; break;
			}
			buf[pc++] = (byte) origBC;
			int coding = bc_initref.getUnsigned();
			// Find the nth overloading of <init> in classRef.
			MemberEntry ref = pkg.cp.getOverloadingForIndex(CONSTANT_Methodref, classRef, "<init>", coding);
			code.noteRef(curPC, ref); pc += 2;
			if(D)assert0(Instruction.opLength(origBC) == (pc - curPC));
			continue;
		    }
		    if (Instruction.isSelfLinkerOp(bc)) {
			int idx = (bc - _self_linker_op);
			boolean isSuper = (idx >= _self_linker_super_flag);
			if (isSuper)  idx -= _self_linker_super_flag;
			boolean isAload = (idx >= _self_linker_aload_flag);
			if (isAload)  idx -= _self_linker_aload_flag;
			int origBC = _first_linker_op + idx;
			boolean isField = Instruction.isFieldOp(origBC);
			CPRefBand bc_which;
			ClassEntry which_cls  = isSuper ? superClass : thisClass;
			Index which_ix;
			if (isField) {
			    bc_which = isSuper ? bc_superfield  : bc_thisfield;
			    which_ix = pkg.cp.getMemberIndex(CONSTANT_Fieldref, which_cls);
			} else {
			    bc_which = isSuper ? bc_supermethod : bc_thismethod;
			    which_ix = pkg.cp.getMemberIndex(CONSTANT_Methodref, which_cls);
			}
			if(D)assert0(bc_which == selfOpRefBand(bc));
			MemberEntry ref = (MemberEntry) bc_which.getRef(which_ix);
			if (isAload) {
			    buf[pc++] = (byte) _aload_0;
			    curPC = pc;
			}
			buf[pc++] = (byte) origBC;
			code.noteRef(curPC, ref); pc += 2;
			if(D)assert0(Instruction.opLength(origBC) == (pc - curPC));
			continue;
		    }
		    if (Instruction.isBranchOp(bc)) {
			buf[pc++] = (byte) bc;
			int lVal = bc_label.getSigned();
			if(D)assert0(!isWide);  // no wide prefix for branches
			switch (Instruction.opLength(bc)) {
			case 3:
			    Instruction.setShort(buf, pc, lVal); pc += 2;
			    break;
			case 5:
			    Instruction.setInt(buf, pc, lVal); pc += 4;
			    break;
			default: if(D)assert0(false);
			}
			if(D)assert0(Instruction.opLength(bc) == (pc - curPC));
			continue;
		    }
		    if (Instruction.isCPRefOp(bc)) {
			CPRefBand bc_which = getCPRefOpBand(bc);
			Entry ref = bc_which.getRef();
			if (ref == null) {
			    if (bc_which == bc_classref) {
				// Shorthand for class self-references.
				ref = thisClass;
			    } else {
				if(D)assert0(false);
			    }
			}
			int origBC = bc;
			boolean isNarrow = false;
			switch (bc) {
			case _ildc:
			case _fldc:
			case _aldc:
			    origBC = _ldc;
			    isNarrow = true;
			    break;
			case _ildc_w:
			case _fldc_w:
			case _aldc_w:
			    origBC = _ldc_w;
			    break;
			case _lldc2_w:
			case _dldc2_w:
			    origBC = _ldc2_w;
			    break;
			case _new:
			    newClass = (ClassEntry) ref;
			    break;
			}
			buf[pc++] = (byte) origBC;
			if (isNarrow) {
			    code.noteNarrowRef(curPC, ref); pc += 1;
			} else {
			    code.noteRef(curPC, ref); pc += 2;
			}
			if (origBC == _multianewarray) {
			    // Copy the trailing byte also.
			    int val = bc_byte.getSigned();
			    buf[pc++] = (byte) val;
			} else if (origBC == _invokeinterface) {
			    int argSize = ((MemberEntry)ref).descRef.typeRef.computeSize(true);
			    buf[pc++] = (byte)( 1 + argSize );
			    buf[pc++] = 0;
			}
			if(D)assert0(Instruction.opLength(origBC) == (pc - curPC));
			continue;
		    }
		    if (Instruction.isLocalSlotOp(bc)) {
			buf[pc++] = (byte) bc;
			int local = bc_local.getUnsigned();
			if (isWide) {
			    Instruction.setShort(buf, pc, local);
			    pc += 2;
			    if (bc == _iinc) {
				int iVal = bc_short.getSigned();
				Instruction.setShort(buf, pc, iVal);
				pc += 2;
			    }
			} else {
			    Instruction.setByte(buf, pc, local);
			    pc += 1;
			    if (bc == _iinc) {
				int iVal = bc_byte.getSigned();
				Instruction.setByte(buf, pc, iVal);
				pc += 1;
			    }
			}
			if(D)assert0(Instruction.opLength(bc) == (pc - curPC));
			continue;
		    }
		    // Random bytecode.  Just copy it.
		    if (bc >= _bytecode_limit)  System.out.println("*** bc?? "+bc+" "+Instruction.byteName(bc));
		    if(D)assert0(bc < _bytecode_limit);
		    buf[pc++] = (byte) bc;
		    if(D)assert0(Instruction.opLength(bc) == (pc - curPC));
		    continue;
		}
	    }
	    // now make a permanent copy of the bytecodes
	    code.bytes = new byte[pc];
	    System.arraycopy(buf, 0, code.bytes, 0, pc);
	}
    }
}
