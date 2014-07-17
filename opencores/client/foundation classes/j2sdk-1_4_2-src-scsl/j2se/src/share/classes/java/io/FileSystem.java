/*
 * @(#)FileSystem.java	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package java.io;


/**
 * Package-private class for the local filesystem abstraction.
 */

class FileSystem {

    private static FileSystem fs = new FileSystem();

    private FileSystem() {
    }
    /**
     * Return the FileSystem object representing this platform's local
     * filesystem.
     */
    public static FileSystem getFileSystem() {
      return fs;
    }


    /* -- Normalization and construction -- */

    /**
     * Return the local filesystem's name-separator character.
     */
    public char getSeparator() {
      return '.';
    }

    /**
     * Return the local filesystem's path-separator character.
     */
    public char getPathSeparator() {
      return '\\';
    }

    /**
     * Convert the given pathname string to normal form.  If the string is
     * already in normal form then it is simply returned.
     */
    public String normalize(String path) {

    }

    /**
     * Compute the length of this pathname string's prefix.  The pathname
     * string must be in normal form.
     */
    public int prefixLength(String path) {
      int i = 0;

      if (path.charAt(1) == ':')
       i = 2;
      if (path.charAt(i) == '\\')
       i++;

      return i;
    }

    /**
     * Resolve the child pathname string against the parent.
     * Both strings must be in normal form, and the result
     * will be in normal form.
     */
    public String resolve(String parent, String child) {
      int l = child.lastIndexOf('\\');

      if (l < 0 || parent.endsWith(child.substring(0, l)))
       return parent + '\\' + child.substring(l + 1);
      else
       return child;
    }

    /**
     * Return the parent pathname string to be used when the parent-directory
     * argument in one of the two-argument File constructors is the empty
     * pathname.
     */
    public String getDefaultParent() {
      return "C:\\Temp";
    }

    /**
     * Post-process the given URI path string if necessary.  This is used on
     * win32, e.g., to transform "/c:/foo" into "c:/foo".  The path string
     * still has slash separators; code in the File class will translate them
     * after this method returns.
     */
    public String fromURIPath(String path) {
      int i = path.indexOf(':');

      return i < 0?path:path.substring(i - 1);
    }


    /* -- Path operations -- */

    /**
     * Tell whether or not the given pathname is absolute.
     */
    public boolean isAbsolute(File f) {
      return f.getPath().charAt(1) == ':';
    }

    /**
     * Resolve the given pathname into absolute form.  Invoked by the
     * getAbsolutePath and getCanonicalPath methods in the File class.
     */
    public String resolve(File f) {
      String s;
      try
      {
       s = canonicalize(f.getPath());
      }
      catch(IOException e) { s = ""; }
      return s;
    }

    public String canonicalize(String path) throws IOException {
      byte [] b = path.getBytes();
      int length = b.length, body = ((JavaArray)b).getElem(0);
      #ass "HERE -1 body length ABSOLUTE-FILE DROP TO length TO body";
      return JavaArray.createString(length, body);
    }


    /* -- Attribute accessors -- */

    /* Constants for simple boolean attributes */
    public static final int BA_EXISTS    = 0x01;
    public static final int BA_REGULAR   = 0x02;
    public static final int BA_DIRECTORY = 0x04;
    public static final int BA_HIDDEN    = 0x08;

    /**
     * Return the simple boolean attributes for the file or directory denoted
     * by the given pathname, or zero if it does not exist or some
     * other I/O error occurs.
     */
    public int getBooleanAttributes(File f) {
      byte [] b = f.getPath().getBytes();
      int length = b.length, body = ((JavaArray)b).getElem(0);
      #ass "body length FILE-STATUS 0= AND";
    }

    /**
     * Check whether the file or directory denoted by the given abstract
     * pathname may be accessed by this process.  If the second argument is
     * <code>false</code>, then a check for read access is made; if the second
     * argument is <code>true</code>, then a check for write (not read-write)
     * access is made.  Return false if access is denied or an I/O error
     * occurs.
     */
    public boolean checkAccess(File f, boolean write) {
     int a = getBooleanAttributes(f);

     return write?(a & 16) != 0:((a & 32) != 0);
    }

    /**
     * Return the time at which the file or directory denoted by the given
     * pathname was last modified, or zero if it does not exist or
     * some other I/O error occurs.
     */
    public long getLastModifiedTime(File f) {
     byte [] b = resolve(f).getBytes();
     int length = b.length, body =((JavaArray)b).getElem(0);
     #ass "body length GET-FILEDATE DROP";
    }

    /**
     * Return the length in bytes of the file denoted by the given abstract
     * pathname, or zero if it does not exist, is a directory, or some other
     * I/O error occurs.
     */
    public long getLength(File f) {
     long l = 0L;
     String s;
     if (f.exists())
      {
       /*byte[] b =*/ s = resolve(f);/*.getBytes();/*
       int length = b.length, body = ( (JavaArray) b).getElem(0);
       #ass "body length R/O OPEN-FILE DROP A:R@ FILE-SIZE DROP R> CLOSE-FILE DROP TO l";*/
      }
      else;
     return l;
    }


    /* -- File operations -- */

    /**
     * Create a new empty file with the given pathname.  Return
     * <code>true</code> if the file was created and <code>false</code> if a
     * file or directory with the given pathname already exists.  Throw an
     * IOException if an I/O error occurs.
     */
    public boolean createFileExclusively(String pathname)
	throws IOException {
        byte [] b = canonicalize(pathname).getBytes();
        int length = b.length, body =((JavaArray)b).getElem(0);
        #ass "body length R/W CREATE-FILE 0= OVER CLOSE-FILE 0= AND NIP";
       }

    /**
     * Delete the file or directory denoted by the given pathname,
     * returning <code>true</code> if and only if the operation succeeds.
     */
    public boolean delete(File f) {
     byte [] b = resolve(f).getBytes();
     int length = b.length, body =((JavaArray)b).getElem(0);
     #ass "body length DELETE-FILE 0=";
    }

    /**
     * Arrange for the file or directory denoted by the given abstract
     * pathname to be deleted when the VM exits, returning <code>true</code> if
     * and only if the operation succeeds.
     */
    public boolean deleteOnExit(File f) { }

    /**
     * List the elements of the directory denoted by the given abstract
     * pathname.  Return an array of strings naming the elements of the
     * directory if successful; otherwise, return <code>null</code>.
     */
    public String[] list(File f) {
     byte [] b = resolve(f).getBytes();
     int c, a, length = b.length, body = ((JavaArray)b).getElem(0);
     #ass "HERE -1 body length ENTRIES-FILE A:R> TO length";
     #ass "DUP 0 R> 0= length AND 0 ?DO OVER I + C@ 10 = - LOOP TO length TO body";
     String [] s = new String[length];
     for(int i = 0; i < length; i++)
      {
       a = body;
       #ass "body 0 BEGIN OVER OVER + C@ 10 <> WHILE 1+ REPEAT DUP TO c 1+ + TO body";
       s[i] = JavaArray.createString(c, a);
      }
     return s;
    }

    /**
     * Create a new directory denoted by the given pathname,
     * returning <code>true</code> if and only if the operation succeeds.
     */
    public boolean createDirectory(File f) {
     byte [] b = resolve(f).getBytes();
     int length = b.length, body =((JavaArray)b).getElem(0);
     #ass "body length DIR CREATE-FILE 0= OVER CLOSE-FILE 0= AND NIP";
    }

    /**
     * Rename the file or directory denoted by the first pathname to
     * the second pathname, returning <code>true</code> if and only if
     * the operation succeeds.
     */
    public boolean rename(File f1, File f2) {
     byte [] b = resolve(f1).getBytes();
     int length = b.length, body =((JavaArray)b).getElem(0);
     byte [] c = resolve(f2).getBytes();
     int length1 = c.length, body1 =((JavaArray)c).getElem(0);
     #ass "body length body1 length1 RENAME-FILE 0=";
    }

    /**
     * Set the last-modified time of the file or directory denoted by the
     * given pathname, returning <code>true</code> if and only if the
     * operation succeeds.
     */
    public boolean setLastModifiedTime(File f, long time) {
     byte [] b = resolve(f).getBytes();
     int length = b.length, body =((JavaArray)b).getElem(0);
     #ass "time body length SET-FILEDATE 0=";
    }

    /**
     * Mark the file or directory denoted by the given pathname as
     * read-only, returning <code>true</code> if and only if the operation
     * succeeds.
     */
    public boolean setReadOnly(File f) { }


    /* -- Filesystem interface -- */

    /**
     * List the available filesystem roots.
     */
    public File[] listRoots() {
     String s [] = list(new File("C:\\"));
     File [] f = new File[s.length];
     for(int i = 0; i < f.length; i++)
      f[i] = new File(s[i]);
     return f;
    }


    /* -- Basic infrastructure -- */

    /**
     * Compare two pathnames lexicographically.
     */
    public int compare(File f1, File f2) {
     String s = resolve(f1);
     String t = resolve(f2);
     return s.compareToIgnoreCase(t);
    }

    /**
     * Compute the hash code of an pathname.
     */
    public int hashCode(File f) {
     return resolve(f).hashCode();
    }

    // Flags for enabling/disabling performance optimizations for file
    // name canonicalization
/*    static boolean useCanonCaches      = true;
    static boolean useCanonPrefixCache = true;

    private static boolean getBooleanProperty(String prop, boolean defaultVal) {
        String val = System.getProperty("sun.io.useCanonCaches");
        if (val == null) return defaultVal;
        if (val.equalsIgnoreCase("true")) {
            return true;
        } else {
            return false;
        }
    }

    static {
        useCanonCaches      = getBooleanProperty("sun.io.useCanonCaches",
                                                 useCanonCaches);
        useCanonPrefixCache = getBooleanProperty("sun.io.useCanonPrefixCache",
                                                 useCanonPrefixCache);
    }*/
}
