/*
 * @(#)SignatureFile.java	1.18 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.util;

import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.security.cert.CertificateException;
import java.security.*;
import java.io.*;
import java.util.*;
import java.util.jar.*;
import java.math.BigInteger;

import sun.security.pkcs.*;
import sun.security.x509.*;

import sun.misc.BASE64Decoder;
import sun.misc.BASE64Encoder;
import sun.security.provider.*;

public class SignatureFile {

    /** SignatureFile */
    Manifest sf;

    /** .SF base name */
    String baseName;

    public SignatureFile(MessageDigest digests[],
			 Manifest mf,
			 ManifestDigester md,
			 String baseName,
			 boolean signManifest)

    {
	this.baseName = baseName;

	String version = System.getProperty("java.version");
	String javaVendor = System.getProperty("java.vendor");

	sf = new Manifest();
	Attributes mattr = sf.getMainAttributes();
	BASE64Encoder encoder = new BASE64Encoder();

	mattr.putValue(Attributes.Name.SIGNATURE_VERSION.toString(), "1.0");
	mattr.putValue("Created-By", version + " (" + javaVendor + ")");

	if (signManifest) {
	    // sign the whole manifest
	    for (int i=0; i < digests.length; i++) {
		mattr.putValue(digests[i].getAlgorithm()+"-Digest-Manifest",
			       encoder.encode(md.manifestDigest(digests[i])));
	    }
	}

	Map entries = sf.getEntries();

	/* go through the manifest entries and create the digests */
	Iterator mit = mf.getEntries().entrySet().iterator();
	while(mit.hasNext()) {
	    Map.Entry e = (Map.Entry) mit.next();
	    String name = (String)e.getKey();
	    ManifestDigester.Entry mde = md.get(name, false);
	    if (mde != null) {
		Attributes attr = new Attributes();
		for (int i=0; i < digests.length; i++) {
		    attr.putValue(digests[i].getAlgorithm()+"-Digest",
				  encoder.encode(mde.digest(digests[i])));
		}
		entries.put(name, attr);
	    }
	}
    }

    /**
     * Writes the SignatureFile to the specified OutputStream.
     *
     * @param out the output stream
     * @exception IOException if an I/O error has occurred
     */

    public void write(OutputStream out) throws IOException
    {
	sf.write(out);
    }

    /**
     * get .SF file name
     */
    public String getMetaName()
    {
	return "META-INF/"+ baseName + ".SF";
    }

    /**
     * get base file name
     */
    public String getBaseName()
    {
	return baseName;
    }

    public Block generateBlock(PrivateKey privateKey,
			       X509Certificate[] certChain,
			       boolean externalSF)
	throws NoSuchAlgorithmException, InvalidKeyException, IOException,
	    SignatureException, CertificateException
    {
	return new Block(this, privateKey, certChain, externalSF);
    }

    public static class Block {

	private PKCS7 block;
	private String blockFileName;

	Block(SignatureFile sfg,
	      PrivateKey privateKey,
	      X509Certificate[] certChain,
	      boolean externalSF)
	    throws NoSuchAlgorithmException, InvalidKeyException, IOException,
	    SignatureException, CertificateException
	{
	    Principal issuerName = certChain[0].getIssuerDN();
	    if (!(issuerName instanceof X500Name)) {
		// must extract the original encoded form of DN for subsequent
		// name comparison checks (converting to a String and back to
	        // an encoded DN could cause the types of String attribute 
                // values to be changed)
		X509CertInfo tbsCert = new 
		    X509CertInfo(certChain[0].getTBSCertificate());
		issuerName = (Principal) 
		    tbsCert.get(CertificateIssuerName.NAME + "." + 
				CertificateIssuerName.DN_NAME);
	    }
	    BigInteger serial = certChain[0].getSerialNumber();
	    String keyAlgorithm = privateKey.getAlgorithm();

	    String digestAlgorithm;
	    if (keyAlgorithm.equalsIgnoreCase("DSA"))
		digestAlgorithm = "SHA1";
	    else if (keyAlgorithm.equalsIgnoreCase("RSA"))
		digestAlgorithm = "MD5";
	    else {
		throw new RuntimeException("private key is not a DSA or "
					   + "RSA key");
	    }

	    String signatureAlgorithm = digestAlgorithm + "with" +
		keyAlgorithm;

	    blockFileName = "META-INF/"+sfg.getBaseName()+"."+keyAlgorithm;

	    AlgorithmId digestAlg = AlgorithmId.get(digestAlgorithm);
	    AlgorithmId sigAlg = AlgorithmId.get(signatureAlgorithm);
	    AlgorithmId digEncrAlg = AlgorithmId.get(keyAlgorithm);

	    Signature sig = Signature.getInstance(signatureAlgorithm);
	    sig.initSign(privateKey);

	    ByteArrayOutputStream baos = new ByteArrayOutputStream();
	    sfg.write(baos);

	    byte[] bytes = baos.toByteArray();
	    ContentInfo contentInfo;

	    if (externalSF) {
		contentInfo = new ContentInfo(ContentInfo.DATA_OID, null);
	    } else {
		contentInfo = new ContentInfo(bytes);
	    }

	    sig.update(bytes);
	    byte[] signature = sig.sign();

	    SignerInfo signerInfo = new SignerInfo((X500Name)issuerName, serial,
						   digestAlg, digEncrAlg,
						   signature);
	    AlgorithmId[] algs = {digestAlg};
	    SignerInfo[] infos = {signerInfo};

	    block = new PKCS7(algs, contentInfo, certChain, infos);
	}

	/*
	 * get block file name.
	 */
	public String getMetaName()
	{
	    return blockFileName;
	}

	/**
	 * Writes the block file to the specified OutputStream.
	 *
	 * @param out the output stream
	 * @exception IOException if an I/O error has occurred
	 */

	public void write(OutputStream out) throws IOException
	{
	    block.encodeSignedData(out);
	}
    }
}
