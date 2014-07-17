/*
 * @(#)WrapToken.java	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.jgss.krb5;

import org.ietf.jgss.*;
import sun.security.jgss.*;
import java.security.GeneralSecurityException;
import javax.crypto.Cipher;
import javax.crypto.CipherInputStream;
import javax.crypto.CipherOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.IOException;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import sun.security.krb5.Confounder;
import sun.security.krb5.KrbException;

/**
 * This class represents a token emitted by the GSSContext.wrap()
 * call. It is a MessageToken except that it also contains plaintext
 * or encrypted data at the end. A wrapToken has certain other rules
 * that are peculiar to it and different from a MICToken, which is
 * another type of MessageToken. All data in a WrapToken is prepended 
 * by a random counfounder of 8 bytes. All data in a WrapToken is
 * also padded with one to eight bytes where all bytes are equal in
 * value to the number of bytes being padded. Thus, all application
 * data is replaced by (confounder || data || padding).
 *
 * @author Mayank Upadhyay
 * @version 1.10, 01/23/03
 */
class WrapToken extends MessageToken {

    /**
     * The size of the random confounder used in a WrapToken.
     */
    public static int CONFOUNDER_SIZE = 8;
    
    /*
     * The padding used with a WrapToken. All data is padded to the
     * next multiple of 8 bytes, even if its length is already
     * multiple of 8.
     * Use this table as a quick way to obtain padding bytes by
     * indexing it with the number of padding bytes required.
     */
    private static byte[][] pads = {
	null, // No, no one escapes padding
	{0x01}, 
	{0x02, 0x02}, 
	{0x03, 0x03, 0x03}, 
	{0x04, 0x04, 0x04, 0x04}, 
	{0x05, 0x05, 0x05, 0x05, 0x05}, 
	{0x06,0x06, 0x06,0x06,0x06, 0x06}, 
	{0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07},
	{0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08}
    };
    
    /*
     * A token may come in either in an InputStream or as a
     * byte[]. Store a reference to it in either case and process
     * it's data only later when getData() is called and
     * decryption/copying is needed to be done. Note that JCE can
     * decrypt both from a byte[] and from an InputStream.
     */
    private boolean readTokenFromInputStream = true;
    private InputStream is = null;
    private byte[] tokenBytes = null;
    private int tokenOffset = 0;
    private int tokenLen = 0;
    
    /*
     * Application data may come from an InputStream or from a
     * byte[]. However, it will always be stored and processed as a
     * byte[] since
     * (a) the MessageDigest class only accepts a byte[] as input and
     * (b) It allows writing to an OuputStream via a CipherOutputStream.
     */
    private byte[] dataBytes = null;
    private int dataOffset = 0;
    private int dataLen = 0;
    
    // the len of the token data: (confounder || data || padding)
    private int dataSize = 0;

    private byte[] encKey = null;
    private byte[] confounder = null;
    private byte[] padding = null;
    
    private boolean privacy = false;
    
    /**
     * Constructs a WrapToken from token bytes obtained from the
     * peer.
     * @param context the mechanism context associated with this
     * token
     * @param tokenBytes the bytes of the token
     * @param tokenOffset the offset of the token
     * @param tokenLen the length of the token
     * @param prop the MessageProp into which characteristics of the
     * parsed token will be stored.
     * @throws GSSException if the token is defective
     */
    public WrapToken(Krb5Context context,
		     byte[] tokenBytes, int tokenOffset, int tokenLen,
		     MessageProp prop)  throws GSSException {

	// Just parse the MessageToken part first
	super(Krb5Token.WRAP_ID, context, 
	      tokenBytes, tokenOffset, tokenLen, prop);

	this.readTokenFromInputStream = false;

	// Will need the token bytes again when extracting data
	this.tokenBytes = tokenBytes;
	this.tokenOffset = tokenOffset;
	this.tokenLen = tokenLen;
	this.privacy = prop.getPrivacy();
	dataSize = 
	    getGSSHeader().getMechTokenLength() - super.TOKEN_SIZE;
    }
    
    /**
     * Constructs a WrapToken from token bytes read on the fly from
     * an InputStream.
     * @param context the mechanism context associated with this
     * token
     * @param is the InputStream containing the token bytes
     * @param prop the MessageProp into which characteristics of the
     * parsed token will be stored.
     * @throws GSSException if the token is defective or if there is
     * a problem reading from the InputStream
     */
    public WrapToken(Krb5Context context,
		     InputStream is, MessageProp prop)
	throws GSSException {

	// Just parse the MessageToken part first
	super(Krb5Token.WRAP_ID, context, is, prop);
	
	// Will need the token bytes again when extracting data	
	this.is = is;
	this.privacy = prop.getPrivacy();
	/*
	  debug("WrapToken Cons: gssHeader.getMechTokenLength=" +
	  getGSSHeader().getMechTokenLength());
	  debug("\n                super.TOKEN_SIZE=" 
	  + super.TOKEN_SIZE);
	*/
	dataSize = 
	    getGSSHeader().getMechTokenLength() - super.TOKEN_SIZE;
	//debug("\n                dataSize=" + dataSize);
	//debug("\n");
    }
    
    /**
     * Obtains the application data that was transmitted in this
     * WrapToken.
     * @return a byte array containing the application data
     * @throws GSSException if an error occurs while decrypting any
     * cipher text and checking for validity
     */
    public byte[] getData() throws GSSException {

	byte[] temp = new byte[dataSize];
	getData(temp, 0);

	// Remove the confounder and the padding
	byte[] retVal = new byte[dataSize - confounder.length -
				padding.length];
	System.arraycopy(temp, 0, retVal, 0, retVal.length);

	return retVal;
    }

    /**
     * Obtains the application data that was transmitted in this
     * WrapToken, writing it into an application provided output
     * array.
     * @param dataBuf the output buffer into which the data must be
     * written
     * @param dataBufOffset the offset at which to write the data
     * @return the size of the data written
     * @throws GSSException if an error occurs while decrypting any
     * cipher text and checking for validity
     */
    public int getData(byte[] dataBuf, int dataBufOffset) 
	throws GSSException {
	
	if (readTokenFromInputStream)
	    getDataFromStream(dataBuf, dataBufOffset);
	else
	    getDataFromBuffer(dataBuf, dataBufOffset);

	return (dataSize - confounder.length - padding.length);
    }

    /**
     * Helper routine to obtain the application data transmitted in
     * this WrapToken. It is called if the WrapToken was constructed
     * with a byte array as input.
     * @param dataBuf the output buffer into which the data must be
     * written
     * @param dataBufOffset the offset at which to write the data
     * @throws GSSException if an error occurs while decrypting any
     * cipher text and checking for validity
     */
    private void getDataFromBuffer(byte[] dataBuf, int dataBufOffset) 
	throws GSSException {
	
	GSSHeader gssHeader = getGSSHeader();
	int dataPos = tokenOffset + 
	    gssHeader.getLength() + super.TOKEN_SIZE;
	
	if (dataPos + dataSize > tokenOffset + tokenLen)
	    throw new GSSException(GSSException.DEFECTIVE_TOKEN, -1,
				   "Insufficient data in " 
				   + getTokenName(getTokenId()));
	
	//debug("WrapToken cons: data is token is [" + 
	//	      getHexBytes(tokenBytes, tokenOffset, tokenLen) + "]\n");
	
	confounder = new byte[CONFOUNDER_SIZE];
	
	
	// Do decryption if this token was privacy protected.

	if(privacy) {

	    encKey = getEncryptionKey(getContextKey());
	    desCbcDecrypt(encKey, tokenBytes, dataPos, dataSize, 
			  dataBuf, dataBufOffset);
	    
	    //debug("\t\tDecrypted data is [" + 
	    //  getHexBytes(confounder) + " " +
	    //  getHexBytes(dataBuf, dataBufOffset,
	    //	      dataSize - CONFOUNDER_SIZE - padding.length) +
	    //  getHexBytes(padding) +
	    //  "]\n");

	} else {

	    // Token data is in cleartext
	    //debug("\t\tNo encryption was performed by peer.\n");
	    System.arraycopy(tokenBytes, dataPos, 
			     confounder, 0, CONFOUNDER_SIZE);
	    int padSize = tokenBytes[dataPos + dataSize - 1];
	    if (padSize < 0)
		padSize = 0;
	    if (padSize > 8)
		padSize %= 8;

	    padding = pads[padSize];
	    //debug("\t\tPadding applied was: " + padSize + "\n");
	    System.arraycopy(tokenBytes, dataPos + CONFOUNDER_SIZE,
			     dataBuf, dataBufOffset, dataSize -
			     CONFOUNDER_SIZE - padSize);
	}
	
	/*
	 * Make sure sign and sequence number are not corrupt
	 */
	
	if (!verifySignAndSeqNumber(confounder, 
				    dataBuf, dataBufOffset, 
				    dataSize - CONFOUNDER_SIZE
				    - padding.length,
				    padding))
	    throw new GSSException(GSSException.BAD_MIC, -1, 
                         "Corrupt checksum or sequence number in Wrap token");
    }
    
    /**
     * Helper routine to obtain the application data transmitted in
     * this WrapToken. It is called if the WrapToken was constructed
     * with an Inputstream.
     * @param dataBuf the output buffer into which the data must be
     * written
     * @param dataBufOffset the offset at which to write the data
     * @throws GSSException if an error occurs while decrypting any
     * cipher text and checking for validity
     */
    private void getDataFromStream(byte[] dataBuf, int dataBufOffset) 
	throws GSSException {
	
	GSSHeader gssHeader = getGSSHeader();
	
	// Don't check the token length. Data will be read on demand from
	// the InputStream.
	
	//debug("WrapToken cons: data will be read from InputStream.\n");
	
	confounder = new byte[CONFOUNDER_SIZE];
	
	try {
	    
	    // Do decryption if this token was privacy protected.

	    if(privacy) {

		encKey = getEncryptionKey(getContextKey());
		desCbcDecrypt(encKey, is, dataSize,
			      dataBuf, dataBufOffset);
		
		//debug("\t\tDecrypted data is [" + 
		//     getHexBytes(confounder) + " " +
		//     getHexBytes(dataBuf, dataBufOffset, 
		//dataSize - CONFOUNDER_SIZE - padding.length) +
		//     getHexBytes(padding) +
		//     "]\n");

	    } else {

		// Token data is in cleartext
		//debug("\t\tNo encryption was performed by peer.\n");
		readFully(is, confounder);

		// Data is always a multiple of 8 with this GSS Mech
		// Copy all but last block as they are
		int numBlocks = (dataSize - CONFOUNDER_SIZE)/8 - 1;
		int offset = dataBufOffset;
		for (int i = 0; i < numBlocks; i++) {
		    readFully(is, dataBuf, offset, 8);
		    offset += 8;
		}

		byte[] finalBlock = new byte[8];
		readFully(is, finalBlock);

		int padSize = finalBlock[7];
		padding = pads[padSize];

		//debug("\t\tPadding applied was: " + padSize + "\n");
		System.arraycopy(finalBlock, 0, dataBuf, offset,
				 finalBlock.length - padSize);
	    }
	} catch (IOException e) {
	    throw new GSSException(GSSException.DEFECTIVE_TOKEN, -1, 
				   getTokenName(getTokenId()) 
				   + ": " + e.getMessage());
	}
	
	/*
	 * Make sure sign and sequence number are not corrupt
	 */
	
	if (!verifySignAndSeqNumber(confounder, 
				    dataBuf, dataBufOffset, 
				    dataSize - CONFOUNDER_SIZE 
				    - padding.length,
				    padding))
	    throw new GSSException(GSSException.BAD_MIC, -1, 
                         "Corrupt checksum or sequence number in Wrap token");
    }

    /**
     * Helper routine to decrypt fromm a byte array and write the
     * application data straight to an output array with minimal
     * buffer copies. The confounder and the padding are stored
     * separately and not copied into this output array.
     * @param key the DES key to use
     * @param cipherText the encrypted data
     * @param offset the offset for the encrypted data
     * @param len the length of the encrypted data
     * @param dataOutBuf the output buffer where the application data 
     * should be writte
     * @param dataOffset the offser where the application data should 
     * be written.
     * @throws GSSException is an error occurs while decrypting the
     * data
     */
    private void desCbcDecrypt(byte[] key, 
			       byte[] cipherText, int offset, int len, 
			       byte[] dataOutBuf, int dataOffset) 
	throws GSSException {
	
	try {
	    
	    int temp = 0;
	    
	    Cipher des = getInitializedDes(false, key, ZERO_IV);

	    /*
	     * Remove the counfounder first.
	     * CONFOUNDER_SIZE is one DES block ie 8 bytes.
	     */
	    temp = des.update(cipherText, offset, CONFOUNDER_SIZE,
			      confounder);
	    // temp should be CONFOUNDER_SIZE
	    //debug("\n\ttemp is " + temp + " and CONFOUNDER_SIZE is " 
	    //  + CONFOUNDER_SIZE);

	    offset += CONFOUNDER_SIZE;
	    len -= CONFOUNDER_SIZE;
      
	    /*
	     * len is a multiple of 8 due to padding.
	     * Decrypt all blocks directly into the output buffer except for
	     * the very last block. Remove the trailing padding bytes from the
	     * very last block and copy that into the output buffer.
	     */
      
	    int blockSize = des.getBlockSize();
	    int numBlocks = len / blockSize - 1;
	    
	    // Iterate over all but the last block
	    for (int i = 0; i < numBlocks; i++) {
		temp = des.update(cipherText, offset, blockSize,
				  dataOutBuf, dataOffset);
		// temp should be blockSize
		//debug("\n\ttemp is " + temp + " and blockSize is " 
		//    + blockSize);

		offset += blockSize;
		dataOffset += blockSize;
	    }
      
	    // Now process the last block
	    byte[] finalBlock = new byte[blockSize];
	    des.update(cipherText, offset, blockSize, finalBlock);

	    des.doFinal();
      
	    /*
	     * There is always at least one padding byte. The padding bytes
	     * are all the value of the number of padding bytes.
	     */

	    int padSize = finalBlock[blockSize - 1];
	    padding = pads[padSize];
	    blockSize -= padSize;
	    
	    // Copy this last block into the output buffer
	    System.arraycopy(finalBlock, 0, dataOutBuf, dataOffset,
			     blockSize);
	     
	} catch (GeneralSecurityException e) {
	    throw new GSSException(GSSException.FAILURE, -1, 
				   "Could not use DES cipher - " + 
				   e.getMessage());
	}
    }
    
   /**
     * Helper routine to decrypt from an InputStream and write the
     * application data straight to an output array with minimal
     * buffer copies. The confounder and the padding are stored
     * separately and not copied into this output array.
     * @param key the DES key to use
     * @param is the InputStream from which the cipher text should be 
     * read
     * @param len the length of the ciphertext data
     * @param dataOutBuf the output buffer where the application data 
     * should be writte
     * @param dataOffset the offser where the application data should 
     * be written.
     * @throws GSSException is an error occurs while decrypting the
     * data
     */
    private void desCbcDecrypt(byte[] key, InputStream is, int len,
			       byte[] dataOutBuf, int dataOffset) 
	throws GSSException, IOException {

	int temp = 0;
	
	Cipher des = getInitializedDes(false, key, ZERO_IV);
	
	WrapTokenInputStream truncatedInputStream = 
	    new WrapTokenInputStream(is, len);
	CipherInputStream cis = new CipherInputStream(truncatedInputStream,
						      des);
	/*
	 * Remove the counfounder first.
	 * CONFOUNDER_SIZE is one DES block ie 8 bytes.
	 */
	temp = cis.read(confounder);
	
	len -= temp;
	// temp should be CONFOUNDER_SIZE
	//debug("Got " + temp + " bytes; CONFOUNDER_SIZE is " 
	//     + CONFOUNDER_SIZE + "\n");
	//debug("Confounder is " + getHexBytes(confounder) + "\n");
	
	
	/*
	 * len is a multiple of 8 due to padding.
	 * Decrypt all blocks directly into the output buffer except for
	 * the very last block. Remove the trailing padding bytes from the
	 * very last block and copy that into the output buffer.
	 */
	
	int blockSize = des.getBlockSize();
	int numBlocks = len / blockSize - 1;
	
	// Iterate over all but the last block
	for (int i = 0; i < numBlocks; i++) {
	    //debug("dataOffset is " + dataOffset + "\n");
	    temp = cis.read(dataOutBuf, dataOffset, blockSize);
	    
	    // temp should be blockSize
	    //debug("Got " + temp + " bytes and blockSize is " 
	    //	  + blockSize + "\n");
	    //debug("Bytes are: " 
	    //	  + getHexBytes(dataOutBuf, dataOffset, temp) + "\n");
	    dataOffset += blockSize;
	}
	
	// Now process the last block
	byte[] finalBlock = new byte[blockSize];
	//debug("Will call read on finalBlock" + "\n");
	temp = cis.read(finalBlock);
	// temp should be blockSize
	/*
	  debug("Got " + temp + " bytes and blockSize is " 
	  + blockSize + "\n");
	  debug("Bytes are: " 
	  + getHexBytes(finalBlock, 0, temp) + "\n");
	  debug("Will call doFinal" + "\n");
	*/
	try {
	    des.doFinal();
	} catch (GeneralSecurityException e) {
	    throw new GSSException(GSSException.FAILURE, -1, 
				   "Could not use DES cipher - " + 
				   e.getMessage());
	}
	
	/*
	 * There is always at least one padding byte. The padding bytes
	 * are all the value of the number of padding bytes.
	 */
	
	int padSize = finalBlock[blockSize - 1];
	if (padSize < 1  || padSize > 8)
	    throw new GSSException(GSSException.DEFECTIVE_TOKEN, -1,
				   "Invalid padding on Wrap Token");
	padding = pads[padSize];
	blockSize -= padSize;
	
	// Copy this last block into the output buffer
	System.arraycopy(finalBlock, 0, dataOutBuf, dataOffset,
			 blockSize);
    }

    /**
     * Helper routine to pick the right padding for a certain length
     * of application data. Every application message has some
     * padding between 1 and 8 bytes.
     * @param len the length of the application data
     * @return the padding to be applied
     */
    private byte[] getPadding(int len) {
	int padSize = len % 8;
	padSize = 8 - padSize;
	return pads[padSize];
    }

    public WrapToken(Krb5Context context, MessageProp prop,
		     byte[] dataBytes, int dataOffset, int dataLen)
	throws GSSException {
    
	super(Krb5Token.WRAP_ID, context);

	try {
	    confounder = new Confounder().bytes(CONFOUNDER_SIZE);
	} catch (KrbException e) {
	    throw new GSSException(GSSException.FAILURE, -1, e.getMessage());
	}
	padding = getPadding(dataLen);
	dataSize = confounder.length + dataLen + padding.length;
	this.dataBytes = dataBytes;
	this.dataOffset = dataOffset;
	this.dataLen = dataLen;

	/*
	  debug("\nWrapToken cons: data to wrap is [" + 
	  getHexBytes(confounder) + " " +
	  getHexBytes(dataBytes, dataOffset, dataLen) + " " +
	  // padding is never null for Wrap
	  getHexBytes(padding) + "]\n");
	*/    
	genSignAndSeqNumber(prop, 
			    confounder, 
			    dataBytes, dataOffset, dataLen, 
			    padding);

	/*
	 * If the application decides to ask for privacy when the context
	 * did not negotiate for it, do not provide it. The peer might not
	 * have support for it. The app will realize this with a call to
	 * pop.getPrivacy() after wrap().
	 */
	if (!context.getConfState())
	    prop.setPrivacy(false);

	privacy = prop.getPrivacy();
    }

    public void encode(OutputStream os) throws IOException, GSSException {

	super.encode(os);
    
	//debug("Writing data: [");
	if (!privacy) {
	
	    //debug(getHexBytes(confounder, confounder.length));
	    os.write(confounder);
	    
	    //debug(" " + getHexBytes(dataBytes, dataOffset, dataLen));
	    os.write(dataBytes, dataOffset, dataLen);
	    
	    //debug(" " + getHexBytes(padding, padding.length));
	    os.write(padding);

	} else {

	    encKey = getEncryptionKey(getContextKey());
	    // Encrypt on the fly and write
	    Cipher des = getInitializedDes(true, encKey, ZERO_IV);
	    CipherOutputStream cos = new CipherOutputStream(os, des);
	    //debug(getHexBytes(confounder, confounder.length));
	    cos.write(confounder);
	    //debug(" " + getHexBytes(dataBytes, dataOffset, dataLen));
	    cos.write(dataBytes, dataOffset, dataLen);
	    //debug(" " + getHexBytes(padding, padding.length));
	    cos.write(padding);

	}
	//debug("]\n");
    }

    public byte[] encode() throws IOException, GSSException {
	// TBD: Fine tune this initial size
	ByteArrayOutputStream bos = new ByteArrayOutputStream(dataSize + 50);
	encode(bos);
	return bos.toByteArray();
    }

    public int encode(byte[] outToken, int offset)
	throws IOException, GSSException  {

	// Token header is small
	ByteArrayOutputStream bos = new ByteArrayOutputStream();
	super.encode(bos);
	byte[] header = bos.toByteArray();
	System.arraycopy(header, 0, outToken, offset, header.length);
	offset += header.length;

	//debug("WrapToken.encode: Writing data: [");
	if (!privacy) {
	
	    //debug(getHexBytes(confounder, confounder.length));
	    System.arraycopy(confounder, 0, outToken, offset,
			     confounder.length);
	    offset += confounder.length;

	    //debug(" " + getHexBytes(dataBytes, dataOffset, dataLen));
	    System.arraycopy(dataBytes, dataOffset, outToken, offset,
			     dataLen);
	    offset += dataLen;
	    
	    //debug(" " + getHexBytes(padding, padding.length));
	    System.arraycopy(padding, 0, outToken, offset, padding.length);

	} else {
	    int pos = offset; 
	    // Encrypt and write
	    encKey = getEncryptionKey(getContextKey());
	    Cipher des = getInitializedDes(true, encKey, ZERO_IV);
	    try {
		//debug(getHexBytes(confounder, confounder.length));
		pos += des.update(confounder, 0, confounder.length,
				  outToken, pos);
		//debug(" " + getHexBytes(dataBytes, dataOffset, dataLen));
		pos += des.update(dataBytes, dataOffset, dataLen, 
				  outToken, pos);
		//debug(" " + getHexBytes(padding, padding.length));
		des.update(padding, 0, padding.length, 
			   outToken, pos);
		des.doFinal();
	    } catch (GeneralSecurityException e) {
		throw new GSSException(GSSException.FAILURE, -1, 
				       "Could not use DES Cipher - " + 
				       e.getMessage());
	    }
	    //debug(getHexBytes(outToken, offset, dataSize));

	}

	//debug("]\n");
    
	return (header.length + confounder.length + dataLen + padding.length);

    }

    protected int getKrb5TokenSize() {
	return (super.getKrb5TokenSize() + dataSize);
    }

    protected int getSealAlg(boolean confRequested) {
	return (confRequested ? SEAL_DES_CBC : SEAL_NONE); 
    }

    private byte[] getEncryptionKey(byte[] key) 
	throws GSSException {

	/* 
	 * To meet export control requirements, double check that the 
	 * key being used is no longer than 64 bits.  
	 * 
	 * Note that from a protocol point of view, an 
	 * algorithm that is not DES will be rejected before this 
	 * point. Also, a DES key that is not 64 bits will be 
	 * rejected by a good JCE provider.
	 */ 
	if (key.length > 8)
	    throw new GSSException(GSSException.FAILURE, -100,
				   "Invalid DES Key!"); 

	byte[] retVal = new byte[key.length];
	for (int i = 0; i < key.length; i++)
	    retVal[i] = (byte)(key[i] ^ 0xf0);
	return retVal;
    }

    // This implementation is way to conservative. And it certainly
    // doesn't return the maximum limit.
    static int getSizeLimit(int qop, boolean confReq, int maxTokenSize) {
	return (GSSHeader.getMaxMechTokenSize(OID, maxTokenSize) - 
		(MessageToken.TOKEN_SIZE + CONFOUNDER_SIZE) - 8 /* safety*/);
    }

    /**
     * This class provides a truncated inputstream needed by WrapToken. The
     * truncated inputstream is passed to CipherInputStream. It prevents
     * the CipherInputStream from treating the bytes of the following token 
     * as part fo the ciphertext for this token.
     */
    class WrapTokenInputStream extends InputStream {

	private InputStream is;
	private int length;
	private int remaining;

	private int temp;

	public WrapTokenInputStream(InputStream is, int length) {
	    this.is = is;
	    this.length = length;
	    remaining = length;
	}

	public final int read() throws IOException {
	    if (remaining == 0)
		return -1;
	    else {
	        temp = is.read();
		if (temp != -1)
		    remaining -= temp;
		return temp;
	    }
	}

	public final int read(byte[] b) throws IOException {
	    if (remaining == 0)
		return -1;
	    else {
		temp = Math.min(remaining, b.length);
		temp = is.read(b, 0, temp);
		if (temp != -1)
		    remaining -= temp;
		return temp;
	    }
	}

	public final int read(byte[] b,
			      int off,
			      int len) throws IOException {
	    if (remaining == 0)
		return -1;
	    else {
		temp = Math.min(remaining, len);
		temp = is.read(b, off, temp);
		if (temp != -1)
		    remaining -= temp;
		return temp;
	    }
	}

	public final long skip(long n)  throws IOException {
	    if (remaining == 0)
		return 0;
	    else {
		temp = (int) Math.min(remaining, n);
		temp = (int) is.skip(temp);
		remaining -= temp;
		return temp;
	    }
	}

	public final int available() throws IOException {
	    return Math.min(remaining, is.available()); 
	}

	public final void close() throws IOException {
	    remaining = 0;
	}

	/*
	 * InputStream already returns false from marKSupported, does nothing 
	 * for mark and throws an IOException from reset.
	 */
    }

}
