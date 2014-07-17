/*
 * @(#)MessageToken.java	1.12 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package sun.security.jgss.krb5;

import org.ietf.jgss.*;
import sun.security.jgss.*;
import sun.security.krb5.*;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.IOException;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.security.MessageDigest;
import java.security.GeneralSecurityException;
import java.security.NoSuchAlgorithmException;
import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;

/**
 * This class is a base class for other token definitions that pertain to
 * per-message GSS-API calls. Conceptually GSS-API has two types of
 * per-message tokens: WrapToken and MicToken. They differ in the respect
 * that a WrapToken carries additional plaintext or ciphertext application
 * data besides just the sequence number and checksum. This class
 * encapsulates the commonality in the structure of the WrapToken and the
 * MicToken. This structure can be represented as:
 * <p>
 * <pre>
 *     0..1           TOK_ID          Identification field.
 *                                    01 01 - Mic token
 *                                    02 01 - Wrap token
 *     2..3           SGN_ALG         Checksum algorithm indicator.
 *                                    00 00 - DES MAC MD5
 *                                    01 00 - MD2.5
 *                                    02 00 - DES MAC
 *     4..5           SEAL_ALG        ff ff - none
 *                                    00 00 - DES
 *     6..7           Filler          Contains ff ff
 *     8..15          SND_SEQ         Encrypted sequence number field.
 *     16..23         SGN_CKSUM       Checksum of plaintext padded data,
 *                                    calculated according to algorithm
 *                                    specified in SGN_ALG field.
 * </pre>
 * <p>
 * As always, this is preceeded by a GSSHeader.
 *
 * @author Mayank Upadhyay
 * @author Ram Marti 
 * @version 1.12, 01/23/03
 * @see sun.security.jgss.GSSHeader
 */

abstract class MessageToken extends Krb5Token {
    
    /**
     * A zero initial vector to be used for checksum calculation and for
     * DesCbc application data encryption/decryption.
     */
    public static final byte[] ZERO_IV = new byte[8];
    
    /**
     * The size of a MessageToken that is common to both the WrapToken and
     * the MicToken, exlcuding the GSSHeader that will be prepended.
     */
    public static final int TOKEN_SIZE = 24;

    /**
     * Filler data as defined in the specification of the Kerberos v5 GSS-API 
     * Mechanism.
     */
    public static final int FILLER = 0xffff;

    /**
     * A value for the SNG_ALG field that indicates a Des Mac MD5 checksum.
     */
    public static final int SIGN_DES_MAC_MD5 = 0x0000;

    /**
     * A value for the SNG_ALG field that indicates a Des Mac checksum.
     */
    public static final int SIGN_DES_MAC     = 0x0200;

    /**
     * A value for the SEAL_ALG field that indicates a Des Cbc encryption
     * algorithm.
     */
    public static final int SEAL_DES_CBC = 0x0000;

    /**
     * A value for the SEAL_ALG field that indicates that no encryption was
     * used.
     */
    public static final int SEAL_NONE    = 0xffff;

    private static final int TOKEN_ID_POS = 0;
    private static final int SIGN_ALG_POS = 2;
    private static final int SEAL_ALG_POS = 4;
  
    private byte[] contextKey = null;
    private int seqNumber;

    private boolean confState = true;
    private boolean initiator = true;

    private int tokenId = 0;
    private GSSHeader gssHeader = null;
    private MessageTokenHeader tokenHeader = null;
    private byte[] checksum = null;
    private byte[] encSeqNumber = null;
    private byte[] seqNumberData = null;

    /* DESCipher instance used by the corresponding GSSContext */
    private Cipher desCipher = null;


    /**
     * Constructs a MessageToken from a byte array. If there are more bytes
     * in the array than needed, the extra bytes are simply ignroed.
     *
     * @param tokenId the token id that should be contained in this token as
     * it is read.
     * @param context the Kerberos context associated with this token
     * @param tokenBytes the byte array containing the token
     * @param tokenOffset the offset where the token begins
     * @param tokenLen the length of the token
     * @param prop the MessageProp structure in which the properties of the
     * token should be stored.
     * @throws GSSException if there is a problem parsing the token
     */
    // TBD: List the specific exception codes that could come out of this
    MessageToken(int tokenId, Krb5Context context,
		 byte[] tokenBytes, int tokenOffset, int tokenLen,
		 MessageProp prop) throws GSSException {
	this(tokenId, context, 
	     new ByteArrayInputStream(tokenBytes, tokenOffset, tokenLen),
	     prop);
    }
    
    /**
     * Constructs a MessageToken from an InputStream. Bytes will be read on
     * demand and the thread might block if there are not enough bytes to
     * complete the token.
     *
     * @param tokenId the token id that should be contained in this token as
     * it is read.
     * @param context the Kerberos context associated with this token
     * @param is the InputStream from which to read
     * @param prop the MessageProp structure in which the properties of the
     * token should be stored.
     * @throws GSSException if there is a problem reading from the
     * InputStream or parsing the token
     */
    // TBD: List the specific exception codes that could come out of this
    MessageToken(int tokenId, Krb5Context context, InputStream is, 
		 MessageProp prop) throws GSSException {
	init(tokenId, context);

	try {
	    gssHeader = new GSSHeader(is);
      
	    if (!gssHeader.getOid().equals(OID)) {
		throw new GSSException(GSSException.DEFECTIVE_TOKEN, -1,
				       getTokenName(tokenId));
	    }
	    if (!confState) {
	        prop.setPrivacy(false);
	    }

	    tokenHeader = new MessageTokenHeader(is, prop);
      
	    encSeqNumber = new byte[8];
	    readFully(is, encSeqNumber);
	    // debug("\n\tEncSeq#=" + 
	    //		getHexBytes(encSeqNumber, encSeqNumber.length));
      
	    checksum = new byte[8];
	    readFully(is, checksum);

	    /*
	     * debug("\n\tChecksum=" + getHexBytes(checksum, checksum.length));
	     * debug("\nLeaving MessageToken.Cons\n");
	     */

	} catch (IOException e) {
	    throw new GSSException(GSSException.DEFECTIVE_TOKEN, -1, 
				   getTokenName(tokenId));
	}
    }

    /**
     * Used to obtain the GSSHeader that was at the start of this
     * token.
     */
    public final GSSHeader getGSSHeader() {
	return gssHeader;
    }

    /**
     * Used to obtain the token id that was contained in this token.
     * @return the token id in the token
     */
    public final int getTokenId() {
	return tokenId;
    }

    /**
     * Used to determine if this token contains any encrypted data.
     * @return true if it contains any encrypted data, false if there is only 
     * plaintext data or if there is no data.
     */
    public final boolean getConfState() {
	return confState;
    }

    /**
     * Generates the checksum field and the encrypted sequence number
     * field. The encrypted sequence number uses the 8 bytes fo the checksum
     * as an initial vector in a fixed DesCbc algorithm.
     *
     * @param prop the MessageProp structure that determines what sort of
     * checksum algorithm should be used. If the prop contains a qop of 3
     * then the Des Mac algorithm is used. For every other value, the Des
     * Cbc MD5 algorithm is used
     *
     * @param optionalHeader an optional header that will be processed first
     * during  checksum calculation
     *
     * @param data the application data to checksum
     * @param offset the offset where the data starts
     * @param len the length of the data
     *
     * @param optionalTrailer an optional trailer that will be processed
     * last during checksum calculation. e.g., padding that should be
     * appended to the application data
     *
     * @throws GSSException if an error occurs in the checksum calculation or
     * encryption sequence number calculation.
     */
    public void genSignAndSeqNumber(MessageProp prop, 
				    byte[] optionalHeader,
				    byte[] data, int offset, int len,
				    byte[] optionalTrailer)
	throws GSSException {

	//    debug("Inside MessageToken.genSignAndSeqNumber:\n");

	int qop = prop.getQOP();

	// Return correct values to the caller indicating what QOP was really
	// applied
	if (qop != 0 && qop != 2 && qop != 3) {
	    qop = 0;
	    prop.setQOP(qop);
	}

	if (!confState) {
	    prop.setPrivacy(false);
	}

	// Create a token header with the correct sign and seal algorithm
	// values.
	tokenHeader = 
	    new MessageTokenHeader(tokenId, prop.getPrivacy(), qop);
	
	// Calculate SGN_CKSUM
    
	checksum = 
	    getChecksum(optionalHeader, data, offset, len, optionalTrailer);
	
	// Calculate SND_SEQ
    
	seqNumberData = new byte[8];
	
	writeLittleEndian(seqNumber, seqNumberData);
	if (!initiator) {
	    seqNumberData[4] = (byte)0xff;
	    seqNumberData[5] = (byte)0xff;
	    seqNumberData[6] = (byte)0xff;
	    seqNumberData[7] = (byte)0xff;
	}
	
	try {
	    Cipher des = getInitializedDes(true, contextKey, checksum);
	    encSeqNumber = new byte[8];
	    des.update(seqNumberData, 0, 8, encSeqNumber, 0);
	    des.doFinal();
	} catch (GeneralSecurityException e) {
	    throw new GSSException(GSSException.FAILURE, -1, 
	   "Could not use DES Cipher while creating sequence number- " +
				   e.getMessage());
	}
    }

    /**
     * Verifies that the checksum field and sequence number direction bytes 
     * are valid and consistent with the application data.
     *
     * @param optionalHeader an optional header that will be processed first
     * during checksum calculation.
     *
     * @param data the application data
     * @param offset the offset where the data begins
     * @param len the length of the application data
     *
     * @param optionalTrailer an optional trailer that will be processed last 
     * during checksum calculation. e.g., padding that should be appended to
     * the application data
     *
     * @throws GSSException if an error occurs in the checksum calculation or
     * encryption sequence number calculation.
     */
    public final boolean verifySignAndSeqNumber(byte[] optionalHeader, 
			       		byte[] data, int offset, int len,
			       		byte[] optionalTrailer) 
	throws GSSException {
	//	debug("\tIn verifySign:\n");
	
	byte[] myChecksum = 
	    getChecksum(optionalHeader, data, offset, len, optionalTrailer);

	if (MessageDigest.isEqual(checksum, myChecksum)) {

	    seqNumberData = new byte[8];
	    try { 
		Cipher des = getInitializedDes(false, contextKey, checksum); 
		des.update(encSeqNumber, 0, 8, seqNumberData, 0); 
		des.doFinal(); 
	    } catch (GeneralSecurityException e) { 
		throw new GSSException(GSSException.FAILURE, -1,  
	        "Could not use DES Cipher while obtaining sequence number - "
				       + e.getMessage()); 
	    }

	    /*
	     * The token from the initiator has direction bytes 0x00 and
	     * the token from the acceptor has direction bytes 0xff.
	     */
	    byte directionByte = 0;
	    if (initiator)
		directionByte = (byte) 0xff; // Received token from acceptor
	    
	    if ((seqNumberData[4] == directionByte) &&
		  (seqNumberData[5] == directionByte) &&
		  (seqNumberData[6] == directionByte) &&
		  (seqNumberData[7] == directionByte))
		return true;
	}

	return false;

    }

    public final int getSequenceNumber() {
	return readLittleEndian(seqNumberData, 0, 4);
    }

    /**
     * Computes the checksum based on the algorithm stored in the
     * tokenHeader.
     *
     * @param optionalHeader an optional header that will be processed first
     * during checksum calculation.
     *
     * @param data the application data
     * @param offset the offset where the data begins
     * @param len the length of the application data
     *
     * @param optionalTrailer an optional trailer that will be processed last 
     * during checksum calculation. e.g., padding that should be appended to
     * the application data
     *
     * @throws GSSException if an error occurs in the checksum calculation.
     */
    private byte[] getChecksum(byte[] optionalHeader,
			       byte[] data, int offset, int len,
			       byte[] optionalTrailer) 
	throws GSSException {

	//      debug("Will do getChecksum:\n");
      
	/*
	 * For checksum calculation the token header bytes i.e., the first 8
	 * bytes following the GSSHeader, are logically prepended to the
	 * application data to bind the data to this particular token.
	 *
	 * Note: There is no such requirement wrt adding padding to the
	 * application data for checksumming, although the cryptographic
	 * algorithm used might itself apply some padding.
	 */

	byte[] tokenHeaderBytes = tokenHeader.getBytes();
	byte[] existingHeader = optionalHeader;
	byte[] checksumDataHeader = tokenHeaderBytes;
      
	if (existingHeader != null) {
	    checksumDataHeader = new byte[tokenHeaderBytes.length +
					 existingHeader.length];
	    System.arraycopy(tokenHeaderBytes, 0, 
			     checksumDataHeader, 0, tokenHeaderBytes.length);
	    System.arraycopy(existingHeader, 0,
			     checksumDataHeader, tokenHeaderBytes.length,
			     existingHeader.length);
	}
      
	if (tokenHeader.getSignAlg() == SIGN_DES_MAC_MD5) {

	    /*
	     * With this sign algorithm, first an MD5 hash is computed on the
	     * application data. The 16 byte hash is then DesCbc encrypted.
	     */

	    try {
		MessageDigest md5 = MessageDigest.getInstance("MD5");
          
		// debug("\t\tdata=[");
          
		// debug(getHexBytes(checksumDataHeader, 
	 	//			checksumDataHeader.length) + " ");
		md5.update(checksumDataHeader);
          
		// debug(getHexBytes(data, offset, len));
		md5.update(data, offset, len);
          
		if (optionalTrailer != null) {
		    // debug(" " + 
		    // 	     getHexBytes(optionalTrailer, 
		    // 			   optionalTrailer.length));
		    md5.update(optionalTrailer);
		}
		//          debug("]\n");
          
		data = md5.digest();
		offset = 0;
		len = data.length;
		//          System.out.println("\tMD5 Checksum is [" + 
		//                             getHexBytes(data) + "]\n");
		checksumDataHeader = null;
		optionalTrailer = null;
	    } catch (NoSuchAlgorithmException e) {
		throw new GSSException(GSSException.FAILURE, -1, 
				       "Could not get MD5 Message Digest - " 
				       + e.getMessage());
	    }
	}
      
	byte[] checksum = 
	    getDesCbcChecksum(contextKey, checksumDataHeader, 
				data, offset, len);
	byte[] retVal = new byte[8];
	System.arraycopy(checksum, checksum.length - 8, retVal, 0, 8);
      
	return retVal;
    }

    /**
     * Computes the DesCbc checksum based on the algorithm published in FIPS
     * Publication 113. This involves applying padding to the data passed
     * in, then performing DesCbc encryption on the data with a zero initial
     * vector, and finally returning the last 8 bytes of the encryption
     * result.
     *
     * @param key the bytes for the DES key
     * @param header a header to process first before the data is.
     * @param data the data to checksum
     * @param offset the offset where the data begins
     * @param len the length of the data
     * @throws GSSException when an error occuse in the encryption
     */
    private byte[] getDesCbcChecksum(byte key[],
				     byte[] header,
				     byte[] data, int offset, int len)
	throws GSSException {

	Cipher des = getInitializedDes(true, key, ZERO_IV);
        
	int blockSize = des.getBlockSize();
      
	/*
	 * Here the data need not be a multiple of the blocksize
	 * (8). Encrypt and throw away results for all blocks except for 
	 * the very last block.
	 */

	byte[] finalBlock = new byte[blockSize];
      
	int numBlocks = len / blockSize;
	int lastBytes = len % blockSize;
	if (lastBytes == 0) {
	    // No need for padding. Save last block from application data
	    numBlocks -= 1;
	    System.arraycopy(data, offset + numBlocks*blockSize, 
			     finalBlock, 0, blockSize);
	} else {
	    System.arraycopy(data, offset + numBlocks*blockSize,
			     finalBlock, 0, lastBytes);
	    // Zero padding automatically done
	}
      
	try {
        
	    // Iterate over all but the last block
        
	    byte[] temp = new byte[blockSize];

	    if (header != null) {
		// header will be null when doing DES-MD5 Checksum
		des.update(header, 0, header.length, temp, 0);
	    }
        
	    for (int i = 0; i < numBlocks; i++) {
		des.update(data, offset, blockSize,
			   temp, 0);
		offset += blockSize;
	    }
        
	    // Now process the final block
	    byte[] retVal = new byte[blockSize];
	    des.update(finalBlock, 0, blockSize, retVal, 0);
	    des.doFinal();
	    
	    return retVal;
	} catch (GeneralSecurityException e) {
	    throw new GSSException(GSSException.FAILURE, -1, 
				"Could not use DES Cipher - " + e.getMessage());
	}
    }
    
    /**
     * Obtains an initialized DES cipher.
     *
     * @param encryptMode true if encryption is desired, false is decryption
     * is desired.
     * @param key the bytes for the DES key
     * @param ivBytes the initial vector bytes
     */
    public final Cipher getInitializedDes(boolean encryptMode, byte[] key, 
					  byte[] ivBytes) 
	throws  GSSException  {
    
    
	try {
	    IvParameterSpec iv = new IvParameterSpec(ivBytes);
	    SecretKey jceKey = (SecretKey) (new SecretKeySpec(key, "DES")); 
	    // TBD: assert desCipher != null
	    if (desCipher == null) {
	        throw new GSSException(GSSException.FAILURE, -1, 
				"Internal Error:Uninitialized desCipher");
	    }
	    desCipher.init((encryptMode ? 
				Cipher.ENCRYPT_MODE : Cipher.DECRYPT_MODE),
		      	    jceKey, iv);
	    return desCipher;
	} catch (GeneralSecurityException e) {
	    throw new GSSException(GSSException.FAILURE, -1, e.getMessage());
	}
    }
  
    /**
     * Constructs an empty MessageToken for the local context to send to
     * the peer. It also increments the local sequence number in the
     * Krb5Context instance it uses after obtaining the object lock for
     * it.
     *
     * @param tokenId the token id that should be contained in this token
     * @param context the Kerberos context associated with this token
     */
    MessageToken(int tokenId, Krb5Context context) throws GSSException {
	/*
	  debug("\n============================");
	  debug("\nMySessionKey=" +
	  getHexBytes(context.getMySessionKey().getBytes()));
	  debug("\nPeerSessionKey=" + 
	  getHexBytes(context.getPeerSessionKey().getBytes()));
	  debug("\n============================\n");
	*/
	init(tokenId, context);
	this.seqNumber = context.incrementMySequenceNumber();
    }

    private void init(int tokenId, Krb5Context context) throws GSSException {
	this.tokenId = tokenId;
	this.contextKey = context.getKey().getBytes();
	// Just for consistency check in Wrap
	this.confState = context.getConfState();

	this.initiator = context.isInitiator();
	try {
	   this.desCipher = context.getDesCipher();
	} catch (GeneralSecurityException e) {
	    throw new GSSException(GSSException.FAILURE, -1, e.getMessage());
	}
	//    debug("In MessageToken.Cons");
    
    }

    /**
     * Encodes a GSSHeader and this token onto an OutputStream.
     *
     * @param os the OutputStream to which this should be written
     * @throws GSSException if an error occurs while writing to the OutputStream
     */
    public void encode(OutputStream os) throws IOException, GSSException {
	gssHeader = new GSSHeader(OID, getKrb5TokenSize());
	gssHeader.encode(os);
	tokenHeader.encode(os);
	// debug("Writing seqNumber: " + getHexBytes(encSeqNumber));
	os.write(encSeqNumber);
	// debug("Writing checksum: " + getHexBytes(checksum));
	os.write(checksum);
    }
    
    /**
     * Obtains the size of this token. Note that this excludes the size of
     * the GSSHeader.
     * @return TOKEN_SIZE
     */
    protected int getKrb5TokenSize() {
	return TOKEN_SIZE;
    }
    
    /**
     * Obtains the conext key that is associated with this token.
     * @return the context key
     */
    public final byte[] getContextKey() {
	return contextKey;
    }

    /**
     * Obtains the encryption algorithm that should be used in this token
     * given the state of confidentiality the application requested.
     * @param confRequested true if the application desired confidentiality
     * on this token, false otherwise
     */
    protected abstract int getSealAlg(boolean confRequested);
    
    // ******************************************* //
    //  I N N E R    C L A S S E S    F O L L O W
    // ******************************************* //
    
    /**
     * This inner class represents the initial portion of the message token
     * and contains information about the checksum and encryption algorithms
     * that are in use. It constitutes the first 8 bytes of the
     * message token:
     * <pre>
     *     0..1           TOK_ID          Identification field.
     *                                    01 01 - Mic token
     *                                    02 01 - Wrap token
     *     2..3           SGN_ALG         Checksum algorithm indicator.
     *                                    00 00 - DES MAC MD5
     *                                    01 00 - MD2.5
     *                                    02 00 - DES MAC
     *     4..5           SEAL_ALG        ff ff - none
     *                                    00 00 - DES
     *     6..7           Filler          Contains ff ff
     * </pre>
     */
    class MessageTokenHeader {
	
	private int tokenId = 0;
	
	private int signAlg = SIGN_DES_MAC;
	private int sealAlg = SEAL_DES_CBC;
	
	private byte[] bytes = new byte[8];
      
	/**
	 * Constructs a MessageTokenHeader for the specified token type with
	 * appropriate checksum and encryption algorithms fields.
	 *
	 * @param tokenId the token id for this mesage token
	 * @param conf true if confidentiality will be resuested with this
	 * message token, false otherwise.
	 * @param qop the value of the quality of protection that will be
	 * desired.
	 */
	public MessageTokenHeader(int tokenId, boolean conf, int qop) {

	    this.tokenId = tokenId;

	    switch (qop) {
	    case 0:
	    case 2:
	    default: // correct any erroneous values
		signAlg = SIGN_DES_MAC_MD5;
		break;
	    case 3:
		signAlg = SIGN_DES_MAC;
		break;
	    }

	    sealAlg = MessageToken.this.getSealAlg(conf);	

	    bytes[0] = (byte) (tokenId >>> 8);
	    bytes[1] = (byte) (tokenId);

	    bytes[2] = (byte) (signAlg >>> 8);
	    bytes[3] = (byte) (signAlg);

	    bytes[4] = (byte) (sealAlg >>> 8);
	    bytes[5] = (byte) (sealAlg);

	    bytes[6] = (byte) (MessageToken.FILLER >>> 8);
	    bytes[7] = (byte) (MessageToken.FILLER);
	}

	/**
	 * Constructs a MessageTokenHeader by reading it from an InputStream
	 * and sets the appropriate confidentiality and quality of protection 
	 * values in a MessageProp structure.
	 * 
	 * @param is the InputStream to read from
	 * @param prop the MessageProp to populate
	 * @throws IOException is an error occurs while reading from the
	 * InputStream
	 */
	public MessageTokenHeader(InputStream is, MessageProp prop) 
	    throws IOException {
	    readFully(is, bytes);
	    tokenId = readInt(bytes, TOKEN_ID_POS);
	    signAlg = readInt(bytes, SIGN_ALG_POS);
	    sealAlg = readInt(bytes, SEAL_ALG_POS);
	    //		debug("\nMessageTokenHeader read tokenId=" + 
	    //		      getHexBytes(bytes) + "\n");
	    // TBD: compare to FILLER
	    int temp = readInt(bytes, SEAL_ALG_POS + 2);
	    if (signAlg == SIGN_DES_MAC) {
		//		    debug("SIGN_ALG=" + signAlg + 
		// " which is SIGN_DES_MAC\n");
		prop.setQOP(3);
	    } else {
		//		    debug("SIGN_ALG=" + signAlg + 
		//			  " which is SIGN_DES_MAC_MD5\n");
		signAlg = SIGN_DES_MAC_MD5;
		prop.setQOP(2);
	    }
	    if (sealAlg == SEAL_DES_CBC) {
		prop.setPrivacy(true);
	    } else {
		prop.setPrivacy(false);
	    }
	}

	/**
	 * Encodes this MessageTokenHeader onto an OutputStream
	 * @param os the OutputStream to write to 
	 * @throws IOException is an error occurs while writing
	 */
	public final void encode(OutputStream os) throws IOException {
	    os.write(bytes);
	}
	    

	/**
	 * Returns the token id for the message token.
	 * @return the token id
	 * @see sun.security.jgss.krb5.Krb5Token#MIC_ID
	 * @see sun.security.jgss.krb5.Krb5Token#WRAP_ID
	 */
	public final int getTokenId() {
	    return tokenId;
	}
	   
	/**
	 * Returns the sign algorithm for the message token.
	 * @return the sign algorithm
	 * @see sun.security.jgss.krb5.MessageToken#SIGN_DES_MAC
	 * @see sun.security.jgss.krb5.MessageToken#SIGN_DES_MAC_MD5
	 */
	public final int getSignAlg() {
	    return signAlg;
	}
	    
	/**
	 * Returns the seal algorithm for the message token.
	 * @return the seal algorithm
	 * @see sun.security.jgss.krb5.MessageToken#SEAL_DES_CBC
	 * @see sun.security.jgss.krb5.MessageToken#SEAL_NONE
	 */
	public final int getSealAlg() {
	    return sealAlg;
	}
	    
	/**
	 * Returns the bytes of this header.
	 * @return 8 bytes that form this header
	 */
	public final byte[] getBytes() {
	    return bytes;
	}
	    
    } // end of class MessageTokenHeader
    
}
