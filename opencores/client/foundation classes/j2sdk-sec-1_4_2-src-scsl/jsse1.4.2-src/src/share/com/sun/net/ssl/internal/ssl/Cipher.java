/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)Cipher.java	1.10 03/06/24
 *
 * Portions Copyright 2002 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


// Cipher - an encryption template
//
// Copyright (C) 1996 by Jef Poskanzer <jef@acme.com>.  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
// SUCH DAMAGE.
//
// Visit the ACME Labs Java page for up-to-date versions of this and other
// fine Java utilities: http://www.acme.com/java/

package com.sun.net.ssl.internal.ssl;



abstract class Cipher
{

    /** Constructor, records key size used by this cipher. */
    Cipher( int keySize )
	{
	this.keySize = keySize;
	}

    /** How big a key is.  Keyless ciphers use 0. */
    int keySize;

    /** Return how big a key is. */
    final int keySize()
	{
	return keySize;
	}

    /** Set the key from a block of bytes. */
    abstract void setKey( byte[] key );


    // Utility routines.

    /** Utility routine to fill memory with zeros. */
    static void zeroBlock (byte[] block, int off, int len)
    {
	for ( int i = off; i < off + len; ++i )
	    block[i] = 0;
    }

    /** Utility routine to XOR two memory blocks. */
    static void xorBlock(
	byte[] a, int aOff,
	byte[] b, int bOff,
	byte[] dst, int dstOff,
	int len )
    {
	for ( int i = 0; i < len; ++i )
	    dst[dstOff + i] = (byte) ( a[aOff + i] ^ b[bOff + i] );
    }

    /** Utility routine to copy one block of memory to another. */
    static void copyBlock(
	byte[] src, int srcOff,
	byte[] dst, int dstOff,
	int len )
    {
	for ( int i = 0; i < len; ++i )
	    dst[dstOff + i] = src[srcOff + i];
    }
}
