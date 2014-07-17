/*
 * %W% %E%
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.cache;
import java.net.URL;
import java.util.Date;
import java.security.cert.Certificate;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;

/**
 * Wrapper class to contain a contents of an entry
 *
 * @version %I%, %G%
 */
public class DiskCacheEntry {
    
    // Fields
    private char         _type;              // Category of resource (Resource, JNLP, ...)
    private URL          _location;          // Location of entry
    private String       _versionId;         // Version ID of entry (NULL if no version info)
    private long         _timestamp;         // Time-stamp for entry
    private Certificate[] _certificateChain; // Certificate chain stored with this entry
    private File         _file;              // Main File
    private File         _directory;         // Directory File
    private File         _certificateFile;   // Directory File
    private File         _mappedBitmap;      // Filename for a mapped image belong to this entry
    private File         _muffinTag;         // For muffin support, not used with main cache
    
    
    /** Creates empty entry */
    public DiskCacheEntry() {
        this((char)0, null, null, null, 0);
    }
    
    public DiskCacheEntry(char type, URL location, String versionId, File file, long timestamp) {
        this(type, location, versionId, file, timestamp, null, null, null);
    }
    
    public DiskCacheEntry(char category, URL location, String versionId, File file, long timestamp,
                          File certFile, File directory, File mappedFile) {
        this (category, location, versionId, file, timestamp, certFile, directory, mappedFile, null);
    }

    public DiskCacheEntry(char category, URL location, String versionId, File file, long timestamp,
                          File certFile, File directory, File mappedFile, File muffinTag) {
        _type      = category;
        _location  = location;
        _versionId = versionId;
        _timestamp = timestamp;
        _file = file;
        _certificateFile = certFile;
        _directory = directory;
        _mappedBitmap = mappedFile;
        _certificateChain = null;
        _muffinTag = muffinTag;
    }

    
    public char getType() { return _type; }
    public void setType(char type) { _type = type; }
    
    public URL  getLocation() { return _location; }
    public void setLocataion(URL location) { _location = location; }
    
    public long getTimeStamp() { return _timestamp; }
    public void setTimeStamp(long timestamp) { _timestamp = timestamp; }
    
    public Certificate[] getCertificateChain()          { return _certificateChain; }
    public void setCertificateChain(Certificate[] chain) { _certificateChain = chain; }
    
    public File getMuffinTagFile() { return _muffinTag; }
    public void setMuffinTagFile(File muffinTag) { _muffinTag = muffinTag; }
    
    public String getVersionId() { return _versionId; }
    public void setVersionId(String versionId) { _versionId = versionId; }
    
    public File getFile() { return _file; }
    public void setFile(File file) { _file = file; }
    
    public File getDirectory() { return _directory; }
    public void setDirectory(File dir) { _directory = dir; }
    
    public File getCertificateFile() { return _certificateFile; }
    
    public File getMappedBitmap() { return _mappedBitmap; }
    public void setMappedBitmap(File bitmap) { _mappedBitmap = bitmap; }
    
    public long getLastAccess() { return (_file == null) ? 0 : _file.lastModified(); }
    // Update last access
    public void setLastAccess(long timestamp) {
        if (_file != null) {
            _file.setLastModified(timestamp);
        }
    }
    
    /** Returns true if the entry is empty */
    public boolean isEmpty() { return _location == null; }
    
    /** Returns size of entry */
    public long getSize() {
        if (_directory != null && _directory.isDirectory()) {
            // Compute size of native libraries
            long size = 0;
            File[] children = _directory.listFiles();
            for (int i = 0; i < children.length; i++) {
                size += children[i].length();
            }
            return size;
        } else {
            return _file.length();
        }
    }
    
    /** toString method */
    public String toString() {
        if (isEmpty()) {
            return "DisckCacheEntry[<empty>]";
        } else {
            return    "DisckCacheEntry[" +
                _type + ";" +
                _location + ";" +
                _versionId +  ";" +
                new Date(_timestamp) + ";" +
                _file + ";" +
                _directory + "]";
        }
    }
}


