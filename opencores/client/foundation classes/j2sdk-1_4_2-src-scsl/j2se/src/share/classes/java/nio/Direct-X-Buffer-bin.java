/*
 * @(#)Direct-X-Buffer-bin.java	1.13 03/01/29
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#warn This file is preprocessed before being compiled

class XXX {

#begin

#if[rw]

    private $type$ get$Type$(long a) {
	if (unaligned) {
	    $memtype$ x = unsafe.get$Memtype$(a);
	    return $fromBits$(nativeByteOrder ? x : Bits.swap(x));
	}
	return Bits.get$Type$(a, bigEndian);
    }

    public $type$ get$Type$() {
	return get$Type$(ix(nextGetIndex($BYTES_PER_VALUE$)));
    }

    public $type$ get$Type$(int i) {
	return get$Type$(ix(checkIndex(i, $BYTES_PER_VALUE$)));
    }

#end[rw]

    private ByteBuffer put$Type$(long a, $type$ x) {
#if[rw]
	if (unaligned) {
	    $memtype$ y = $toBits$(x);
	    unsafe.put$Memtype$(a, (nativeByteOrder ? y : Bits.swap(y)));
	} else {
	    Bits.put$Type$(a, x, bigEndian);
	}
	return this;
#else[rw]
	throw new ReadOnlyBufferException();
#end[rw]
    }

    public ByteBuffer put$Type$($type$ x) {
#if[rw]
	put$Type$(ix(nextPutIndex($BYTES_PER_VALUE$)), x);
	return this;
#else[rw]
	throw new ReadOnlyBufferException();
#end[rw]
    }

    public ByteBuffer put$Type$(int i, $type$ x) {
#if[rw]
	put$Type$(ix(checkIndex(i, $BYTES_PER_VALUE$)), x);
	return this;
#else[rw]
	throw new ReadOnlyBufferException();
#end[rw]
    }

    public $Type$Buffer as$Type$Buffer() {
	int off = this.position();
	int lim = this.limit();
	assert (off <= lim);
	int rem = (off <= lim ? lim - off : 0);

	int size = rem >> $LG_BYTES_PER_VALUE$;
 	if (!unaligned && ((address + off) % $BYTES_PER_VALUE$ != 0)) {
	    return (bigEndian
		    ? ($Type$Buffer)(new ByteBufferAs$Type$Buffer$RW$B(this,
								       -1,
								       0,
								       size,
								       size,
								       off))
		    : ($Type$Buffer)(new ByteBufferAs$Type$Buffer$RW$L(this,
								       -1,
								       0,
								       size,
								       size,
								       off)));
	} else {
	    return (nativeByteOrder
		    ? ($Type$Buffer)(new Direct$Type$Buffer$RW$U(this,
								 -1,
								 0,
								 size,
								 size,
								 off))
		    : ($Type$Buffer)(new Direct$Type$Buffer$RW$S(this,
								 -1,
								 0,
								 size,
								 size,
								 off)));
	}
    }

#end

}
