/*
 * @(#)PluginClassLoader.java	1.54 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.security;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.IOException;
import java.io.FilePermission;
import java.net.URL;
import java.net.MalformedURLException;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;
import java.util.jar.Attributes.Name;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Enumeration;
import java.util.NoSuchElementException;
import java.util.Iterator;
import java.util.Set;
import java.security.AccessController;
import java.security.CodeSource;
import java.security.PermissionCollection;
import java.security.Policy;
import java.security.PrivilegedAction;
import java.security.cert.CertificateException;
import java.security.cert.CertificateExpiredException;
import java.security.cert.CertificateNotYetValidException;
import sun.applet.AppletClassLoader;
import sun.applet.AppletResourceLoader;
import sun.misc.Resource;
import sun.plugin.util.Trace;
import sun.plugin.util.ProgressTracker;
import sun.plugin.resources.ResourceHandler;
import sun.net.www.ParseUtil;

/**
 * This class defines the class loader for loading applet classes and
 * resources. It extends AppletClassLoader to search the applet code base
 * for the class or resource after checking any loaded JAR files.
 */
public final class PluginClassLoader extends AppletClassLoader {

    private static RuntimePermission usePolicyPermission;
    private URL base;
    
    private HashMap JARJARtoJAR = new HashMap();    

    /*
     * Creates a new PluginClassLoader for the specified base URL.
     */
    public PluginClassLoader(URL base) {
	super(base);
	this.base = base;
    }

    /*
     * Returns the applet code base URL.
     */
    public URL getBaseURL() {
	return base;
    }

    /**
     * Returns the permissions for the given codesource object.
     * The implementation of this method first calls super.getPermissions,
     * to get the permissions
     * granted by the super class, and then adds additional permissions
     * based on the URL and signers of the codesource.
     *
     * @param cs the codesource
     * @return the permissions granted to the codesource
     */
    protected PermissionCollection getPermissions(CodeSource cs)
    {
	final PermissionCollection perms = super.getPermissions(cs);

	//Give all permissions for java beans embedded thru activex bridge
	URL url = cs.getLocation();
	if(url != null && url.getProtocol().equals("file")) {
	    String path = ParseUtil.decode(url.getFile());
	    if(path != null) {
		path = path.replace('/', File.separatorChar);
		String axBridgePath = File.separator + System.getProperty("java.home") + 
				  File.separator + "axbridge" + File.separator + "lib";
		try {
		    path = new File(path).getCanonicalPath();
		    axBridgePath = new File(axBridgePath).getCanonicalPath();
		    if( path != null && axBridgePath != null && 
			path.startsWith(axBridgePath) ){
			perms.add(new java.security.AllPermission());
			return perms;
		    }
		}catch(IOException exc) {
		    //Exception when getCanonicalPath() is called is ignored 
		}
	    }
	}

	// Added to check usePolicyPermission only
	PermissionCollection perms2 = null;

	// Get Policy object
	Policy newPolicy = (Policy) AccessController.doPrivileged(new PrivilegedAction() {
		public Object run() {
			return Policy.getPolicy();
		}
	} );

	// Get Policy permisions
	perms2 = newPolicy.getPermissions(cs);

	// Inject permission to access sun.audio package
	//
	perms.add(new java.lang.RuntimePermission("accessClassInPackage.sun.audio"));

	// Inject permission to read certain generic browser properties
	//
	perms.add(new java.util.PropertyPermission("browser", "read"));
	perms.add(new java.util.PropertyPermission("browser.version", "read"));
	perms.add(new java.util.PropertyPermission("browser.vendor", "read"));
	perms.add(new java.util.PropertyPermission("http.agent", "read"));
    
	// Inject permission to read certain Java Plug-in specific properties
	//
	perms.add(new java.util.PropertyPermission("javaplugin.version", "read"));

	if (usePolicyPermission == null)
	    usePolicyPermission = new RuntimePermission("usePolicy");

	if (!perms2.implies(usePolicyPermission) &&
	    (cs.getCertificates() != null)) {
	    // code is signed, and user wants to be prompted for AllPermission  

	    try
	    {
		if (TrustDecider.isAllPermissionGranted(cs))
		    perms.add(new java.security.AllPermission());
	    }
	    catch (CertificateExpiredException e1)
	    {
		Trace.securityPrintException(e1, ResourceHandler.getMessage("rsa.cert_expired"),
					     ResourceHandler.getMessage("security_dialog.caption"));    
	    }
	    catch (CertificateNotYetValidException e2)
	    {
		Trace.securityPrintException(e2, ResourceHandler.getMessage("rsa.cert_notyieldvalid"),
					     ResourceHandler.getMessage("security_dialog.caption"));    
	    }
	    catch (Exception e3)
	    {
		Trace.securityPrintException(e3, ResourceHandler.getMessage("rsa.general_error"),
					     ResourceHandler.getMessage("security_dialog.caption"));    
	    }
	}

	return perms;
    }

    /*
     * Adds the specified JAR file to the search path of loaded JAR files.
     */
    public void addLocalJar(URL url) 
    {
	addURL(url);
    }

    /**
     * Returns an Enumeration of URLs representing all of the resources
     * on the URL search path having the specified name.
     *
     * @param name the resource name
     * @exception if an I/O exception occurs
     * @return an <code>Enumeration</code> of <code>URL</code>s
     */
    public Enumeration findResources(String name) throws IOException 
    {
	return findResourcesByURLPath(name);
    }

    /* 
     * Returns an enumeration of URLs representing the resources
     * on the codebase having the specified name.
     */
    private Enumeration findResourcesByURLPath(final String name) throws IOException
    {
	// The following code is to avoid the sun.misc.Service to 
	// locate the sound service from the applet codebase.
	// Since the service is located for every sound file, this
	// has became a performance bottleneck. The workaround will
	// be to avoid looking up the sound file class from the
	// codebase. This speeds up the audio file loading through 
	// HTTP/HTTPS quite a lot. [stanleyh]
	//		
	if (name != null && name.indexOf("META-INF/services/javax.sound") != -1)
	{
	    return new Enumeration() {
		    public Object nextElement() 
		    {
			throw new NoSuchElementException();
		    }
		    public boolean hasMoreElements() 
		    {
			return false;
		    }
		};	
	}

	return super.findResources(name);
    }
    
    /*
     * Add Jar files.  If this is a file url 
     * and we are adding a JARJAR file, unpack it,
     * save Jar file in temp directory, and add that 
     * saved Jar file with base=temp_directory.
     */
    protected void addJar(String name) throws IOException{       
        if (name.toUpperCase().endsWith(".JARJAR") && 
            base.getProtocol().equalsIgnoreCase("file") )
        {
            /*
             * Extract JAR from JARJAR
             * put extracted JAR in a temp file 
             * add temp file to search path.
             */
            String fullPath = base.toString() + name;
            
            // See if this file is already loaded and unjared to cache.
            if (JARJARtoJAR.containsKey(fullPath))
                ; //Do nothing, it's already there.
            
            else
            {
                // Load the jarjar
	        JarFile jf1 = null;

	        File tempFile = null;
	        boolean bJarJar = false;
	        try
	        {
	            jf1 = new JarFile(base.getPath()+name, true);
		    Enumeration enum = jf1.entries();
                    
                    // Count number of JAR files inside the JARJAR file:
                    int jarCounter = 0;

                    /*
                     * JARJAR must have only one JAR file inside (META file is ok too).
                     * Multiple JAR files inside a JARJAR file are not supported.
                     */
		    if (enum.hasMoreElements() == false)
		        throw new IOException("Invalid jarjar file");

		    JarEntry entry = null; 
                    while (enum.hasMoreElements())
                    {
                        entry = (JarEntry) enum.nextElement();
                    
                        /*
                         * See if this is a META file.  ignore it.
                         */
                        if (entry.toString().toUpperCase().startsWith("META-INF/"))                                    
                            continue;
                    
                        /*
                         * See if this is anything else but .JAR extension - should not
                         * be there, throw exception.
                         */
                        else if (! entry.toString().toUpperCase().endsWith(".JAR"))
                            throw new IOException("Invalid entry in jarjar file.");
                    
                        // This is the JAR file inside a JARJAR, increment counter
                        else
                        {
                            jarCounter++;
                            if (jarCounter>1)
                                break;
                        }
                        

                    }// end while (enum.hasMoreEntries())
                    
                    if (jarCounter > 1)
                    {
                        //There is more then one JAR file inside JARJAR, throw exception.
                        entry = null;
                        throw new IOException("Multiple JAR files inside JARJAR file");
                    }
                
                    /*
                     * At this point entry must be equal to a JAR file.
                     */
                    byte[] buffer = new byte[8192];

    	            // Read the input stream and store it 
	            // in a temp file. This is necessary so we
	            // may obtain the signature properly.
	            BufferedInputStream bis = null;
	            BufferedOutputStream bos = null;
	            FileOutputStream fos = null;
                        
                    InputStream is = jf1.getInputStream(entry);
                                
                    boolean bSuccess = false;

                    try
                    {
                        tempFile = File.createTempFile(entry.toString().substring(0, entry.toString().lastIndexOf('.')), 
                                                       ".jar");

                        Trace.msgPrintln("pluginclassloader.created_file", new Object[] {tempFile.getPath()});
                        
                        ProgressTracker.onProgressComplete(base.toString()+name);
                        
	                bis = new BufferedInputStream(is);
	                fos = new FileOutputStream(tempFile);
	                bos = new BufferedOutputStream(fos);

	                int n;
	                while((n = bis.read(buffer, 0, buffer.length))!= -1) 
		            bos.write(buffer, 0, n);

	                bos.flush();

                        JARJARtoJAR.put(base.toString()+name, tempFile);
                        
	                bSuccess = true;
                        URL url;
	                try {
                            url=tempFile.toURL();
	                } catch (MalformedURLException e) {
                            throw new IllegalArgumentException("name");
	                }
	                addURL(url);                
                    }// end try
                    finally
	            {
	                if (bis != null)
		            bis.close();

	                if (bos != null)
		            bos.close();

	                if (fos != null)
		            fos.close();

	                bis = null;
	                bos = null;
	                fos = null;

	                if (bSuccess == false)
	                {
                            Trace.msgPrintln("pluginclassloader.empty_file", new Object[] {tempFile.getName()});
		            if (tempFile != null)
		                tempFile.delete();
	                }
	            }// end finally

                }//end the very first try...
                finally
                {
	            if (jf1 != null)
	                jf1.close();
	        }
            }// end if ( JARJAR is not in hashmap)                     
        }// end if(this is a JARJAR)
        else //this is not a JARJAR            
        {	
            super.addJar(name);
            if (base.getProtocol().equalsIgnoreCase("file"))
            {
                ProgressTracker.onProgressComplete(base.toString()+name);
            }
        }
    }
    
    protected void release()
    {
        // Remove files from cache.
        if (!JARJARtoJAR.isEmpty())
        {
            Trace.msgPrintln("pluginclassloader.deleting_files");

	    Set keys = JARJARtoJAR.keySet();    
	    Iterator iter = keys.iterator();	
	    while (iter.hasNext())
	    {
		Object key = iter.next();
                File tempFile = (File) JARJARtoJAR.get(key);
                if (tempFile != null)
                {
                    Trace.msgPrintln("pluginclassloader.file", new Object[] {tempFile.getPath()});
                    tempFile.delete();
                }
            }
            // Empty the HashMap
            JARJARtoJAR.clear();
        }
        super.release();        
    }    
}
