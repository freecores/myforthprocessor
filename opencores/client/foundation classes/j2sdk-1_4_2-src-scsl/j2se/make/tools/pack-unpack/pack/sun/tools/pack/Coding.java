/*
 * @(#)Coding.java	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.pack;

import java.io.*;
import java.util.*;

/**
 * Define the conversions between sequences of small integers and raw bytes.
 * This is a schema of encodings which incorporates varying lengths,
 * varying degrees of length variability, and varying amounts of signed-ness.
 * @author John Rose
 * @version 1.4, 01/23/03
 */
class Coding implements Constants, Comparable {
    /*
      Coding schema for single integers, parameterized by (B,H,S):

      Let B in [1,5], H in [1,256], S in [0,3].
      (S limit is arbitrary.  B follows the 32-bit limit.  H is byte size.)

      A given (B,H,S) code varies in length from 1 to B bytes.

      The 256 values a byte may take on are divided into L=(256-H) and H
      values, with all the H values larger than the L values.
      (That is, the L values are [0,L) and the H are [L,256).)

      The last byte is always either the B-th byte, a byte with "L value"
      (<L), or both.  There is no other byte that satisfies these conditions.
      All bytes before the last always have "H values" (>=L).

      Therefore, if L==0, the code always has the full length of B bytes.
      The coding then becomes a classic B-byte little-endian unsigned integer.
      (Also, if L==128, the high bit of each byte acts signals the presence
      of a following byte, up to the maximum length.)

      In the unsigned case (S==0), the coding is compact and monotonic
      in the ordering of byte sequences defined by appending zero bytes
      to pad them to a common length B, reversing them, and ordering them
      lexicographically.  (This agrees with "little-endian" byte order.)

      Therefore, the unsigned value of a byte sequence may be defined as:
      <pre>
	U(b0)           == b0
			   in [0..L)
			   or [0..256) if B==1 (**)

	U(b0,b1)        == b0 + b1*H
			   in [L..L*(1+H))
			   or [L..L*(1+H) + H^2) if B==2

	U(b0,b1,b2)     == b0 + b1*H + b2*H^2
			   in [L*(1+H)..L*(1+H+H^2))
			   or [L*(1+H)..L*(1+H+H^2) + H^3) if B==3

	U(b[i]: i<n)    == Sum[i<n]( b[i] * H^i )
			   up to  L*Sum[i<n]( H^i )
			   or to  L*Sum[i<n]( H^i ) + H^n if n==B
      </pre>

      (**) If B==1, the values H,L play no role in the coding.
      As a convention, we require that any (1,H,S) code must always
      encode values less than H.  Thus, a simple unsigned byte is coded
      specifically by the code (1,256,0).

      (Properly speaking, the unsigned case should be parameterized as
      S==Infinity.  If the schema were regular, the case S==0 would really
      denote a numbering in which all coded values are negative.)

      If S>0, the unsigned value of a byte sequence is regarded as a binary
      integer.  If any of the S low-order bits are zero, the corresponding
      signed value will be non-negative.  If all of the S low-order bits
      (S>0) are one, the the corresponding signed value will be negative.

      The non-negative signed values are compact and monotonically increasing
      (from 0) in the ordering of the corresponding unsigned values.

      The negative signed values are compact and monotonically decreasing
      (from -1) in the ordering of the corresponding unsigned values.

      In essence, the low-order S bits function as a collective sign bit
      for negative signed numbers, and as a low-order base-(2^S-1) digit
      for non-negative signed numbers.

      Therefore, the signed value corresponding to an unsigned value is:
      <pre>
	Sgn(x)  == x                               if S==0
	Sgn(x)  == (x / 2^S)*(2^S-1) + (x % 2^S),  if S>0, (x % 2^S) < 2^S-1
	Sgn(x)  == -(x / 2^S)-1,                   if S>0, (x % 2^S) == 2^S-1
      </pre>

      Finally, the value of a byte sequence, given the coding parameters
      (B,H,S), is defined as:
      <pre>
	V(b[i]: i<n)  == Sgn(U(b[i]: i<n))
      </pre>

      The extremal positive and negative signed value for a given range
      of unsigned values may be found by sign-encoding the largest unsigned
      value which is not 2^S-1 mod 2^S, and that which is, respectively.

      Because B,H,S are variable, this is not a single coding but a schema
      of codings.  For optimal compression, it is necessary to adaptively
      select specific codings to the data being compressed.

      For example, if a sequence of values happens never to be negative,
      S==0 is the best choice.  If the values are equally balanced between
      negative and positive, S==1.  If negative values are rare, then S>1
      is more appropriate.

      A (B,H,S) encoding is called a "subrange" if it does not encode
      the largest 32-bit value, and if the number R of values it does
      encode can be expressed as a positive 32-bit value.  (Note that
      B=1 implies R<=256, B=2 implies R<=65536, etc.)

      A delta version of a given (B,H,S) coding encodes an array of integers
      by writing their successive differences in the (B,H,S) coding.
      The original integers themselves may be recovered by making a
      running accumulation of sum of the differences as they are read.

      As a special case, if a (B,H,S) encoding is a subrange, its delta
      version will only encode arrays of numbers in the coding's unsigned
      range, [0..R-1].  The coding of deltas is still in the normal signed
      range, if S!=0.  During delta encoding, all subtraction results are
      reduced to the signed range, by adding multiples of R.  Likewise,
.     during encoding, all addition results are reduced to the unsigned range.
      This special case for subranges allows the benefits of wraparound
      when encoding correlated sequences of very small positive numbers.
     */

    // Code-specific limits:
    private static int saturate32(long x) {
	if (x > Integer.MAX_VALUE)   return Integer.MAX_VALUE;
	if (x < Integer.MIN_VALUE)   return Integer.MIN_VALUE;
	return (int)x;
    }
    private static long codeRangeLong(int B, int H) {
	return codeRangeLong(B, H, B);
    }
    private static long codeRangeLong(int B, int H, int nMax) {
	// Code range for a all (B,H) codes of length <=nMax (<=B).
	// n < B:   L*Sum[i<B]( H^i ) + H^B
	// n == B:  L*Sum[i<B]( H^i ) + H^B
	if(D)assert0(nMax >= 0 && nMax <= B);
	if(D)assert0(B >= 1 && B <= 5);
	if(D)assert0(H >= 1 && H <= 256);
	if (nMax == 0)  return 0;  // no codes of zero length
	if (B == 1)     return H;  // special case; see (**) above
	int L = 256-H;
	long sum = 0;
	long H_i = 1;
	for (int n = 1; n <= nMax; n++) {
	    sum += H_i;
	    H_i *= H;
	}
	sum *= L;
	if (nMax == B)
	    sum += H_i;
	return sum;
    }
    /** Largest int representable by (B,H,S) in up to nMax bytes. */
    public static int codeMax(int B, int H, int S, int nMax) {
	//if(D)assert0(S >= 0 && S <= S_MAX);
	long max = codeRangeLong(B, H, nMax) - 1;
	if (S == 0 || max == -1)
	    return saturate32(max);
	long maxPos = max;
	if (isNegativeLong(maxPos, S)) {
	    --maxPos;
	}
	return saturate32(toPositiveLong(maxPos, S));
    }
    /** Smallest int representable by (B,H,S) in up to nMax bytes.
	Returns Integer.MIN_VALUE if S==0 but 32-bit wraparound covers
	the entire negative range.
     */
    public static int codeMin(int B, int H, int S, int nMax) {
	//if(D)assert0(S >= 0 && S <= S_MAX);
	long range = codeRangeLong(B, H, nMax);
	if (S == 0) {
	    if (range >= (long)1<<32)
		// Can code negative values via 32-bit wraparound.
		return Integer.MIN_VALUE;
	    return 0;
	}
	int Smask = (1<<S)-1;
	long maxNeg = (range & ~Smask)-1;
	if (maxNeg < 0)  return 0;  // No negative codings at all.
	return saturate32(toNegativeLong(maxNeg, S));
    }
    //public static int codeRange(int B, int H) {
    //	return saturate32(codeRangeLong(B, H));
    //}

    // Sign encoding:
    private static boolean isNegativeLong(long x, int S) {
	if(D)assert0(x >= 0);
	if(D)assert0(S > 0);
	int Smask = (1<<S)-1;
	return ((x+1) & Smask) == 0;
    }
    private static long toPositiveLong(long x, int S) {
	// Sgn(x)  == (x / 2^S)*(2^S-1) + (x % 2^S)
	if(D)assert0(!isNegativeLong(x, S) && S > 0);
	return x - (x >> S);
    }
    private static long toNegativeLong(long x, int S) {
	// Sgn(x)  == -(x / 2^S)-1
	if(D)assert0(isNegativeLong(x, S) && S > 0);
	return ~ (x >> S);
    }
    private static long toSignedLong(long x, int S) {
	if (S == 0)
	    return x;
	else if (isNegativeLong(x, S))
	    return toNegativeLong(x, S);
	else
	    return toPositiveLong(x, S);
    }
    private static int toSigned32(long lx, int S) {
	int x = (int)lx;
	if (S == 0) {
	    return x;
	}
	long ux32 = ((long)x << 32) >>> 32;
	int Smask = (1<<S)-1;
	int sx;
	if ((ux32+1 & Smask) == 0)
	    sx = (int) ~(ux32 >> S);
	else
	    sx = (int)( ux32 - (ux32 >> S) );
	if(D)assert0((S >= 2 && lx >= (1<<24)) || sx == toSignedLong(lx, S));
	if(D)assert0(!(S == 1) || sx == ((x >>> 1) ^ -(x & 1)));

	return (int)toSignedLong(lx, S);
    }
    private static long toUnsignedLong(long sx, int S) {
	if (S == 0) {
	    if(D)assert0(sx >= 0);
	    return sx;
	}
	int Smask = (1<<S)-1;
	long x;
	if (sx >= 0) {
	    // InvSgn(sx) = (sx / (2^S-1))*2^S + (sx % (2^S-1))
	    x = sx + (sx / Smask);
	} else {
	    // InvSgn(sx) = (-sx-1)*2^S + (2^S-1)
	    x = (-sx << S) - 1;
	}
	if(D)assert0(sx == toSignedLong(x, S));
	return x;
    }

    private static long toLong(int x, boolean signed) {
	long lx = x;
	if (!signed) { lx <<= 32; lx >>>= 32; }
	return lx;
    }

    // Top-level coding of single integers:
    public static void writeLong(byte[] out, int[] outpos, long sx, int B, int H, int S) {
	long x = toUnsignedLong(sx, S);
	if(D)assert0(x >= 0);
	//if (x >= codeRangeLong(B, H))  System.out.println("Bad value "+sx+" in "+new Coding(B,H,S));
	if(D)assert0(x < codeRangeLong(B, H));
	int L = 256-H;
	int pos = outpos[0];
	for (int i = 0; i < B-1; i++) {
	    if (x < L)
		break;
	    x -= L;
	    int b_i = (int)( L + (x % H) );
	    x /= H;
	    out[pos++] = (byte)b_i;
	}
	out[pos++] = (byte)x;
	// Report number of bytes written by updating outpos[0]:
	outpos[0] = pos;
	// Check right away for mis-coding.
	//if(D)assert0(sx == readLong(out, new int[1], B, H, S));
    }
    public static long readLong(byte[] in, int[] inpos, int B, int H, int S) {
	// U(b[i]: i<n) == Sum[i<n]( b[i] * H^i )
	int L = 256-H;
	long sum = 0;
	long H_i = 1;
	int pos = inpos[0];
	for (int i = 0; i < B; i++) {
	    int b_i = in[pos++] & 0xFF;
	    sum += b_i*H_i;
	    H_i *= H;
	    if (b_i < L)  break;
	}
	//if(D)assert0(sum >= 0 && sum < codeRangeLong(B, H));
	// Report number of bytes read by updating inpos[0]:
	inpos[0] = pos;
	return toSigned32(sum, S);
    }
    // The Stream version doesn't fetch a byte unless it is needed for coding.
    public static long readLongFrom(InputStream in, int B, int H, int S) throws IOException {
	// U(b[i]: i<n) == Sum[i<n]( b[i] * H^i )
	int L = 256-H;
	long sum = 0;
	long H_i = 1;
	for (int i = 0; i < B; i++) {
	    int b_i = in.read();
	    if (b_i < 0)  throw new RuntimeException("unexpected EOF");
	    sum += b_i*H_i;
	    H_i *= H;
	    if (b_i < L)  break;
	}
	if(D)assert0(sum >= 0 && sum < codeRangeLong(B, H));
	return toSigned32(sum, S);
    }

    public static final int B_MAX = 5;    /* B: [1,5] */
    public static final int H_MAX = 256;  /* H: [1,256] */
    public static final int S_MAX = 2;    /* S: [0,2] */

    // END OF STATICS.

    private final int B; /*1..5*/	// # bytes (1..5)
    private final int H; /*1..256*/	// # codes requiring a higher byte
    private final int L; /*0..255*/	// # codes requiring a higher byte
    private final int S; /*0..3*/	// # low-order bits representing sign
    private final int del; /*0..2*/	// type of delta encoding (0 == none)
    private final int min;		// smallest representable value
    private final int max;		// largest representable value
    private final int umin;		// smallest representable uns. value
    private final int umax;		// largest representable uns. value
    private final int[] byteMin;	// smallest repr. value, given # bytes
    private final int[] byteMax;	// largest repr. value, given # bytes

    private Coding(int B, int H, int S) {
	this(B, H, S, 0);
    }
    private Coding(int B, int H, int S, int del) {
	this.B = B;
	this.H = H;
	this.L = 256-H;
	this.S = S;
	this.del = del;
	this.min = codeMin(B, H, S, B);
	this.max = codeMax(B, H, S, B);
	this.umin = codeMin(B, H, 0, B);
	this.umax = codeMax(B, H, 0, B);
	this.byteMin = new int[B];
	this.byteMax = new int[B];

	for (int nMax = 1; nMax <= B; nMax++) {
	    byteMin[nMax-1] = codeMin(B, H, S, nMax);
	    byteMax[nMax-1] = codeMax(B, H, S, nMax);
	}
    }

    public boolean equals(Object x) {
	if (!(x instanceof Coding))  return false;
	Coding that = (Coding) x;
	if (this.B != that.B)  return false;
	if (this.H != that.H)  return false;
	if (this.S != that.S)  return false;
	if (this.del != that.del)  return false;
	return true;
    }

    public int hashCode() {
	return (del<<14)+(S<<11)+(B<<8)+(H<<0);
    }

    private static HashMap codeMap;

    private static synchronized Coding of(int B, int H, int S, int del) {
	if (codeMap == null)  codeMap = new HashMap();
	Coding x0 = new Coding(B, H, S, del);
	Coding x1 = (Coding) codeMap.get(x0);
	if (x1 == null)  codeMap.put(x0, x1 = x0);
	return x1;
    }

    public static Coding of(int B, int H, int S) {
	return of(B, H, S, 0);
    }

    public boolean canRepresent(int x) {
	return (x >= min && x <= max);
    }
    public boolean canRepresentUnsigned(int x) {
	return (x >= umin && x <= umax);
    }

    // object-oriented code/decode
    public int readFrom(byte[] in, int[] inpos) {
	return (int) readLong(in, inpos, B, H, S);
    }
    public void writeTo(byte[] out, int[] outpos, int x) {
	writeLong(out, outpos, toLong(x, S>0), B, H, S);
    }

    // Stream versions
    public int readFrom(InputStream in) throws IOException {
	long x = readLongFrom(in, B, H, S);
	if(D)assert0(x == (int)x);
	return (int)x;
    }
    public void writeTo(OutputStream out, int x) throws IOException {
	byte[] buf = new byte[B];
	int[] pos = new int[1];
	writeLong(buf, pos, toLong(x, S>0), B, H, S);
	out.write(buf, 0, pos[0]);
    }

    // Stream/array versions
    public void readArrayFrom(InputStream in, int[] a, int start, int end) throws IOException {
	readArrayFrom(in, a, start, end, null);
    }
    public void readArrayFrom(InputStream in, int[] a, int start, int end, int[] dstate) throws IOException {
	// %%% use byte[] buffer
	for (int i = start; i < end; i++)
	    a[i] = readFrom(in);
	for (int dstep = 0; dstep < del; dstep++) {
	    int state = 0;
	    if (dstate != null)  state = dstate[dstep];
	    for (int i = start; i < end; i++) {
		state += a[i];
		// Reduce array values to the required range.
		if (isSubrange()) {
		    state = reduceToUnsignedRange(state);
		}
		a[i] = state;
	    }
	    if (dstate != null)  dstate[dstep] = state;
	}
    }
    public void writeArrayTo(OutputStream out, int[] a, int start, int end) throws IOException {
	writeArrayTo(out, a, start, end, null);
    }
    public void writeArrayTo(OutputStream out, int[] a, int start, int end, int[] dstate) throws IOException {
	for (int dstep = 0; dstep < del; dstep++) {
	    if(D)assert0(dstep == 0); // dstate[dstep] must be dstate[0]
	    int[] deltas = makeDeltas(a, start, end, dstate);
	    if (isSubrange()) {
		// Reduce array values to the required range.
		for (int i = 0; i < deltas.length; i++) {
		    deltas[i] = reduceToSignedRange(deltas[i]);
		}
	    }
	    a = deltas;
	    start = 0;
	    end = deltas.length;
	}
	// The following code is a buffered version of this loop:
	//    for (int i = start; i < end; i++)
	//        writeTo(out, a[i]);
	byte[] buf = new byte[1<<8];
	final int bufmax = buf.length-B;
	int[] pos = { 0 };
	for (int i = start; i < end; ) {
	    while (pos[0] <= bufmax) {
		writeTo(buf, pos, a[i++]);
		if (i >= end)  break;
	    }
	    out.write(buf, 0, pos[0]);
	    pos[0] = 0;
	}
    }

    /** Tell if the range of this coding (number of distinct
     *  representable values) can be expressed in 32 bits.
     */
    boolean isSubrange() {
	return max < Integer.MAX_VALUE
	    && ((long)max - (long)min + 1) <= Integer.MAX_VALUE;
    }

    /** Tell if this coding can represent all 32-bit values.
     *  Note:  Some codings, such as unsigned ones, can be neither
     *  subranges nor full-range codings.
     */
    boolean isFullRange() {
	return max == Integer.MAX_VALUE && min == Integer.MIN_VALUE;
    }

    /** Return the number of values this coding (a subrange) can represent. */
    int getRange() {
	if(D)assert0(isSubrange());
	return (max - min) + 1;  // range includes both min & max
    }

    Coding setB(int B) { return Coding.of(B, H, S, del); }
    Coding setH(int H) { return Coding.of(B, H, S, del); }
    Coding setS(int S) { return Coding.of(B, H, S, del); }

    Coding getDeltaCoding() {
	return Coding.of(B, H, S, del+1);
    }
    /** Return a coding suitable for representing summed values. */
    Coding getValueCoding() {
	if (isDelta())
	    return Coding.of(B, H, 0, del-1);
	else
	    return this;
    }

    /** Reduce the given value to be within this coding's unsigned range,
     *  by adding or subtracting a multiple of (max-min+1).
     */
    int reduceToUnsignedRange(int value) {
	if (canRepresentUnsigned(value))
	    // already in unsigned range
	    return value;
	int range = getRange();
	if(D)assert0(range > 0);
	value %= range;
	if (value < 0)  value += range;
	if(D)assert0(canRepresentUnsigned(value));
	return value;
    }

    int reduceToSignedRange(int value) {
	if (canRepresent(value))
	    // already in signed range
	    return value;
	int range = getRange();
	if(D)assert0(range > 0);
	value -= min;
	value %= range;
	if (value < 0)  value += range;
	value += min;
	if(D)assert0(canRepresent(value));
	return value;
    }

    /** Does this coding support at least one negative value?
	Includes codings that can do so via 32-bit wraparound.
     */
    boolean isSigned() {
	return min < 0;
    }
    /** Does this coding code arrays by making successive differences? */
    boolean isDelta() {
	return del != 0;
    }

    public int B() { return B; }
    public int H() { return H; }
    public int L() { return L; }
    public int S() { return S; }
    public int del() { return del; }
    public int min() { return min; }
    public int max() { return max; }
    public int umin() { return umin; }
    public int umax() { return umax; }
    public int byteMin(int b) { return byteMin[b]; }
    public int byteMax(int b) { return byteMax[b]; }

    public int compareTo(Object x) {
	Coding that = (Coding) x;
	int dkey = this.del - that.del;
	if (dkey == 0)
	    dkey = this.B - that.B;
	if (dkey == 0)
	    dkey = this.H - that.H;
	if (dkey == 0)
	    dkey = this.S - that.S;
	return dkey;
    }

    /** Heuristic measure of the difference between two codings. */
    public int distanceFrom(Coding that) {
	int diffdel = this.del - that.del;
	if (diffdel < 0)  diffdel = -diffdel;
	int diffS = this.S - that.S;
	if (diffS < 0)  diffS = -diffS;
	int diffB = this.B - that.B;
	if (diffB < 0)  diffB = -diffB;
	int diffHL;
	if (this.H == that.H) {
	    diffHL = 0;
	} else {
	    // Distance in log space of H (<=128) and L (<128).
	    int thisHL = this.getHL();
	    int thatHL = that.getHL();
	    // Double the accuracy of the log:
	    thisHL *= thisHL;
	    thatHL *= thatHL;
	    if (thisHL > thatHL)
		diffHL = ceil_lg2(1+(thisHL-1)/thatHL);
	    else
		diffHL = ceil_lg2(1+(thatHL-1)/thisHL);
	}
	int norm = 5*(diffdel + diffS + diffB) + diffHL;
	if(D)assert0(norm != 0 || this.compareTo(that) == 0);
	return norm;
    }
    private int getHL() {
	// Follow H in log space by the multiplicative inverse of L.
	if (H <= 128)  return H;
	if (L >= 1)    return 128*128/L;
	return 128*256;
    }

    /** ceiling(log[2](x)): {1->0, 2->1, 3->2, 4->2, ...} */
    static int ceil_lg2(int x) {
	if(D)assert0(x-1 >= 0);  // x in range (int.MIN_VALUE -> 32)
	x -= 1;
	int lg = 0;
	while (x != 0) {
	    lg++;
	    x >>= 1;
	}
	return lg;
    }

    static private final byte[] byteBitWidths = new byte[0x100];
    static {
	for (int b = 0; b < byteBitWidths.length; b++) {
	    byteBitWidths[b] = (byte) ceil_lg2(b + 1);
	}
	if(D) {
	    for (int i = 10; i >= 0; i = (i << 1) - (i >> 3)) {
		if(D)assert0(bitWidth(i) == ceil_lg2(i + 1));
	    }
	}
    }

    /** Number of significant bits in i, not counting sign bits.
     *  For positive i, it is ceil_lg2(i + 1).
     */
    static int bitWidth(int i) {
	if (i < 0)  i = ~i;  // change sign
	int w = 0;
	int lo = i;
	if (lo < byteBitWidths.length)
	    return byteBitWidths[lo];
	int hi;
	hi = (lo >>> 16);
	if (hi != 0) {
	    lo = hi;
	    w += 16;
	}
	hi = (lo >>> 8);
	if (hi != 0) {
	    lo = hi;
	    w += 8;
	}
	w += byteBitWidths[lo];
	//if(D)assert0(w == ceil_lg2(i + 1));
	return w;
    }

    static int[] makeDeltas(int[] values, int start, int end, int[] dstate) {
	int count = end-start;
	int[] deltas = new int[count];
	int state = 0;
	if (dstate != null)  state = dstate[0];
	for (int i = 0; i < count; i++) {
	    int value = values[start+i];
	    deltas[i] = value - state;
	    state = value;
	}
	if (dstate != null)  dstate[0] = state;
	return deltas;
    }

    boolean canRepresent(int minValue, int maxValue) {
	if(D)assert0(minValue <= maxValue);
	if (del > 0) {
	    if (isSubrange()) {
		// We will force the values to reduce to the right subrange.
		return canRepresentUnsigned(maxValue)
		    && canRepresentUnsigned(minValue);
	    } else if ((long)maxValue - (long)minValue <= Integer.MAX_VALUE) {
		// Values lie in a relatively narrow range (31 bits).
		return max >= (maxValue - minValue)
		    && min <= (minValue - maxValue);
	    } else {
		// Huge range; delta values must assume full 32-bit range.
		return isFullRange();
	    }
	}
	else
	    // final values must be representable
	    return canRepresent(maxValue) && canRepresent(minValue);
    }

    public String keyString() {
	return "("+B+","+H+","+S+","+del+")";
    }

    public String toString() {
	if(D)return stringForDebug();
	// Skip preceding goo if not debugging.
	return "Coding"+keyString();
    }

    private String stringForDebug() {
	String minS = (min == Integer.MIN_VALUE ? "min" : ""+min);
	String maxS = (max == Integer.MAX_VALUE ? "max" : ""+max);
	String str = keyString()+" L="+L+" r=["+minS+","+maxS+"]";
	if (false) {
	    str += " {";
	    int prev_range = 0;
	    for (int n = 1; n <= B; n++) {
		int range_n = saturate32((long)byteMax[n-1] - byteMin[n-1] + 1);
		if(D)assert0(range_n == saturate32(codeRangeLong(B, H, n)));
		range_n -= prev_range;
		prev_range = range_n;
		String rngS = (range_n == Integer.MAX_VALUE ? "max" : ""+range_n);
		str += " #"+n+"="+rngS;
	    }
	    str += " }";
	}
	return str;
    }

    static void assert0(boolean z) {
	if (!z) throw new RuntimeException("assert failed");
    }

}
