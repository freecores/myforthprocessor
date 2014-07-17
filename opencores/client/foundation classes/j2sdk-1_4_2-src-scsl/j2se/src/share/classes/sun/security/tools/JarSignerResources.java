/*
 * @(#)JarSignerResources.java	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.tools;

/**
 * <p> This class represents the <code>ResourceBundle</code>
 * for JarSigner.
 *
 * @version 1.5, 01/23/03
 */
public class JarSignerResources extends java.util.ListResourceBundle {

    private static final Object[][] contents = {

	// shared (from jarsigner)
	{" ", " "},
	{"  ", "  "},
	{"      ", "      "},
	{", ", ", "},

	{"provName not a provider", "{0} not a provider"},
	{"jarsigner error: ", "jarsigner error: "},
	{"Illegal option: ", "Illegal option: "},
	{"Usage: jarsigner [options] jar-file alias",
		"Usage: jarsigner [options] jar-file alias"},
	{"       jarsigner -verify [options] jar-file",
		"       jarsigner -verify [options] jar-file"},
	{"  [-keystore <url>]           keystore location",
		"  [-keystore <url>]           keystore location"},
	{"  [-storepass <password>]     password for keystore integrity",
	    "  [-storepass <password>]     password for keystore integrity"},
	{"  [-storetype <type>]         keystore type",
		"  [-storetype <type>]         keystore type"},
	{"  [-keypass <password>]       password for private key (if different)",
		"  [-keypass <password>]       password for private key (if different)"},
	{"  [-sigfile <file>]           name of .SF/.DSA file",
		"  [-sigfile <file>]           name of .SF/.DSA file"},
	{"  [-signedjar <file>]         name of signed JAR file",
		"  [-signedjar <file>]         name of signed JAR file"},
	{"  [-verify]                   verify a signed JAR file",
		"  [-verify]                   verify a signed JAR file"},
	{"  [-verbose]                  verbose output when signing/verifying",
		"  [-verbose]                  verbose output when signing/verifying"},
	{"  [-certs]                    display certificates when verbose and verifying",
		"  [-certs]                    display certificates when verbose and verifying"},
	{"  [-internalsf]               include the .SF file inside the signature block",
		"  [-internalsf]               include the .SF file inside the signature block"},
	{"  [-sectionsonly]             don't compute hash of entire manifest",
		"  [-sectionsonly]             don't compute hash of entire manifest"},
	{"  [-provider]                 name of cryptographic service provider's master class file",
		"  [-provider]                 name of cryptographic service provider's master class file"},
	{"  ...", "  ..."},
	{"s", "s"},
	{"m", "m"},
	{"k", "k"},
	{"i", "i"},
	{"  s = signature was verified ",
		"  s = signature was verified "},
	{"  m = entry is listed in manifest",
		"  m = entry is listed in manifest"},
	{"  k = at least one certificate was found in keystore",
		"  k = at least one certificate was found in keystore"},
	{"  i = at least one certificate was found in identity scope",
		"  i = at least one certificate was found in identity scope"},
	{"no manifest.", "no manifest."},
	{"jar is unsigned. (signatures missing or not parsable)",
		"jar is unsigned. (signatures missing or not parsable)"},
	{"jar verified.", "jar verified."},
	{"jarsigner: ", "jarsigner: "},
	{"signature filename must consist of the following characters: A-Z, 0-9, _ or -",
		"signature filename must consist of the following characters: A-Z, 0-9, _ or -"},
	{"unable to open jar file: ", "unable to open jar file: "},
	{"unable to create: ", "unable to create: "},
	{"   adding: ", "   adding: "},
	{" updating: ", " updating: "},
	{"  signing: ", "  signing: "},
	{"attempt to rename signedJarFile to jarFile failed",
		"attempt to rename {0} to {1} failed"},
	{"attempt to rename jarFile to origJar failed",
		"attempt to rename {0} to {1} failed"},
	{"unable to sign jar: ", "unable to sign jar: "},
	{"Enter Passphrase for keystore: ", "Enter Passphrase for keystore: "},
	{"keystore load: ", "keystore load: "},
	{"certificate exception: ", "certificate exception: "},
	{"unable to instantiate keystore class: ",
		"unable to instantiate keystore class: "},
	{"Certificate chain not found for: alias.  alias must reference a valid KeyStore key entry containing a private key and corresponding public key certificate chain.",
		"Certificate chain not found for: {0}.  {1} must reference a valid KeyStore key entry containing a private key and corresponding public key certificate chain."},
	{"found non-X.509 certificate in signer's chain",
		"found non-X.509 certificate in signer's chain"},
	{"incomplete certificate chain", "incomplete certificate chain"},
	{"Enter key password for alias: ", "Enter key password for {0}: "},
	{"unable to recover key from keystore",
		"unable to recover key from keystore"},
	{"key associated with alias not a private key",
		"key associated with {0} not a private key"},
	{"you must enter key password", "you must enter key password"},
	{"unable to read password: ", "unable to read password: "}

    };

    /**
     * Returns the contents of this <code>ResourceBundle</code>.
     *
     * <p>
     *
     * @return the contents of this <code>ResourceBundle</code>.
     */
    public Object[][] getContents() {
	return contents;
    }
}
