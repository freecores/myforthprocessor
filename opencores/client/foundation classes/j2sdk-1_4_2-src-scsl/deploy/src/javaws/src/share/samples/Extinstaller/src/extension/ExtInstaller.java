/*
 * @(#)ExtInstaller.java	1.3 03/01/23
 * 
 * Copyright (c) 2003 Sun Microsystems, Inc. All Rights Reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * -Redistribution of source code must retain the above copyright notice, this
 *  list of conditions and the following disclaimer.
 * 
 * -Redistribution in binary form must reproduce the above copyright notice, 
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 * 
 * Neither the name of Sun Microsystems, Inc. or the names of contributors may 
 * be used to endorse or promote products derived from this software without 
 * specific prior written permission.
 * 
 * This software is provided "AS IS," without a warranty of any kind. ALL 
 * EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND WARRANTIES, INCLUDING
 * ANY IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * OR NON-INFRINGEMENT, ARE HEREBY EXCLUDED. SUN MIDROSYSTEMS, INC. ("SUN")
 * AND ITS LICENSORS SHALL NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY LICENSEE
 * AS A RESULT OF USING, MODIFYING OR DISTRIBUTING THIS SOFTWARE OR ITS
 * DERIVATIVES. IN NO EVENT WILL SUN OR ITS LICENSORS BE LIABLE FOR ANY LOST 
 * REVENUE, PROFIT OR DATA, OR FOR DIRECT, INDIRECT, SPECIAL, CONSEQUENTIAL, 
 * INCIDENTAL OR PUNITIVE DAMAGES, HOWEVER CAUSED AND REGARDLESS OF THE THEORY 
 * OF LIABILITY, ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, 
 * EVEN IF SUN HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * 
 * You acknowledge that this software is not designed, licensed or intended
 * for use in the design, construction, operation or maintenance of any
 * nuclear facility.
 */

import java.io.*;
import java.net.*;
import java.util.Properties;

public class ExtInstaller {
    
    private static final String configDir = "inst_test";
    private static final String configFile = "config.txt";
    private static final String libNameRoot = "inst";

    public static boolean install() {
	String path = Config.getInstallService().getInstallPath();
	if (!path.endsWith(File.separator)) {
	    path += File.separator;
	}
	String libName = System.mapLibraryName(libNameRoot);
	path += libName;


	File extPath = new File(path);
	if (extPath.exists()) {
	    extPath.delete();
	} else {
	    extPath.getParentFile().mkdirs();
	}
	
	// Write the native library
	URL u = ExtInstaller.class.getResource(libName);
	InputStream is = null;
	OutputStream os = null;
	byte buf[] = new byte[255];
	try {
	    is = u.openStream();
	    os = new FileOutputStream(extPath);
	    int len = 0;
	    while ((len=is.read(buf)) > 0) {
		os.write(buf, 0, len);
	    }
	} catch (IOException ioe) {
	    System.err.println("I/O Exception: " + ioe);
	    ioe.printStackTrace();
	    return false;
	} finally {
	    try {if (is!=null) is.close();} catch (IOException ioe) {}
	    try {if (os!=null) os.close();} catch (IOException ioe) {}
	}
	
	Properties p = new Properties();
	p.setProperty("libPath", extPath.toString());
	String home = System.getProperty("user.home");
	if (!home.endsWith(File.separator)) {
	    home += File.separator;
	}
	File propsFile = new File(home + configDir + File.separator + configFile);
	if (propsFile.exists()) {
	    propsFile.delete();
	} else {
	    propsFile.getParentFile().mkdirs();
	}
	OutputStream os2 = null;
	try {
	    os2 = new FileOutputStream(propsFile);
	    p.store(os2, "Installer test");
	} catch (IOException ioe) {
	    System.err.println("I/O Exception: " + ioe);
	    ioe.printStackTrace();
	    return false;
	} finally {
	    try {if (os2!=null) os2.close();} catch (IOException ioe) {}
	}
	Config.getInstallService().setNativeLibraryInfo(extPath.getParentFile().toString());
	return true;
	
    }
    
    public static void main (String [] args) {
	if (args.length > 0 && args[0].equals("install")) {
            System.out.println("Installing ...");
	    boolean success = install();
            System.out.println("End of Extension Installation.");
	    if (success) {
		Config.getInstallService().installSucceeded(false);
	    } else {
		Config.getInstallService().installFailed();
	    }
	} else {
	    System.out.println("Installer called without install arg");
	}
	System.exit(0);
    }
    
}


