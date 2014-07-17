/*
 * @(#)BandStructure.java	1.6 03/01/23
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
import sun.tools.io.ByteCountOutputStream;

/**
 * Define the structure and ordering of "bands" in a packed file.
 * @author John Rose
 * @version 1.6, 01/23/03
 */
abstract
class BandStructure implements Constants {
    public static int verbose = 0;
    static void setVerbose(int verbose) {
	BandStructure.verbose = verbose;
	ConstantPool.verbose = verbose;
	Package.verbose = verbose;
	Code.verbose = verbose;
	ClassReader.verbose = verbose;
	ClassWriter.verbose = verbose;
    }

    abstract Index getCPIndex(byte tag);

    private final boolean isReader = this instanceof PackageReader;
    protected BandStructure(boolean isReader) {
	if(D)assert0(this.isReader == isReader);
    }

    // sharp zips 0.4% better than medium and 1.1% better than flat
    final static Coding SIGNED_CODING = Coding.of(5, 64, 1);  //sharp
//  final static Coding SIGNED_CODING = Coding.of(5, 128, 1); //medium
//  final static Coding SIGNED_CODING = Coding.of(5, 192, 1); //flat
//  final static Coding SIGNED_CODING = Coding.of(5, 128, 2); //pos
//  final static Coding SIGNED_CODING = Coding.of(5, 192, 2); //pos-flat
    final static Coding UNSIGNED_CODING = SIGNED_CODING.setS(0);

    final static Coding BYTE_CODING = Coding.of(1,256,0);
//  final static Coding CHAR_CODING = Coding.of(3,16,0); // sharp (zip +-0.3%?)
    final static Coding CHAR_CODING = Coding.of(3,128,0); // medium

    final static Coding SIGNED_DELTA_CODING  = SIGNED_CODING.getDeltaCoding();
    final static Coding UNSIGNED_DELTA_CODING  = UNSIGNED_CODING.getDeltaCoding();

    // This is best used with BCI values:
    final static Coding BCI_CODING = Coding.of(5,4,0);

    final private static Coding[] basicCodings = {
	null,

	// Fixed-length codings:
	Coding.of(1,256,0),
	Coding.of(1,256,1),
	Coding.of(1,256,0).getDeltaCoding(),
	Coding.of(1,256,1).getDeltaCoding(),
	Coding.of(2,256,0),
	Coding.of(2,256,1),
	Coding.of(2,256,0).getDeltaCoding(),
	Coding.of(2,256,1).getDeltaCoding(),
	Coding.of(3,256,0),
	Coding.of(3,256,1),
	Coding.of(3,256,0).getDeltaCoding(),
	Coding.of(3,256,1).getDeltaCoding(),
	Coding.of(4,256,0),
	Coding.of(4,256,1),
	Coding.of(4,256,0).getDeltaCoding(),
	Coding.of(4,256,1).getDeltaCoding(),

	// Full-range variable-length codings:
	Coding.of(5,  4,0),
	Coding.of(5,  4,1),
	Coding.of(5,  4,2),
	Coding.of(5, 16,0),
	Coding.of(5, 16,1),
	Coding.of(5, 16,2),
	Coding.of(5, 32,0),
	Coding.of(5, 32,1),
	Coding.of(5, 32,2),
	Coding.of(5, 64,0),
	Coding.of(5, 64,1),
	Coding.of(5, 64,2),
	Coding.of(5,128,0),
	Coding.of(5,128,1),
	Coding.of(5,128,2),

	Coding.of(5,  4,0).getDeltaCoding(),
	Coding.of(5,  4,1).getDeltaCoding(),
	Coding.of(5,  4,2).getDeltaCoding(),
	Coding.of(5, 16,0).getDeltaCoding(),
	Coding.of(5, 16,1).getDeltaCoding(),
	Coding.of(5, 16,2).getDeltaCoding(),
	Coding.of(5, 32,0).getDeltaCoding(),
	Coding.of(5, 32,1).getDeltaCoding(),
	Coding.of(5, 32,2).getDeltaCoding(),
	Coding.of(5, 64,0).getDeltaCoding(),
	Coding.of(5, 64,1).getDeltaCoding(),
	Coding.of(5, 64,2).getDeltaCoding(),
	Coding.of(5,128,0).getDeltaCoding(),
	Coding.of(5,128,1).getDeltaCoding(),
	Coding.of(5,128,2).getDeltaCoding(),

	// Variable length subrange codings:
	Coding.of(2,192,0),
	Coding.of(2,224,0),
	Coding.of(2,240,0),
	Coding.of(2,248,0),
	Coding.of(2,252,0),

	Coding.of(2,  8,0).getDeltaCoding(),
	Coding.of(2,  8,1).getDeltaCoding(),
	Coding.of(2, 16,0).getDeltaCoding(),
	Coding.of(2, 16,1).getDeltaCoding(),
	Coding.of(2, 32,0).getDeltaCoding(),
	Coding.of(2, 32,1).getDeltaCoding(),
	Coding.of(2, 64,0).getDeltaCoding(),
	Coding.of(2, 64,1).getDeltaCoding(),
	Coding.of(2,128,0).getDeltaCoding(),
	Coding.of(2,128,1).getDeltaCoding(),
	Coding.of(2,192,0).getDeltaCoding(),
	Coding.of(2,192,1).getDeltaCoding(),
	Coding.of(2,224,0).getDeltaCoding(),
	Coding.of(2,224,1).getDeltaCoding(),
	Coding.of(2,240,0).getDeltaCoding(),
	Coding.of(2,240,1).getDeltaCoding(),
	Coding.of(2,248,0).getDeltaCoding(),
	Coding.of(2,248,1).getDeltaCoding(),

	Coding.of(3,192,0),
	Coding.of(3,224,0),
	Coding.of(3,240,0),
	Coding.of(3,248,0),
	Coding.of(3,252,0),

	Coding.of(3,  8,0).getDeltaCoding(),
	Coding.of(3,  8,1).getDeltaCoding(),
	Coding.of(3, 16,0).getDeltaCoding(),
	Coding.of(3, 16,1).getDeltaCoding(),
	Coding.of(3, 32,0).getDeltaCoding(),
	Coding.of(3, 32,1).getDeltaCoding(),
	Coding.of(3, 64,0).getDeltaCoding(),
	Coding.of(3, 64,1).getDeltaCoding(),
	Coding.of(3,128,0).getDeltaCoding(),
	Coding.of(3,128,1).getDeltaCoding(),
	Coding.of(3,192,0).getDeltaCoding(),
	Coding.of(3,192,1).getDeltaCoding(),
	Coding.of(3,224,0).getDeltaCoding(),
	Coding.of(3,224,1).getDeltaCoding(),
	Coding.of(3,240,0).getDeltaCoding(),
	Coding.of(3,240,1).getDeltaCoding(),
	Coding.of(3,248,0).getDeltaCoding(),
	Coding.of(3,248,1).getDeltaCoding(),

	Coding.of(4,192,0),
	Coding.of(4,224,0),
	Coding.of(4,240,0),
	Coding.of(4,248,0),
	Coding.of(4,252,0),

	Coding.of(4,  8,0).getDeltaCoding(),
	Coding.of(4,  8,1).getDeltaCoding(),
	Coding.of(4, 16,0).getDeltaCoding(),
	Coding.of(4, 16,1).getDeltaCoding(),
	Coding.of(4, 32,0).getDeltaCoding(),
	Coding.of(4, 32,1).getDeltaCoding(),
	Coding.of(4, 64,0).getDeltaCoding(),
	Coding.of(4, 64,1).getDeltaCoding(),
	Coding.of(4,128,0).getDeltaCoding(),
	Coding.of(4,128,1).getDeltaCoding(),
	Coding.of(4,192,0).getDeltaCoding(),
	Coding.of(4,192,1).getDeltaCoding(),
	Coding.of(4,224,0).getDeltaCoding(),
	Coding.of(4,224,1).getDeltaCoding(),
	Coding.of(4,240,0).getDeltaCoding(),
	Coding.of(4,240,1).getDeltaCoding(),
	Coding.of(4,248,0).getDeltaCoding(),
	Coding.of(4,248,1).getDeltaCoding(),

	null
    };
    final private static HashMap basicCodingIndexes;
    static {
	HashMap map = new HashMap();
	for (int i = 0; i < basicCodings.length; i++) {
	    Coding c = basicCodings[i];
	    if (c == null)  continue;
	    map.put(c, new Integer(i));
	}
	basicCodingIndexes = map;
    }
    public static Coding codingForIndex(int i) {
	return i < basicCodings.length ? basicCodings[i] : null;
    }
    public static int indexOf(Coding c) {
	Integer i = (Integer) basicCodingIndexes.get(c);
	if(D)assert0(i != null);
	if (i == null)  return 0;
	return i.intValue();
    }

    static final int ALTERNATE_CODING_LIMIT = 128;

    /** A "short" band has too few elements for variable coding
      * to be a likely payoff.  It has no coding prefix.
      */
    static final int SHORT_BAND_LENGTH = 100;

    public static final int NO_PHASE        = 0;

    // package writing phases:
    public static final int COLLECT_PHASE   = 1; // collect data before write
    public static final int WRITE_PHASE     = 3; // ready to write bytes

    // package reading phases:
    public static final int EXPECT_PHASE    = 2; // gather expected counts
    public static final int READ_PHASE      = 4; // ready to read bytes
    public static final int DISBURSE_PHASE  = 6; // pass out data after read

    public static final int DONE_PHASE      = 8; // done writing or reading

    static boolean phaseIsRead(int p) {
	return (p % 2) == 0;
    }
    static int phaseCmp(int p0, int p1) {
	if(D)assert0((p0 % 2) == (p1 % 2) || (p0 % 8) == 0 || (p1 % 8) == 0);
	return p0 - p1;
    }

    /** The packed file is divided up into a number of segments.
     *  Most segments are typed as ValueBand, strongly-typed sequences
     *  of integer values, all interpreted in a single way.
     *  A few segments are ByteBands, which hetergeneous sequences
     *  of bytes.
     *
     *  The two phases for writing a packed file are COLLECT and WRITE.
     *  1. When writing a packed file, each band collects
     *  data in an ad-hoc order.
     *  2. At the end, each band is assigned a coding scheme,
     *  and then all the bands are written in their global order.
     *
     *  The three phases for reading a packed file are EXPECT, READ,
     *  and DISBURSE.
     *	1. For each band, the expected number of integers  is determined.
     *  2. The data is actually read from the file into the band.
     *  3. The band pays out its values as requested, in an ad hoc order.
     *
     *  When the last phase of a band is done, it is marked so (DONE).
     *  Clearly, these phases must be properly ordered WRT each other.
     */
    abstract class Band {
	private int    phase = NO_PHASE;
	private final  String name;

	private int    valuesExpected;

	final public Coding regularCoding;

	final public int seqForDebug;
	public int       elementCountForDebug;

	protected Band(String name, Coding regularCoding) {
	    this.name = name;
	    this.regularCoding = regularCoding;
	    this.seqForDebug = ++nextSeqForDebug;
	    // caller must call init
	}

	public Band init() {
	    // Cannot due this from the constructor, because constructor
	    // may wish to initialize some subclass variables.
	    // Set initial phase for reading or writing:
	    if (isReader)
		readyToExpect();
	    else
		readyToCollect();
	    return this;
	}

	// common operations
	boolean isReader() { return isReader; }
	int phase() { return phase; }
	String name() { return name; }

	/** Return -1 if data buffer not allocated, else max length. */
	public abstract int capacity();

	/** Allocate data buffer to specified length. */
	protected abstract void setCapacity(int cap);

	/** Return current number of values in buffer, which must exist. */
	public abstract int length();

	protected abstract int valuesRemainingForDebug();

	public final int valuesExpected() {
	    return valuesExpected;
	}

	/** Write out bytes, encoding the values. */
	public final void writeTo(OutputStream out) throws IOException {
	    if(D)assert0(assertReadyToWriteTo(this, out));
	    setPhase(WRITE_PHASE);
	    // subclasses continue by writing their contents to output
	    writeDataTo(out);
	    doneWriting();
	}

	abstract protected void writeDataTo(OutputStream out) throws IOException;

	/** Expect a certain number of values. */
	void expectLength(int l) {
	    if(D)assert0(phase == EXPECT_PHASE);
	    if(D)assert0(valuesExpected == 0);  // all at once
	    if(D)assert0(l >= 0);
	    valuesExpected = l;
	}
	/** Expect more values.  (Multiple calls accumulate.) */
	void expectMoreLength(int l) {
	    if(D)assert0(phase == EXPECT_PHASE);
	    valuesExpected += l;
	}


	/// Phase change markers.

	private void readyToCollect() { // called implicitly by constructor
	    setCapacity(1);
	    setPhase(COLLECT_PHASE);
	}
	protected void doneWriting() {
	    if(D)assert0(phase == WRITE_PHASE);
	    setPhase(DONE_PHASE);
	}
	private void readyToExpect() { // called implicitly by constructor
	    setPhase(EXPECT_PHASE);
	}
	/** Read in bytes, decoding the values. */
	public final void readFrom(InputStream in) throws IOException {
	    if(D)assert0(assertReadyToReadFrom(this, in));
	    setCapacity(valuesExpected());
	    setPhase(READ_PHASE);
	    // subclasses continue by reading their contents from input:
	    readDataFrom(in);
	    readyToDisburse();
	}
	abstract protected void readDataFrom(InputStream in) throws IOException;
	protected void readyToDisburse() {
	    if (verbose > 1)  System.out.println("readyToDisburse "+this);
	    setPhase(DISBURSE_PHASE);
	}
	public final void doneDisbursing() {
	    if(D)assert0(phase == DISBURSE_PHASE);
	    setPhase(DONE_PHASE);
	}

	protected void setPhase(int newPhase) {
	    if(D)assert0(assertPhaseChangeOK(this, phase, newPhase));
	    this.phase = newPhase;
	}

	protected int lengthForDebug = -1;  // DEBUG ONLY
	public String toString() {  // DEBUG ONLY
	    int length = (lengthForDebug != -1 ? lengthForDebug : length());
	    String str = name;
	    if (length != 0)
		str += "[" + length + "]";
	    if (elementCountForDebug != 0)
		str += "(" + elementCountForDebug + ")";
	    return str;
	}
    }

    class ValueBand extends Band {
	private int[]  values;   // must be null in EXPECT phase
	private int    length;
	private int    valuesDisbursed;

	protected ValueBand(String name, Coding regularCoding) {
	    super(name, regularCoding);
	}

	public int capacity() {
	    return values == null ? -1 : values.length;
	}

	/** Declare predicted or needed capacity. */
	protected void setCapacity(int cap) {
	    if(D)assert0(length <= cap);
	    if (cap == -1) { values = null; return; }
	    int[] newValues = new int[cap];
	    if (values != null) {
		int toCopy = Math.min(length, cap);
		System.arraycopy(values, 0, newValues, 0, toCopy);
	    }
	    values = newValues;
	}

	public int length() {
	    return length;
	}
	protected int valuesRemainingForDebug() {
	    return length - valuesDisbursed;
	}
	protected int valueAtForDebug(int i) {
	    return values[i];
	}

	protected void initializeValues(int[] values) {
	    if(D)assert0(assertCanChangeLength(this));
	    if(D)assert0(length == 0);
	    this.values = values;
	    this.length = values.length;
	}

	/** Collect one value, or store one decoded value. */
	protected void addValue(int x) {
	    if(D)assert0(assertCanChangeLength(this));
	    if (length == values.length)
		setCapacity(length < 1000 ? length * 10 : length * 2);
	    values[length++] = x;
	}

	public void writeDataUsing(Coding c, OutputStream out) throws IOException {
	    c.writeArrayTo(out, values, 0, length);
	}

	public void readDataUsing(Coding c, InputStream in) throws IOException {
	    c.readArrayFrom(in, values, 0, length);
	}

	private boolean canVaryCoding() {
	    if (!Pack.varyCodings)  return false;
	    if (length == 0)        return false;
	    // unsigned bands: cp_Utf8_chars, code_headers, bc_byte
	    if (!regularCoding.isSigned())  return false;
	    return true;
	}

	private boolean shouldVaryCoding() {
	    if(D)assert0(canVaryCoding());
	    if (Pack.effort < 9 && length < 100)  return false;
	    return true;
	}

	protected void writeDataTo(OutputStream out) throws IOException {
	    if (length == 0)  return;  // nothing to write
	    Coding c;
	    if (!canVaryCoding() || !shouldVaryCoding()) {
		c = regularCoding;
	    } else {
		c = chooseCoding(values, 0, length, regularCoding, name());
	    }
	    if (c != regularCoding) {
		// Use an alternate coding.
		if (verbose > 1)
		    System.out.println("alternate coding "+this+" "+c);
		int cval = encodeCoding(c);
		if(D)assert0(cval < -1);
		regularCoding.writeTo(out, cval);
	    } else if (canVaryCoding() && values[0] < 0) {
		// Initial negative expresses an alternate coding.
		// Escape this by specifying the default coding (-1).
		if (verbose > 1)
		    System.out.println("escaping regular coding "+this+" for "+values[0]);
		regularCoding.writeTo(out, -1);
	    }
	    writeDataUsing(c, out);
	    if (Pack.dumpBands)  dumpBand(c);
	}

	protected void readDataFrom(InputStream in) throws IOException {
	    length = valuesExpected();
	    if (length == 0)  return;  // nothing to read
	    Coding c;
	    if (!canVaryCoding()) {
		c = regularCoding;
	    } else {
		if(D)assert0(in.markSupported());
		in.mark(5);  // maximum B value of codings
		long availForDebug = 0;
		if(D)assert0((availForDebug = in.available()) > 0);
		int cval = regularCoding.readFrom(in);
		if (cval < 0) {
		    // Consume this value.
		    if (cval == -1)
			c = regularCoding;
		    else
			c = decodeCoding(cval);
		} else {
		    // Back up; there is no alternate coding.
		    in.reset();
		    if(D)assert0(in.available() == availForDebug);
		    c = regularCoding;
		}
	    }
	    if (c != regularCoding) {
		if (verbose > 1)
		    System.out.println(name()+": irregular coding "+c);
	    }
	    readDataUsing(c, in);
	    if (Pack.dumpBands)  dumpBand(c);
	}

	private void dumpBand(Coding c) throws IOException {
	    if(D)assert0(Pack.dumpBands);
	    PrintStream ps = new PrintStream(getDumpStream(this, ".txt"));
	    String irr = (c == regularCoding) ? "" : " irregular";
	    ps.print("# length="+length+irr+" coding="+c);
	    for (int i = 0; i < length; i++) {
		if (i % 10 == 0)
		    ps.println();
		else
		    ps.print(" ");
		ps.print(values[i]);
	    }
	    ps.println();
	    ps.close();
	    OutputStream ds = getDumpStream(this, ".bnd");
	    writeDataUsing(c, ds);
	    ds.close();
	}

	/** Disburse one value. */
	protected int getValue() {
	    if(D)assert0(phase() == DISBURSE_PHASE);
	    if(D)assert0(valuesDisbursed < length);
	    return values[valuesDisbursed++];
	}

	/** Disburse all values in one shot. */
	protected int[] getValueArray() {
	    if(D)assert0(phase() == DISBURSE_PHASE);
	    if(D)assert0(valuesDisbursed == 0);  // only get the whole array
	    if (values.length != length) {
		// Might as well shrink-to-fit right now.
		int[] a = new int[length()];
		System.arraycopy(values, 0, a, 0, length);
		values = a;
	    }
	    valuesDisbursed += length;
	    return values;
	}
    }

    // one-element pool of reusable buffers
    ByteArrayOutputStream bufPool;
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

    class ByteBand extends Band {
	private ByteArrayOutputStream bytes;
	private DataOutputStream      dataOut;
	private DataInputStream       dataIn;

	public ByteBand(String name) {
	    super(name, BYTE_CODING);
	}

	public int capacity() {
	    return bytes == null ? -1 : Integer.MAX_VALUE;
	}
	protected void setCapacity(int cap) {
	    if(D)assert0(bytes == null);  // do this just once
	    bytes = getBuf();  // buffer is variable capacity
	    dataOut = new DataOutputStream(bytes);
	}
	public void destroy() {
	    lengthForDebug = length();
	    if (bytes != null)  putBuf(bytes);  // recycle this guy
	    bytes = null;
	}

	public int length() {
	    return bytes == null ? -1 : bytes.size();
	}
	public void reset() {
	    bytes.reset();
	}
	protected int valuesRemainingForDebug() {
	    if (bytes == null)  return -1;
	    try {
		return dataIn.available();
	    } catch (IOException ee) {
		throw new RuntimeException(ee); // cannot happen
	    }
	}

	protected void writeDataTo(OutputStream out) throws IOException {
	    bytes.writeTo(out);
	    if (Pack.dumpBands)  dumpBand();
	    destroy();  // done with the bits!
	}

	private void dumpBand() throws IOException {
	    if(D)assert0(Pack.dumpBands);
	    if (this instanceof AttrBlock)  return;
	    OutputStream ds = getDumpStream(this, ".bnd");
	    bytes.writeTo(ds);
	    ds.close();
	}

	protected void readDataFrom(InputStream in) throws IOException {
	    int vex = valuesExpected();
	    byte[] buf = new byte[1<<8];
	    while (vex > 0) {
		int nr = in.read(buf, 0, Math.min(vex, buf.length));
		if (nr < 0)  throw new EOFException();
		bytes.write(buf, 0, nr);
		vex -= nr;
	    }
	    if (Pack.dumpBands)  dumpBand();
	}

	protected void readyToDisburse() {
	    InputStream in = new ByteArrayInputStream(bytes.toByteArray());
	    dataIn = new DataInputStream(in);
	    super.readyToDisburse();
	}

	// alternative to readFrom:
	public void setInputStreamFrom(InputStream in) throws IOException {
	    if(D)assert0(assertReadyToReadFrom(this, in));
	    setPhase(READ_PHASE);
	    // subclasses continue by reading their contents from input:
	    dataIn = new DataInputStream(in);
	    super.readyToDisburse();
	}

	public DataOutputStream collectorStream() {
	    if(D)assert0(phase() == COLLECT_PHASE);
	    if(D)assert0(dataOut != null);
	    return dataOut;
	}
	public void putByte(int x) throws IOException {
	    collectorStream().writeByte(x);
	}
	public void putShort(int x) throws IOException {
	    collectorStream().writeShort(x);
	}
	public void putInt32(int x) throws IOException {
	    collectorStream().writeInt(x);
	}
	public void putUnsigned(int x) throws IOException {
	    UNSIGNED_CODING.writeTo(collectorStream(), x);
	}

	/** Put a CP ref into a heterogeneous byte band.  Relatively rare. */
	public void putSingleRef(Entry e, Index index) throws IOException {
	    putUnsigned(encodeRef(e, index));
	}
	public void putSingleRef(Entry e, byte cptag) throws IOException {
	    putSingleRef(e, getCPIndex(cptag));
	}

	public byte[] getBytes() {
	    // Make sure we are at the begining of the input stream,
	    // and fast-forward it to the end.
	    try {
		getInputStream();
		int avail = dataIn.available();
		if(D)assert0(avail == length());
		getInputStream().skip(avail);
	    } catch (IOException ee) {
		throw new RuntimeException(ee); // cannot happen
	    }
	    // Return the bytes from which we created the input stream.
	    return bytes.toByteArray();
	}
	public DataInputStream getInputStream() {
	    if(D)assert0(phase() == DISBURSE_PHASE);
	    if(D)assert0(dataIn != null);
	    return dataIn;
	}
	public int getByte() throws IOException {
	    return getInputStream().readByte() & 0xFF;
	}
	public int getShort() throws IOException {
	    return getInputStream().readShort() & 0xFFFF;
	}
	public int getInt32() throws IOException {
	    return getInputStream().readInt();
	}
	public int getUnsigned() throws IOException {
	    return UNSIGNED_CODING.readFrom(getInputStream());
	}

	/** Get a CP ref from a heterogeneous byte band.  Relatively rare. */
	public Entry getSingleRef(Index index) throws IOException {
	    return decodeRef(getUnsigned(), index);
	}
	public Entry getSingleRef(byte cptag) throws IOException {
	    return getSingleRef(getCPIndex(cptag));
	}

	public String toString() {
	    return "byte "+super.toString();
	}
    }

    // helper for generic attributes of the form {utf8Ref, size, bytes}
    class AttrBlock extends ByteBand {
	final Entry attrName;
	Object client;  // Placeholder for containing field, method, etc.
	// Constructor for package reader:
	public AttrBlock(AttrBands attr) throws IOException {
	    this((Utf8Entry) attr.name.getRef(), /*isReader=*/ true);
	    init();
	    expectLength(attr.size.getUnsigned());
	    readFrom(attr.bits.getInputStream());
	}
	// Constructor for package writer:
	public AttrBlock(Utf8Entry attrName) {
	    this(attrName, /*isReader=*/ false);
	    init();
	}
	private AttrBlock(Utf8Entry attrName, boolean isReader) {
	    super("("+attrName.stringValue()+")");
	    this.attrName = attrName;
	    if(D)assert0(isReader == BandStructure.this.isReader);
	}
	// Write my bytes to the output bands:
	public void putAttrTo(AttrBands attr) throws IOException {
	    if(D)assert0(!isReader);
	    attr.name.putRef(attrName);
	    attr.size.putUnsigned(length());
	    writeTo(attr.bits.collectorStream());
	    attr.totalCount += 1;  // one more attribute
	}
	public String name() { return attrName.stringValue(); }
	public void skip() {
	    System.out.println(client+": skipping "+length()+" bytes of unrecognized "+name()+" attribute.");
	}
    }

    class IntBand extends ValueBand {
	// The usual coding for bands is 7bit/5byte/delta.
	public IntBand(String name, Coding regularCoding) {
	    super(name, regularCoding);
	}

	public void putInt(int x) {
	    if(D)assert0(phase() == COLLECT_PHASE);
	    addValue(x);
	}
	public void putUnsigned(int x) {
	    if(D)assert0(x >= 0);
	    if(D)assert0(assertSignOK(false));
	    putInt(x);
	}
	public void putSigned(int x) {
	    if(D)assert0(assertSignOK(true));
	    putInt(x);
	}

	public int getInt() {
	    return getValue();
	}
	public int getUnsigned() {
	    if(D)assert0(assertSignOK(false));
	    int x = getValue();
	    if(D)assert0(x >= 0);
	    return x;
	}
	public int getSigned() {
	    if(D)assert0(assertSignOK(true));
	    return getValue();
	}
	public int[] getUnsignedArray() {
	    if(D)assert0(assertSignOK(false));
	    int[] a = getValueArray();
	    if(D)assert0(assertAllUnsigned(a));
	    return a;
	}

	// DEBUG ONLY check to make sure sign/unsign usage is consistent
	private int signForAssert = 0;
	private boolean assertSignOK(boolean isSigned) {
	    int OKsign = (isSigned ? -1 : 1);
	    if (signForAssert == 0)
		signForAssert = OKsign;
	    return (signForAssert == OKsign);
	}

    }

    class FlagsBand extends ValueBand {
	public FlagsBand(String name) {
	    super(name, UNSIGNED_CODING);
	}

	// %%% get rid of na arg
	public void putFlags(int flags, int na) throws IOException {
	    // common case of zero attrs is compressed into a flag bit
	    if(D)assert0((flags & ACC_HAS_ATTRIBUTES) == 0);  // unused flag bit!
	    if (na > 0) {
		flags |= ACC_HAS_ATTRIBUTES;
	    }
	    addValue(flags);
	}

	public int getFlags() throws IOException {
	    return getValue();
	}

	public int[] getFlagsArray() throws IOException {
	    return getValueArray();
	}
    }

    class CPRefBand extends ValueBand {
	Index index;
	public CPRefBand(String name, Coding regularCoding) {
	    super(name, regularCoding);
	}

	public void setIndex(Index index) {
	    this.index = index;
	}

	protected void readDataFrom(InputStream in) throws IOException {
	    super.readDataFrom(in);
	    if(D)assert0(assertValidCPRefs(this));
	}

	/** Write a constant pool reference. */
	public void putRef(Entry e) throws IOException {
	    if(D)assert0(index != null);
	    addValue(encodeRef(e, index));
	}
	public void putRef(Entry e, Index index) throws IOException {
	    if(D)assert0(this.index == null);
	    addValue(encodeRef(e, index));
	}
	public void putRef(Entry e, byte cptag) throws IOException {
	    putRef(e, getCPIndex(cptag));
	}

	public Entry getRef() throws IOException {
	    if(D)assert0(index != null);
	    return decodeRef(getValue(), index);
	}
	public Entry[] getRefArray() throws IOException {
	    int[] ia = getValueArray();
	    Entry[] a = new Entry[ia.length];
	    for (int i = 0; i < a.length; i++) {
		a[i] = decodeRef(ia[i], index);
	    }
	    return a;
	}
	public Entry getRef(Index index) throws IOException {
	    if(D)assert0(this.index == null);
	    if(D)assert0(index != null);
	    return decodeRef(getValue(), index);
	}
	public Entry getRef(byte cptag) throws IOException {
	    return getRef(getCPIndex(cptag));
	}
    }

    int encodeRef(Entry e, Index ix) {
	int coding;
	if (e == null || e.isEmpty()) {
	    coding = 0;
	} else {
	    int offset = ix.indexOf(e);
	    // add 1 to allow separate coding for null
	    coding = 1+offset;
	}
	if (verbose > 2)
	    System.out.println("putRef "+coding+" => "+e);
	return coding;
    }

    Entry decodeRef(int n, Index ix) {
	Entry e;
	if (n == 0) {
	    e = null;
	} else {
	    // subtract 1 to allow separate coding for null
	    if (n <= 0 || n > ix.size())
		System.out.println("*** decoding bad ref "+n);
	    e = ix.getEntry(n-1);
	}
	if (verbose > 2)
	    System.out.println("getRef "+n+" => "+e);
	return e;
    }

    int decodeRefLimit(Index ix) {
	return ix.size()+1;
    }

    private CodingChooser codingChooser;
    protected CodingChooser getCodingChooser() {
	if (codingChooser == null) {
	    codingChooser = new CodingChooser(Pack.effort, basicCodings);
	    codingChooser.verbose = verbose;
	}
	return codingChooser;
    }

    public Coding chooseCoding(int[] values, int start, int end,
			       Coding regular, String bandName) {
	if(D)assert0(Pack.varyCodings);
	if (Pack.effort <= 1) {
	    return regular;
	}
	if (verbose > 1) {
	    System.out.println("--- chooseCoding "+bandName);
	}
	CodingChooser cc = getCodingChooser();
	if (outputWindow != null) {
	    ByteArrayOutputStream context = codingChooser.getContext();
	    try {
		context.reset();
		//int maxBytes = (end - start) * 2*8;  // 2 b/v, 8x context
		//outputWindow.writeTo(context, maxBytes);
		outputWindow.writeTo(context); // write all 32Kb
	    } catch (IOException ee) {
		throw new RuntimeException(ee); // cannot happen
	    }
	}
	return getCodingChooser().choose(values, start, end, regular, null);
    }

    public int encodeCoding(Coding c) {
	int id = indexOf(c);
	if(D)assert0(id >= 1);
	int cval = -1 - id;
	return cval;
    }

    public Coding decodeCoding(int cval) {
	// cval == -1 must be handled by caller
	if(D)assert0(cval <= -2);
	int id = -1 - cval;
	Coding c = codingForIndex(id);
	if(D)assert0(c != null);
	if(D)assert0(cval == encodeCoding(c));
	return c;
    }

    class MultiBand extends Band {
	MultiBand(String name, Coding regularCoding) {
	    super(name, regularCoding);
	}

	public Band init() {
	    super.init();
	    // This is all just to keep the asserts happy:
	    setCapacity(0);
	    if (phase() == EXPECT_PHASE) {
		// Fast forward:
		setPhase(READ_PHASE);
		setPhase(DISBURSE_PHASE);
	    }
	    return this;
	}

	Band[] bands     = new Band[100];
	int    bandCount = 0;

	void add(Band b) {
	    if(D)assert0(bandCount == 0 || notePrevForAssert(b, bands[bandCount-1]));
	    if (bandCount == bands.length) {
		Band[] newBands = new Band[bandCount * 2];
		System.arraycopy(bands, 0, newBands, 0, bandCount);
		bands = newBands;
	    }
	    bands[bandCount++] = b;
	}

	ByteBand newByteBand(String name) {
	    ByteBand b = new ByteBand(name);
	    b.init(); add(b);
	    return b;
	}
	IntBand newIntBand(String name) {
	    IntBand b = new IntBand(name, regularCoding);
	    b.init(); add(b);
	    return b;
	}
	IntBand newIntBand(String name, Coding regularCoding) {
	    IntBand b = new IntBand(name, regularCoding);
	    b.init(); add(b);
	    return b;
	}
	MultiBand newMultiBand(String name, Coding regularCoding) {
	    MultiBand b = new MultiBand(name, regularCoding);
	    b.init(); add(b);
	    return b;
	}
	FlagsBand newFlagsBand(String name) {
	    FlagsBand b = new FlagsBand(name);
	    b.init(); add(b);
	    return b;
	}
	CPRefBand newCPRefBand(String name) {
	    CPRefBand b = new CPRefBand(name, regularCoding);
	    b.init(); add(b);
	    return b;
	}
	CPRefBand newCPRefBand(String name, Coding regularCoding) {
	    CPRefBand b = new CPRefBand(name, regularCoding);
	    b.init(); add(b);
	    return b;
	}

	int bandCount() { return bandCount; }

	private int cap = -1;
	public int capacity() { return cap; }
	public void setCapacity(int cap) { this.cap = cap; }

	public int length() { return 0; }
	public int valuesRemainingForDebug() { return 0; }

	protected void writeDataTo(OutputStream out) throws IOException {
	    long preCount = 0;
	    if (outputCounter != null)  preCount = outputCounter.getCount();
	    for (int i = 0; i < bandCount; i++) {
		Band b = bands[i];
		b.writeTo(out);
		if (outputCounter != null) {
		    long postCount = outputCounter.getCount();
		    long len = postCount - preCount;
		    preCount = postCount;
		    if ((verbose > 0 && len > 0) || verbose > 1) {
			System.out.println("  ...wrote "+len+" bytes from "+b);
		    }
		}
	    }
	}

	protected void readDataFrom(InputStream in) throws IOException {
	    if(D)assert0(false);  // not called?
	    for (int i = 0; i < bandCount; i++) {
		Band b = bands[i];
		b.readFrom(in);
		if ((verbose > 0 && b.length() > 0) || verbose > 1) {
		    System.out.println("  ...read "+b);
		}
	    }
	}

	public String toString() {
	    return "{"+bandCount()+" bands: "+super.toString()+"}";
	}
    }

    class WindowOutputStream extends FilterOutputStream {
	private final byte[] window; // wraparound
	private int pos; // always in [0..window.length-1]
	private boolean wrapped;
	WindowOutputStream(OutputStream out, int windowSize) {
	    super(out);
	    window = new byte[windowSize];
	}
	public void write(int b) throws IOException {
	    window[pos++] = (byte) b;
	    if (pos == window.length) {
		pos = 0;
		wrapped = true;
	    }
	    out.write(b);
	}
	public void write(byte b[], int off, int len) throws IOException {
	    int wlen = window.length;
	    int off1 = off;
	    int len1 = len;
	    if (len1 > wlen) {
		off1 = off + len - wlen;
		len1 = window.length;
	    }
	    int tail = wlen - pos;
	    if (len1 >= tail) {
		System.arraycopy(b, off1, window, pos, tail);
		off1 += tail;
		len1 -= tail;
		pos = 0;
		wrapped = true;
	    }
	    if (len1 > 0) {
		if(D)assert0(len1 < wlen);
		System.arraycopy(b, off1, window, pos, len1);
		pos += len1;
		if(D)assert0(pos < wlen);
	    }
	    out.write(b, off, len);
	}
	public void reset() {
	    pos = 0;
	    wrapped = false;
	}
	public int length() {
	    return wrapped ? window.length : pos;
	}
	// Write the most recent bytes to str (at most nBytes).
	public int writeTo(OutputStream str) throws IOException {
	    return writeTo(str, window.length);
	}
	public int writeTo(OutputStream str, int nBytes) throws IOException {
	    int wlen = window.length;
	    int realMax = wrapped ? wlen : pos;
	    if (nBytes > realMax)  nBytes = realMax;
	    if (nBytes <= pos) {
		str.write(window, pos - nBytes, nBytes);
	    } else {
		int len1 = pos;
		int len2 = nBytes - pos;
		str.write(window, wlen - len2, len2);
		str.write(window, 0, len1);
	    }
	    return nBytes;
	}
    }

    WindowOutputStream outputWindow;

    ByteCountOutputStream outputCounter;

    void writeAllBandsTo(OutputStream out) throws IOException {
	if (verbose >= 0) {
	    // Wrap a byte-counter around the output stream.
	    outputCounter = new ByteCountOutputStream(out);
	    out = outputCounter;
	}
	if (Pack.useCodingChooserWindow && Pack.effort > 3) {
	    // Put a tap on the outgoing line:
	    int wsize = 1 << 15;  // See MAX_WBITS in zip's zconf.h.
	    outputWindow = new WindowOutputStream(out, wsize);
	    out = outputWindow;
	}
	all_bands.writeTo(out);
	if (verbose >= 0) {
	    System.out.println("Wrote total of "+outputCounter.getCount()+" bytes.");
	}

	outputWindow = null;
	outputCounter = null;
    }

    MultiBand all_bands = (MultiBand) new MultiBand("(package)", UNSIGNED_CODING).init();

    // package header (various random bytes)
    ByteBand package_magic = all_bands.newByteBand("package_magic");
    IntBand  package_header = all_bands.newIntBand("package_header", UNSIGNED_CODING);

    // constant pool contents
    MultiBand cp_bands = all_bands.newMultiBand("(constant_pool)", SIGNED_DELTA_CODING);
    IntBand   cp_Utf8_prefix = cp_bands.newIntBand("cp_Utf8_prefix");
    IntBand   cp_Utf8_suffix = cp_bands.newIntBand("cp_Utf8_suffix", UNSIGNED_CODING);
    IntBand   cp_Utf8_chars = cp_bands.newIntBand("cp_Utf8_chars", CHAR_CODING);
    IntBand   cp_Utf8_big_length = cp_bands.newIntBand("cp_Utf8_big_length");
    MultiBand cp_Utf8_big_chars = cp_bands.newMultiBand("(cp_Utf8_big_chars)", SIGNED_DELTA_CODING);
    IntBand   cp_Int = cp_bands.newIntBand("cp_Int", UNSIGNED_DELTA_CODING);
    IntBand   cp_Float = cp_bands.newIntBand("cp_Float", UNSIGNED_DELTA_CODING);
    IntBand   cp_Long_hi = cp_bands.newIntBand("cp_Long_hi", UNSIGNED_DELTA_CODING);
    IntBand   cp_Long_lo = cp_bands.newIntBand("cp_Long_lo");
    IntBand   cp_Double_hi = cp_bands.newIntBand("cp_Double_hi", UNSIGNED_DELTA_CODING);
    IntBand   cp_Double_lo = cp_bands.newIntBand("cp_Double_lo");
    CPRefBand cp_String = cp_bands.newCPRefBand("cp_String", UNSIGNED_DELTA_CODING);
    CPRefBand cp_Class = cp_bands.newCPRefBand("cp_Class", UNSIGNED_DELTA_CODING);
    CPRefBand cp_Signature_form = cp_bands.newCPRefBand("cp_Signature_form");
    CPRefBand cp_Signature_classes = cp_bands.newCPRefBand("cp_Signature_classes", UNSIGNED_DELTA_CODING);
    CPRefBand cp_Descr_name = cp_bands.newCPRefBand("cp_Descr_name", UNSIGNED_DELTA_CODING);
    CPRefBand cp_Descr_type = cp_bands.newCPRefBand("cp_Descr_type");
    CPRefBand cp_Field_class = cp_bands.newCPRefBand("cp_Field_class");
    CPRefBand cp_Field_desc = cp_bands.newCPRefBand("cp_Field_desc", UNSIGNED_DELTA_CODING);
    CPRefBand cp_Method_class = cp_bands.newCPRefBand("cp_Method_class");
    CPRefBand cp_Method_desc = cp_bands.newCPRefBand("cp_Method_desc", UNSIGNED_DELTA_CODING);
    CPRefBand cp_Imethod_class = cp_bands.newCPRefBand("cp_Imethod_class");
    CPRefBand cp_Imethod_desc = cp_bands.newCPRefBand("cp_Imethod_desc", UNSIGNED_DELTA_CODING);

    // bands for carrying class schema information:
    MultiBand class_bands = all_bands.newMultiBand("(class_bands)", SIGNED_DELTA_CODING);
    FlagsBand class_flags = class_bands.newFlagsBand("class_flags");
    CPRefBand class_this = class_bands.newCPRefBand("class_this");
    CPRefBand class_super = class_bands.newCPRefBand("class_super");
    IntBand   class_interface_count = class_bands.newIntBand("class_interface_count");
    CPRefBand class_interface = class_bands.newCPRefBand("class_interface");

    // band for hardwired InnerClasses attribute (shared across the package)
    CPRefBand ic_this_class = class_bands.newCPRefBand("ic_this_class", UNSIGNED_DELTA_CODING);
    FlagsBand ic_flags = class_bands.newFlagsBand("ic_flags");
    // These bands contain data only where flags sets ACC_IC_LONG_FORM:
    CPRefBand ic_outer_class = class_bands.newCPRefBand("ic_outer_class");
    CPRefBand ic_name = class_bands.newCPRefBand("ic_name");

    // bands for class members
    IntBand   class_field_count = class_bands.newIntBand("class_field_count");
    IntBand   class_method_count = class_bands.newIntBand("class_method_count");
    FlagsBand field_flags = class_bands.newFlagsBand("field_flags");
    FlagsBand method_flags = class_bands.newFlagsBand("method_flags");
    CPRefBand field_descr = class_bands.newCPRefBand("field_descr");
    CPRefBand method_descr = class_bands.newCPRefBand("method_descr", SIGNED_DELTA_CODING.setS(2));

    // band for hardwired ConstantValue attribute
    // there are entries here only for fields with the ACC_CONSTANT_VALUE bit set
    CPRefBand field_constant_value = class_bands.newCPRefBand("field_constant_value");

    // band for hardwired Exceptions attribute
    // there are entries here only for methods with the ACC_EXCEPTIONS bit set
    IntBand   method_exception_count = class_bands.newIntBand("method_exception_count", UNSIGNED_CODING);
    CPRefBand method_exceptions = class_bands.newCPRefBand("method_exceptions");

    IntBand   code_headers = class_bands.newIntBand("code_headers", BYTE_CODING);
    IntBand   code_max_stack = class_bands.newIntBand("code_max_stack", UNSIGNED_CODING);
    IntBand   code_max_locals = class_bands.newIntBand("code_max_locals", UNSIGNED_CODING);
    IntBand   code_handler_count = class_bands.newIntBand("code_handler_count", UNSIGNED_CODING);
    IntBand   code_handler_start = class_bands.newIntBand("code_handler_start", BCI_CODING);
    IntBand   code_handler_end = class_bands.newIntBand("code_handler_end", BCI_CODING);
    IntBand   code_handler_catch = class_bands.newIntBand("code_handler_catch", BCI_CODING);
    CPRefBand code_handler_class = class_bands.newCPRefBand("code_handler_class", UNSIGNED_CODING);

    // bands for generic attributes
    MultiBand attr_bands = all_bands.newMultiBand("(attr_bands)", UNSIGNED_CODING);
    IntBand   code_attr_count = attr_bands.newIntBand("code_attr_count");
    IntBand   method_attr_count = attr_bands.newIntBand("method_attr_count");
    IntBand   field_attr_count = attr_bands.newIntBand("field_attr_count");
    IntBand   class_attr_count = attr_bands.newIntBand("class_attr_count");
    // No "package_attr_count" band:  It's an Unsigned in package_header.

    CPRefBand code_attr_name = attr_bands.newCPRefBand("code_attr_name");
    CPRefBand method_attr_name = attr_bands.newCPRefBand("method_attr_name");
    CPRefBand field_attr_name = attr_bands.newCPRefBand("field_attr_name");
    CPRefBand class_attr_name = attr_bands.newCPRefBand("class_attr_name");
    CPRefBand package_attr_name = attr_bands.newCPRefBand("package_attr_name");

    IntBand   code_attr_size = attr_bands.newIntBand("code_attr_size");
    IntBand   method_attr_size = attr_bands.newIntBand("method_attr_size");
    IntBand   field_attr_size = attr_bands.newIntBand("field_attr_size");
    IntBand   class_attr_size = attr_bands.newIntBand("class_attr_size");
    IntBand   package_attr_size = attr_bands.newIntBand("package_attr_size");

    ByteBand  code_attr_bits = attr_bands.newByteBand("code_attr_bits");
    ByteBand  method_attr_bits = attr_bands.newByteBand("method_attr_bits");
    ByteBand  field_attr_bits = attr_bands.newByteBand("field_attr_bits");
    ByteBand  class_attr_bits = attr_bands.newByteBand("class_attr_bits");
    ByteBand  package_attr_bits = all_bands.newByteBand("package_attr_bits");

    // bands for bytecodes
    MultiBand bc_bands = all_bands.newMultiBand("(byte_codes)", UNSIGNED_CODING);
    ByteBand  bc_codes = bc_bands.newByteBand("bc_codes"); //BYTE_CODING
    // remaining bands provide typed opcode fields required by the bc_codes

    IntBand   bc_case_count = bc_bands.newIntBand("bc_case_count");  // *switch
    IntBand   bc_case_value = bc_bands.newIntBand("bc_case_value", SIGNED_DELTA_CODING);  // *switch
    IntBand   bc_byte = bc_bands.newIntBand("bc_byte", BYTE_CODING);    // bipush, iinc, *newarray
    IntBand   bc_short = bc_bands.newIntBand("bc_short", SIGNED_DELTA_CODING);  // sipush, wide iinc
    IntBand   bc_local = bc_bands.newIntBand("bc_local");    // *load, *store, iinc, ret
    IntBand   bc_label = bc_bands.newIntBand("bc_label", BCI_CODING.setS(2));    // if*, goto*, jsr*, *switch

    // Most CP refs exhibit some correlation, and benefit from delta coding.
    // The notable exceptions are class and method references.

    // ldc* operands:
    CPRefBand bc_intref = bc_bands.newCPRefBand("bc_intref", SIGNED_DELTA_CODING);
    CPRefBand bc_floatref = bc_bands.newCPRefBand("bc_floatref", SIGNED_DELTA_CODING);
    CPRefBand bc_longref = bc_bands.newCPRefBand("bc_longref", SIGNED_DELTA_CODING);
    CPRefBand bc_doubleref = bc_bands.newCPRefBand("bc_doubleref", SIGNED_DELTA_CODING);
    CPRefBand bc_stringref = bc_bands.newCPRefBand("bc_stringref", SIGNED_DELTA_CODING);

    CPRefBand bc_classref = bc_bands.newCPRefBand("bc_classref");   // new, *anew*, c*cast, i*of
    CPRefBand bc_fieldref = bc_bands.newCPRefBand("bc_fieldref", SIGNED_DELTA_CODING);   // get*, put*
    CPRefBand bc_methodref = bc_bands.newCPRefBand("bc_methodref"); // invoke[vs]*
    CPRefBand bc_imethodref = bc_bands.newCPRefBand("bc_imethodref", SIGNED_DELTA_CODING); // invokeinterface

    // _self_linker_op family
    CPRefBand bc_thismethod = bc_bands.newCPRefBand("bc_thismethod");   // any method within cur. class
    CPRefBand bc_supermethod = bc_bands.newCPRefBand("bc_supermethod"); // any method within superclass
    CPRefBand bc_thisfield = bc_bands.newCPRefBand("bc_thisfield");     // any field within cur. class
    CPRefBand bc_superfield = bc_bands.newCPRefBand("bc_superfield");   // any field within superclass
    // bc_invokeinit family:
    IntBand   bc_initref = bc_bands.newIntBand("bc_initref");

    // End of band definitions!

    // helps for generic attribute processing
    class AttrBands {
	public final IntBand   count;
	public final CPRefBand name;
	public final IntBand   size;
	public final ByteBand  bits;
	public final ArrayList clients = new ArrayList();

	private int[] counts;
	private int nextNonZeroCount;
	int totalCount;

	AttrBands(IntBand count, CPRefBand name, IntBand size, ByteBand bits) {
	    this.count = count;
	    this.name = name;
	    this.size = size;
	    this.bits = bits;
	}

	void expectClient(Object client) {
	    // Each client has a count.
	    // Each count in turn refers to that number of name/size/bits groups
	    // for that particular client.  This method records a client so its
	    // attributes can be parsed later on, when the bands become available.
	    clients.add(client);
	    if (count != null)
		count.expectMoreLength(1);
	    // expected # of names and sizes is computed later (from counts)
	}

	void readCountsFrom(InputStream in) throws IOException {
	    count.readFrom(in);
	    setCounts(count.getValueArray());
	}
	void setCounts(int[] counts) throws IOException {
	    if(D)assert0(this.counts == null);  // do just once
	    this.counts = counts;
	    if (verbose > 2) {
		System.out.print(count+": {");
		for (int i = 0; i < counts.length; i++)  System.out.print(" "+counts[i]);
		System.out.println(" }");
	    }
	    if(D)assert0(counts.length == clients.size());
	    skipZeroCounts(0);
	    int na = 0;
	    for (int i = 0; i < counts.length; i++)
		na += counts[i];
	    name.expectLength(na);
	    size.expectLength(na);
	}

	void readNamesFrom(InputStream in) throws IOException {
	    name.readFrom(in);
	}

	void readSizesFrom(InputStream in) throws IOException {
	    size.readFrom(in);
	}

	void readBitsFrom(InputStream in) throws IOException {
	    // do not actually copy; just alias dataIn:
	    bits.setInputStreamFrom(in);
	}

	void doneDisbursing() {
	    if (count != null)
		count.doneDisbursing();
	    name.doneDisbursing();
	    size.doneDisbursing();
	    bits.doneDisbursing();
	}

	private void skipZeroCounts(int nnzc) {
	    while (nnzc < counts.length && counts[nnzc] == 0)
		++nnzc;
	    nextNonZeroCount = nnzc;
	}

	boolean hasNextBlock() {
	    //System.out.println(count+" hasNext="+(nextNonZeroCount < counts.length));
	    return nextNonZeroCount < counts.length;
	}

	AttrBlock nextBlock() throws IOException {
	    AttrBlock b = new AttrBlock(this);
	    b.client = clients.get(nextNonZeroCount);
	    int count = --counts[nextNonZeroCount];
	    if(D)assert0(count >= 0);
	    if (count == 0)  skipZeroCounts(nextNonZeroCount+1);
	    if (verbose > 2)  System.out.println("nextBlock "+b+" for "+b.client);
	    return b;
	}
    }

    // Note:  package attrs carry side files.

    AttrBands package_attrs = new AttrBands(
	null /*count*/,     package_attr_name, package_attr_size, package_attr_bits);
    AttrBands class_attrs = new AttrBands(
	class_attr_count,   class_attr_name,   class_attr_size,   class_attr_bits);
    AttrBands field_attrs = new AttrBands(
	field_attr_count,   field_attr_name,   field_attr_size,   field_attr_bits);
    AttrBands method_attrs = new AttrBands(
	method_attr_count,  method_attr_name,  method_attr_size,  method_attr_bits);
    AttrBands code_attrs = new AttrBands(
	code_attr_count,    code_attr_name,    code_attr_size,    code_attr_bits);
    /** Given cp indexes, distribute tag-specific indexes to bands. */
    protected void setBandIndexes() {
	cp_Class.setIndex(getCPIndex(CONSTANT_Utf8));
	cp_String.setIndex(getCPIndex(CONSTANT_Utf8));
	cp_Field_class.setIndex(getCPIndex(CONSTANT_Class));
	cp_Field_desc.setIndex(getCPIndex(CONSTANT_NameandType));
	cp_Method_class.setIndex(getCPIndex(CONSTANT_Class));
	cp_Method_desc.setIndex(getCPIndex(CONSTANT_NameandType));
	cp_Imethod_class.setIndex(getCPIndex(CONSTANT_Class));
	cp_Imethod_desc.setIndex(getCPIndex(CONSTANT_NameandType));
	cp_Descr_name.setIndex(getCPIndex(CONSTANT_Utf8));
	cp_Descr_type.setIndex(getCPIndex(CONSTANT_Signature));
	cp_Signature_form.setIndex(getCPIndex(CONSTANT_Utf8));
	cp_Signature_classes.setIndex(getCPIndex(CONSTANT_Class));
	class_this.setIndex(getCPIndex(CONSTANT_Class));
	class_super.setIndex(getCPIndex(CONSTANT_Class));
	class_interface.setIndex(getCPIndex(CONSTANT_Class));
	ic_this_class.setIndex(getCPIndex(CONSTANT_Class));
	ic_outer_class.setIndex(getCPIndex(CONSTANT_Class));
	ic_name.setIndex(getCPIndex(CONSTANT_Utf8));
	field_descr.setIndex(getCPIndex(CONSTANT_NameandType));
	method_descr.setIndex(getCPIndex(CONSTANT_NameandType));
	//field_constant_value.setIndex(<field specific>)
	method_exceptions.setIndex(getCPIndex(CONSTANT_Class));
	code_handler_class.setIndex(getCPIndex(CONSTANT_Class));
	bc_intref.setIndex(getCPIndex(CONSTANT_Integer));
	bc_floatref.setIndex(getCPIndex(CONSTANT_Float));
	bc_longref.setIndex(getCPIndex(CONSTANT_Long));
	bc_doubleref.setIndex(getCPIndex(CONSTANT_Double));
	bc_stringref.setIndex(getCPIndex(CONSTANT_String));
	bc_classref.setIndex(getCPIndex(CONSTANT_Class));
	bc_fieldref.setIndex(getCPIndex(CONSTANT_Fieldref));
	bc_methodref.setIndex(getCPIndex(CONSTANT_Methodref));
	bc_imethodref.setIndex(getCPIndex(CONSTANT_InterfaceMethodref));
	//bc_thisfield.setIndex(<class specific>);
	//bc_superfield.setIndex(<class specific>);
	//bc_thismethod.setIndex(<class specific>);
	//bc_supermethod.setIndex(<class specific>);
	cp_Class.setIndex(getCPIndex(CONSTANT_Utf8));
	package_attr_name.setIndex(getCPIndex(CONSTANT_Utf8));
	class_attr_name.setIndex(getCPIndex(CONSTANT_Utf8));
	field_attr_name.setIndex(getCPIndex(CONSTANT_Utf8));
	method_attr_name.setIndex(getCPIndex(CONSTANT_Utf8));
	code_attr_name.setIndex(getCPIndex(CONSTANT_Utf8));
	if (verbose > 5) {
	    printCDecl(all_bands);
	}
    }


    // encodings found in the code_headers band
    private static final int[][] shortCodeLimits = {
	{ 12, 12 }, // s<12, l<12, e=0 [1..144]
	{  8,  8 }, //  s<8,  l<8, e=1 [145..208]
	{  7,  7 }, //  s<7,  l<7, e=2 [209..256]
    };
    public final int shortCodeHeader_h_limit = shortCodeLimits.length;

    // return 0 if it won't encode, else a number in [1..255]
    static int shortCodeHeader(Code code) {
	int s = code.max_stack;
	int l0 = code.max_locals;
	int h = code.handler_class.length;
	if (h >= shortCodeLimits.length)  return 0;
	int siglen = code.getMethod().getArgumentSize();
	if(D)assert0(l0 >= siglen);  // enough locals for signature!
	if (l0 < siglen)  return 0;
	int l1 = l0 - siglen;  // do not count locals required by the signature
	int lims = shortCodeLimits[h][0];
	int liml = shortCodeLimits[h][1];
	if (s >= lims || l1 >= liml)  return 0;
	int sc = shortCodeHeader_h_base(h);
	sc += s + lims*l1;
	if (sc > 255)  return 0;
	if(D)assert0(shortCodeHeader_max_stack(sc) == s);
	if(D)assert0(shortCodeHeader_max_locals(sc, code.getMethod()) == l0);
	if(D)assert0(shortCodeHeader_handler_count(sc) == h);
	return sc;
    }

    static int shortCodeHeader_handler_count(int sc) {
	if(D)assert0(sc > 0 && sc <= 255);
	for (int h = 0; ; h++) {
	    if (sc < shortCodeHeader_h_base(h+1))
		return h;
	}
    }
    static int shortCodeHeader_max_stack(int sc) {
	int h = shortCodeHeader_handler_count(sc);
	int lims = shortCodeLimits[h][0];
	return (sc - shortCodeHeader_h_base(h)) % lims;
    }
    static int shortCodeHeader_max_locals(int sc, Class.Method m) {
	int h = shortCodeHeader_handler_count(sc);
	int lims = shortCodeLimits[h][0];
	int siglen = m.getArgumentSize();
	return siglen + (sc - shortCodeHeader_h_base(h)) / lims;
    }

    private static int shortCodeHeader_h_base(int h) {
	if(D)assert0(h <= shortCodeLimits.length);
	int sc = 1;
	for (int h0 = 0; h0 < h; h0++) {
	    int lims = shortCodeLimits[h0][0];
	    int liml = shortCodeLimits[h0][1];
	    sc += lims * liml;
	}
	return sc;
    }

    protected CPRefBand getCPRefOpBand(int bc) {
	switch (Instruction.getCPRefOpTag(bc)) {
	case CONSTANT_Class:
	    return bc_classref;
	case CONSTANT_Fieldref:
	    return bc_fieldref;
	case CONSTANT_Methodref:
	    return bc_methodref;
	case CONSTANT_InterfaceMethodref:
	    return bc_imethodref;
	case CONSTANT_Literal:
	    switch (bc) {
	    case _ildc: case _ildc_w:
		return bc_intref;
	    case _fldc: case _fldc_w:
		return bc_floatref;
	    case _lldc2_w:
		return bc_longref;
	    case _dldc2_w:
		return bc_doubleref;
	    case _aldc: case _aldc_w:
		return bc_stringref;
	    }
	    break;
	}
	if(D)assert0(false);
	return null;
    }

    protected CPRefBand selfOpRefBand(int self_bc) {
	if(D)assert0(Instruction.isSelfLinkerOp(self_bc));
	int idx = (self_bc - _self_linker_op);
	boolean isSuper = (idx >= _self_linker_super_flag);
	if (isSuper)  idx -= _self_linker_super_flag;
	boolean isAload = (idx >= _self_linker_aload_flag);
	if (isAload)  idx -= _self_linker_aload_flag;
	int origBC = _first_linker_op + idx;
	boolean isField = Instruction.isFieldOp(origBC);
	if (!isSuper)
	    return isField? bc_thisfield: bc_thismethod;
	else
	    return isField? bc_superfield: bc_supermethod;
    }

    // Utility routine.
    public static byte[] readAll(InputStream in) throws IOException {
	byte[] contents = noBytes;
	int estimate = in.available();
	for (;;) {
	    byte[] buf = new byte[Math.max(1, estimate)];
	    int nr = in.read(buf);
	    //System.out.println("Attempted "+buf.length+", got "+nr);
	    if (nr <= 0)  break;
	    if (nr == buf.length)
		estimate *= 2;  // try a larger bite
	    else
		estimate = nr;  // some sort of buffer size
	    if (nr == buf.length && contents.length == 0) {
		contents = buf;
	    } else {
		int nc = contents.length;
		byte[] all = new byte[nc + nr];
		System.arraycopy(contents, 0, all, 0, nc);
		System.arraycopy(buf, 0, all, nc, nr);
		contents = all;
	    }
	}
	in.close();
	//System.out.println("Result size = "+contents.length);
	return contents;
    }

    /** Value used in the cp_Utf8_chars band to signal an entry
     *  in cp_Utf8_packed.
     */
    final static int notAChar = 1+(int)Character.MAX_VALUE;

    ////////////////////////////////////////////////////////////////////

    static int nextSeqForDebug;
    static File dumpDir;
    static OutputStream getDumpStream(Band b, String ext) throws IOException {
	if (dumpDir == null) {
	    dumpDir = File.createTempFile("BD_", "", new File("."));
	    dumpDir.delete();
	    if (dumpDir.mkdir())
		System.out.println("Dumping bands to "+dumpDir);
	}
	String name = b.name;
	int seq = b.seqForDebug;
	name = name.replace('(', ' ').replace(')', ' ');
	name = name.replace('/', ' ');
	name = name.trim().replace(' ','_');
	name = ((10000+seq) + "_" + name).substring(1);
	File dumpFile = new File(dumpDir, name+ext);
	System.out.println("Dumping "+b+" to "+dumpFile);
	return new BufferedOutputStream(new FileOutputStream(dumpFile));
    }

    // DEBUG ONLY:  Validate me at each length change.
    static boolean assertCanChangeLength(Band b) {
	switch (b.phase) {
	case COLLECT_PHASE:
	case READ_PHASE:
	    return true;
	}
	return false;
    }

    // DEBUG ONLY:  Validate me at each phase change.
    static boolean assertPhaseChangeOK(Band b, int p0, int p1) {
	switch (p0*10+p1) {
	/// Writing phases:
	case NO_PHASE*10+COLLECT_PHASE:
	    // Ready to collect data from the input classes.
	    if(D)assert0(!b.isReader());
	    if(D)assert0(b.capacity() >= 0);
	    if(D)assert0(b.length() == 0);
	    return true;
	case COLLECT_PHASE*10+WRITE_PHASE:
	    // Data is all collected.  Ready to write bytes to disk.
	    return true;
	case WRITE_PHASE*10+DONE_PHASE:
	    // Done writing to disk.  Ready to reset, in principle.
	    return true;

	/// Reading phases:
	case NO_PHASE*10+EXPECT_PHASE:
	    if(D)assert0(b.isReader());
	    if(D)assert0(b.capacity() < 0);
	    return true;
	case EXPECT_PHASE*10+READ_PHASE:
	    // Ready to read values from disk.
	    if(D)assert0(Math.max(0,b.capacity()) >= b.valuesExpected());
	    if(D)assert0(b.length() <= 0);
	    return true;
	case READ_PHASE*10+DISBURSE_PHASE:
	    // Ready to disburse values.
	    if(D)assert0(b.valuesRemainingForDebug() == b.length());
	    return true;
	case DISBURSE_PHASE*10+DONE_PHASE:
	    // Done disbursing values.  Ready to reset, in principle.
	    if(D)assert0(assertDoneDisbursing(b));
	    return true;
	}
	if (p0 == p1)
	    System.out.println("*** Already in phase "+p0);
	else
	    System.out.println("*** Unexpected phase "+p0+" -> "+p1);
	return false;
    }

    static private boolean assertDoneDisbursing(Band b) {
	if (b.phase != DISBURSE_PHASE) {
	    System.out.println("assertDoneDisbursing: still in phase "+b.phase+": "+b);
	    if (verbose <= 1)  return false;  // fail now
	}
	int left = b.valuesRemainingForDebug();
	if (left > 0) {
	    System.out.println("assertDoneDisbursing: "+left+" values left in "+b);
	    if (verbose <= 1)  return false;  // fail now
	}
	if (b instanceof MultiBand) {
	    MultiBand mb = (MultiBand) b;
	    for (int i = 0; i < mb.bandCount; i++) {
		Band sub = mb.bands[i];
		if (sub.phase != DONE_PHASE) {
		    System.out.println("assertDoneDisbursing: sub-band still in phase "+sub.phase+": "+sub);
		    if (verbose <= 1)  return false;  // fail now
		}
	    }
	}
	return true;
    }

    static private void printCDecl(Band b) {
	if (b instanceof MultiBand) {
	    MultiBand mb = (MultiBand) b;
	    for (int i = 0; i < mb.bandCount; i++) {
		printCDecl(mb.bands[i]);
	    }
	    return;
	}
	String ixS = "NULL";
	if (b instanceof CPRefBand) {
	    Index ix = ((CPRefBand)b).index;
	    if (ix != null)  ixS = "INDEX("+ix.debugName+")";
	}
	System.out.println("  BAND_INIT(\""+b.name()+"\""
			   +", CODING"+b.regularCoding.keyString()
			   +", "+ixS+"),");
    }

    private HashMap prevForAssertMap;

    // DEBUG ONLY:  Record something about the band order.
    boolean notePrevForAssert(Band b, Band p) {
	if (prevForAssertMap == null)
	    prevForAssertMap = new HashMap();
	prevForAssertMap.put(b, p);
	return true;
    }

    // DEBUG ONLY:  Validate next input band.
    private boolean assertReadyToReadFrom(Band b, InputStream in) throws IOException {
	Band p = (Band) prevForAssertMap.get(b);
	// Any previous band must be done reading before this one starts.
	if (p != null && phaseCmp(p.phase(), DISBURSE_PHASE) < 0) {
	    System.out.println("*** Previous band not done reading.");
	    System.out.println("    Previous band: "+p);
	    System.out.println("        Next band: "+b);
	    Thread.dumpStack();
	    if(D)assert0(verbose > 0);  // die unless verbose is true
	}
	String name = b.name;
	if (Pack.debugBands && !name.startsWith("(")) {
	    // Verify synchronization between reader & writer:
	    StringBuffer buf = new StringBuffer();
	    int ch;
	    while ((ch = in.read()) > 0)
		buf.append((char)ch);
	    String inName = buf.toString();
	    if (!inName.equals(name)) {
		System.out.print("*** Expected "+name+" but read: ");
		inName += (char)ch;
		while (inName.length() < 10)
		    inName += (char)in.read();
		for (int i = 0; i < inName.length(); i++)
		    System.out.write(inName.charAt(i));
		System.out.println();
		return false;
	    }
	}
	return true;
    }

    // DEBUG ONLY:  Make sure a bunch of cprefs are correct.
    private boolean assertValidCPRefs(CPRefBand b) {
	if (b.index == null)  return true;
	int limit = decodeRefLimit(b.index);
	for (int i = 0; i < b.length(); i++) {
	    int v = b.valueAtForDebug(i);
	    if (v < 0 || v >= limit) {
		System.out.println("*** CP ref out of range "+
				   "["+i+"] = "+v+" in "+b);
		return false;
	    }
	}
	return true;
    }

    // DEBUG ONLY:  Maybe write a debugging cookie to next output band.
    private boolean assertReadyToWriteTo(Band b, OutputStream out) throws IOException {
	Band p = (Band) prevForAssertMap.get(b);
	// Any previous band must be done writing before this one starts.
	if (p != null && phaseCmp(p.phase(), DONE_PHASE) < 0) {
	    System.out.println("*** Previous band not done writing.");
	    System.out.println("    Previous band: "+p);
	    System.out.println("        Next band: "+b);
	    Thread.dumpStack();
	    if(D)assert0(verbose > 0);  // die unless verbose is true
	}
	String name = b.name;
	if (Pack.debugBands && !name.startsWith("(")) {
	    // Verify synchronization between reader & writer:
	    for (int j = 0; j < name.length(); j++) {
		out.write((byte)name.charAt(j));
	    }
	    out.write((byte)0);
	}
	return true;
    }

    // DEBUG ONLY:  Test an array for negative elements.
    static boolean assertAllUnsigned(int[] a) {
	for (int i = 0; i < a.length; i++) {
	    if (a[i] < 0) {
		if (verbose > 0) {
		    throw new RuntimeException("Negative a["+i+"] = "+a[i]);
		}
		return false;
	    }
	}
	return true;
    }

    static void assert0(boolean z) {
	if (!z) throw new RuntimeException("assert failed");
    }
}

