/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)CbcBlockCipher.java	1.9 03/06/24
 *
 * Portions Copyright 2002 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


// CbcBlockCipher - use a block cipher in CBC mode
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


/**
 * Use a block cipher in CBC mode.
 *
 * <P>A plain old block cipher, key and cleartext-block in, ciphertext-block
 * out, is said to be in Electronic Code Book (ECB) mode.  A given block
 * of plaintext always encrypts to the same block of ciphertext.  This
 * makes it somewhat vulnerable to known plaintext attacks, block replay
 * attacks, etc.
 *
 * <P>A fairly cheap alternative is to use it in Cipher Block Chaining (CBC)
 * mode.  All this does is XOR each plaintext block with the previous
 * ciphertext block before encryption.  For the first block, where there
 * is no previous ciphertext block, a caller-specified Initialization
 * Vector (IV) is used for the XOR.  This makes each block's encryption
 * depend on all the previous blocks
 *
 * <P> This class lets you use any given block cipher in CBC mode.
 */
final class CbcBlockCipher extends BlockCipher
{
    private BlockCipher blockCipher;
    private byte[] iv;
    private byte[] temp;
    private byte[] temp2;


    // NOTE:  this won't do well with Fortezza, since the
    // init vector is not used to do the block chaining!!


    /** Constructor */
    CbcBlockCipher( BlockCipher blockCipher )
    {
	super( blockCipher.keySize(), blockCipher.blockSize() );
	this.blockCipher = blockCipher;
	iv = new byte[blockSize];
	zeroBlock( iv, 0, blockSize);
	temp = new byte[blockSize];
	temp2 = new byte[blockSize];
    }


    // Key routines.

    /** Set the key. */
    void setKey( byte[] key )
    {
	blockCipher.setKey( key );
    }


    // IV routines.

    /** Set the Initialization Vector. */
    void setIv( byte[] iv )
    {
	copyBlock( iv, 0, this.iv, 0, blockSize );
    }

    // Block encryption routines.

    /** Encrypt a block of bytes ... in-place encryption is supported. */
    void encrypt(
	byte[] clearText, int clearOff,
	byte[] cipherText, int cipherOff )
    {
	xorBlock( clearText, clearOff, iv, 0, temp, 0, blockSize );
	blockCipher.encrypt( temp, 0, cipherText, cipherOff );
	copyBlock( cipherText, cipherOff, iv, 0, blockSize );
    }

    /** Decrypt a block of bytes ... in-place decryption is supported */
    void decrypt(
	byte[] cipherText, int cipherOff,
	byte[] clearText, int clearOff )
    {
	blockCipher.decrypt( cipherText, cipherOff, temp, 0 );
	xorBlock( temp, 0, iv, 0, temp2, 0, blockSize );
	copyBlock( cipherText, cipherOff, iv, 0, blockSize );
	copyBlock( temp2, 0, clearText, clearOff, blockSize );
    }
}
