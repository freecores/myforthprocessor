/*
 * @(#)CachedJarFile.java	1.12 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.cache;

import java.io.File;
import java.io.IOException;
import java.security.cert.Certificate;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.jar.Attributes;
import java.util.jar.JarFile;
import java.util.jar.JarEntry;
import java.util.jar.Manifest;
import java.util.zip.ZipEntry;
import sun.plugin.util.Trace;

/**
 * This class is used to read jar files from the jar cache.
 *
 * @see     java.util.zip.ZipFile
 * @see     java.util.jar.JarEntry
 */
public class CachedJarFile extends JarFile {

    // The certificates used to sign this jar file
    private Certificate[] certificates;

    // Map of entries to their signers
    private HashMap signerMap = null;
    
    // Manifest for this Jar file
    private Manifest manifest = null;

    /**
     * Constructor.  This is protected, since it only makes sense for the
     * caching code to instantiate this class.
     */
    protected CachedJarFile(File file, Certificate[] certificates,
                            HashMap signerMap, Manifest manifest) 
                            throws IOException {
        // Pass in false for the verify parameter to ensure that the
        // superclass does not attempt to authenticate the Jar file.
        super(file, false);
        this.certificates = certificates;
        this.signerMap = signerMap;
        this.manifest = manifest;

        //if manLoaded is not set in the super class, whenever
        //getInputStream() is called, getManfiest() is called which
        //is expensive. This overhead is avoided by making a call to 
        //the super.getManifest() which sets the manLoaded flag
        super.getManifest();
    }

    /**
     * Returns the <code>ZipEntry</code> for the given entry name or
     * <code>null</code> if not found.
     *
     * @param  name the JAR file entry name
     * @return the <code>ZipEntry</code> for the given entry name or
     *         <code>null</code> if not found
     * @see    java.util.zip.ZipEntry
     */
    public ZipEntry getEntry(String name) {
        ZipEntry ze = super.getEntry(name);
        if (ze != null) {
            return new JarFileEntry(ze);
        }
        return null;
    }

    /**
     * Returns an enumeration of the entries in this JAR file.
     *
     * @return an <code>Enumeration</code> of the entries in this JAR file
     * @see    java.util.Enumeration
     */
    public Enumeration entries() {
        final Enumeration enum = super.entries();
        return new Enumeration() {
            public boolean hasMoreElements() {
                return enum.hasMoreElements();
            }
            public Object nextElement() {
                ZipEntry ze = (ZipEntry)enum.nextElement();
                return new JarFileEntry(ze);
            }
        };
    }


    /**
     * Returns the JAR file manifest, or <code>null</code> if none.
     *
     * @return the JAR file manifest, or <code>null</code> if none
     */
    public Manifest getManifest() throws IOException {
	Manifest copy = null;

	if(manifest != null) {
	    // Return a copy of the manifest for security reasons
	    copy = new Manifest();
	    Attributes copyAttr = copy.getMainAttributes();
	    copyAttr.putAll((Map)manifest.getMainAttributes().clone());
	    Map entries = manifest.getEntries();
	    if (entries != null) {
		Map copyEntries = copy.getEntries();
		Iterator it = entries.keySet().iterator();
		while (it.hasNext()) {
		    Object key = it.next();
		    Attributes at = (Attributes)entries.get(key);
		    copyEntries.put(key, at.clone());
		}
	    }
	}

	return copy;
    }


    // Private class to represent an entry in a cached JAR file
    private class JarFileEntry extends JarEntry {
        JarFileEntry(ZipEntry ze) {
            super(ze);
        }
        public Attributes getAttributes() throws IOException {

            // Get the entry's attributes from the JAR manifest
            if (manifest != null) {
                return (Attributes)manifest.getAttributes(getName()).clone();
            } else {
                return null;
            }
        }
        public java.security.cert.Certificate[] getCertificates() {
            Certificate[] certs = null;
            if (signerMap != null && !signerMap.isEmpty()) {

                // Find this entry in the signer map
                int[] signers = (int[])signerMap.get(getName());
                if (signers != null) {

                    // Create an array listing the certificates of each signer
                    certs = new Certificate[signers.length];
                    for (int i = 0; i < signers.length; i++) {
                        certs[i] = certificates[signers[i]];
                    }
                }
            }
            return certs;
        }
    }
}


