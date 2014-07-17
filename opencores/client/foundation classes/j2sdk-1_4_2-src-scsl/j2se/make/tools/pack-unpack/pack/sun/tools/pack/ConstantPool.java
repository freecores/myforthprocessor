/*
 * @(#)ConstantPool.java	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.pack;

import java.io.*;
import java.util.*;
import sun.tools.io.*;

/**
 * Represents a constant pool, either packed or classic.
 * @author John Rose
 * @version 1.6, 01/23/03
 */
public
class ConstantPool extends AbstractCollection implements Constants {
    public static int verbose = 0;

    protected HashMap entries = new HashMap();

    private boolean permutable = true;

    private boolean frozen = false;

    private final boolean isClassic = (this instanceof Classic);  // cached bit

    /** A CP is permutable only up to the first time someone observes an index.
     *  However, it can always grow by adding new indexes, unless it is frozen.
     */
    public boolean isPermutable() {
	return permutable;
    }

    public void setNotPermutable() {
	//if(D)assert0(isClassic);
	permutable = false;
    }

    /** A CP may add new entries until it is frozen. */
    public boolean isFrozen() {
	return frozen;
    }

    public void setFrozen() {
	//if(D)assert0(isClassic);
	setNotPermutable();
	frozen = true;
    }

    public int size() {
	return entries.size();
    }

    public boolean isEmpty() {
	return entries.isEmpty();
    }

    public boolean contains(Object o) {
	return entries.containsKey(o);
    }

    public Iterator iterator() {
	return entries.keySet().iterator();
    }

    Entry nullEntry = new EmptyEntry();

    public Entry getNull() {
	return nullEntry;
    }

    // Do not change a CP entry once it is installed.
    // Always add to the end of the CP.
    public Entry addNewEntry(Entry e) {
	if(D)assert0(e.thisConstantPool() == this);
	if(D)assert0(!isFrozen());
	if (e.isEmpty())
	    return e;
	entries.put(e,e);
	// This is done by callers which require it:
	//if (e.isDoubleWord() && isClassic)
	//    addNewEntry(new EmptyEntry(e));
	return e;
    }

    public Entry findOldEntry(Entry keyToMatch) {
	if (keyToMatch.isEmpty()) {
	    Entry pred = ((EmptyEntry)keyToMatch).predecessor;
	    if (pred == null)
		return getNull();
	    else
		return null;
	} else {
	    return (Entry) entries.get(keyToMatch);
	}
    }

    private HashMap utf8Cache;

    /** Accessor for well-known strings like "SourceFile", "<init>", etc. */
    public Utf8Entry ensureUtf8Entry(String value) {
	if (utf8Cache == null) {
	    utf8Cache = new HashMap();
	}
	Utf8Entry e = (Utf8Entry) utf8Cache.get(value);
	if (e == null) {
	    e = (Utf8Entry) ensureEntry(new Utf8Entry(value));
	    utf8Cache.put(e.stringValue(), e);
	}
	return e;
    }

    private HashMap classCache;

    /** Accessor for well-known strings like "SourceFile", "<init>", etc. */
    public ClassEntry ensureClassEntry(String name) {
	if (classCache == null) {
	    classCache = new HashMap();
	}
	ClassEntry e = (ClassEntry) classCache.get(name);
	if (e == null) {
	    Utf8Entry utf8name = ensureUtf8Entry(name);
	    e = (ClassEntry) ensureEntry(new ClassEntry(utf8name));
	    classCache.put(e.stringValue(), e);
	}
	return e;
    }

    private HashMap literalCache;

    /** Accessor for building fresh literal entries. */
    public LiteralEntry ensureLiteralEntry(Comparable value) {
	if (literalCache == null) {
	    literalCache = new HashMap();
	}
	LiteralEntry e = (LiteralEntry) literalCache.get(value);
	if (e == null) {
	    if (value instanceof String)
		e = new StringEntry(ensureUtf8Entry((String)value));
	    else
		e = new NumberEntry((Number)value);
	    e = (LiteralEntry) ensureEntry(e);
	    literalCache.put(e.stringValue(), e);
	}
	return e;
    }

    private HashMap typeCache;

    /** Accessor for building fresh signature entries. */
    public SignatureEntry ensureSignatureEntry(String type) {
	if (typeCache == null) {
	    typeCache = new HashMap();
	}
	SignatureEntry e = (SignatureEntry) typeCache.get(type);
	if (e == null) {
	    e = (SignatureEntry) ensureEntry(new SignatureEntry(type));
	    typeCache.put(e.stringValue(), e);
	}
	return e;
    }

    public DescriptorEntry ensureDescriptorEntry(String name, String type) {
	return (DescriptorEntry) ensureEntry(new DescriptorEntry(name, type));
    }

    public Entry ensureEntry(Entry newEntry) {
	Entry e = findOldEntry(newEntry);
	if (e == null) {
	    if (newEntry.thisConstantPool() == this)
		e = newEntry;
	    else
		e = newEntry.cloneIn(this);
	    addNewEntry(e);
	}
	return e;
    }

    public boolean add(Object newEntry) {
	return addEntry((Entry)newEntry);
    }

    public boolean addEntry(Entry newEntry) {
	if (!newEntry.isEmpty() && contains(newEntry))
	    return false;
	ensureEntry(newEntry);
	return true;
    }

    public boolean remove(Object oldEntry) {
	return entries.remove(oldEntry) != null;
    }

    /** Absorb the given CP into self.  Return a mapping from
     *  indexes into CP to entries in self.
     */
    public Entry[] mapEntriesFrom(Index ix) {
	Entry[] cpMap = new Entry[ix.size()];
	// Temporarily put cp's entries into the result array:
	ix.toArray(cpMap);
	// Now exchange my entries into the result array:
	for (int i = 0; i < cpMap.length; i++) {
	    // Record the mapping from e0 (in cp) to e1 (in self).
	    cpMap[i] = ensureEntry(cpMap[i]);
	}
	return cpMap;
    }

    public void resetUsages() {
	for (Iterator i = entries.keySet().iterator(); i.hasNext(); ) {
	    Entry e = (Entry)i.next();
	    e.usage = 0;
	}
    }

    /** Remove unused entries */
    public void pruneZeroUsages() {
	if(D)assert0(isPermutable() && !isFrozen());
	// Prune the hash table.
	for (Iterator i = entries.keySet().iterator(); i.hasNext(); ) {
	    Entry e = (Entry) i.next();
	    if (e.usage == 0) {
		if (verbose > 2)  System.out.println(">> Unused "+e);
		i.remove();
	    }
	}
    }

    public int countZeroUsages() {
	int count = 0;
	for (Iterator i = entries.keySet().iterator(); i.hasNext(); ) {
	    Entry e = (Entry) i.next();
	    if (e.usage == 0) {
		count++;
	    }
	}
	return count;
    }

    /** Entries in the constant pool. */
    public abstract class Entry implements Comparable {
	protected final byte tag;	// a CONSTANT_foo code
	protected int valueHash;	// cached hashCode
	protected int usage;		// how heavily used is it?

	protected Entry(byte tag) {
	    this.tag = tag;
	}
	// Caller must use setValue() or setRef() to further initialize.

	// Access outer object.
	public final ConstantPool thisConstantPool() {
	    return ConstantPool.this;
	}

	public final byte getTag() {
	    return tag;
	}

	public abstract boolean hasValue();
	public Entry getRef(int i) {
	    return null;
	}

	public void noteUsage() {
	    usage += 1;
	    if (usage == 1) {
		// Recursively note usage just once.
		Entry ref;
		for (int i = 0; (ref = getRef(i)) != null; i++) {
		    ref.noteUsage();
		}
	    }
	}

	protected boolean sameTagAs(Object o) {
	    return (o instanceof Entry) && ((Entry)o).tag == tag;
	}
	protected boolean eq(Entry that) {  // same reference
	    if(D)assert0(that != null);
	    return this == that || this.equals(that);
	}

	// Cloning entries between CPs, private to ConstantPool:
	abstract Entry cloneIn(ConstantPool cp);

	// Equality of Entries is value-based.
	public abstract boolean equals(Object o);
	public final int hashCode() {
	    if (valueHash == 0) {
		if(D)assert0(hasValue());
		valueHash = computeValueHash();
		if (valueHash == 0)  valueHash = 1;
	    }
	    return valueHash;
	}
	protected abstract int computeValueHash();

	// Order of entries is based on usage as well as value:
	public abstract int compareTo(Object o);

	protected int superCompareTo(Object o) {
	    Entry that = (Entry) o;
	    if(D)assert0(that.thisConstantPool() == this.thisConstantPool());
	    if(D)assert0(!this.isEmpty());  // don't sort these
	    if(D)assert0(!that.isEmpty());  // don't sort these

	    if (this.tag != that.tag) {
		return TAG_ORDER[this.tag] - TAG_ORDER[that.tag];
	    }

	    return 0;  // subclasses must refine this
	}

	public final boolean isEmpty() {
	    return tag == CONSTANT_None || tag == CONSTANT_Filler;
	}

	public final boolean isDoubleWord() {
	    return tag == CONSTANT_Double || tag == CONSTANT_Long;
	}

	public final boolean tagMatches(int tag) {
	    return (this.tag == tag);
	}

	public final boolean assertTagMatches(int tag) {
	    if (tag >= CONSTANT_NullOKMask) {
		if (isEmpty())  return true;
		tag -= CONSTANT_NullOKMask;
	    }
	    return tagMatches(tag)
		|| tag == CONSTANT_Signature && assertIsSignature();
	}

	public final boolean assertIsSignature() {
	    if (isClassic && tag == CONSTANT_Utf8) {
		String sig = stringValue();
		if(D)assert0(sig.length() > 0);
		return (JAVA_SIGNATURE_CHARS.indexOf(sig.charAt(0)) != -1);
	    } else {
		return tag == CONSTANT_Signature;
	    }
	}

	String toString(Index ix) {
	    int index = (ix == null) ? -1 : ix.findIndexOf(this);
	    if (index >= 0)
		return index+"\t"+toString();
	    else
		return toString();
	}
	public String toString() {
	    String usagePrint = "";
	    if (usage > 1)  usagePrint = " usage="+usage;
	    String valuePrint = "<unresolved>";
	    if (hasValue())
		valuePrint = stringValue();
	    if (verbose > 4) {
		if (valueHash != 0)
		    valuePrint += " hash="+valueHash;
		valuePrint += " id="+System.identityHashCode(this);
	    }
	    return tagName(tag)+"="+valuePrint+usagePrint;
	}
	public abstract String stringValue();
    }

    public static final Comparator USAGE_ORDER = new Comparator() {
	public int compare(Object o1, Object o2) {
	    Entry e1 = (Entry)o1;
	    Entry e2 = (Entry)o2;
	    return -( e1.usage - e2.usage );
	}
    };

    public class EmptyEntry extends Entry {
	Entry predecessor;
	public EmptyEntry() {
	    this(null);
	}
	public EmptyEntry(Entry predecessor) {
	    super(predecessor == null ? CONSTANT_None : CONSTANT_Filler);
	    this.predecessor = predecessor;
	}
	public boolean hasValue() { return false; }
	protected int computeValueHash() {
	    throw new RuntimeException("empty entries are not hashed");
	}
	Entry cloneIn(ConstantPool cp) {
	    Entry pred = predecessor;
	    if (pred != null)  pred = pred.cloneIn(cp);
	    return cp.new EmptyEntry(pred);
	}
	public boolean equals(Object o) {
	    throw new RuntimeException("empty entries are not compared");
	}
	public int compareTo(Object o) {
	    throw new RuntimeException("empty entries are not compared");
	}
	public String stringValue() {
	    return "<empty>";
	}
    }

    public class Utf8Entry extends Entry {
	String value;
	public Utf8Entry() {
	    super(CONSTANT_Utf8);
	}
	public void setValue(String value) {
	    if(D)assert0(this.value == null);
	    value = value.intern();  // always do this
	    this.value = value;
	}
	public boolean hasValue() { return value != null; }
	public Utf8Entry(String value) {
	    this();
	    setValue(value);
	    hashCode();  // force computation of valueHash
	}
	protected int computeValueHash() {
	    return value.hashCode();
	}
	Entry cloneIn(ConstantPool cp) {
	    return cp.new Utf8Entry(value);
	}
	public boolean equals(Object o) {
	    if (!sameTagAs(o))  return false;
	    // Use reference equality of interned strings:
	    return ((Utf8Entry)o).value == value;
	}
	public int compareTo(Object o) {
	    int x = superCompareTo(o);
	    if (x == 0) {
		x = value.compareTo(((Utf8Entry)o).value);
	    }
	    return x;
	}
	public String stringValue() {
	    return value.toString();
	}
    }

    static boolean isMemberTag(byte tag) {
	switch (tag) {
	case CONSTANT_Fieldref:
	case CONSTANT_Methodref:
	case CONSTANT_InterfaceMethodref:
	    return true;
	}
	return false;
    }

    static byte numberTagOf(Number value) {
	if (value instanceof Integer)  return CONSTANT_Integer;
	if (value instanceof Float)    return CONSTANT_Float;
	if (value instanceof Long)     return CONSTANT_Long;
	if (value instanceof Double)   return CONSTANT_Double;
	throw new RuntimeException("bad literal value "+value);
    }

    public abstract class LiteralEntry extends Entry {
	public LiteralEntry(byte tag) {
	    super(tag);
	}

	public abstract Comparable literalValue();
    }

    public class NumberEntry extends LiteralEntry {
	Number value;
	public NumberEntry(byte tag) {
	    super(tag);
	}
	public void setValue(Number value) {
	    if(D)assert0(this.value == null);
	    this.value = value;
	}
	public boolean hasValue() { return value != null; }
	public NumberEntry(Number value) {
	    this(numberTagOf(value));
	    setValue(value);
	    hashCode();  // force computation of valueHash
	}
	protected int computeValueHash() {
	    return value.hashCode();
	}

	Entry cloneIn(ConstantPool cp) {
	    return cp.new NumberEntry(value);
	}
	public boolean equals(Object o) {
	    if (!sameTagAs(o))  return false;
	    return (((NumberEntry)o).value).equals(value);
	}
	public int compareTo(Object o) {
	    int x = superCompareTo(o);
	    if (x == 0) {
		x = ((Comparable)value).compareTo(((NumberEntry)o).value);
	    }
	    return x;
	}
	public Number numberValue() {
	    return value;
	}
	public Comparable literalValue() {
	    return (Comparable) value;
	}
	public String stringValue() {
	    return value.toString();
	}
    }

    public class StringEntry extends LiteralEntry {
	Utf8Entry ref;
	public Entry getRef(int i) { return i == 0 ? ref : null; }

	public StringEntry() {
	    super(CONSTANT_String);
	}
	public void setRef(Utf8Entry ref) {
	    if(D)assert0(this.ref == null);
	    this.ref = ref;
	}
	public boolean hasValue() { return ref != null; }
	public StringEntry(String value) {
	    this(ensureUtf8Entry(value));
	}
	protected int computeValueHash() {
	    return ref.hashCode() + tag;
	}

	StringEntry(Entry ref) {
	    this();
	    setRef((Utf8Entry)ref);
	    hashCode();  // force computation of valueHash
	}
	Entry cloneIn(ConstantPool cp) {
	    return cp.new StringEntry(cp.ensureEntry(ref));
	}
	public boolean equals(Object o) {
	    if (!sameTagAs(o))  return false;
	    return ((StringEntry)o).ref.eq(ref);
	}
	public int compareTo(Object o) {
	    int x = superCompareTo(o);
	    if (x == 0) {
		x = ref.compareTo(((StringEntry)o).ref);
	    }
	    return x;
	}
	public Comparable literalValue() {
	    return stringValue();
	}
	public String stringValue() {
	    return ref.stringValue();
	}
    }

    public class ClassEntry extends Entry {
	Utf8Entry ref;
	public Entry getRef(int i) { return i == 0 ? ref : null; }

	public ClassEntry() {
	    super(CONSTANT_Class);
	}
	public void setRef(Utf8Entry ref) {
	    if(D)assert0(this.ref == null);
	    this.ref = ref;
	}
	public boolean hasValue() { return ref != null; }
	public ClassEntry(String className) {
	    this(ensureUtf8Entry(className));
	}
	protected int computeValueHash() {
	    return ref.hashCode() + tag;
	}

	ClassEntry(Entry ref) {
	    this();
	    setRef((Utf8Entry)ref);
	    hashCode();  // force computation of valueHash
	}
	Entry cloneIn(ConstantPool cp) {
	    return cp.new ClassEntry(cp.ensureEntry(ref));
	}
	public boolean equals(Object o) {
	    if (!sameTagAs(o))  return false;
	    return ((ClassEntry)o).ref.eq(ref);
	}
	public int compareTo(Object o) {
	    int x = superCompareTo(o);
	    if (x == 0) {
		x = ref.compareTo(((ClassEntry)o).ref);
	    }
	    return x;
	}
	public String stringValue() {
	    return ref.stringValue();
	}
    }

    public class DescriptorEntry extends Entry {
	Utf8Entry      nameRef;
	SignatureEntry typeRef;
	public Entry getRef(int i) {
	    if (i == 0)  return nameRef;
	    if (i == 1)  return typeRef;
	    return null;
	}
	public DescriptorEntry() {
	    super(CONSTANT_NameandType);
	}
	public void setRefs(Utf8Entry nameRef, SignatureEntry typeRef) {
	    if(D)assert0(this.nameRef == null);
	    this.nameRef = nameRef;
	    this.typeRef = typeRef;
	}
	public void setRefs(Utf8Entry nameRef, Utf8Entry typeRef) {
	    SignatureEntry sigRef = new SignatureEntry();
	    sigRef.setRef(typeRef);
	    setRefs(nameRef, sigRef);
	}
	public boolean hasValue() {
	    return nameRef != null && nameRef.hasValue()
		&& typeRef != null && typeRef.hasValue();
	}
	public DescriptorEntry(String name, String type) {
	    this(ensureUtf8Entry(name), ensureSignatureEntry(type));
	}
	protected int computeValueHash() {
	    int hc2 = typeRef.hashCode();
	    return (nameRef.hashCode() + (hc2 << 8)) ^ hc2;
	}

	DescriptorEntry(Entry nameRef, Entry typeRef) {
	    this();
	    setRefs((Utf8Entry) nameRef, (SignatureEntry) typeRef);
	    hashCode();  // force computation of valueHash
	}
	Entry cloneIn(ConstantPool cp) {
	    return cp.new DescriptorEntry(cp.ensureEntry(nameRef),
					  cp.ensureEntry(typeRef));
	}
	public boolean equals(Object o) {
	    if (!sameTagAs(o))  return false;
	    DescriptorEntry that = (DescriptorEntry)o;
	    return this.nameRef.eq(that.nameRef)
		&& this.typeRef.eq(that.typeRef);
	}
	public int compareTo(Object o) {
	    int x = superCompareTo(o);
	    if (x == 0) {
		DescriptorEntry that = (DescriptorEntry)o;
		// Primary key is typeRef, not nameRef.
		x = this.typeRef.compareTo(that.typeRef);
		if (x == 0)
		    x = this.nameRef.compareTo(that.nameRef);
	    }
	    return x;
	}
	public String stringValue() {
	    return typeRef.stringValue()+","+nameRef.stringValue();
	}

	public String prettyString() {
	    return nameRef.stringValue()+typeRef.prettyString();
	}

	public boolean isMethod() {
	    return typeRef.isMethod();
	}

	public byte getLiteralTag() {
	    return typeRef.getLiteralTag();
	}
    }

    public class MemberEntry extends Entry {
	ClassEntry classRef;
	DescriptorEntry descRef;
	public Entry getRef(int i) {
	    if (i == 0)  return classRef;
	    if (i == 1)  return descRef;
	    return null;
	}
	public MemberEntry(byte tag) {
	    super(tag);
	    if(D)assert0(isMemberTag(tag));
	}
	public void setRefs(ClassEntry classRef, DescriptorEntry descRef) {
	    if(D)assert0(this.classRef == null);
	    this.classRef = classRef;
	    this.descRef  = descRef;
	}
	public boolean hasValue() {
	    return classRef != null && classRef.hasValue()
		&& descRef  != null && descRef.hasValue();
	}
	/*
	public MemberEntry(String className, String name, String type) {
	    this(ensureEntry(ConstantPool.this.new ClassEntry(className)),
		 ensureEntry(ConstantPool.this.new DescriptorEntry(name, type)));
	}
	*/
	protected int computeValueHash() {
	    int hc2 = descRef.hashCode();
	    return (classRef.hashCode() + (hc2 << 8)) ^ hc2;
	}

	MemberEntry(byte tag, Entry classRef, Entry descRef) {
	    this(tag);
	    setRefs((ClassEntry) classRef, (DescriptorEntry) descRef);
	    hashCode();  // force computation of valueHash
	}
	Entry cloneIn(ConstantPool cp) {
	    return cp.new MemberEntry(tag,
				      cp.ensureEntry(classRef),
				      cp.ensureEntry(descRef));
	}
	public boolean equals(Object o) {
	    if (!sameTagAs(o))  return false;
	    MemberEntry that = (MemberEntry)o;
	    return this.classRef.eq(that.classRef)
		&& this.descRef.eq(that.descRef);
	}
	public int compareTo(Object o) {
	    int x = superCompareTo(o);
	    if (x == 0) {
		MemberEntry that = (MemberEntry)o;
		// Primary key is classRef.
		x = this.classRef.compareTo(that.classRef);
		if (x == 0)
		    x = this.descRef.compareTo(that.descRef);
	    }
	    return x;
	}
	public String stringValue() {
	    return classRef.stringValue()+","+descRef.stringValue();
	}

	public boolean isMethod() {
	    return descRef.isMethod();
	}
    }

    public class SignatureEntry extends Entry {
	Utf8Entry    formRef;
	ClassEntry[] classRefs;
	String value;
	public Entry getRef(int i) {
	    if (i == 0)  return formRef;
	    return i-1 < classRefs.length ? classRefs[i-1] : null;
	}
	public SignatureEntry() {
	    super(CONSTANT_Signature);
	}
	public void setRefs(Utf8Entry formRef, ClassEntry[] classRefs) {
	    if(D)assert0(!isClassic);
	    if(D)assert0(this.formRef == null);
	    this.formRef   = formRef;
	    this.classRefs = classRefs;
	    if(D)assert0(countClassParts(formRef) == classRefs.length);
	}
	public void setRef(Utf8Entry formRef) {
	    if(D)assert0(isClassic);
	    if(D)assert0(this.formRef == null);
	    this.formRef = formRef;
	    this.classRefs = noClassRefs;
	}
	public boolean hasValue() {
	    if (formRef == null || !formRef.hasValue())  return false;
	    for (int i = 0; i < classRefs.length; i++)
		if (!classRefs[i].hasValue())  return false;
	    return true;
	}
	public SignatureEntry(String value) {
	    super(CONSTANT_Signature);
	    value = value.intern();  // always do this
	    this.value = value;
	    if (isClassic) {
		setRef((Utf8Entry) ensureEntry(new Utf8Entry(value)));
	    } else {
		String[] parts = structureSignature(value);
		Utf8Entry formRef = (Utf8Entry) ensureEntry(new Utf8Entry(parts[0]));
		ClassEntry classRefs[] = new ClassEntry[parts.length-1];
		for (int i = 1; i < parts.length; i++)
		    classRefs[i-1] = (ClassEntry) ensureEntry(new ClassEntry(parts[i]));
		setRefs(formRef, classRefs);
	    }
	    hashCode();  // force computation of valueHash
	}
	protected int computeValueHash() {
	    stringValue();  // force computation of value
	    return value.hashCode() + tag;
	}
	public SignatureEntry(Utf8Entry formRef, ClassEntry[] classRefs) {
	    this();
	    setRefs(formRef, classRefs);
	}

	Entry cloneIn(ConstantPool cp) {
	    return cp.new SignatureEntry(stringValue());
	}
	public boolean equals(Object o) {
	    if (!sameTagAs(o))  return false;
	    return ((SignatureEntry)o).value == value;
	}
	public int compareTo(Object o) {
	    int x = superCompareTo(o);
	    if (x == 0) {
		SignatureEntry that = (SignatureEntry)o;
		x = compareSignatures(this.value, that.value);
	    }
	    return x;
	}
	public String stringValue() {
	    if (value == null) {
		if(D)assert0(hasValue());
		String[] parts = new String[1+classRefs.length];
		parts[0] = formRef.stringValue();
		for (int i = 1; i < parts.length; i++)
		    parts[i] = classRefs[i-1].stringValue();
		value = flattenSignature(parts);
	    }
	    return value;
	}

	public int computeSize(boolean countDoublesTwice) {
	    String form = formRef.stringValue();
	    int min = 0;
	    int max = 1;
	    if (isMethod()) {
		min = 1;
		max = form.indexOf(')');
	    }
	    int size = 0;
	    for (int i = min; i < max; i++) {
		switch (form.charAt(i)) {
		case 'D':
		case 'J':
		    if (countDoublesTwice) size++;
		    break;
		case '[':
		    // Skip rest of array info.
		    while (form.charAt(i) == '[') ++i;
		    break;
		case ';':
		    continue;
		default:
		    if(D)assert0(0 <= JAVA_SIGNATURE_CHARS.indexOf(form.charAt(i)));
		    break;
		}
		size++;
	    }
	    return size;
	}
	public boolean isMethod() {
	    return formRef.stringValue().charAt(0) == '(';
	}
	public byte getLiteralTag() {
	    switch (formRef.stringValue().charAt(0)) {
	    case 'L': return CONSTANT_String;
	    case 'I': return CONSTANT_Integer;
	    case 'J': return CONSTANT_Long;
	    case 'F': return CONSTANT_Float;
	    case 'D': return CONSTANT_Double;
	    case 'B': case 'S': case 'C': case 'Z':
		return CONSTANT_Integer;
	    }
	    if(D)assert0(false);
	    return CONSTANT_None;
	}
	public String prettyString() {
	    String s;
	    if (isMethod()) {
		s = formRef.stringValue();
		s = s.substring(0, 1+s.indexOf(')'));
	    } else {
		s = "/" + formRef.stringValue();
	    }
	    int i;
	    while ((i = s.indexOf(';')) >= 0)
		s = s.substring(0,i) + s.substring(i+1);
	    return s;
	}
    }

    static int compareSignatures(String s1, String s2) {
	return compareSignatures(s1, s2, null, null);
    }
    static int compareSignatures(String s1, String s2, String[] p1, String[] p2) {
	final int S1_COMES_FIRST = -1;
	final int S2_COMES_FIRST = +1;
	char c1 = s1.charAt(0);
	char c2 = s2.charAt(0);
	// fields before methods (because there are fewer of them)
	if (c1 != '(' && c2 == '(')  return S1_COMES_FIRST;
	if (c2 != '(' && c1 == '(')  return S2_COMES_FIRST;
	if (p1 == null)  p1 = structureSignature(s1);
	if (p2 == null)  p2 = structureSignature(s2);
	/*
	 // non-classes before classes (because there are fewer of them)
	 if (p1.length == 1 && p2.length > 1)  return S1_COMES_FIRST;
	 if (p2.length == 1 && p1.length > 1)  return S2_COMES_FIRST;
	 // all else being equal, use the same comparison as for Utf8 strings
	 return s1.compareTo(s2);
	 */
	if (p1.length != p2.length)  return p1.length - p2.length;
	int length = p1.length;
	for (int i = length; --i >= 0; ) {
	    int res = p1[i].compareTo(p2[i]);
	    if (res != 0)  return res;
	}
	if(D)assert0(s1.equals(s2));
	return 0;
    }

    static int countClassParts(Utf8Entry formRef) {
	int num = 0;
	String s = formRef.stringValue();
	for (int i = 0; i < s.length(); i++) {
	    if (s.charAt(i) == 'L')  ++num;
	}
	return num;
    }

    static String flattenSignature(String[] parts) {
	String form = parts[0];
	if (parts.length == 1)  return form;
	int len = form.length();
	for (int i = 1; i < parts.length; i++) {
	    len += parts[i].length();
	}
	char[] sig = new char[len];
	int j = 0;
	int k = 1;
	for (int i = 0; i < form.length(); i++) {
	    char ch = form.charAt(i);
	    sig[j++] = ch;
	    if (ch == 'L') {
		String cls = parts[k++];
		cls.getChars(0, cls.length(), sig, j);
		j += cls.length();
		//sig[j++] = ';';
	    }
	}
	if(D)assert0(j == len);
	if(D)assert0(k == parts.length);
	return new String(sig).intern();
    }

    // Hack to speed parsing and cut down (a little) on heap usage.
    private static HashMap signatureCache = new HashMap();

    static String[] structureSignature(String sig) {
	String[] hit = (String[]) signatureCache.get(sig);
	if (hit != null)  return hit;
	sig = sig.intern();

	int formLen = 0;
	int nparts = 1;
	for (int i = 0; i < sig.length(); i++) {
	    char ch = sig.charAt(i);
	    formLen++;
	    if (ch == 'L') {
		nparts++;
		int i2 = sig.indexOf(';', i+1);
		if(D)assert0(i2 > 0);
		if(D)assert0(0 > sig.substring(i+1, i2).indexOf('<'));  // no GJC yet please
		i = i2;
		--i;  // keep the semicolon in the form
	    }
	}
	char[] form = new char[formLen];
	if (nparts == 1) {
	    String[] parts = { sig };
	    signatureCache.put(sig, parts);
	    return parts;
	}
	String[] parts = new String[nparts];
	int j = 0;
	int k = 1;
	for (int i = 0; i < sig.length(); i++) {
	    char ch = sig.charAt(i);
	    form[j++] = ch;
	    if (ch == 'L') {
		int i2 = sig.indexOf(';', i+1);
		parts[k++] = sig.substring(i+1, i2);
		i = i2;
		--i;  // keep the semicolon in the form
	    }
	}
	if(D)assert0(j == formLen);
	if(D)assert0(k == parts.length);
	parts[0] = new String(form);
	//if(D)assert0(flattenSignature(parts).equals(sig));
	signatureCache.put(sig, parts);
	return parts;
    }

    // Handy constants:
    public static final Entry[] noRefs = {};
    public static final ClassEntry[] noClassRefs = {};

    /** An Index is a mapping between CP entries and small integers. */
    public class Index extends AbstractList {
	protected String debugName;
	protected Entry[] cpMap;
	public Index(String debugName) {
	    this.debugName = debugName;
	}
	public Index(String debugName, Entry[] cpMap) {
	    this(debugName);
	    setMap(cpMap);
	}
	public void setMap(Entry[] cpMap) {
	    clearIndex();
	    this.cpMap = cpMap;
	}
	public Index(String debugName, List cpMapList) {
	    this(debugName);
	    setMap(cpMapList);
	}
	public void setMap(List cpMapList) {
	    cpMap = new Entry[cpMapList.size()];
	    cpMapList.toArray(cpMap);
	    setMap(cpMap);
	}
	public final ConstantPool thisConstantPool() {
	    return ConstantPool.this;
	}
	public int size() {
	    return cpMap.length;
	}
	public Object get(int i) {
	    return cpMap[i];
	}
	public Entry getEntry(int i) {
	    //if (i < 0 || i >= cpMap.length)  System.out.println("getEntry "+i+" in "+dumpString());
	    return cpMap[i];
	}

	public int findIndexOf(Entry e) {
	    if (indexKey == null)  initializeIndex();
	    int probe = findIndexLocation(e);
	    if (indexKey[probe] != e)
		return -1;
	    int index = indexValue[probe];
	    if(D)assert0(cpMap[index] == e);  // found right index?
	    return index;
	}
	public int indexOf(Entry e) {
	    int index = findIndexOf(e);
	    if (index < 0 && verbose > 0) {
		System.out.println("not found: "+e);
		System.out.println("       in: "+this.dumpString());
	    }
	    if(D)assert0(index >= 0);
	    return index;
	}
	public int indexOf(Object e) {
	    return indexOf((Entry)e);
	}
	public int lastIndexOf(Object e) {
	    return indexOf((Entry)e);
	}

	public boolean assertIsSorted() {
	    for (int i = 1; i < cpMap.length; i++) {
		if (cpMap[i-1].compareTo(cpMap[i]) > 0) {
		    System.out.println("Not sorted at "+(i-1)+"/"+i+": "+this.dumpString());
		    return false;
		}
	    }
	    return true;
	}

	// internal hash table
	protected Entry[] indexKey;
	protected int[]   indexValue;
	protected void clearIndex() {
	    indexKey   = null;
	    indexValue = null;
	}
	private int findIndexLocation(Entry e) {
	    if(D)assert0(e.thisConstantPool() == ConstantPool.this);
	    if(D)assert0(e.hasValue() && e == entries.get(e));
	    int size   = indexKey.length;
	    int hash   = e.hashCode();
	    int probe  = hash & (size - 1);
	    int stride = ((hash >>> 8) | 1) & (size - 1);
	    for (;;) {
		Entry e1 = indexKey[probe];
		if (e1 == e || e1 == null)
		    return probe;
		probe += stride;
		if (probe >= size)  probe -= size;
	    }
	}
	private void initializeIndex() {
	    if (verbose > 2)
		System.out.println("initialize Index "+debugName+" ["+size()+"]");
	    int hsize0 = (int)(cpMap.length * 1.5);
	    int hsize = 1;
	    while (hsize < hsize0)  hsize <<= 1;
	    indexKey   = new Entry[hsize];
	    indexValue = new int[hsize];
	    for (int i = 0; i < cpMap.length; i++) {
		Entry e = cpMap[i];
		if (e == null)  continue;
		int probe = findIndexLocation(e);
		if(D)assert0(indexKey[probe] == null);  // e has unique index
		indexKey[probe] = e;
		indexValue[probe] = i;
	    }
	}
	public Object[] toArray(Object[] a) {
	    int sz = size();
	    if (a.length < sz)  return super.toArray(a);
	    System.arraycopy(cpMap, 0, a, 0, sz);
	    if (a.length > sz)  a[sz] = null;
	    return a;
	}
	public Object[] toArray() {
	    return toArray(new Entry[size()]);
	}
	public Object clone() {
	    return new Index(debugName, (Entry[]) cpMap.clone());
	}
	public String toString() {
	    return "Index "+debugName+" ["+size()+"]";
	}
	public String dumpString() {
	    String s = toString();
	    s += " {\n";
	    for (int i = 0; i < cpMap.length; i++) {
		s += "    "+i+": "+cpMap[i]+"\n";
	    }
	    s += "}";
	    return s;
	}
    }

    /** A "classic" constant pool represents a single standard class file.
     *  It has no CONSTANT_Signature entries, and is indexed globally.
     */
    public static class Classic extends ConstantPool {
	protected ArrayList order = new ArrayList();

	public Entry addNewEntry(Entry e) {
	    e = super.addNewEntry(e);
	    order.add(e);
	    return e;
	}

	// element zero is always empty
	{ add(new EmptyEntry()); }

	// Collection overrides:
	public Iterator iterator() {
	    return order.iterator();
	}

	/** Comprehensive index of all CP entries. */
	public Index getIndex() {
	    if (index == null) {
		setFrozen();  // no more changes after indexing
		index = makeIndex();
	    }
	    return index;
	}
	Index makeIndex() {
	    Entry[] cpMap = new Entry[order.size()];
	    order.toArray(cpMap);
	    return new Index("classic", cpMap);
	}
	private Index index;

	public void setAllEntries(Entry[] cpMap) {
	    if(D)assert0(cpMap[0].isEmpty());
	    order = new ArrayList(Arrays.asList(cpMap));
	    entries.clear();
	    for (int i = 0; i < cpMap.length; i++) {
		cpMap[i] = ensureEntry(cpMap[i]);
	    }
	}

	public Entry[] mapEntries() {
	    Entry[] cpMap = new Entry[order.size()];
	    order.toArray(cpMap);
	    return cpMap;
	}

	// Override, to update the order as well as the entries map:
	public void pruneZeroUsages() {
	    super.pruneZeroUsages();

	    // Also prune the ordered list.
	    for (Iterator i = order.iterator(); i.hasNext(); ) {
		Entry e = (Entry) i.next();
		if (e.isEmpty())  continue;
		if (entries.containsKey(e))  continue;
		// Remove this entry:
		i.remove();
		if (e.isDoubleWord()) {
		    e = (Entry) i.next();
		    if(D)assert0(e.isEmpty());
		    i.remove();
		}
	    }
	}
    }

    // Index methods.

    /** Sort this index (destructively) into canonical order. */
    public void sort(Index ix) {
	if(D)assert0(ix.thisConstantPool() == this);

	ix.clearIndex();
	Arrays.sort(ix.cpMap);

	if (verbose > 2)
	    System.out.println("sorted "+ix.dumpString());

	// do not add to the CP after it has been sorted
	setFrozen();
    }

    /** Return a set of indexes partitioning these entries.
     *  The keys array must of length this.size(), and marks entries.
     *  The result array is as long as one plus the largest key value.
     *  Entries with a negative key are dropped from the partition.
     */
    public Index[] partition(Index ix, int[] keys) {
	ArrayList parts = new ArrayList();
	if(D)assert0(keys.length == ix.size());
	for (int i = 0; i < keys.length; i++) {
	    int key = keys[i];
	    if (key < 0)  continue;
	    while (key >= parts.size())  parts.add(null);
	    ArrayList part = (ArrayList) parts.get(key);
	    if (part == null) {
		parts.set(key, part = new ArrayList());
	    }
	    part.add(ix.get(i));
	}
	Index[] indexes = new Index[parts.size()];
	for (int key = 0; key < indexes.length; key++) {
	    ArrayList part = (ArrayList) parts.get(key);
	    if (part == null)  continue;
	    indexes[key] = new Index(ix.debugName+"/part#"+key, part);
	    if(D)assert0(indexes[key].indexOf(part.get(0)) == 0);
	}
	return indexes;
    }

    private Index[] indexByTag;
    private Index[][] indexByTagAndClass;

    protected void clearIndexes() {
	if(D)assert0(!isFrozen());
	if (verbose > 1) {
	    System.out.println("Clearing indexes; here's why:");
	    Thread.dumpStack();
	}
	indexByTag = null;
	indexByTagAndClass = null;
	utf8Cache = null;
	literalCache = null;
    }

    /** Index of all CP entries of a given tag. */
    public Index getIndexByTag(byte tag) {
	if (indexByTag == null)
	    indexByTag = new Index[CONSTANT_Limit];
	Index ix = indexByTag[tag];
	if (ix == null) {
	    // Make a single pass over the CP collecting all concrete tags.
	    ArrayList[] parts = new ArrayList[CONSTANT_Limit];
	    for (int tag1 = 0; tag1 < parts.length; tag1++) {
		parts[tag1] = new ArrayList();
	    }
	    for (Iterator i = entries.keySet().iterator(); i.hasNext(); ) {
		Entry e = (Entry) i.next();
		parts[e.getTag()].add(e);
	    }
	    for (int tag1 = 0; tag1 < parts.length; tag1++) {
		//if (parts[tag1].size() == 0)  continue;
		if (indexByTag[tag1] != null)  continue;  // do not overwrite
		indexByTag[tag1] = new Index(tagName(tag1), parts[tag1]);
	    }
	    ix = indexByTag[tag];
	}
	return ix;
    }

    public Index makeIndexByTag(byte tag, Entry[] cpMap) {
	if (indexByTag == null)
	    indexByTag = new Index[CONSTANT_Limit];
	if(D)assert0(indexByTag[tag] == null);  // do not init twice
	for (int i = 0; i < cpMap.length; i++) {
	    Entry e = cpMap[i];
	    if (entries.get(e) != null) {
		verbose += 100;
		System.out.println("***                 "+entries.get(e));
		System.out.println("***                 "+e);
		verbose -= 100;
	    }
	    cpMap[i] = ensureEntry(e);
	    if(D)assert0(e == cpMap[i]);  // no change, please
	}
	Index ix = new Index(ConstantPool.tagName(tag), cpMap);
	indexByTag[tag] = ix;
	return ix;
    }

    /** Index of all CP entries of a given tag and class. */
    public Index getMemberIndex(byte tag, ClassEntry classRef) {
	if (indexByTagAndClass == null)
	    indexByTagAndClass = new Index[CONSTANT_Limit][];
	Index allClasses =  getIndexByTag(CONSTANT_Class);
	Index[] perClassIndexes = indexByTagAndClass[tag];
	if (perClassIndexes == null) {
	    // Create the partition now.
	    // Divide up all entries of the given tag according to their class.
	    Index allMembers = getIndexByTag(tag);
	    int[] whichClasses = new int[allMembers.size()];
	    for (int i = 0; i < whichClasses.length; i++) {
		MemberEntry e = (MemberEntry) allMembers.get(i);
		int whichClass = allClasses.indexOf(e.classRef);
		whichClasses[i] = whichClass;
	    }
	    perClassIndexes = partition(allMembers, whichClasses);
	    for (int i = 0; i < perClassIndexes.length; i++)
		if(D)assert0(perClassIndexes[i]==null
			|| perClassIndexes[i].assertIsSorted());
	    indexByTagAndClass[tag] = perClassIndexes;
	}
	int whichClass = allClasses.indexOf(classRef);
	return perClassIndexes[whichClass];
    }

    // Given the sequence of all methods of the given name and class,
    // produce the ordinal of this particular given overloading.
    public int getOverloadingIndex(MemberEntry methodRef) {
	Index ix = getMemberIndex(methodRef.tag, methodRef.classRef);
	Utf8Entry nameRef = methodRef.descRef.nameRef;
	int ord = 0;
	for (int i = 0; i < ix.cpMap.length; i++) {
	    MemberEntry e = (MemberEntry) ix.cpMap[i];
	    if (e == methodRef)
		return ord;
	    if (e.descRef.nameRef == nameRef)
		// Found a different overloading.  Increment the ordinal.
		ord++;
	}
	throw new RuntimeException("should not reach here");
    }

    // Inverse of getOverloadingIndex
    public MemberEntry getOverloadingForIndex(byte tag, ClassEntry classRef, String name, int which) {
	Index ix = getMemberIndex(tag, classRef);
	int ord = 0;
	for (int i = 0; i < ix.cpMap.length; i++) {
	    MemberEntry e = (MemberEntry) ix.cpMap[i];
	    if (e.descRef.nameRef.stringValue() == name) {
		if (ord == which)  return e;
		ord++;
	    }
	}
	throw new RuntimeException("should not reach here");
    }

    public void addReferencedEntries(Collection cpRefs, boolean forClassic) {
	for (ListIterator work =
		 new ArrayList(cpRefs).listIterator(cpRefs.size());
	     work.hasPrevious(); ) {
	    Entry e = (Entry) work.previous();
	    work.remove();          // pop stack
	    if (forClassic && e instanceof SignatureEntry) {
		// The signature will be represented by a simple string:
		ensureUtf8Entry(e.stringValue());
		continue;
	    }
	    // Recursively add the refs of e to cpRefs:
	    for (int i = 0; ; i++) {
		Entry re = e.getRef(i);
		if (re == null)
		    break;          // no more refs in e
		if (cpRefs.add(re)) // output the ref
		    work.add(re);   // push stack, if a new ref
	    }
	}
    }

    public String toString() {
	StringWriter w = new StringWriter();
	w.write("ConstantPool["+size()+"]");
	if (isClassic)  w.write(" classic");
	if (isPermutable())  w.write(" permutable");
	w.write(" {\n");
	Index ix = null;
	if (isClassic)  ix = ((Classic)this).makeIndex();
	Iterator fakeItr;
	if (indexByTag != null) {
	    ArrayList fakeOrder = new ArrayList(size());
	    HashMap fakeMap = new HashMap(size());
	    Set fakeSet = fakeMap.keySet();
	    for (int i = 0; i < TAGS_IN_ORDER.length; i++) {
		byte tag = TAGS_IN_ORDER[i];
		if (indexByTag[tag] != null) {
		    ArrayList tem = new ArrayList(indexByTag[tag]);
		    tem.removeAll(fakeSet);
		    fakeOrder.addAll(tem);
		    for (Iterator ti = tem.iterator(); ti.hasNext(); ) {
			fakeMap.put(ti.next(), "");
		    }
		}
	    }
	    ArrayList tem = new ArrayList(entries.keySet());
	    tem.removeAll(fakeSet);
	    fakeOrder.addAll(tem);
	    fakeItr = fakeOrder.iterator();
	} else {
	    fakeItr = iterator();
	}
	while (fakeItr.hasNext()) {
	    Object e = fakeItr.next();
	    //if (e.isEmpty())  continue;
	    w.write(e.toString());
	    w.write("\n");
	}
	w.write("}\n");
	return w.toString();
    }

    static double percent(int num, int den) {
	return (int)((10000.0*num)/den + 0.5) / 100.0;
    }

    public static String tagName(int tag) {
	if ((tag & CONSTANT_NullOKMask) != 0) {
	    return tagName(tag - CONSTANT_NullOKMask) + "OrNull";
	}
	switch (tag) {
	    case CONSTANT_Utf8:			return "Utf8";
	    case CONSTANT_Integer:		return "Integer";
	    case CONSTANT_Float:		return "Float";
	    case CONSTANT_Long:			return "Long";
	    case CONSTANT_Double:		return "Double";
	    case CONSTANT_Class:		return "Class";
	    case CONSTANT_String:		return "String";
	    case CONSTANT_Fieldref:		return "Fieldref";
	    case CONSTANT_Methodref:		return "Methodref";
	    case CONSTANT_InterfaceMethodref:	return "InterfaceMethodref";
	    case CONSTANT_NameandType:		return "NameandType";

		// pseudo-tags:
	    case CONSTANT_None:			return "*None";
	    case CONSTANT_Filler:		return "*Filler";
	    case CONSTANT_Signature:		return "*Signature";
	}
	return "tag#"+tag;
    }

    static final byte TAGS_IN_ORDER[] = {
	CONSTANT_Utf8,
	CONSTANT_Integer,
	CONSTANT_Float,
	CONSTANT_Long,
	CONSTANT_Double,
	CONSTANT_String,
	CONSTANT_Class,
	CONSTANT_Signature,
	CONSTANT_NameandType,
	CONSTANT_Fieldref,
	CONSTANT_Methodref,
	CONSTANT_InterfaceMethodref
    };
    static final byte TAG_ORDER[];
    static {
	TAG_ORDER = new byte[CONSTANT_Limit];
	for (int i = 0; i < TAGS_IN_ORDER.length; i++) {
	    TAG_ORDER[TAGS_IN_ORDER[i]] = (byte)(i+1);
	}
	/*
	System.out.println("TAG_ORDER[] = {");
	for (int i = 0; i < TAG_ORDER.length; i++)
	    System.out.println("  "+TAG_ORDER[i]+",");
	System.out.println("};");
	*/
    }

    static void assert0(boolean z) {
	if (!z) throw new RuntimeException("assert failed");
    }

}
