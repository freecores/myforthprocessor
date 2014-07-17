/*
 * @(#)InstallCache.java	1.73 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.cache;

import java.io.*;
import java.net.*;
import java.text.*;
import java.util.*;
import java.util.jar.*;
import java.util.zip.*;
import java.security.cert.Certificate;
import sun.net.www.protocol.jar.Handler;
import java.security.AccessControlException;
import java.security.*;
import com.sun.javaws.*;
import com.sun.javaws.util.BASE64Encoder;
import com.sun.javaws.debug.*;
import com.sun.javaws.jnl.*;
import com.sun.javaws.ConfigProperties;
import com.sun.javaws.LocalApplicationProperties;
import com.sun.javaws.exceptions.*;
import com.sun.javaws.security.JavawsHttpsCertStore;

/**
 *  InstallCache
 *
 * A Cache for resources loaded and cached on the
 * local computer. The is mostly a policy object.
 * The nity-gritty of managing the cache is delegated
 * to the DiskCache object.
 *
 * @version 1.52, 10/21/00
 */


public class InstallCache {
    static private InstallCache _cache = null;
    static private DiskCache _diskcache = null;
    static private DiskCache _muffincache = null;
    /** File name of the file used to indicated the last time the
     * cache was accessed (that is a file was added/removed).
     */
    private static final String LAST_ACCESSED_FILE_NAME = "lastAccessed";
    
    /**
     * Extension of indirect files in the cache.
     */
    private static final String INDIRECT_EXTENSION = ".ind";
    
    private File baseDir = null;

   
    
    /**
     * Determines if the cache should be updated. This is used
     * during extension installers download, in order not to
     * update the cache
     */
    private boolean _cacheUpdateFlag = true;
    
    /**
     * Maps from path (either URL, or path on file system) to
     * LocalApplicationProperties.
     */
    private HashMap _loadedProperties;
    
    /**
     *  The disk cache object implements the real cache
     */
    
    
    /** Returns singleton instance of the cache */
    static public InstallCache getCache() {
        if (_cache == null) {
            synchronized(InstallCache.class) {
                if (_cache == null) {
                    _cache = new InstallCache();
                }
            }
        }
        return _cache;
    }

   

    // refresh Cache Location
    // returns true if Cache location refreshed, else false
    static public boolean refreshCacheLocation() {
	if (Globals.TraceCache) {
	    Debug.println("_cache.baseDir: " + _cache.baseDir);
	    Debug.println("_cache.getCacheBaseDir(): " + _cache.getCacheBaseDir());
	}
	// check if Cache Location changed
	if (!_cache.baseDir.equals(_cache.getCacheBaseDir())) {
	    synchronized(InstallCache.class) {
		_cache = new InstallCache();
	    }
	    return true;
	}
	return false;
    }
    
    /** Constructor is private. Cache is accessed through the getCache factory method */
    private InstallCache() {
        // Setup base directory. Do this during initialization, so this does not change
        // during midrun of an application.
        baseDir = getCacheBaseDir();
        _diskcache = new DiskCache(baseDir);
        File muffinbasedir = getMuffinCacheBaseDir();
        _muffincache = new DiskCache(muffinbasedir);
        _loadedProperties = new HashMap();
    }

    private File getMuffinCacheBaseDir() {
        String base = ConfigProperties.getInstance().getMuffinDir();
        File baseDir = new File(base);
        if (Globals.TraceCache) {
            Debug.println("Cache Dir = " + baseDir );
        }
        return baseDir ;
    }
         
    private File getCacheBaseDir() {

        String base = ConfigProperties.getInstance().getCacheDir();

        File baseDir = new File(base);
        if (!baseDir .exists()) {
            baseDir .mkdirs();
        }
        if (Globals.TraceCache) {
            Debug.println("Cache Dir = " + baseDir );
        }
        return baseDir ;
    }
    
    /**
     * Returns the time (as a long) that the cache was last accessed.
     */
    public long getLastAccessed() {
        return _diskcache.getLastUpdate();
    }

    public synchronized static DiskCache getMuffinCache() {
        if (_muffincache == null) {
            getCache();
        }
        return _muffincache;
    }

    public synchronized static DiskCache getDiskCache() {
        if (_diskcache == null) {
            getCache();
        }
        return _diskcache;
    }

    /**
     * Removes all the related entry for all applications installed in the cache
     */
    public void remove() {
	Iterator cachedApps = _cache.getCachedApplications();
        if (cachedApps != null) {
            while (cachedApps.hasNext()) {
		CachedApplication app = (CachedApplication)cachedApps.next();
		LaunchDesc ld = app.getLaunchDescriptor();
		LocalApplicationProperties lap = 
		    app.getLocalApplicationProperties();
		if (ld != null) {
		    remove(lap, ld);
		}
	    }
	}
        getDiskCache().uninstallCache();
        getMuffinCache().uninstallCache();

	// remove user added https-cert
	JavawsHttpsCertStore.removeJavawsHttpsCert(); 

    }
    
    /**
     * Removes all the related entry for <code>ld</code> from the cache.
     */
    public void remove(LocalApplicationProperties lap, LaunchDesc ld) {
        InformationDesc id = ld.getInformation();
	LocalInstallHandler lih = LocalInstallHandler.getInstance();
        ResourcesDesc cbd = ld.getResources();

	// first uninstall shortcuts
	if (lap.isLocallyInstalled() && lih != null) {
	    lih.uninstall(ld, lap);
	    try {
		lap.store();
	    } catch (IOException ioe) {
		Debug.ignoredException(ioe);
	    }
	}

	// remove custom splash if present
	SplashScreen.removeCustomSplash(ld);

        // next Remove images and mapped images.
        if (id != null) {
            IconDesc[] icons = id.getIcons();
            if (icons != null) {
                for (int i = 0; i < icons.length; i++) {
                    URL url = icons[i].getLocation();
                    String version = icons[i].getVersion();
                    removeEntries(DiskCache.RESOURCE_TYPE, url, version);
                }
            }
        }
        
        // Resources
        if (cbd != null) {
            JARDesc[] cps = cbd.getEagerOrAllJarDescs(true);
            if (cps != null) {
                for (int counter = cps.length - 1; counter >= 0; counter--) {
                    URL location = cps[counter].getLocation();
                    String version = cps[counter].getVersion();
                    removeEntries(DiskCache.RESOURCE_TYPE, location, version);
                }
            }
        }
        
        // Indirect file.
        if (lap != null) {
	    if (lap instanceof DefaultLocalApplicationProperties) {
		((DefaultLocalApplicationProperties)lap).removeIndirectLaunchFile();
	    }
        }
        
        if (ld.getCanonicalHome() != null) {
            removeEntries(DiskCache.APPLICATION_TYPE, ld.getCanonicalHome(), null);
        }
    }
    
    /** Removes all entries matching a given url and Version String */
    private void removeEntries(char type, URL location, String version) {
        if (location == null) return;
        try {
            DiskCacheEntry[] dces = _diskcache.getCacheEntries(
                type,
                location,
                version,
                true);
            for(int i = 0; i < dces.length; i++) {
                _diskcache.removeEntry(dces[i]);
            }
            
        } catch (IOException ioe) {
            Debug.ignoredException(ioe);
        }
    }
    
    /**
     * Returns the location of a JNLP file given a codebase
     */
    public File getCachedLaunchedFile(URL codebase) throws IOException {
        DiskCacheEntry dce = _diskcache.getCacheEntry(DiskCache.APPLICATION_TYPE, codebase, null);
        return (dce == null) ? null : dce.getFile();
    }
    
    /**
     * Returns an enumeration of the files in the cache. The elements of
     * the enumerator will be of type CachedApplication.
     */
    public Iterator getCachedApplications() {
        final ArrayList al = new ArrayList();
        try {
            _diskcache.visitDiskCache(DiskCache.APPLICATION_TYPE, new DiskCache.DiskCacheVisitor() {
                        public void visitEntry(DiskCacheEntry dce) {
                            al.add(new DefaultCachedApplication(dce));
                        }
                    });
        } catch(IOException ioe) {
            // Just ignore this - not critical
            Debug.ignoredException(ioe);
        }
        return al.iterator();
    }
    
    /**
     * Returns the LocalApplciationProperties for the launch descriptor loaded from the file
     * pointed into the cache
     */
    public LocalApplicationProperties getLocalApplicationProperties(String path, LaunchDesc ld) {
        try {
            DiskCacheEntry dce = _diskcache.getCacheEntryFromFile(new File(path));
            boolean isApplicationDesc = (dce.getType() == DiskCache.APPLICATION_TYPE);
            if (dce != null) {
                return getLocalApplicationProperties(dce.getLocation(), dce.getVersionId(), ld, isApplicationDesc);
            } else {
                return null;
            }
        } catch(IOException ioe) {
            Debug.ignoredException(ioe);
            return null;
        }
    }
    
    /**
     * Returns the LocalApplicationProperties for the launch descriptor loaded
     * from <code>codebase</code>. Unless home is null, this will always return
     * a non-null value.
     */
    public LocalApplicationProperties getLocalApplicationProperties(URL codebase, LaunchDesc ld) {
        return getLocalApplicationProperties(codebase, null, ld, true);
    }
    
    /**
     * Returns the LocalApplicationProperties for the launch descriptor loaded
     * from <code>codebase</code>. Unless home is null, this will always return
     * a non-null value.
     */
    public LocalApplicationProperties getLocalApplicationProperties(URL codebase, String versionId,
                                                                    LaunchDesc ld, boolean isApplicationDesc) {
        if (codebase == null) {
            return null;
        }
        
        // Generating the hashcode for a URL requires a nameserver lookup, i.e., we
        // need to be online. This should fix the problem.
        String key = codebase.toString().intern() + "?" + versionId;
        
        LocalApplicationProperties props;
        synchronized(_loadedProperties) {
            props = (LocalApplicationProperties)_loadedProperties.get(key);
            if (props == null) {
                props = new DefaultLocalApplicationProperties(codebase, versionId, ld, isApplicationDesc);
                _loadedProperties.put(key, props);
            } else {
		props.refreshIfNecessary();
	    }
        }
        return props;
    }
    
    public LaunchDesc getLaunchDesc(URL codebase, String versionId) {
        try {
            DiskCacheEntry dce = _diskcache.getCacheEntry(DiskCache.APPLICATION_TYPE, codebase, versionId);
            if (dce != null) {
                try {
                    return LaunchDescFactory.buildDescriptor(dce.getFile());
                } catch(Exception e) {
                    return null;
                }
            }
        } catch(IOException ioe) {
            // Not critical
            Debug.ignoredException(ioe);
        }
        return null;
    }
    
    /** Returns a new directory where to install an extension */
    public String getNewExtensionInstallDirectory() throws IOException {
        String dir = getCacheBaseDir().getAbsolutePath() + File.separator + ".ext";
        String tempname = null;
        int count = 0;
        do {
            tempname =  dir + File.separator + "E" + (new Date().getTime()) + File.separator;
            // Create path
            File cacheDir = new File(tempname);
            if (!cacheDir.mkdirs()) {
                tempname = null;
            }
            Thread.yield(); // Just to improve scheduling.
        } while(tempname == null && ++count < 50); // max 50 attempts
        if (tempname == null) {
            throw new IOException("Unable to create temp. dir for extension");
        }
        return tempname;
    }
    
    /**
     * Returns a file in the cache that is unique, or null if there is
     * an error in creating one.
     */
    private String createUniqueIndirectFile() throws IOException {
        String dir = getCacheBaseDir().getAbsolutePath() + File.separator +
            "indirect";
        File parent = new File(dir);
        
        parent.mkdirs();
        File file = File.createTempFile("indirect", INDIRECT_EXTENSION,
                                        parent);
        return file.getAbsolutePath();
    }
    
    /**
     * Returns an InputStream to the contents of local application properties
     * for the URL <code>url</code>.
     */
    private Properties getLocalApplicationPropertiesStorage(DefaultLocalApplicationProperties lap) {
        Properties props = new Properties();
        try {
            URL url = lap.getLocation();
            String versionId = lap.getVersionId();
            if (url != null) {
                char type = (lap.isApplicationDescriptor()) ? DiskCache.APPLICATION_TYPE : DiskCache.EXTENSION_TYPE;
                byte[] data = _diskcache.getEntryContent(type,
                                                         DiskCache.LAP_FILE_TAG,
                                                         url,
                                                         versionId);
                if (data != null) {
                    props.load(new ByteArrayInputStream(data));
                }
            }
        } catch(IOException ioe) {
            Debug.ignoredException(ioe);
        }
        return props;
    }
    
    /**
     * Saves the LocalApplicationProperties to the cache.
     */
    private void putLocalApplicationPropertiesStorage(DefaultLocalApplicationProperties lap, Properties props) throws IOException {
        ByteArrayOutputStream bos = new ByteArrayOutputStream();
        try {
	    props.store(bos, "LAP");
	} catch (IOException ioe) {}
        bos.close();
        char type = (lap.isApplicationDescriptor()) ? DiskCache.APPLICATION_TYPE : DiskCache.EXTENSION_TYPE;
        _diskcache.storeAtomic(type,
                               DiskCache.LAP_FILE_TAG,
                               lap.getLocation(),
                               lap.getVersionId(),
                               bos.toByteArray());
    }
    
    
    /**
     * This caches implementation of <code>CachedApplication</code>. The
     * LaunchDesc and LocalApplicationProperties are lazily created when
     * asked for.
     */
    private class DefaultCachedApplication implements CachedApplication {
        /**
         * Path of the LaunchDesc.
         */
        private DiskCacheEntry _dce;  // The LaunchDesc entry in a cache
        private LaunchDesc _descriptor;
        private LocalApplicationProperties _lap;
        /**
         * A bitmask indicating what has/hasn't been loaded. 1 indicates the
         * LaunchDesc has been loaded and 2 indicates the
         * LocalApplicationProperties have been loaded.
         */
        private int _state;
        
        DefaultCachedApplication(DiskCacheEntry dce) {
            _dce = dce;
        }
        
        public LaunchDesc getLaunchDescriptor() {
            if ((_state & 1) == 0) {
                synchronized(this) {
                    loadLaunchDescriptorIfNecessary();
                }
            }
            return _descriptor;
        }
        
        public URL getCodebase() {
            return _dce.getLocation();
        }
        
        public String getVersionId() {
            return _dce.getVersionId();
        }
        
        public LocalApplicationProperties getLocalApplicationProperties() {
            if ((_state & 2) == 0) {
                synchronized(this) {
                    loadLocalApplicationPropertiesIfNecessary();
                }
            }
            return _lap;
        }
        
        private synchronized void loadLaunchDescriptorIfNecessary() {
            if ((_state & 1) == 0) {
                _state |= 1;
                try {
                    _descriptor = LaunchDescFactory.buildDescriptor(_dce.getFile());
                } catch (IOException ioe) {
                    Debug.ignoredException(ioe);
                } catch (BadFieldException bfe) {
                    Debug.ignoredException(bfe);
                } catch (MissingFieldException mfe) {
                    Debug.ignoredException(mfe);
                } catch(JNLParseException jnlpe) {
                    Debug.ignoredException(jnlpe);
                }
            }
        }
        
        private synchronized void loadLocalApplicationPropertiesIfNecessary() {
            if ((_state & 2) == 0) {
                loadLaunchDescriptorIfNecessary();
                _state |= 2;
                _lap = InstallCache.this.
                    getLocalApplicationProperties(_dce.getLocation(), getLaunchDescriptor());
            }
        }
    }
    
    
    // This are really private to the DefaultLocalApplicationProperties. However,
    // CodeGuide insist that it is not legal to have static private fields in
    // inner classes, so put here for now.
    private static final String REBOOT_NEEDED_KEY = "_default.rebootNeeded";
    private static final String FORCED_UPDATE_CHECK_KEY = "_default.forcedUpdateCheck";
    private static final String NATIVELIB_DIR_KEY = "_default.nativeLibDir";
    private static final String INSTALL_DIR_KEY = "_default.installDir";
    private static final String LAST_ACCESSED_KEY = "_default.lastAccessed";
    private static final String LAUNCH_COUNT_KEY = "_default.launchCount";
    private static final String ASK_INSTALL_KEY = "_default.askedInstall";
    private static final String LOCALLY_INSTALLED_KEY = "_default.locallyInstalled";
    private static final String INDIRECT_PATH_KEY = "_default.indirectPath";
    
    /**
     * This caches implementation of the LocalApplicationProperties.
     */
    private class DefaultLocalApplicationProperties implements LocalApplicationProperties {
        /**
         * Descriptor we represent.
         */
        private LaunchDesc _descriptor;
        
        /**
         * How we store information.
         */
        private Properties _properties;
        /**
         * Location for application/extension
         */
        private URL _location;
        /**
         * VersionId for application/extension
         */
        private String _versionId;
        /**
         * Last time the disk was accessed.
         */
        private long _lastAccessed;
        /*
         * Application or Extension descriptor
         */
        private boolean _isApplicationDescriptor;
        /**
         * True if the properties have changed without being saved.
         */
        private boolean _dirty;
        
        private DefaultLocalApplicationProperties(URL location,
						  String versionId,
						  LaunchDesc descriptor,
						  boolean isApplicationDescriptor) {
            _descriptor = descriptor;
            _location = location;
            _versionId = versionId;
            _isApplicationDescriptor = isApplicationDescriptor;
            _properties = getLocalApplicationPropertiesStorage(this);
            
        }
        
        /**
         * Returns the codebase for the application
         */
        public URL getLocation() {
            return _location;
        }
        
        /**
         * Returns the codebase for the application
         */
        public String getVersionId() {
            return _versionId;
        }
        
        /**
         * Returns the LaunchDescriptor these properties are associated with.
         */
        public LaunchDesc getLaunchDescriptor() {
            return _descriptor;
        }
        
        /**
         * Dates this application was last accessed (run).
         */
        public void setLastAccessed(Date date) {
            put(LAST_ACCESSED_KEY, date.toString());
        }
        
        public Date getLastAccessed() {
            return getDate(LAST_ACCESSED_KEY);
        }
        
        public void incrementLaunchCount() {
            int count = getLaunchCount();
            
            put(LAUNCH_COUNT_KEY, Integer.toString(++count));
        }
        
        /**
         * Number of times this application was launched. This will be
         * at least 1.
         */
        public int getLaunchCount() {
            return getInteger(LAUNCH_COUNT_KEY);
        }
        
        /**
         * Indicates if the user has been prompted if they want to do an
         * install.
         */
        public void setAskedForInstall(boolean askedForInstall) {
            put(ASK_INSTALL_KEY, new Boolean(askedForInstall).toString());
        }
        
        public boolean getAskedForInstall() {
            return getBoolean(ASK_INSTALL_KEY);
        }
        
        public void setRebootNeeded(boolean reboot) {
            put(REBOOT_NEEDED_KEY, new Boolean(reboot).toString());
        }
        
        public boolean isRebootNeeded() {
            return getBoolean(REBOOT_NEEDED_KEY);
        }
	
        public void setLocallyInstalled(boolean installed) {
            put(LOCALLY_INSTALLED_KEY, new Boolean(installed).toString());
        }
        
        public boolean isLocallyInstalled() {
            return getBoolean(LOCALLY_INSTALLED_KEY);
        }
        
        public boolean forceUpdateCheck() {
            return getBoolean(FORCED_UPDATE_CHECK_KEY);
        }
        
        public void setForceUpdateCheck(boolean state) {
            put(FORCED_UPDATE_CHECK_KEY, new Boolean(state).toString());
        }
        
        public boolean isApplicationDescriptor() { return _isApplicationDescriptor; }
        public boolean isExtensionDescriptor()   { return !_isApplicationDescriptor; }
        
        public String getNativeLibDirectory() {
            return get(NATIVELIB_DIR_KEY);
        }
        
        public String getInstallDirectory() {
            return get(INSTALL_DIR_KEY);
        }
        
        public void setNativeLibDirectory(String path) {
            put(NATIVELIB_DIR_KEY, path);
        }
        
        public void setInstallDirectory(String path) {
            put(INSTALL_DIR_KEY, path);
        }
        
        
        public void put(String key, String value) {
            synchronized(DefaultLocalApplicationProperties.this) {
                if (value == null) {
                    _properties.remove(key);
                }
                else {
                    _properties.put(key, value);
                }
                _dirty = true;
            }
        }
        
        public String get(String key) {
            synchronized(DefaultLocalApplicationProperties.this) {
                return (String)_properties.get(key);
            }
        }
        
        /**
         * Returns an integer representation of <code>key</code>, returns 0
         * if not found, or there is an error in parsing the string.
         */
	public int getInteger(String key) {
            String value = get(key);
            
            if (value == null) {
                return 0;
            }
            int count = 0;
            try {
                count = Integer.parseInt(value);
            } catch (NumberFormatException nfe) {
                count = 0;
            }
            return count;
        }
        
        /**
         * Returns the boolean representation of <code>key</code>. This will
         * return false if <code>key</code> is not defined.
         */
        public boolean getBoolean(String key) {
            String value = get(key);
            
            if (value == null) {
                return false;
            }
            return Boolean.valueOf(value).booleanValue();
        }
        
        /**
         * Returns the date representation of <code>key</code>. This will
         * return null if <code>key</code> is not defined, or there is a parse
         * error.
         */
        public Date getDate(String key) {
            String value = get(key);
            
            if (value == null) {
                return null;
            }
            try {
                DateFormat df = DateFormat.getDateTimeInstance();
                return df.parse(value);
            } catch (ParseException pe) {
                return null;
            }
        }
        
        /**
         * Saves the properties.
         */
        synchronized public void store() throws IOException {
            putLocalApplicationPropertiesStorage(this, _properties);
            _dirty = false;
            
        }
        
        public boolean doesNewVersionExist() {
            synchronized(DefaultLocalApplicationProperties.this) {
                long cache = _diskcache.getLastUpdate();
                
                if (cache == 0) {
                    return false;
                }
                if (cache > _lastAccessed) {
                    return true;
                }
            }
            return false;
        }
        
        /**
         * Reloads the state of the receiver.
         */
        public void refreshIfNecessary() {
            synchronized(DefaultLocalApplicationProperties.this) {
                if (!_dirty && doesNewVersionExist()) {
                    refresh();
                }
            }
        }
        
        /**
         * Reloads the state of the receiver.
         */
        public void refresh() {
            synchronized(DefaultLocalApplicationProperties.this) {
                Properties props = getLocalApplicationPropertiesStorage(this);
                _properties = props;
                _dirty = false;
            }
        }
        
        /**
         * Returns the path to the indirect launch file name. Use
         * <code>create</code> to indicate if the file should be created if
         * it currently does not exist.
         */
        public String getIndirectLauchFilePath(boolean create) throws IOException {
            String path;
            synchronized(DefaultLocalApplicationProperties.this) {
                path = get(INDIRECT_PATH_KEY);
		// check to make sure if the file exist
		if (path != null) {
		    File f = new File(path);
		    if (f.exists() && !f.isDirectory() && f.canRead()) return path;
		}
		// create the file if it should be created
                if (create) {
                    path = createIndirectLaunchFile();
		    return path;
                }
		// the file does not exist
		put(INDIRECT_PATH_KEY, null);
            }
            return null;
        }
        
        /**
         * Creates the file that indirectly points to the JNL file on the
         * local file system. This will return the path to the file.
         */
        private String createIndirectLaunchFile() throws IOException {
            DiskCacheEntry dce = _diskcache.getCacheEntry(DiskCache.APPLICATION_TYPE, _location, null);
            
            if (dce == null || dce.isEmpty()) {
                // PENDING: show an error here?
                throw new IOException("no launch descriptor path!");
            }
            // Give the cache ten tries.
            for (int counter = 0; counter < 10; counter++) {
                try {
                    String path = createUniqueIndirectFile();
                    
                    if (path != null) {
                        Writer fw = new FileWriter(path);
                        fw.write(dce.getFile().getAbsolutePath());
                        fw.close();
                        put(INDIRECT_PATH_KEY, path);
			try {
			    store();
			} catch (IOException ioe) {}
                        return path;
                    }
                } catch (IOException ioe) {
                    Debug.ignoredException(ioe);
                }
            }
            // give up
            throw new IOException("couldn't create indirect file");
        }
        
        /**
         * Removes the indirect launch file, assuming it has been created.
         */
        private void removeIndirectLaunchFile() {
            refreshIfNecessary();
            try {
                String path = getIndirectLauchFilePath(false);
                if (path != null) {
                    new File(path).delete();
                }
            } catch (IOException ioe) {}
            put(INDIRECT_PATH_KEY, null);
            try {
                store();
            } catch (IOException ioe) {}
        }
    }
}


