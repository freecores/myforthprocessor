/*
 * @(#)Package.java	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.pack;

import java.lang.reflect.Modifier;
import java.util.*;
import java.util.zip.*;
import java.util.jar.*;
import sun.tools.pack.ConstantPool.*;

class Package implements Constants {
    public static int verbose = 0;

    int magic;
    int package_major_version;
    int package_minor_version;

    int default_class_major_version = -1; // fill in later
    int default_class_minor_version = 0;  // fill in later

    // What constants are used in this unit?
    ConstantPool cp = new ConstantPool();

    Package() {
	magic = JAVA_PACKAGE_MAGIC;
	package_major_version = JAVA_PACKAGE_MAJOR_VERSION;
	package_minor_version = JAVA_PACKAGE_MINOR_VERSION;
    }

    void checkVersion() {
	if (magic != JAVA_PACKAGE_MAGIC) {
	    String gotMag = Integer.toHexString(magic);
	    String expMag = Integer.toHexString(JAVA_PACKAGE_MAGIC);
	    throw new RuntimeException("Unexpected package magic number: got "+gotMag+"; expected "+expMag);
	}
	if (package_major_version != JAVA_PACKAGE_MAJOR_VERSION ||
	    package_minor_version != JAVA_PACKAGE_MINOR_VERSION) {
	    String gotVer = package_major_version+"."+package_minor_version;
	    String expVer = JAVA_PACKAGE_MAJOR_VERSION+"."+JAVA_PACKAGE_MINOR_VERSION;
	    throw new RuntimeException("Unexpected package minor version: got "+gotVer+"; expected "+expVer);
	}
    }

    // What Java classes are in this unit?
    ArrayList classes = new ArrayList();

    class Class implements Comparable {
	public Package getPackage() { return Package.this; }

	String classFile;  // optional source of class bits

	// File header
	int magic;
	int major_version;
	int minor_version;

	// Local constant pool (one-way mapping of index => package cp).
	Entry[] cpMap;
	Index   cpIndex; // not used on input
	HashSet cpRefs;  // not used on input

	// Class header
	int flags;
	ClassEntry thisClass;
	ClassEntry superClass;
	ClassEntry[] interfaces;

	// Class members
	HashSet members = new HashSet();

	// Class attributes
	boolean isDeprecated;
	boolean isSynthetic;
	Utf8Entry sourceFile;
	NumberEntry retroVersion;
	HashSet innerClasses = new HashSet();

	Class(int flags, ClassEntry thisClass, ClassEntry superClass, ClassEntry[] interfaces) {
	    this.flags      = flags;
	    this.thisClass  = thisClass;
	    this.superClass = superClass;
	    this.interfaces = interfaces;
	    boolean added = classes.add(this);
	    if(D)assert0(added);
	}

	Class(String classFile) {
	    this.classFile = classFile;
	    // A blank class; must be read with a ClassReader, etc.
	}

	public String getName() {
	    return thisClass.stringValue();
	}

	public int attrCount() {
	    int na = 0;
	    if (retroVersion != null)     na++;
	    if (isDeprecated)             na++;
	    if (isSynthetic)              na++;
	    if (sourceFile != null)       na++;
	    if (innerClasses.size() > 0)  na++;
	    // Note that InnerClasses may be collected at the package level.
	    return na;
	}

	// The name is the key.  Ignore other things.
	public boolean equals(Object o) {
	    Class that = (Class)o;
	    // classes of same name must be identical
	    if(D)assert0(that.thisClass != this.thisClass || this == that);
	    return that.thisClass == this.thisClass;
	}
	public int hashCode() {
	    return thisClass.hashCode();
	}
	public int compareTo(Object o) {
	    Class that = (Class)o;
	    String n0 = this.getName();
	    String n1 = that.getName();
	    return n0.compareTo(n1);
	}

	abstract class Member implements Comparable {
	    int flags;
	    DescriptorEntry descriptor;
	    // Member attributes
	    boolean isDeprecated;
	    boolean isSynthetic;

	    public Member(int flags, DescriptorEntry descriptor) {
		this.flags = flags;
		this.descriptor = descriptor;
		boolean added = members.add(this);
		if(D)assert0(added);
	    }

	    public Class thisClass() { return Class.this; }

	    public DescriptorEntry getDescriptor() {
		return descriptor;
	    }
	    public String getName() {
		return descriptor.nameRef.stringValue();
	    }
	    public String getType() {
		return descriptor.typeRef.stringValue();
	    }

	    boolean strip(String what) {
		if (what == "Compile") {
		    isDeprecated = false;
		    isSynthetic = false;
		}
		return false;
	    }
	    Entry[] getCPMap() {
		return cpMap;
	    }
	    void countRefs() {
		if (verbose > 2)  System.out.println("countRefs "+this);
		// Careful:  The descriptor is used by the package,
		// but the classfile breaks it into component refs.
		countRef(descriptor.nameRef);
		countRef(descriptor.typeRef);
		Package.this.countRef(descriptor);
		if (isDeprecated)
		    countRef(getRefString("Deprecated"));
		if (isSynthetic)
		    countRef(getRefString("Synthetic"));
	    }

	    public int attrCount() {
		int na = 0;
		if (isDeprecated)           na++;
		if (isSynthetic)            na++;
		return na;
	    }

	    public String toString() {
		return Class.this + "." + descriptor.prettyString();
	    }
	}

	class Field extends Member {
	    // Order is significant for fields:  It is visible to reflection.
	    int order = members.size();

	    // Field attributes
	    LiteralEntry constantValue;

	    public Field(int flags, DescriptorEntry descriptor) {
		super(flags, descriptor);
		if(D)assert0(!descriptor.isMethod());
	    }

	    public int attrCount() {
		int na = super.attrCount();
		if (constantValue != null)  na++;
		return na;
	    }

	    public int compareTo(Object o) {
		Field that = (Field)o;
		return this.order - that.order;
	    }

	    boolean strip(String what) {
		if (super.strip(what)) {
		    return true;
		}
		if (what == "Constant"
		    && constantValue != null
		    && Modifier.isFinal(super.flags)
		    // do not strip non-static finals:
		    && Modifier.isStatic(super.flags)
		    && !super.getName().startsWith("serial")) {
		    if (verbose > 2) {
			System.out.println(">> Strip "+this+" = "+constantValue.stringValue());
			return true;
		    }
		}
		return false;
	    }
	    void countRefs() {
		super.countRefs();
		if (constantValue != null) {
		    maybeCountRef(getRefString("ConstantValue"));
		    countRef(constantValue);
		}
	    }
	}

	class Method extends Member {
	    // Method attributes
	    Code code;
	    ClassEntry[] exceptions = noClassRefs;

	    public Method(int flags, DescriptorEntry descriptor) {
		super(flags, descriptor);
		if(D)assert0(descriptor.isMethod());
	    }

	    public int getArgumentSize() {
		int argSize  = descriptor.typeRef.computeSize(true);
		int thisSize = Modifier.isStatic(flags) ? 0 : 1;
		return thisSize + argSize;
	    }

	    public int attrCount() {
		int na = super.attrCount();
		if (exceptions.length > 0)  na++;
		if (code != null)           na++;
		return na;
	    }

	    // Sort methods in a canonical order (by type, then by name).
	    public int compareTo(Object o) {
		Method that = (Method)o;
		return this.getDescriptor().compareTo(that.getDescriptor());
	    }

	    boolean strip(String what) {
		if (super.strip(what)) {
		    return true;
		}
		if (what == "Exceptions") {
		    // Keep the exceptions normally.
		    // Although they have no effect on execution,
		    // the Reflection API exposes them, and JCK checks them.
		    exceptions = noClassRefs;
		}
		if (code != null && code.strip(what)) {
		    code = null;
		}
		return false;
	    }
	    void countRefs() {
		super.countRefs();
		if (code != null) {
		    maybeCountRef(getRefString("Code"));
		    code.countRefs();
		}
		if (exceptions.length > 0)
		    maybeCountRef(getRefString("Exceptions"));
		for (int i = 0; i < exceptions.length; i++)
		    countRef(exceptions[i]);
	    }
	}

	Field[] getFields() {
	    ArrayList ma = new ArrayList(members.size());
	    for (Iterator i = members.iterator(); i.hasNext(); ) {
		Object m = i.next();
		if (m instanceof Field)
		    ma.add(m);
	    }
	    //System.out.println(this+" getFields => "+ma);
	    Field[] fields = new Field[ma.size()];
	    ma.toArray(fields);
	    Arrays.sort(fields);
	    return fields;
	}
	Method[] getMethods() {
	    ArrayList ma = new ArrayList(members.size());
	    for (Iterator i = members.iterator(); i.hasNext(); ) {
		Object m = i.next();
		if (m instanceof Method)
		    ma.add(m);
	    }
	    Method[] methods = new Method[ma.size()];
	    ma.toArray(methods);
	    Arrays.sort(methods);
	    return methods;
	}

	void checkForRetroVersion() {
	    // Nonstandard magic numbers are treated as a special attribute
	    if (retroVersion != null)  return;  // do not do it twice
	    Package pkg = Package.this;
	    Class   cls = this;
	    if (cls.major_version != pkg.default_class_major_version ||
		cls.minor_version != pkg.default_class_minor_version) {
		String clsVer = cls.major_version+"."+cls.minor_version;
		String pkgVer = pkg.default_class_major_version+"."+pkg.default_class_minor_version;
		if (verbose > 1)
		    System.out.println("Version "+clsVer+" of "+this+
				       " doesn't match package version "+
				       pkgVer);
		int version = (cls.major_version << 16) | cls.minor_version;
		if(D)assert0(cls.minor_version >= 0 && cls.minor_version < 1<<16);
		retroVersion = (NumberEntry) getRefLiteral(new Integer(version));
	    }
	}

	void setMagicNumbers() {
	    magic = JAVA_MAGIC;
	    if (retroVersion == null) {
		major_version = default_class_major_version;
		minor_version = default_class_minor_version;
	    } else {
		int version = retroVersion.numberValue().intValue();
		major_version = version >>> 16;
		minor_version = version & 0xFFFF;
	    }
	}

	boolean strip(String what) {
	    for (Iterator i = members.iterator(); i.hasNext(); ) {
		Member m = (Member)i.next();
		if (m.strip(what))  i.remove();
	    }
	    if (what == "Debug") {
		sourceFile = null;
	    }
	    if (what == "Compile") {
		isDeprecated = false;
		isSynthetic = false;
	    }
	    return false;
	}

	Entry[] getCPMap() {
	    return cpMap;
	}

	Index getCPIndex() {
	    if (cpIndex == null || cpIndex.cpMap != cpMap)
		cpIndex = cp.new Index(thisClass.stringValue(), cpMap);
	    return cpIndex;
	}

	void countRefs() {
	    countRef(thisClass);
	    countRef(superClass);
	    for (int i = 0; i < interfaces.length; i++) {
		countRef(interfaces[i]);
	    }
	    for (Iterator i = members.iterator(); i.hasNext(); ) {
		Member m = (Member)i.next();
		boolean ok = false;
		try {
		    m.countRefs();
		    ok = true;
		} finally {
		    if (!ok)
			System.out.println("*** Error scanning "+m);
		}
	    }
	    if (isDeprecated)
		countRef(getRefString("Deprecated"));
	    if (isSynthetic)
		countRef(getRefString("Synthetic"));
	    if (sourceFile != null) {
		countRef(getRefString("SourceFile"));
		countRef(sourceFile);
	    }
	    if (innerClasses.size() > 0) {
		maybeCountRef(getRefString("InnerClasses"));
		countInnerClassRefs(innerClasses, this);
	    }
	    if (retroVersion != null) {
		countRef(getRefString(CLASS_FILE_VERSION));
		countRef(retroVersion);
	    }
	}

	void countRef(Entry ref) {
	    if (ref != null) {
		Package.this.countRef(ref);
		if (cpRefs != null)
		    cpRefs.add(ref);
	    }
	}
	// Use this to count refs that are implicit in the package
	// file format but explicit in the class file format.
	void maybeCountRef(Entry ref) {
	    //countRef(reF);
	}

	// Use this as a hook when reading classes from package files.
	void reconstruct() {
	    // Empty, except in PackageReader.
	}

	public java.io.File getFileName() {
	    return getFileName(null);
	}
	public java.io.File getFileName(java.io.File parent) {
	    String name = thisClass.stringValue();
	    name = name.replace('/', java.io.File.separatorChar);
	    return new java.io.File(parent, name + ".class");
	}

	public String toString() {
	    return thisClass.stringValue();
	}
    }

    void addClass(Class c) {
	if(D)assert0(c.getPackage() == this);
	boolean added = classes.add(c);
	if(D)assert0(added);

	// Capture most recent version number:
	if (default_class_major_version < c.major_version
	    || (default_class_major_version == c.major_version
		&& default_class_minor_version < c.minor_version)) {
	    default_class_major_version = c.major_version;
	    default_class_minor_version = c.minor_version;
	}

	// Capture inner classes:
	for (Iterator i = c.innerClasses.iterator(); i.hasNext(); ) {
	    InnerClass ic = (InnerClass) i.next();
	    i.remove();
	    addInnerClass(ic);
	}
    }

    // What non-class files are in this unit?
    ArrayList files = new ArrayList();

    class File implements Comparable {
	Utf8Entry name;
	byte[] contents;

	File(String name) {
	    this.name = getRefString(fixupFileName(name));
	    // caller must fill in contents
	}

	// The name is the key.  Ignore other things.
	public boolean equals(Object o) {
	    File that = (File)o;
	    return that.name == this.name;
	}
	public int hashCode() {
	    return name.hashCode();
	}
	public int compareTo(Object o) {
	    File that = (File)o;
	    String f0 = this.name.stringValue();
	    String f1 = that.name.stringValue();
	    String n0 = f0.substring(1+f0.lastIndexOf('/'));
	    String n1 = f1.substring(1+f1.lastIndexOf('/'));
	    String x0 = n0.substring(1+n0.lastIndexOf('.'));
	    String x1 = n1.substring(1+n1.lastIndexOf('.'));
	    int r;
	    // Primary sort key is file extension.
	    // This keeps files of similar format near each other.
	    r = x0.compareTo(x1);
	    if (r != 0)  return r;
	    r = f0.compareTo(f1);
	    return r;
	}

	public java.io.File getFileName() {
	    return getFileName(null);
	}
	public java.io.File getFileName(java.io.File parent) {
	    String name = this.name.stringValue();
	    if (name.startsWith("./"))  name = name.substring(2);
	    name = name.replace('/', java.io.File.separatorChar);
	    return new java.io.File(parent, name);
	}

	void countRefs() {
	    countRef(name);
	}
    }

    private static String fixupFileName(String name) {
	name = name.replace(java.io.File.separatorChar, '/');
	if(D)assert0(!name.startsWith("/"));
	if (name.indexOf('/') < 0)  name = "./" + name;
	return name;
    }

    public static boolean isFileName(String name) {
	return name.indexOf('/') > 0;
    }

    void addFile(File file) {
	boolean added = files.add(file);
	if(D)assert0(added);
    }

    // What inner classes have been declared in this unit?
    HashSet allInnerClasses = new HashSet();

    static class InnerClass implements Comparable {
	final ClassEntry thisClass;
	final ClassEntry outerClass;
	final Utf8Entry name;
	final int flags;

	// Can name and outerClass be derived from thisClass?
	final boolean predictable;

	InnerClass(ClassEntry thisClass, ClassEntry outerClass,
		   Utf8Entry name, int flags) {
	    this.thisClass = thisClass;
	    this.outerClass = outerClass;
	    this.name = name;
	    this.flags = flags;
	    this.predictable = computePredictable();
	}

	private boolean computePredictable() {
	    //System.out.println("computePredictable "+outerClass+" "+this.name);
	    String[] parse = parseInnerClassName(thisClass.stringValue());
	    if (parse == null)  return false;
	    String pkgOuter = parse[0];
	    //String number = parse[1];
	    String name     = parse[2];
	    String haveName  = (this.name == null)  ? null : this.name.stringValue();
	    String haveOuter = (outerClass == null) ? null : outerClass.stringValue();
	    boolean predictable = (name == haveName && pkgOuter == haveOuter);
	    //System.out.println("computePredictable => "+predictable);
	    return predictable;
	}

	// The name is the key.  Ignore other things.
	public boolean equals(Object o) {
	    InnerClass that = (InnerClass)o;
	    if(D)assert0(that.thisClass != this.thisClass
		    || this.assertConsistentWith(that));
	    return that.thisClass == this.thisClass;
	}
	private boolean assertConsistentWith(InnerClass that) {
	    if(D)assert0(this.outerClass == that.outerClass);
	    if(D)assert0(this.name == that.name);
	    if(D)assert0(this.flags == that.flags);
	    return true;
	}
	public int hashCode() {
	    return thisClass.hashCode();
	}
	public int compareTo(Object o) {
	    InnerClass that = (InnerClass)o;
	    return this.thisClass.compareTo(that.thisClass);
	}

	boolean strip(String what) {
	    return false;
	}
	void countRefs(Class cls) {
	    cls.countRef(thisClass);
	    if (predictable) {
		cls.maybeCountRef(outerClass);
		cls.maybeCountRef(name);
	    } else {
		cls.countRef(outerClass);
		cls.countRef(name);
	    }
	}

	public String toString() {
	    return thisClass.stringValue();
	}
    }

    void addInnerClass(InnerClass ic) {
	allInnerClasses.add(ic);
    }

    static void countInnerClassRefs(HashSet innerClasses, Class cls) {
	Package pkg = cls.getPackage();
	if (innerClasses.size() > 0) {
	    // Do not count the name of the InnerClasses attribute:
	    //cls.countRef(pkg.getRefString("InnerClasses"));
	    // Count the entries themselves:
	    for (Iterator i = innerClasses.iterator(); i.hasNext(); ) {
		InnerClass c = (InnerClass) i.next();
		c.countRefs(cls);
	    }
	}
    }

    static String[] parseInnerClassName(String n) {
	//System.out.println("parseInnerClassName "+n);
	String pkgOuter, number, name;
	int dollar1, dollar2;  // pointers to $ in the pattern
	// parse n = (<pkg>/)*<outer>($<number>)?($<name>)?
	int nlen = n.length();
	int pkglen = n.lastIndexOf('/')+1;
	dollar2 = n.lastIndexOf('$');
	if (dollar2 < pkglen)  return null;
	if (isDigitString(n, dollar2+1, nlen)) {
	    // n = (<pkg>/)*<outer>$<number>
	    number = n.substring(dollar2+1, nlen);
	    name = null;
	    dollar1 = dollar2;
	} else if ((dollar1 = n.lastIndexOf('$', dollar2-1)) > pkglen
		   && isDigitString(n, dollar1+1, dollar2)) {
	    // n = (<pkg>/)*<outer>$<number>$<name>
	    number = n.substring(dollar1+1, dollar2);
	    name = n.substring(dollar2+1, nlen).intern();
	} else {
	    // n = (<pkg>/)*<outer>$<name>
	    dollar1 = dollar2;
	    number = null;
	    name = n.substring(dollar2+1, nlen).intern();
	}
	if (number == null)
	    pkgOuter = n.substring(0, dollar1).intern();
	else
	    pkgOuter = null;
	//System.out.println("parseInnerClassName parses "+pkgOuter+" "+number+" "+name);
	return new String[] { pkgOuter, number, name };
    }

    private static boolean isDigitString(String x, int beg, int end) {
	if (beg == end)  return false;  // null string
	for (int i = beg; i < end; i++) {
	    char ch = x.charAt(i);
	    if (!(ch >= '0' && ch <= '9'))  return false;
	}
	return true;
    }

    Utf8Entry getRefString(String s) {
	return cp.ensureUtf8Entry(s);
    }

    LiteralEntry getRefLiteral(Comparable s) {
	return cp.ensureLiteralEntry(s);
    }

    public int attrCount() {
	int na = 0;
	// One attribute per side file:
	na += files.size();
	return na;
    }

    void strip(String what) {
	// what is one of { Debug, Compile, Constant, Exceptions, InnerClasses }
	if (verbose > 0)
	    System.out.println("Stripping "+what.toLowerCase()+" data and attributes...");
	// Keep the inner classes normally.
	// Although they have no effect on execution,
	// the Reflection API exposes them, and JCK checks them.
	if (what == "InnerClasses")
	    allInnerClasses.clear();
	// NO: // if (what == "Compile")  innerClasses.clear();
	for (Iterator i = classes.iterator(); i.hasNext(); ) {
	    Class c = (Class)i.next();
	    if (c.strip(what))
		i.remove();
	}
	for (Iterator i = allInnerClasses.iterator(); i.hasNext(); ) {
	    InnerClass c = (InnerClass)i.next();
	    if (c.strip(what))
		i.remove();
	}
    }

    void countRefs() {
	for (Iterator i = classes.iterator(); i.hasNext(); ) {
	    Class c = (Class)i.next();
	    c.countRefs();
	}

	if (classes.size() > 0)
	    countInnerClassRefs(allInnerClasses, (Class) classes.iterator().next());

	for (Iterator i = files.iterator(); i.hasNext(); ) {
	    File f = (File)i.next();
	    f.countRefs();
	}
    }

    void countRef(Entry ref) {
	if (ref != null) {
	    if (verbose > 3)  System.out.println("countRef "+ref);
	    ref.noteUsage();
	}
    }

    // Use this before writing the package file.
    void reconstructConstantPool() {
	if (verbose > 1)
	    System.out.println("Checking for unused CP entries");
	// After seeing all classes, make CP entries for ones with
	// odd major/minor versions.
	for (Iterator i = classes.iterator(); i.hasNext(); ) {
	    Class c = (Class)i.next();
	    c.checkForRetroVersion();
	}
	cp.resetUsages();
	countRefs();
	if (verbose > 0)
	    System.out.println("Removing "+cp.countZeroUsages()+" unused CP entries, out of "+cp.size());
	cp.pruneZeroUsages();
	if (verbose > 1)
	    System.out.println("Sorting CP entries");
	for (int i = 0; i < ConstantPool.TAGS_IN_ORDER.length; i++) {
	    byte tag = ConstantPool.TAGS_IN_ORDER[i];
	    // Work on all entries of a given kind.
	    Index ix = cp.getIndexByTag(tag);
	    cp.sort(ix);
	}
	for (int i = 0; i < ConstantPool.TAGS_IN_ORDER.length; i++) {
	    byte tag = ConstantPool.TAGS_IN_ORDER[i];
	    Index ix = cp.getIndexByTag(tag);
	    if(D)assert0(ix.assertIsSorted());
	    if (verbose > 2)  System.out.println(ix.dumpString());
	}
    }

    // Use this before writing the class files.
    void reconstructClasses() {
	// Finally, now that we know what's in the package and in each class,
	// construct per-class constant pools, attributes, etc.
	for (Iterator i = classes.iterator(); i.hasNext(); ) {
	    Class cls = (Class) i.next();
	    cls.reconstruct();
	}
    }

    static private final Entry[] noRefs = ConstantPool.noRefs;
    static private final ClassEntry[] noClassRefs = ConstantPool.noClassRefs;

    static void assert0(boolean z) {
	if (!z) throw new RuntimeException("assert failed");
    }
}

