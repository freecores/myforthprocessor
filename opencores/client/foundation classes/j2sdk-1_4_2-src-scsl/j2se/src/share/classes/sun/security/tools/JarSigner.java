/*
 * @(#)JarSigner.java	1.52 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.tools;

import java.io.*;
import java.util.*;
import java.util.zip.*;
import java.util.jar.*;
import java.math.BigInteger;
import java.net.URL;
import java.text.Collator;
import java.text.MessageFormat;
import java.security.cert.Certificate;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.security.cert.CertificateException;
import java.security.*;
import java.security.interfaces.*;

import sun.net.www.MessageHeader;
import sun.security.x509.*;
import sun.security.pkcs.*;
import sun.security.util.*;
import sun.misc.BASE64Encoder;

/**
 * <p>The jarsigner utility.
 *
 * @version 1.52 01/23/03
 * @author Roland Schemers
 * @author Jan Luehe
 */

public class JarSigner {

    // for i18n
    private static final java.util.ResourceBundle rb =
	java.util.ResourceBundle.getBundle
	("sun.security.tools.JarSignerResources");
    private static final Collator collator = Collator.getInstance();
    static {
	// this is for case insensitive string comparisions
	collator.setStrength(Collator.PRIMARY);
    }

    public static void main(String args[]) throws Exception {
	JarSigner js = new JarSigner();
	js.run(args);
    }

    static final String VERSION = "1.0";

    static final int IN_KEYSTORE = 0x01;
    static final int IN_SCOPE = 0x02;

    // signer's certificate chain (when composing)
    X509Certificate[] certChain;

    /*
     * private key
     */
    PrivateKey privateKey;
    KeyStore store;

    IdentityScope scope;

    String keystore; // key store file
    String jarfile;  // jar file to sign
    String alias;    // alias to sign jar with
    char[] storepass; // keystore password
    String storetype; // keystore type
    Vector providers = null; // list of providers
    char[] keypass; // private key password
    String sigfile; // name of .SF file
    String signedjar; // output filename
    boolean verify = false; // verify the jar
    boolean verbose = false; // verbose output when signing/verifying
    boolean showcerts = false; // show certs when verifying
    boolean debug = false; // debug
    boolean signManifest = true; // "sign" the whole manifest
    boolean externalSF = true; // leave the .SF out of the PKCS7 block

    // read zip entry raw bytes
    private ByteArrayOutputStream baos = new ByteArrayOutputStream(2048);
    private byte[] buffer = new byte[8192];

    public void run(String args[]) {
	try {
	    parseArgs(args);

	    // Try to load and install the specified providers
	    if (providers != null) {
		ClassLoader cl = ClassLoader.getSystemClassLoader();
		Enumeration e = providers.elements();
		while (e.hasMoreElements()) {
		    String provName = (String)e.nextElement();
		    Class provClass;
		    if (cl != null) {
			provClass = cl.loadClass(provName);
		    } else {
			provClass = Class.forName(provName);
		    }
		    Object obj = provClass.newInstance();
		    if (!(obj instanceof Provider)) {
			MessageFormat form = new MessageFormat(rb.getString
			    ("provName not a provider"));
			Object[] source = {provName};
			throw new Exception(form.format(source));
		    }
		    Security.addProvider((Provider)obj);
		}
	    }

	    if (verify) {
		try {
		    loadKeyStore(keystore, false);
		    scope = IdentityScope.getSystemScope();
		} catch (Exception e) {
		    if ((keystore != null) || (storepass != null)) {
			System.out.println(rb.getString("jarsigner error: ") +
					e.getMessage());
			System.exit(1);
		    }
		}
		/*		if (debug) {
		    SignatureFileVerifier.setDebug(true);
		    ManifestEntryVerifier.setDebug(true);
		}
		*/
		verifyJar(jarfile);
	    } else {
		loadKeyStore(keystore, true);
		getAliasInfo(alias);
		signJar(jarfile, alias);
	    }
	} catch (Exception e) {
	    System.out.println(rb.getString("jarsigner error: ") + e);
	    if (debug) {
		e.printStackTrace();
	    }
	    System.exit(1);
 	} finally {
	    // zero-out private key password
	    if (keypass != null) {
		Arrays.fill(keypass, ' ');
		keypass = null;
	    }
	    // zero-out keystore password
	    if (storepass != null) {
		Arrays.fill(storepass, ' ');
		storepass = null;
	    }
	}
    }

    /*
     * Parse command line arguments.
     */
    void parseArgs(String args[]) {
	/* parse flags */
	int n = 0;

	for (n=0; (n < args.length) && args[n].startsWith("-"); n++) {

	    String flags = args[n];

	    if (collator.compare(flags, "-keystore") == 0) {
		if (++n == args.length) usage();
		keystore = args[n];
	    } else if (collator.compare(flags, "-storepass") ==0) {
		if (++n == args.length) usage();
		storepass = args[n].toCharArray();
	    } else if (collator.compare(flags, "-storetype") ==0) {
		if (++n == args.length) usage();
		storetype = args[n];
	    } else if (collator.compare(flags, "-provider") ==0) {
		if (++n == args.length) usage();
		if (providers == null) {
		    providers = new Vector(3);
		}
		providers.add(args[n]);
	    } else if (collator.compare(flags, "-debug") ==0) {
		debug = true;
	    } else if (collator.compare(flags, "-keypass") ==0) {
		if (++n == args.length) usage();
		keypass = args[n].toCharArray();
	    } else if (collator.compare(flags, "-sigfile") ==0) {
		if (++n == args.length) usage();
		sigfile = args[n];
	    } else if (collator.compare(flags, "-signedjar") ==0) {
		if (++n == args.length) usage();
		signedjar = args[n];
	    } else if (collator.compare(flags, "-sectionsonly") ==0) {
		signManifest = false;
	    } else if (collator.compare(flags, "-internalsf") ==0) {
		externalSF = false;
	    } else if (collator.compare(flags, "-verify") ==0) {
		verify = true;
	    } else if (collator.compare(flags, "-verbose") ==0) {
		verbose = true;
	    } else if (collator.compare(flags, "-certs") ==0) {
		showcerts = true;
	    } else if (collator.compare(flags, "-h") == 0 ||
			collator.compare(flags, "-help") == 0) {
		usage();
	    } else {
		System.err.println(rb.getString("Illegal option: ") + flags);
		usage();
	    }
	}

	if (n == args.length) usage();
	jarfile = args[n++];

	if (!verify) {
	    if (n == args.length) usage();
	    alias = args[n++];
	}
    }

    void usage() {
	System.out.println(rb.getString
		("Usage: jarsigner [options] jar-file alias"));
	System.out.println(rb.getString
		("       jarsigner -verify [options] jar-file"));
	System.out.println();
	System.out.println(rb.getString
		("  [-keystore <url>]           keystore location"));
	System.out.println();
	System.out.println(rb.getString
		("  [-storepass <password>]     password for keystore integrity"));
	System.out.println();
	System.out.println(rb.getString
		("  [-storetype <type>]         keystore type"));
	System.out.println();
	System.out.println(rb.getString
		("  [-keypass <password>]       password for private key (if different)"));
	System.out.println();
	System.out.println(rb.getString
		("  [-sigfile <file>]           name of .SF/.DSA file"));
	System.out.println();
	System.out.println(rb.getString
		("  [-signedjar <file>]         name of signed JAR file"));
	System.out.println();
	System.out.println(rb.getString
		("  [-verify]                   verify a signed JAR file"));
	System.out.println();
	System.out.println(rb.getString
		("  [-verbose]                  verbose output when signing/verifying"));
	System.out.println();
	System.out.println(rb.getString
		("  [-certs]                    display certificates when verbose and verifying"));
	System.out.println();
	System.out.println(rb.getString
		("  [-internalsf]               include the .SF file inside the signature block"));
	System.out.println();
	System.out.println(rb.getString
		("  [-sectionsonly]             don't compute hash of entire manifest"));
	System.out.println();
	System.out.println(rb.getString
		("  [-provider]                 name of cryptographic service provider's master class file"));
	System.out.println("  ...");

	System.out.println();

	System.exit(1);
    }

    void verifyJar(String jarName)
	throws Exception
    {
	boolean anySigned = false;
	boolean hasUnsignedEntry = false;
	try {

	    JarFile jf = new JarFile(jarName, true);

	    Vector entriesVec = new Vector();
	    byte[] buffer = new byte[8192];

	    Enumeration entries = jf.entries();
	    while (entries.hasMoreElements()) {
		JarEntry je = (JarEntry)entries.nextElement();
		entriesVec.addElement(je);
		InputStream is = jf.getInputStream(je);
		int n;
		while ((n = is.read(buffer, 0, buffer.length)) != -1) {
		    // we just read. this will throw a SecurityException
		    // if  a signature/digest check fails.
		}
		is.close();
	    }
	    jf.close();
	    Manifest man = jf.getManifest();
	    if (man != null) {
		if (verbose) System.out.println();
		Enumeration e = entriesVec.elements();
		while (e.hasMoreElements()) {
		    JarEntry je = (JarEntry) e.nextElement();
		    String name = je.getName();
		    Certificate[] certs = je.getCertificates();
		    boolean isSigned = ((certs != null) && (certs.length > 0));
		    anySigned |= isSigned;
		    hasUnsignedEntry |= ((!isSigned) &&
					 (!name.startsWith("META-INF/")));
		   
		    if (verbose) {
			int inStoreOrScope = inKeyStore(certs);
			boolean inStore = (inStoreOrScope & IN_KEYSTORE) != 0;
			boolean inScope = (inStoreOrScope & IN_SCOPE) != 0;
			boolean inManifest =
			    ((man.getAttributes(name) != null) ||
			     (man.getAttributes("./"+name) != null) ||
			     (man.getAttributes("/"+name) != null));
			System.out.print(
			  (isSigned ? rb.getString("s") : rb.getString(" ")) +
			  (inManifest ? rb.getString("m") : rb.getString(" ")) +
			  (inStore ? rb.getString("k") : rb.getString(" ")) +
			  (inScope ? rb.getString("i") : rb.getString(" ")) +
			  rb.getString("  "));
			StringBuffer sb = new StringBuffer();
			String s = Long.toString(je.getSize());
			for (int i = 6 - s.length(); i > 0; --i) {
			    sb.append(' ');
			}
			sb.append(s).append(' ').
			            append(new Date(je.getTime()).toString());
			sb.append(' ').append(je.getName());
			System.out.println(sb.toString());
			if (certs != null && showcerts) {
			    System.out.println();
			    for (int i=0; i<certs.length; i++) {
			    	System.out.println(rb.getString("      ") +
				printCert(certs[i]));
			    }
			    System.out.println();
			}
		    }
		}
	    }
	    if (verbose) {
		System.out.println();
		System.out.println("  s = signature was verified ");
		System.out.println("  m = entry is listed in manifest");
		System.out.println("  k = at least one certificate was found in keystore");
		System.out.println("  i = at least one certificate was found in identity scope");
		System.out.println();
	    }

	    if (man == null)
		System.out.println("no manifest.");

	    if (!anySigned) {
		System.out.println(
                      "jar is unsigned. (signatures missing or not parsable)");
	    } else {
		System.out.println("jar verified.");
		if (hasUnsignedEntry) {
		    System.out.println();
		    System.out.println("Note: This jar contains unsigned " +
		        "entries which are not integrity-checked. " + 
			"Re-run with -verbose to list unsigned entries.");
		}
	    }
	    System.exit(0);
	} catch (Exception e) {
	    System.out.println("jarsigner: "+e);
	    if (debug) {
		e.printStackTrace();
	    }
	    System.exit(1);
	}
	System.exit(1);
    }

    String printCert(Certificate c)
    {
	String certStr;
	if (c instanceof X509Certificate) {
	    certStr = c.getType()+ rb.getString(", ") +
		((X509Certificate) c).getSubjectDN().getName();
	} else {
	    certStr = c.getType();
	}

	String alias = (String) storeHash.get(c);
	if (alias != null) {
	    return certStr + rb.getString(" ") + alias;
	} else {
	    return certStr;
	}
    }

    Hashtable storeHash = new Hashtable();

    int inKeyStore(Certificate[] certs) {
	int result = 0;

	if (certs == null)
	    return 0;

	boolean found = false;
	boolean foundOne = false;

	for (int i=0; i<certs.length; i++) {
	    Certificate c = certs[i];
	    found = false;
	    String alias = (String) storeHash.get(c);

	    if (alias != null) {
		if (alias.startsWith("("))
			result |= IN_KEYSTORE;
		else if (alias.startsWith("["))
			result |= IN_SCOPE;
	    } else {
		if (store != null) {
		    try {
			alias = store.getCertificateAlias(c);
		    } catch (KeyStoreException kse) {
			// this never happens, because keystore has been loaded
		    }
		    if (alias != null) {
			storeHash.put(c, "("+alias+")");
			found = true;
			result |= IN_KEYSTORE;
		    }
		}
		if (!found && (scope != null)) {
		    Identity id = scope.getIdentity(c.getPublicKey());
		    if (id != null) {
			result |= IN_SCOPE;
			storeHash.put(c, "["+id.getName()+"]");
		    }
		}
	    }
	}
	return result;
    }

    void signJar(String jarName, String alias) throws Exception {
	boolean aliasUsed = false;

	if (sigfile == null) {
	    sigfile = alias;
	    aliasUsed = true;
	}

	if (sigfile.length() > 8) {
	    sigfile = sigfile.substring(0, 8).toUpperCase();
	} else {
	    sigfile = sigfile.toUpperCase();
	}

	StringBuffer tmpSigFile = new StringBuffer(sigfile.length());
	for (int j = 0; j < sigfile.length(); j++) {
	    char c = sigfile.charAt(j);
	    if (!
		((c>= 'A' && c<= 'Z') ||
		(c>= '0' && c<= '9') ||
		(c == '-') ||
		(c == '_'))) {
		if (aliasUsed) {
		    // convert illegal characters from the alias to be _'s
		    c = '_';
		} else {
		 throw new
                   RuntimeException(rb.getString
			("signature filename must consist of the following characters: A-Z, 0-9, _ or -"));
		}
	    }
	    tmpSigFile.append(c);
	}

	sigfile = tmpSigFile.toString();

	String tmpJarName;
	if (signedjar == null) tmpJarName = jarName+".sig";
	else tmpJarName = signedjar;

	File jarFile = new File(jarName);
	File signedJarFile = new File(tmpJarName);

	// Open the jar (zip) file
	ZipFile zipFile = null;
	try {
	    zipFile = new ZipFile(jarName);
	} catch (IOException ioe) {
	    error(rb.getString("unable to open jar file: ")+jarName, ioe);
	}

	FileOutputStream fos = null;
	try {
	    fos = new FileOutputStream(signedJarFile);
	} catch (IOException ioe) {
	    error(rb.getString("unable to create: ")+tmpJarName, ioe);
	}

	PrintStream ps = new PrintStream(fos);
	ZipOutputStream zos = new ZipOutputStream(ps);

	/* First guess at what they might be - we don't xclude RSA ones. */
	String sfFilename = ("META-INF/" + sigfile + ".SF").toUpperCase();
	String bkFilename = ("META-INF/" + sigfile + ".DSA").toUpperCase();

	Manifest manifest = new Manifest();
	Map mfEntries = manifest.getEntries();

	boolean mfModified = false;
	boolean mfCreated = false;
	byte[] mfRawBytes = null;

	try {
	    // For now, hard-code the message digest algorithm to SHA-1
	    MessageDigest digests[] = { MessageDigest.getInstance("SHA1") };

	    // Check if manifest exists
	    ZipEntry mfFile;
	    if ((mfFile = getManifestFile(zipFile)) != null) {
		// Manifest exists. Read its raw bytes.
		mfRawBytes = getBytes(zipFile, mfFile);
		manifest.read(new ByteArrayInputStream(mfRawBytes));
	    } else {
		// Create new manifest
		Attributes mattr = manifest.getMainAttributes();
		mattr.putValue(Attributes.Name.MANIFEST_VERSION.toString(),
			       "1.0");
		String javaVendor = System.getProperty("java.vendor");
		String jdkVersion = System.getProperty("java.version");
		mattr.putValue("Created-By", jdkVersion + " (" +javaVendor
			       + ")");
		mfFile = new ZipEntry(JarFile.MANIFEST_NAME);
		mfCreated = true;
	    }

	    /*
	     * For each entry in jar (except for the META-INF entries), do the
	     * following:
	     *
	     * - if entry is not contained in manifest, add it to manifest;
	     * - if entry is contained in manifest, calculate its hash and
	     *   compare it with the one in the manifest; if they are
	     *   different, replace the hash in the manifest with the newly
	     *   generated one. (This may invalidate existing signatures!)
	     */
	    BASE64Encoder encoder = new BASE64Encoder();
	    Vector mfFiles = new Vector();

	    for (Enumeration enum=zipFile.entries(); enum.hasMoreElements();) {
		ZipEntry ze = (ZipEntry)enum.nextElement();

		if (ze.getName().startsWith("META-INF/")) {
		    // Store META-INF files in vector, so they can be written
		    // out first
		    mfFiles.addElement(ze);
		} else if (manifest.getAttributes(ze.getName()) != null) {
		    // jar entry is contained in manifest, check and
		    // possibly update its digest attributes
		    if (updateDigests(ze, zipFile, digests, encoder,
				      manifest) == true) {
			mfModified = true;
		    }
		} else if (!ze.isDirectory()) {
		    // Add entry to manifest
		    Attributes attrs = getDigestAttributes(ze, zipFile,
							   digests,
							   encoder);
		    mfEntries.put(ze.getName(), attrs);
		    mfModified = true;
		}
	    }

	    // Recalculate the manifest raw bytes if necessary
	    if (mfModified) {
		ByteArrayOutputStream baos = new ByteArrayOutputStream();
		manifest.write(baos);
		mfRawBytes = baos.toByteArray();
	    }

	    // Write out the manifest
	    if (mfModified) {
		// manifest file has new length
		mfFile = new ZipEntry(JarFile.MANIFEST_NAME);
	    }
	    if (verbose) {
		if (mfCreated) {
		    System.out.println(rb.getString("   adding: ") +
					mfFile.getName());
		} else if (mfModified) {
		    System.out.println(rb.getString(" updating: ") +
					mfFile.getName());
		}
	    }
	    zos.putNextEntry(mfFile);
	    zos.write(mfRawBytes);

	    // Calculate SignatureFile (".SF") and SignatureBlockFile
	    ManifestDigester manDig = new ManifestDigester(mfRawBytes);
	    SignatureFile sf = new SignatureFile(digests, manifest, manDig,
						 sigfile, signManifest);
	    SignatureFile.Block  block = sf.generateBlock(privateKey,
							  certChain,
							  externalSF);

	    sfFilename = sf.getMetaName();
	    bkFilename = block.getMetaName();

	    ZipEntry sfFile = new ZipEntry(sfFilename);
	    ZipEntry bkFile = new ZipEntry(bkFilename);

	    long time = System.currentTimeMillis();
	    sfFile.setTime(time);
	    bkFile.setTime(time);

	    zos.putNextEntry(sfFile);
	    sf.write(zos);
	    if (verbose) {
		if (zipFile.getEntry(sfFilename) != null) {
		    System.out.println(rb.getString(" updating: ") +
				sfFilename);
		} else {
		    System.out.println(rb.getString("   adding: ") +
				sfFilename);
		}
	    }

	    zos.putNextEntry(bkFile);
	    block.write(zos);
	    if (verbose) {
		if (zipFile.getEntry(bkFilename) != null) {
		    System.out.println(rb.getString(" updating: ") +
				bkFilename);
		} else {
		    System.out.println(rb.getString("   adding: ") +
				bkFilename);
		}
	    }

	    // Write out all other META-INF files that we stored in the
	    // vector
	    for (int i=0; i<mfFiles.size(); i++) {
		ZipEntry ze = (ZipEntry)mfFiles.elementAt(i);
		if (!ze.getName().equalsIgnoreCase(JarFile.MANIFEST_NAME)
		    && !ze.getName().equalsIgnoreCase(sfFilename)
		    && !ze.getName().equalsIgnoreCase(bkFilename)) {
		    writeEntry(zipFile, zos, ze);
		}
	    }

	    // Write out all other files
	    for (Enumeration enum=zipFile.entries(); enum.hasMoreElements();) {
		ZipEntry ze = (ZipEntry)enum.nextElement();

		if (!ze.getName().startsWith("META-INF")) {
		    if (verbose) {
			if (manifest.getAttributes(ze.getName()) != null)
			  System.out.println(rb.getString("  signing: ") +
				ze.getName());
			else
			  System.out.println(rb.getString("   adding: ") +
				ze.getName());
		    }
		    writeEntry(zipFile, zos, ze);
		}
	    }

	    zipFile.close();
	    zos.close();

	    if (signedjar == null) {
		// attempt an atomic rename. If that fails,
		// rename the original jar file, then the signed
		// one, then delete the original.
		if (!signedJarFile.renameTo(jarFile)) {
		    File origJar = new File(jarName+".orig");

		    if (jarFile.renameTo(origJar)) {
			if (signedJarFile.renameTo(jarFile)) {
			    origJar.delete();
			} else {
			    MessageFormat form = new MessageFormat(rb.getString
			("attempt to rename signedJarFile to jarFile failed"));
			    Object[] source = {signedJarFile, jarFile};
			    error(form.format(source));
			}
		    } else {
			MessageFormat form = new MessageFormat(rb.getString
			    ("attempt to rename jarFile to origJar failed"));
			Object[] source = {jarFile, origJar};
			error(form.format(source));
		    }
		}
	    }
	} catch(IOException ioe) {
	    error(rb.getString("unable to sign jar: ")+ioe, ioe);
	}
    }

    private void writeEntry(ZipFile zf, ZipOutputStream os, ZipEntry ze)
    throws IOException
    {
	byte[] data = getBytes(zf, ze);
        ZipEntry ze2 = new ZipEntry(ze.getName());
        ze2.setMethod(ze.getMethod());
        ze2.setTime(ze.getTime());
        ze2.setComment(ze.getComment());
        ze2.setExtra(ze.getExtra());
        if (ze.getMethod() == ZipEntry.STORED) {
            ze2.setSize(ze.getSize());
            ze2.setCrc(ze.getCrc());
        }
	os.putNextEntry(ze2);
	if (data.length > 0) {
	    os.write(data);
	}
    }

    void loadKeyStore(String keyStoreName, boolean prompt) {

	if (keyStoreName == null) {
	    keyStoreName = System.getProperty("user.home") + File.separator
		+ ".keystore";
	}

	try {
	    if (storetype != null)
		store = KeyStore.getInstance(storetype);
	    else
		store = KeyStore.getInstance(KeyStore.getDefaultType());

	    // Get pass phrase
            // XXX need to disable echo; on UNIX, call getpass(char *prompt)Z
            // and on NT call ??
	    if ((storepass == null) && prompt) {
		storepass = getPass
			(rb.getString("Enter Passphrase for keystore: "));
	    }

	    keyStoreName = keyStoreName.replace(File.separatorChar, '/');
	    URL url = null;
	    try {
		url = new URL(keyStoreName);
	    } catch (java.net.MalformedURLException e) {
		// try as file
		File kfile = new File(keyStoreName);
		url = new URL("file:" + kfile.getCanonicalPath());
	    }
	    InputStream is = url.openStream();
	    store.load(is, storepass);
	    is.close();
	} catch (IOException ioe) {
	    throw new RuntimeException(rb.getString("keystore load: ") +
					ioe.getMessage());
	} catch (java.security.cert.CertificateException e) {
	  throw new RuntimeException(rb.getString("certificate exception: ") +
					e.getMessage());
	} catch (NoSuchAlgorithmException e) {
	  throw new RuntimeException(e.getMessage());
	} catch (KeyStoreException kse) {
	    throw new RuntimeException
		(rb.getString("unable to instantiate keystore class: ") +
		kse.getMessage());
	}
    }

    void getAliasInfo(String alias) {

	Key key = null;

	try {

	    java.security.cert.Certificate[] cs = null;

	    try {
		cs = store.getCertificateChain(alias);
	    } catch (KeyStoreException kse) {
		// this never happens, because keystore has been loaded
	    }
	    if (cs == null) {
		MessageFormat form = new MessageFormat(rb.getString
		    ("Certificate chain not found for: alias.  alias must reference a valid KeyStore key entry containing a private key and corresponding public key certificate chain."));
		Object[] source = {alias, alias};
		error(form.format(source));
	    }

	    certChain = new X509Certificate[cs.length];
	    for (int i=0; i<cs.length; i++) {
		if (!(cs[i] instanceof X509Certificate)) {
		    error(rb.getString
			("found non-X.509 certificate in signer's chain"));
		}
		certChain[i] = (X509Certificate)cs[i];
	    }

	    // order the cert chain if necessary (put user cert first,
	    // root-cert last in the chain)
	    X509Certificate userCert
		= (X509Certificate)store.getCertificate(alias);
	    if (!userCert.equals(certChain[0])) {
		// need to order ...
		X509Certificate[] certChainTmp
		    = new X509Certificate[certChain.length];
		certChainTmp[0] = userCert;
		Principal issuer = userCert.getIssuerDN();
		for (int i=1; i<certChain.length; i++) {
		    int j;
		    // look for the cert whose subject corresponds to the
		    // given issuer
		    for (j=0; j<certChainTmp.length; j++) {
			if (certChainTmp[j] == null)
			    continue;
			Principal subject = certChainTmp[j].getSubjectDN();
			if (issuer.equals(subject)) {
			    certChain[i] = certChainTmp[j];
			    issuer = certChainTmp[j].getIssuerDN();
			    certChainTmp[j] = null;
			    break;
			}
		    }
		    if (j == certChainTmp.length) {
			error(rb.getString("incomplete certificate chain"));
		    }

		}
		certChain = certChainTmp; // ordered
	    }

	    try {
		if (keypass == null)
		    key = store.getKey(alias, storepass);
		else
		    key = store.getKey(alias, keypass);
	    } catch (UnrecoverableKeyException e) {
		if (keypass == null) {
		    // Did not work out, so prompt user for key password
		    MessageFormat form = new MessageFormat(rb.getString
			("Enter key password for alias: "));
		    Object[] source = {alias};
		    keypass = getPass(form.format(source));
		    key = store.getKey(alias, keypass);
		}
	    }
	} catch (NoSuchAlgorithmException e) {
	    error(e.getMessage());
	} catch (UnrecoverableKeyException e) {
	    error(rb.getString("unable to recover key from keystore"));
	} catch (KeyStoreException kse) {
	    // this never happens, because keystore has been loaded
	}

	if (!(key instanceof PrivateKey)) {
	    MessageFormat form = new MessageFormat(rb.getString
		("key associated with alias not a private key"));
	    Object[] source = {alias};
	    error(form.format(source));
	} else {
	    privateKey = (PrivateKey)key;
	}
    }

    void error(String message)
    {
	System.out.println(rb.getString("jarsigner: ")+message);
	System.exit(1);
    }


    void error(String message, Exception e)
    {
	System.out.println(rb.getString("jarsigner: ")+message);
	if (debug) {
	    e.printStackTrace();
	}
	System.exit(1);
    }

    char[] getPass(String prompt)
    {
	System.err.print(prompt);
	System.err.flush();
	try {
	    char[] pass = readPasswd(System.in);

	    if (pass == null) {
		error(rb.getString("you must enter key password"));
	    } else {
		return pass;
	    }
	} catch (IOException ioe) {
	    error(rb.getString("unable to read password: ")+ioe.getMessage());
	}
	// this shouldn't happen
	return null;
    }

    /**
     * Reads user password from given input stream.
     */
    private char[] readPasswd(InputStream in) throws IOException {
	char[] lineBuffer;
	char[] buf;
	int i;

	buf = lineBuffer = new char[128];

	int room = buf.length;
	int offset = 0;
	int c;

loop:	while (true) {
	    switch (c = in.read()) {
	      case -1:
	      case '\n':
		break loop;

	      case '\r':
		int c2 = in.read();
		if ((c2 != '\n') && (c2 != -1)) {
		    if (!(in instanceof PushbackInputStream)) {
			in = new PushbackInputStream(in);
		    }
		    ((PushbackInputStream)in).unread(c2);
		} else
		    break loop;

	      default:
		if (--room < 0) {
		    buf = new char[offset + 128];
		    room = buf.length - offset - 1;
		    System.arraycopy(lineBuffer, 0, buf, 0, offset);
		    Arrays.fill(lineBuffer, ' ');
		    lineBuffer = buf;
		}
		buf[offset++] = (char) c;
		break;
	    }
	}

	if (offset == 0) {
	    return null;
	}

	char[] ret = new char[offset];
	System.arraycopy(buf, 0, ret, 0, offset);
	Arrays.fill(buf, ' ');

	return ret;
    }

    /*
     * Reads all the bytes for a given zip entry.
     */
    private synchronized byte[] getBytes(ZipFile zf,
					 ZipEntry ze) throws IOException {
	int n;

	InputStream is = zf.getInputStream(ze);
	baos.reset();
	long left = ze.getSize();

	while((left > 0) && (n = is.read(buffer, 0, buffer.length)) != -1) {
	    baos.write(buffer, 0, n);
	    left -= n;
	}

	is.close();

	return baos.toByteArray();
    }

    /*
     * Returns manifest entry from given jar file, or null if given jar file
     * does not have a manifest entry.
     */
    private ZipEntry getManifestFile(ZipFile zf) {
	ZipEntry ze = zf.getEntry(JarFile.MANIFEST_NAME);
	if (ze == null) {
	    // Check all entries for matching name
	    Enumeration enum = zf.entries();
	    while (enum.hasMoreElements() && ze == null) {
		ze = (ZipEntry)enum.nextElement();
		if (!JarFile.MANIFEST_NAME.equalsIgnoreCase
		    (ze.getName())) {
		    ze = null;
		}
	    }
	}
	return ze;
    }

    /*
     * Computes the digests of a zip entry, and returns them as an array
     * of base64-encoded strings.
     */
    private synchronized String[] getDigests(ZipEntry ze, ZipFile zf,
					     MessageDigest[] digests,
					     BASE64Encoder encoder)
	throws IOException {

	int n, i;
	InputStream is = zf.getInputStream(ze);
	long left = ze.getSize();
	while((left > 0)
	      && (n = is.read(buffer, 0, buffer.length)) != -1) {
	    for (i=0; i<digests.length; i++) {
		digests[i].update(buffer, 0, n);
	    }
	    left -= n;
	}
	is.close();

	// complete the digests
	String[] base64Digests = new String[digests.length];
	for (i=0; i<digests.length; i++) {
	    base64Digests[i] = encoder.encode(digests[i].digest());
	}
	return base64Digests;
    }

    /*
     * Computes the digests of a zip entry, and returns them as a list of
     * attributes
     */
    private Attributes getDigestAttributes(ZipEntry ze, ZipFile zf,
					   MessageDigest[] digests,
					   BASE64Encoder encoder)
	throws IOException {

	String[] base64Digests = getDigests(ze, zf, digests, encoder);
	Attributes attrs = new Attributes();

	for (int i=0; i<digests.length; i++) {
	    attrs.putValue(digests[i].getAlgorithm()+"-Digest",
			   base64Digests[i]);
	}
	return attrs;
    }

    /*
     * Updates the digest attributes of a manifest entry, by adding or
     * replacing digest values.
     * A digest value is added if the manifest entry does not contain a digest
     * for that particular algorithm.
     * A digest value is replaced if it is obsolete.
     *
     * Returns true if the manifest entry has been changed, and false
     * otherwise.
     */
    private boolean updateDigests(ZipEntry ze, ZipFile zf,
				  MessageDigest[] digests,
				  BASE64Encoder encoder,
				  Manifest mf) throws IOException {
	boolean	update = false;

	Attributes attrs = mf.getAttributes(ze.getName());
	String[] base64Digests = getDigests(ze, zf, digests, encoder);

	for (int i=0; i<digests.length; i++) {
	    String name = digests[i].getAlgorithm()+"-Digest";
	    String mfDigest = attrs.getValue(name);
	    if (mfDigest == null
		&& digests[i].getAlgorithm().equalsIgnoreCase("SHA")) {
		// treat "SHA" and "SHA1" the same
		mfDigest = attrs.getValue("SHA-Digest");
	    }
	    if (mfDigest == null) {
		// compute digest and add it to list of attributes
		attrs.putValue(name, base64Digests[i]);
		update=true;
	    } else {
		// compare digests, and replace the one in the manifest
		// if they are different
		if (!mfDigest.equalsIgnoreCase(base64Digests[i])) {
		    attrs.putValue(name, base64Digests[i]);
		    update=true;
		}
	    }
	}
	return update;
    }
}
