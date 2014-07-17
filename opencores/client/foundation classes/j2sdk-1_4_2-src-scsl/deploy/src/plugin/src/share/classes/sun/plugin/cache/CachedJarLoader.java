/*
 * @(#)CachedJarLoader.java	1.21 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package sun.plugin.cache;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.ByteArrayOutputStream;
import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.RandomAccessFile;
import java.net.URL;
import java.net.URLConnection;
import java.text.DateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Set;
import java.util.StringTokenizer;
import java.util.jar.Attributes;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;
import java.util.jar.Manifest;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;
import java.util.zip.ZipOutputStream;
import java.security.cert.Certificate;
import java.security.AccessController;
import java.security.PrivilegedActionException;
import sun.misc.JarIndex;
import java.net.HttpURLConnection;
import sun.plugin.net.protocol.http.HttpUtils;
import sun.plugin.util.Trace;
import sun.plugin.util.ProgressTracker;
import sun.plugin.resources.ResourceHandler;

// Class to handle loading of jar files from the cache
public class CachedJarLoader {
    
    private URL url;
    private HttpURLConnection uc = null;
    private long lastModified = 0;
    private long expiration = 0;
    
    // The jar's up-to-date status, file descriptor and size.
    private boolean upToDate = false;
    private boolean upToDateChecked = false;
    private File dataFile = null;
    private File indexFile = null;
    private boolean cached = false;
    private long size = 0;
    private FileVersion version = new FileVersion();

    // Modifications for initialization of the CachedJarLoader.  When
    // determining sizes for the ProgressDialog, call with a getSize of
    // true.  Other other times, call with a getSize of false.
    public CachedJarLoader(URL url, boolean getSize) throws IOException {
        this.url = url;
        
        // Get the file descriptor up front.
        cached = getCacheFile();
        
        // Should only be done during execution of the progress dialog.
        if (getSize) {
            if (cached) {
                upToDate = isUpToDate();
                upToDateChecked = true;
            } else {
                size = Cache.getFileSizeFromServer(url);
            }
        }
    }
            
    // Public query for the jar size.
    public long getJarSize() {
        return size;
    }

    public URL getURL() {
        return url;
    }

    // Public query for the jar size.
    public void setDataFile(File file) {
        dataFile = file;
    }

    public void setIndexFile(File file) {
        indexFile = file;
    }

    public void setLastModify(long time) {
        lastModified = time;
    }

    public void setExpiration(long time) {
        expiration = time;
    }

    //Set the jarfile version
    public void setVersion(FileVersion ver) {
        version = ver;
    }

    // Load the jar file
    public CachedJarFile load() throws IOException {

        // Try to load the file from the cache
        CachedJarFile jar = loadFromCache();

        // If not successful, download the file
        if (jar == null) {
	    try {
                jar = download();
	    }catch(IOException e) {
		// have to make sure it is connection failure
		if(uc != null) {
		    int response = uc.getResponseCode();
		    if ((response < 200) || (response >= 300)) {
			throw new DownloadException(e, uc);		
		    }
		}
		
		throw e;
	    }

	    if(jar != null && Cache.cleanupThread != null) {
                try {
                    AccessController.doPrivileged(new Cache.CacheIOAction() {
                        public Object run() throws IOException {
                            Cache.cleanupThread.addedJar(dataFile.length() + indexFile.length());
                            //return value is not used
                            return null;
                        }
                    });
                }// end try
                catch (PrivilegedActionException pae) {
                    throw new IOException(pae.getMessage());
                }
	    }
        }
        
        return jar;
    }

    // Load the jar file from the cache
    private CachedJarFile loadFromCache() {
        CachedJarFile jar = null;
        try {
            // Check if the file is in the cache
            if (cached) {
                
                // Found the file.  Check if it is up to date.
                // If we haven't already processed this jar, we need to
                // determine it's up-to-date status.
                if (!upToDateChecked) {
                    upToDate = isUpToDate();
                }
                if (upToDate) {
                    
                    // File is up to date.
                    Cache.msgPrintln("cache.loading", new Object[]{ url });
                    
                    ProgressTracker.onProgressComplete(url);       
       
                    try {
                        jar = (CachedJarFile)AccessController.doPrivileged(new Cache.CacheIOAction() {
                            public Object run() throws IOException {
                                // Touch the file.  Files are deleted from the
                                // cache in order of use, and this marks the
                                // file as recently used.
                                indexFile.setLastModified(System.currentTimeMillis());
                            
                                // Process the cached authentication
                                return authenticateFromCache();                }
                        });
                    }// end try
                    catch (PrivilegedActionException pae) {
                        throw new IOException(pae.getMessage());
                    }
                } else {
                    // File is out of date
                    try {
                        AccessController.doPrivileged(new Cache.CacheIOAction() {
                            public Object run() throws IOException {
                            // Try to delete this file.  This will not 
                            // work if the file is in use, but that's ok
                            // because we marked it as Cache.UNUSABLE above.  
                            // It will get deleted next time cache cleanup
                            // runs and the file is not in use.
			    Cache.removeFromTable(indexFile, JarCache.filesInCache);
			    if(!dataFile.delete()) {
				dataFile.deleteOnExit();
			    }
			    if(!indexFile.delete()) {
				FileOutputStream fos = new FileOutputStream(indexFile);
				fos.write(Cache.UNUSABLE);
				fos.close();
				indexFile.deleteOnExit();
                            }

                            // Need to null file since it's now a class var.
                            dataFile = null;
			    indexFile = null;
                            
                            return null;         }
                        });
                    }// end try
                    catch (PrivilegedActionException pae) {
                        throw new IOException(pae.getMessage());
                    }
                }
            }
        } catch (IOException e) {
            // Ignore, we'll return null
            e.printStackTrace();
            Cache.msgPrintln("cache.load_warning", new Object[] {url});
        }
        return jar;
    }

    // Find the cached copy of this jar file, if it exists
    private boolean getCacheFile() throws IOException {
	final CachedJarLoader thisLoader = this;
        try {
            return ((Boolean)AccessController.doPrivileged(new Cache.CacheIOAction() {
                public Object run() throws IOException {
                    boolean found = false;
                    found = JarCache.getMatchingFile(thisLoader);
                    return Boolean.valueOf(found);
                }           
            })).booleanValue();
        }// end try
        catch (PrivilegedActionException pae) {
            throw new IOException(pae.getMessage());
        }        

    }

    // Checks if the cached copy of the jarfile's version
    // is latest compared to the version specified in the HTML page
    public boolean isVersionUpToDate(FileVersion inVersion)
    {
	boolean current = false;
	current = version.isUpToDate(inVersion);
	upToDate = current;
	upToDateChecked = true;
	return current;
    }

    // Checks if the cached copy of the jar file is up to date
    private boolean isUpToDate() throws IOException {
        boolean upToDate = false;

        // Check the expiration date
        if (expiration != 0) {
            if ((new Date()).before(new Date(expiration))) {
                upToDate = true;
            }
        }

        // Check the last modified date
        if (!upToDate && (lastModified != 0)) {
            uc = (HttpURLConnection)url.openConnection();
            uc.setUseCaches(false);
            uc.setAllowUserInteraction(false);
            uc.setIfModifiedSince(lastModified);
            uc = HttpUtils.followRedirects(uc);
            int response = uc.getResponseCode();
            if (response == HttpURLConnection.HTTP_NOT_MODIFIED) {

                // The file has not been modified
                uc.disconnect();
                upToDate = true;
            } else if (response >= 200 && response <= 299) {

                long oldModified = lastModified;
                long oldExpiration = expiration;
                lastModified = uc.getLastModified();
                expiration = uc.getExpiration();
                if (lastModified == oldModified) {

                    // There is a common web server Y2K bug where
                    // the server will say a file is modified,
                    // then return the exact same last modified
                    // date.  In this case, treat the file as
                    // unmodified.
                    uc.disconnect();
                    upToDate = true;
                } else {
                    // Output the dates in the user's locale
                    // and time zone.
                    DateFormat formatter = 
                        DateFormat.getDateTimeInstance();
                    String cacheDate = 
                        formatter.format(new Date(oldModified));
                    String serverDate =
                        formatter.format(new Date(lastModified));
                    Cache.msgPrintln("cache.out_of_date", new Object[]{ url, cacheDate, serverDate});
                            
                    // Cached jar's out of date, so get the download size.
                    size = uc.getContentLength();
                }
            } else {
                // Unhandled response from server
                Cache.msgPrintln("cache.response_warning", new Object[] {String.valueOf(response), url});
            }
        }
        return upToDate;
    }

    // Download the jar file to the cache
    private CachedJarFile download() throws IOException {
        if (uc == null) {
            // Open the URL if we have not already done so
            uc = (HttpURLConnection)url.openConnection();
            uc.setUseCaches(false);
            uc.setAllowUserInteraction(false);
            uc = HttpUtils.followRedirects(uc);
            int response = uc.getResponseCode();
            if ((response < 200) || (response >= 300)) {
                // Failed to connect
                throw new IOException("Could not connect to " + url + " with response code " + response);
            }
            lastModified = uc.getLastModified();
            expiration = uc.getExpiration();
        }

        // Check that this url supports caching
        if ((lastModified == 0) && (expiration == 0)) {
            Trace.msgPrintln("cache.header_fields_missing");
            return null;
        }
        
        // Download the file
        Cache.msgPrintln("cache.downloading", new Object[] {url});
        try {
            return (CachedJarFile)AccessController.doPrivileged(new Cache.CacheIOAction() {
                public Object run() throws IOException {
                    // Find an unused filename for storing the Jar file
                    String filename = JarCache.generateCacheFileName(url);
                    dataFile = new File(JarCache.directory, filename + JarCache.DATA_FILE_EXT);
                    indexFile = new File(JarCache.directory, filename + Cache.INDEX_FILE_EXT);
                
                    //Before downloading the data file, mark the index file
                    //as incomplete so that the data file is not deleted before
                    //it is used.
                    markAsIncomplete();

                    // Decompress the Jar file into the cache
                    decompress();

                    // Authenticate the Jar file
                    CachedJarFile jar = authenticate();

                    //message to indicate new jar file is created in cache
                    Cache.msgPrintln("cache.cached_name", new Object[] {dataFile.getName()});
                
                    return jar;
                }
            });
        }// end try
        catch (PrivilegedActionException pae) {
            throw new IOException(pae.getMessage());
        }        
    }

    // Decompresses the Jar file into the cache
    private void decompress() throws IOException {
	boolean valid = false;
	ZipInputStream jarjarIn = null;
        ZipInputStream in = null;
        ZipOutputStream out = null;
        try {
            in = new ZipInputStream(new BufferedInputStream(uc.getInputStream()));

            // Open the output stream
            out = new ZipOutputStream(
                  new BufferedOutputStream(
                  new FileOutputStream(dataFile)));
            out.setLevel(JarCache.compression);
            
            ZipEntry entry = in.getNextEntry();

            // Check for .jarjar file
	    if (url.toString().toLowerCase().endsWith(JarCache.JARJAR_FILE_EXT)) {
		while (entry != null) {
		    if (entry.toString().toLowerCase().startsWith(JarCache.META_FILE_DIR)) {
			//Ignore meta-files inside a .jarjar file
			entry = in.getNextEntry();
		    } else if (! entry.toString().toLowerCase().endsWith(JarCache.JAR_FILE_EXT)) {
			//other than .jar files are not allowed in .jarjar 
			throw new IOException(ResourceHandler.getMessage("cache.jarjar.invalid_file"));
		    } else {
			//if jar is found break the loop
			break;
		    }
		}

		jarjarIn = in;						
                in = new ZipInputStream(in);
		entry = in.getNextEntry();
	    }

            // Decompress each entry
            byte[] buffer = new byte[2048];
            while (entry != null) {
                // It is expensive to create new ZipEntry objects
                // when compared to cloning the existing entry. 
                // We need to reset the compression size, since we 
                // are changing the compression ratio of the entry.  
                ZipEntry outEntry = (ZipEntry)entry.clone();
                outEntry.setCompressedSize(-1);
                out.putNextEntry(outEntry);

                int n;
                while((n = in.read(buffer, 0, buffer.length)) != -1) {
                    out.write(buffer, 0, n);
                }

                out.closeEntry();
                entry = in.getNextEntry();
            }

	    //make sure that only one jar file was found in .jarjar file
	    if(jarjarIn != null) {
		entry = jarjarIn.getNextEntry();
		if(entry != null) {
		    String msg = null;
		    if (!entry.toString().toLowerCase().endsWith(JarCache.JAR_FILE_EXT)) {
			msg = ResourceHandler.getMessage("cache.jarjar.invalid_file");
		    }
		    else {
			msg = ResourceHandler.getMessage("cache.jarjar.multiple_jar");
		    }
		    
		    throw new IOException(msg);
		}
	    }

	    valid = true;

        } finally {
            // Close the streams
	    if(jarjarIn != null) {
		jarjarIn.close();
	    }
            if (in != null) {
                in.close();
            }
            if (out != null) {
                out.close();
            }
	    if(!valid) {
		dataFile.delete();
		indexFile.delete();			
	    }
        }
    }

    //marks the index file as incomplete
    private void markAsIncomplete() throws IOException {
	RandomAccessFile raf = null;
	try {
	    //mark the file as incomplete
	    raf = new RandomAccessFile(indexFile, "rw");
	    raf.writeByte(Cache.INCOMPLETE);
	} finally {
	    if (raf != null) {
		raf.close();
	    }
	}
    }

    // Authenticates the cached Jar file and saves the results
    private CachedJarFile authenticate() throws IOException {
        // Open the Jar file
	boolean valid = false;
        JarFile jar = new JarFile(dataFile);
        ObjectOutputStream out = null;
        RandomAccessFile raf = null;
        ArrayList signers = new ArrayList();
        HashMap signerMap = new HashMap();
	Manifest manifest = null;

        try {
            // Write the header
            raf = new RandomAccessFile(indexFile, "rw");
            raf.writeByte(Cache.INCOMPLETE);
            raf.writeUTF(url.toString());
            raf.writeLong(lastModified);
            raf.writeLong(expiration);
            
	    // Add additional headers for Java Plug-in
	    raf.writeInt(FileType.JAR);
	    raf.writeUTF(version.getVersionAsString());

	    // Open the output stream
	    out = new ObjectOutputStream(
		  new BufferedOutputStream(
		  new FileOutputStream(raf.getFD())));

            manifest = jar.getManifest();
	    //If no manifest, no need to worry about authentication
	    if(manifest != null) {
		// Jar file authentication
		byte[] buffer = new byte[2048];
		Enumeration entries = jar.entries();

		while (entries.hasMoreElements()) {
		    JarEntry entry = (JarEntry)entries.nextElement();
		    String name = entry.getName();
                
		    // Skip meta-files
		    if (name.toLowerCase().startsWith(JarCache.META_FILE_DIR) &&
			!name.equals(JarIndex.INDEX_NAME)) {
			continue;
		    }
                
		    // Authenticate the entry.  To do so, we must read the
		    // entire entry through the JarVerifier.VeriferStream
		    InputStream in = null;
		    try {
			in = jar.getInputStream(entry);
			while (in.read(buffer, 0, buffer.length) != -1) {
			    // Do nothing
			}
		    } finally {
			if (in != null) {
			    in.close();
			}
		    }
                
		    // Get the certificates for this entry
		    Certificate[] certs = entry.getCertificates();
		    if ((certs != null) && (certs.length > 0)) {
			int[] entrySigners = new int[certs.length];
			for (int i = 0; i < certs.length; i++) {

			    // Add the certificate to the list of 
			    // signers for this Jar file.
			    int signer = signers.indexOf(certs[i]);
			    if (signer == -1) {
				signer = signers.size();
				signers.add(certs[i]);
			    }

			    // Add the certificate to the list of 
			    // signers for this entry.
			    entrySigners[i] = signer;
			}
			signerMap.put(name, entrySigners);
		    }
                
		    // Now that the signature has been verified for
		    // this entry, remove its attributes from the
		    // manifest.
		    removeSignature(manifest, name);
		}

		// Write the manifest
		ByteArrayOutputStream bos = new ByteArrayOutputStream();
		manifest.write(bos);
		bos.close();
		byte[] manifestBytes = bos.toByteArray();
		out.writeInt(manifestBytes.length);
		out.write(manifestBytes);

		if(!signers.isEmpty()) {
		    // Write the list of signers
		    out.writeInt(signers.size());
		    Iterator iterator = signers.iterator();
		    while (iterator.hasNext()) {
			out.writeObject(iterator.next());
		    }
            
		    // Write the map of entry name and signers
		    BufferedWriter writer = 
			new BufferedWriter(new OutputStreamWriter(out));
		    Iterator keys = signerMap.keySet().iterator();
		    String lastPath = null;
		    while (keys.hasNext()) {
			String name = (String)keys.next();
			int[] entrySigners = (int[])signerMap.get(name);
			if (name.indexOf("/") != -1) {
			    // Compress path names.  We use a very 
			    // rudimentary but effective scheme.  If the
			    // last entry had the same path, we just store
			    // a "/" at the beginning of the entry name.
			    String path =
				name.substring(0, name.lastIndexOf("/"));
			    if ((lastPath != null) && 
				path.equals(lastPath)) {
				name = name.substring(path.length());
			    }
			    lastPath = path;
			}

			// Write the entry name
			writer.write(name);
			writer.newLine();

			//Write the number of signers, and their names
			String line = String.valueOf(entrySigners.length);
			for (int i = 0; i < entrySigners.length; i++) {
			    line += " " + entrySigners[i];
			}
			writer.write(line, 0, line.length());
			writer.newLine();
		    }
		    //flush the data written by writer
		    writer.flush();
		}else {
		    //Write list of signers length as 0
		    out.writeInt(0);
		}
	    }else {
		//Write manifest length as 0
		out.writeInt(0);
	    }

	    //flush the data
	    out.flush();

	    // Mark the file as usable
	    raf.seek(0);
	    raf.writeByte(Cache.VERSION);
	    valid = true;

	} finally {
	    jar.close();
	    if (out != null) {
		out.close();
	    }
	    if (raf != null) {
		raf.close();
	    }
	    if(!valid) {
		dataFile.delete();
		indexFile.delete();			
	    }
	} 

	Certificate[] certificates = new Certificate[signers.size()];
	certificates = (Certificate[])signers.toArray(certificates);

        return new CachedJarFile(dataFile, certificates, signerMap, manifest);
    }
    
    // Reads cached authentication data for this Jar file
    private CachedJarFile authenticateFromCache() throws IOException {
        HashMap signerMap = new HashMap();
        Certificate[] certificates = null;
        Manifest manifest = null;
        RandomAccessFile raf = new RandomAccessFile(indexFile, "r");
        ObjectInputStream in = null;
        try {
            // Skip header information
            raf.readByte();
            raf.readUTF();
            raf.readLong();
            raf.readLong();
	    raf.readInt();
	    raf.readUTF();
            
            // Create an input stream
            in = new ObjectInputStream(
                 new BufferedInputStream(
                 new FileInputStream(raf.getFD())));
            
            // Read the manifest
            int manifestLength = in.readInt();
	    if(manifestLength > 0) {
		byte[] manifestBytes = new byte[manifestLength];
		in.readFully(manifestBytes, 0, manifestLength);
		ByteArrayInputStream bin = 
		    new ByteArrayInputStream(manifestBytes);
		manifest = new Manifest();
		manifest.read(bin);
            
		// Read the certificate array     
		int numCerts = in.readInt();
		if(numCerts > 0) {
		    certificates = new Certificate[numCerts];
		    try {
			for (int i = 0; i < numCerts; i++) {
			    certificates[i] = (Certificate)in.readObject();
			}
		    } catch (ClassNotFoundException e) {
			throw new IOException("Error reading certificates");
		    }

		    // Read the signer map
		    BufferedReader reader = new BufferedReader(
					    new InputStreamReader(in));
		    String line = reader.readLine();
		    String lastPackage = null;
		    while ((line != null) && (!line.equals(""))) {
			// Read the entry name
			String name = line;
			if (name.startsWith("/")) {
			    name = lastPackage + name;
			} else {
			    int lastSlash = name.lastIndexOf("/");
			    if (lastSlash != -1) {
				lastPackage = name.substring(0, lastSlash);
			    }
			}
			line = reader.readLine();
			StringTokenizer tokenizer =
			    new StringTokenizer(line, " ", false);
			int numSigners = Integer.parseInt(tokenizer.nextToken());
			int[] signers = new int[numSigners];
			for (int i = 0; i < numSigners; i++) {
			    signers[i] = Integer.parseInt(tokenizer.nextToken());
			}
			signerMap.put(name, signers);
			line = reader.readLine();
		    }
            
		    Cache.msgPrintln("cache.cert_load", new Object[]{ url });
		}
	    }
	} finally {
	    raf.close();
	    if (in != null) {
		in.close();
	    }
	}

        return new CachedJarFile(dataFile, certificates, signerMap, manifest);
    }


    // Removes the signature for the given entry from the manifest
    private void removeSignature(Manifest manifest, String name) {
        Attributes atts = manifest.getAttributes(name);
        if (atts != null) {
	    // Bug - stanleyh 4/29/01
	    // We are modifying the Attributes object
	    // while enumerating it, and it results
	    // in ConcurrentModificationException.
	    // Solution - clone it first before
	    // enumeration.    
	    Set keySet = ((Attributes) atts.clone()).keySet();
            Iterator keys = keySet.iterator();
            while (keys.hasNext()) {
                Attributes.Name keyName =
                    (Attributes.Name)keys.next();
                String key = keyName.toString();
                if (key.endsWith("-Digest") ||
                    (key.indexOf("-Digest-") != -1)) {
                    atts.remove(keyName);
                }
                if (key.equals("Magic:")) {
                    atts.remove(keyName);
                }
            }
            if (atts.isEmpty()) {
                manifest.getEntries().remove(name);
            }
        }
    }
}





