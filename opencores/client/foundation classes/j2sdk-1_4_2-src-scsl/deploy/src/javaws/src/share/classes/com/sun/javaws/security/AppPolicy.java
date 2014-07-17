/*
 * @(#)AppPolicy.java	1.54 03/04/07
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.security;

import java.security.Policy;
import java.security.CodeSource;
import java.security.KeyStore;
import java.security.AccessControlException;
import java.security.PermissionCollection;
import java.security.Permissions;
import java.security.Permission;
import java.security.AllPermission;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.util.PropertyPermission;
import java.util.HashSet;
import java.util.Properties;
import java.util.Enumeration;
import java.util.Iterator;
import java.lang.RuntimePermission;
import java.awt.AWTPermission;
import java.net.URL;
import java.net.MalformedURLException;
import java.net.SocketPermission;
import java.io.IOException;
import java.io.FilePermission;
import java.io.File;
import com.sun.javaws.ConfigProperties;
import com.sun.javaws.Resources;
import com.sun.javaws.LaunchErrorDialog;
import com.sun.javaws.exceptions.JNLPException;
import com.sun.javaws.exceptions.LaunchDescException;
import com.sun.javaws.exceptions.UnsignedAccessViolationException;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.javaws.jnl.JARDesc;
import com.sun.javaws.cache.InstallCache;
import com.sun.javaws.cache.DiskCache;
import com.sun.javaws.cache.DiskCacheEntry;
import com.sun.javaws.debug.Debug;
import com.sun.javaws.debug.Globals;
import com.sun.jnlp.JNLPClassLoader;

/** Defines the security policy for applications launched
 *   by JavaWS.
 *
 *   It implemetens to access models:
 *   - full-access
 *   - sandbox (applet security model)
 *
 *  @version 1.16, 08/08/00
 *
 */
public class AppPolicy {
    
    // Configuration variables
    private String     _host  = null; // Name of host that is accessible
    
    /* Contains list of certificates that has been trusted in this session. This is to avoid
     * the user to get prompted several times for the same certificate
     */
    private HashSet _sessionCertificates = new HashSet();
    
    /** Location of extension directory for this particular JRE */
    private File _extensionDir = null;
    
    static private AppPolicy _instance = null;

    /** AppPolicy is a singleton object */
    static public AppPolicy getInstance() {
        return _instance;
    }
    
    /** Create the instance e based on current JNLP file */
    static public AppPolicy createInstance(String host) {
        if (_instance == null) {
            _instance = new AppPolicy(host);
        }
        return _instance;
    }

    /** Creates the JNLP "policy" object that knows how to assign the right
     *  resources to the JAR files specified in the JNLP files
     */
    private AppPolicy(String host) {
        _host = host;
        
        // Setup location of extension directory
        _extensionDir = new File(System.getProperty("java.home")
		     + File.separator + "lib" + File.separator + "ext");
    }
    
    /** Return the security policy in form of a PermissionCollection */
    public void addPermissions(PermissionCollection perms, CodeSource target) {
        if (Globals.TraceSecurity) {
	    Debug.println("Permission requested for: " + target.getLocation());
        }
        
        URL location = target.getLocation();
        java.security.cert.Certificate[] certChain = target.getCertificates();
        
        // Find JARDesc for this entry
        JARDesc jd = JNLPClassLoader.getInstance().getJarDescFromFileURL(location);
        if (jd == null) {
	    // Not from a JARDesc attribute. will default to correct permissions
            return;
        }

	
        // Debugging sanity check
        if (Globals.TraceSecurity) {
	    String versionId = null;
	    java.security.cert.Certificate[] certChain2 = null;
	    try  {
        	String locName = location.getFile();
		File f = new File(locName);
		DiskCacheEntry dce = InstallCache.getDiskCache().getCacheEntryFromFile(f);
		if (dce != null) certChain2 = dce.getCertificateChain();
	    } catch(IOException ioe) {
		Debug.ignoredException(ioe);
	    }
	    
	    if (certChain == null) {
		Debug.println("No certificates in codesource");
	    } else {
		Debug.println("Found certificate chain of length " + target.getCertificates().length + " in codesource");
	    }
	    
	    if (certChain2 == null) {
		Debug.println("No certificates in cache");
	    } else {
		Debug.println("Found certificate chain of length " + certChain.length + " in cache");
	    }
	    
	    if (certChain != null && target.getCertificates() != null) {
		Debug.jawsAssert(certChain.length == certChain2.length, "Certificate length must be the same");
		for(int i = 0; i < certChain.length; i++) {
		    if (!certChain[i].equals(certChain2[i])) {
			Debug.fatal("Certificate chain does not match");
		    }
		}
	    }
        }
        
        // Get LaunchDesc for for the JARDesc.
        LaunchDesc ld = jd.getParent().getParent();
        int access = ld.getSecurityModel();        

        if (access != LaunchDesc.SANDBOX_SECURITY && grantUnrestrictedAccess(ld, jd.getLocation(), certChain)) {
	    if (access == LaunchDesc.ALLPERMISSIONS_SECURITY) {
		addAllPermissionsObject(perms);
	    } else {
		addJ2EEApplicationClientPermissionsObject(perms);
	    }
        } else { 	   	 
	    addSandboxPermissionsObject(perms, 
			(ld.getLaunchType() == LaunchDesc.APPLET_DESC_TYPE));  
        }
	// only set the system properties the first time for this ld
	if (!ld.arePropsSet()) {
	    // now - only get the props for this ld (not extensions)
	    Properties props = ld.getResources().getResourceProperties();
            Enumeration keys = props.keys();
            while(keys.hasMoreElements()) {
                String name = (String)keys.nextElement();
                String value = props.getProperty(name);
                Permission perm = new PropertyPermission(name, "write");
		// only set property if app is allowed to
                if (perms.implies(perm)) {
                    System.setProperty(name, value);
                } else {
		    Debug.ignoredException(new AccessControlException(
					       "access denied "+perm, perm));
		}
	    }
	    ld.setPropsSet(true);
	}
    }
   
    private void setUnrestrictedProps(LaunchDesc ld) {
        // only set the system properties the first time for this ld
        if (!ld.arePropsSet()) {
            // now - only get the props for this ld (not extensions)
            Properties props = ld.getResources().getResourceProperties();
            Enumeration keys = props.keys();
            while(keys.hasMoreElements()) {
                String name = (String)keys.nextElement();
                System.setProperty(name, props.getProperty(name));
            }
            ld.setPropsSet(true);
        }
    }
 
    /** Returns true if unrestriced access should be granted to the codesource. This
     *  can happen in a couple of ways:
     *  - the code has been signed by a certificate that the user has previously accepted in this session
     *  - the user has accepted the certificate signed by this code
     *  - the code comes from a trusted codeSource, e.g., kizumbo
     */
    public boolean grantUnrestrictedAccess(LaunchDesc ld, URL location, Certificate[] certChain) {		
        String title = ld.getInformation().getTitle();
        int launchType = ld.getLaunchType();
        char diskType = ((launchType == LaunchDesc.APPLICATION_DESC_TYPE) ||
		 	 (launchType == LaunchDesc.APPLET_DESC_TYPE)) ? 
			  DiskCache.APPLICATION_TYPE : DiskCache.EXTENSION_TYPE;
        URL certLocation = ld.getCanonicalHome();
        
        // If there is no certificates, it is untrusted code
        if (certChain == null || certChain.length == 0) {
	    CodeSource cs = new CodeSource(location, certChain);
	    LaunchErrorDialog.show(null, new UnsignedAccessViolationException(
					 ld, cs.getLocation(), false));
	    Debug.shouldNotReachHere();
            return false;
        }
        
	// Certificate already acepted for this LaunchDesc?
	if (ld.getCertificateChain() != null) {
	    // Make sure all JAR files are using the same
	    if (!SigningInfo.equalChains(certChain, ld.getCertificateChain())) {
		Exception e = new LaunchDescException(ld, Resources.getString(
				    "launch.error.singlecertviolation"), null);
		LaunchErrorDialog.show(null, e);
	        Debug.shouldNotReachHere();
                return false;
	    }
	    setUnrestrictedProps(ld);
	    return true;
	}
	
	// Already accepted by another launchDesc?
	if (_sessionCertificates.contains(certChain[0])) {
	    ld.setCertificateChain(certChain);
            setUnrestrictedProps(ld);
	    return true;
	}
	
	// Check if certificate is in cache, e.g., already trusted
	try {
	    Certificate[] cacheCert = InstallCache.getDiskCache().getCertificateChain(diskType,
										      certLocation,
										      null);
	    if (cacheCert != null && certChain[0].equals(cacheCert[0])) {
		// Code is already trusted
		_sessionCertificates.add(certChain[0]);
		ld.setCertificateChain(certChain);
                setUnrestrictedProps(ld);
		return true;
	    }
	} catch(IOException ie) {
	    // Just ignore, if cache is unreadable.
	    Debug.ignoredException(ie);
	}
	
	// The 'makeTrusted' might add a
	// root certificate to the end - if it is not included already
	Certificate[] chain = KeyStoreManager.makeTrusted(certChain);
	// Prompt user to accepted the certificate
	boolean trusted = CertificateDialog.verifyCertificate(
				null, title, chain, launchType);
	// Add base certificate to trusted list.
	if (trusted) {
	    // Cache for local session
	    _sessionCertificates.add(certChain[0]);
	    ld.setCertificateChain(certChain);
	    // Cache on disk
	    try {
		// Store non-normalized certificate chain
		InstallCache.getDiskCache().putCertificateChain(diskType, certLocation, null, certChain);
	    } catch(IOException ioe) {
		Debug.ignoredException(ioe);
	    }
            setUnrestrictedProps(ld);
	}
	return trusted;
    }
    
    private void addAllPermissionsObject(PermissionCollection perms) {
	if (Globals.TraceSecurity) {
	    Debug.println("adding all-permissions object");
	}
	perms.add(new AllPermission());
    }
    
    private void addJ2EEApplicationClientPermissionsObject(PermissionCollection perms) {
	if (Globals.TraceSecurity) {
	    Debug.println("Creating J222-application-client-permisisons object");
	}
	
	// AWT permissions
	perms .add(new AWTPermission("accessClipboard"));
	perms .add(new AWTPermission("accessEventQueue"));
	perms .add(new AWTPermission("showWindowWithoutWarningBanner"));
	
	// Runtime permissions
	perms.add(new RuntimePermission("exitVM"));
	perms.add(new RuntimePermission("loadLibrary"));
	perms.add(new RuntimePermission("queuePrintJob"));
	
	// Socket permissions
	perms.add(new SocketPermission("*", "connect"));
	perms.add(new SocketPermission("localhost:1024-", "accept,listen"));
	
	// File permissions
	perms.add(new FilePermission("*", "read,write"));
	
	// Property permissions
	perms.add(new PropertyPermission("*", "read"));	
    }
    
    /** Creates a JavaWS Sandbox Security policy. This will deny access to the file-system, limit
     *  access to properties, and only provide access to the ports that are listed in
     *  the classpath
     */
    private void addSandboxPermissionsObject(PermissionCollection perms, 
					     boolean isApplet) {
	if (Globals.TraceSecurity) {
	    Debug.println("Add sandbox permissions");
	}
	
	// Grant read-only access to select number of properties
	perms.add(new PropertyPermission("java.version"       , "read"));
	perms.add(new PropertyPermission("java.vendor"        , "read"));
	perms.add(new PropertyPermission("java.vendor.url"    , "read"));
	perms.add(new PropertyPermission("java.class.version" , "read"));
	perms.add(new PropertyPermission("os.name"            , "read"));
	perms.add(new PropertyPermission("os.arch"            , "read"));
	perms.add(new PropertyPermission("os.version"         , "read"));
	perms.add(new PropertyPermission("file.separator"     , "read"));
	perms.add(new PropertyPermission("path.separator"     , "read"));
	perms.add(new PropertyPermission("line.separator"     , "read"));
	
	perms.add(new PropertyPermission("java.specification.version", "read"));
	perms.add(new PropertyPermission("java.specification.vendor",  "read"));
	perms.add(new PropertyPermission("java.specification.name",    "read"));
	
	perms.add(new PropertyPermission("java.vm.specification.version", "read"));
	perms.add(new PropertyPermission("java.vm.specification.vendor",  "read"));
	perms.add(new PropertyPermission("java.vm.specification.name",    "read"));
	perms.add(new PropertyPermission("java.vm.version",               "read"));
	perms.add(new PropertyPermission("java.vm.vendor",                "read"));
	perms.add(new PropertyPermission("java.vm.name",                  "read"));
	
	// Java Web Start specific permission
	perms.add(new PropertyPermission("javawebstart.version",          "read"));
	
	// Runtime perms
	perms.add(new RuntimePermission("exitVM"));
	perms.add(new RuntimePermission("stopThread"));
	// Accordiing to rev 13. these are not granted. (stricly applet sandbox complient)
	//perms.add(new RuntimePermission("modifyThread"));	
	//perms.add(new RuntimePermission("modifyThreadGroup"));
	
	// According to rev. 9 these are not granted.
	//perms.add(new RuntimePermission("setIO"));
	//perms.add(new RuntimePermission("accessDeclaredMembers"));
	
	// Applets might access stuff in the sun.applet part.
	// NOPE. This is wrong - fails applet security test if so
	//perms.add(new RuntimePermission("accessClassInPackage.sun.applet"));
	
	// AWT perms
	// The following two permissions will be removed in the final revision
	// of the 1.0 JNLP specificaion
	//perms.add(new AWTPermission("accessEventQueue"));
	//perms.add(new AWTPermission("listenToAllAWTEvents"));

	// Note: this string is not translated on purpose
	String warningString =  "Java " + 
				(isApplet ? "Applet" : "Application") + 
				" Window";
	//perms.add(new AWTPermission("showWindowWithoutWarningBanner"));
	//because showWindowWithoutWarningBanner is denied, we set the text:
	System.setProperty("awt.appletWarning", warningString);
	
	// allows anyone to listen on un-privileged ports.
	perms.add(new SocketPermission("localhost:1024-", "listen"));
	// Access to exactly one host
	perms.add(new SocketPermission(_host, "connect, accept"));
	
	// Add user-defined jnlp.* properties to access list
        perms.add(new PropertyPermission("jnlp.*", "read,write"));
        perms.add(new PropertyPermission("javaws.*", "read,write"));

	// Add properties considered "Secure"
	Iterator it = ConfigProperties.getInstance().getSecurePropertyKeys();
	if (it != null) {
	    while (it.hasNext()) {
		String key = (String) it.next();
	        perms.add(new PropertyPermission(key, "read,write"));
	    }
	}
    }
}


