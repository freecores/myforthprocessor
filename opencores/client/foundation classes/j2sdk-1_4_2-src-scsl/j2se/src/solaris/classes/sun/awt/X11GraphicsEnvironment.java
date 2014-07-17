/*
 * @(#)X11GraphicsEnvironment.java	1.50 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt;

import java.awt.GraphicsDevice;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.print.PrinterJob;
import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.InputStream;
import java.io.IOException;
import java.io.StreamTokenizer;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.util.Enumeration;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.Locale;
import java.util.List;
import java.util.Map;
import java.util.NoSuchElementException;
import java.util.StringTokenizer;
import java.util.TreeMap;
import java.util.Vector;
import sun.awt.motif.MFontProperties;
import sun.awt.font.NativeFontWrapper;
import sun.java2d.SunGraphicsEnvironment;
import sun.print.PSPrinterJob;

/**
 * This is an implementation of a GraphicsEnvironment object for the
 * default local GraphicsEnvironment used by the Java Runtime Environment
 * for X11 environments.
 *
 * @see GraphicsDevice
 * @see GraphicsConfiguration
 * @version 1.50 01/23/03
 */

public class X11GraphicsEnvironment extends SunGraphicsEnvironment {

    private static final DebugHelper dbg =
        DebugHelper.create(X11GraphicsEnvironment.class);
    static Boolean xinerState = null;

    /*
     * This is the set of font directories needed to be on the X font path
     * to enable AWT heavyweights to find all of the font properties fonts.
     * It is populated by looking up font dirs in the xFontDirsMap where
     * the key is a fontID (cut down version of the XLFD read from the
     * font properties file. This set is nulled out after use to free
     * heap space.
     */
    private HashSet fontPropDirs;

   /*
    * fontNameMap is a map from a fontID (which is a substring of an XLFD like
    * "-monotype-arial-bold-r-normal-iso8859-7")
    * to font file path like
    * /usr/openwin/lib/locale/iso_8859_7/X11/fonts/TrueType/ArialBoldItalic.ttf
    * It's used in a couple of methods like
    * getFileNameFromPlatformName(..) to help locate the font file.
    * We use this substring of a full XLFD because the font properties files
    * define the XLFDs in a way that's easier to make into a request.
    * E.g., the -0-0-0-0-p-0- reported by X is -*-%d-*-*-p-*- in the font
    * properties files. We need to remove that part for comparisons.
    */
    Map fontNameMap;


    /* xFontDirsMap is also a map from a font ID to a font filepath.
     * The difference from fontNameMap is just that it does not have
     * resolved symbolic links. Normally this is not interesting except
     * that we need to know the directory in which a font was found to
     * add it to the X font server path, since although the files may
     * be linked, the fonts.dir is different and specific to the encoding
     * handled by that directory. This map is nulled out after use to free
     * heap space.
     */
    Map xFontDirsMap;

    /*
     * xlfdMap is a map from a platform path like
     * /usr/openwin/lib/locale/ja/X11/fonts/TT/HG-GothicB.ttf to an XLFD like
     * "-ricoh-hg gothic b-medium-r-normal--0-0-0-0-m-0-jisx0201.1976-0"
     * Because there may be multiple native names, because the font is used
     * to support multiple X encodings for example, the value of an entry in
     * this map is always a vector where we store all the native names.
     * For fonts which we don't understand the key isn't a pathname, its
     * the full XLFD string like :-
     * "-ricoh-hg gothic b-medium-r-normal--0-0-0-0-m-0-jisx0201.1976-0"
     */
    Map xlfdMap;

    /*
     * Used to eliminate redundant work. When a font directory (path) is
     * registered it added to this list. Subsequent registrations for the
     * same path can then be skipped by checking this Vector.
     */
    Vector registeredPaths;

    /*
     * Must be set true by an implementation property to permit
     * registration of native fonts not already needed as a part of
     * a composite font. Native fonts are those which are considered
     * opaque to the JRE since they are known only by their XLFD.
     * This property is primarily a debugging aid in the event that the
     * decision to disable all such registration causes problems.
     */
    private static boolean registerNativeFonts = false;

    private static boolean isSolaris = false;

    static {
	java.security.AccessController.doPrivileged(
			  new sun.security.action.LoadLibraryAction("awt"));
    /*
     * Note: The MToolkit object depends on the static initializer
     * of X11GraphicsEnvironment to initialize the connection to
     * the X11 server.
     */
        if (!isHeadless()) {
            initDisplay();
	    String s = (String)java.security.AccessController.doPrivileged(
                        new sun.security.action.GetPropertyAction(
		           	        "sun.java2d.registerNativeFonts"));
	    if (s != null) {
		registerNativeFonts = true;
	    }
        }
	String os = (String)java.security.AccessController.doPrivileged(
                        new sun.security.action.GetPropertyAction(
		           	        "os.name"));
        isSolaris = (os != null && os.startsWith("SunOS"));
    }

    /**
     * Checks if Shared Memory extension can be used.
     * Returns:
     *	 -1 if server doesn't support MITShm
     *	  1 if server supports it and it can be used
     *	  0 otherwise
     */
    private static native int checkShmExt();

    private static  native String getDisplayString();
    private static String display = null;
    private static boolean isDisplayLocal = false;

    private static native synchronized void initDisplay();

    public X11GraphicsEnvironment() {
    }

    protected native int getNumScreens();

    protected GraphicsDevice makeScreenDevice(int screennum) {
	return new X11GraphicsDevice(screennum);
    }

    protected native int getDefaultScreenNum();
    /**
     * Returns the default screen graphics device.
     */
    public GraphicsDevice getDefaultScreenDevice() {
	return getScreenDevices()[getDefaultScreenNum()];
    }

    public static synchronized boolean isDisplayLocal() {
	if (display != null) {
	    return isDisplayLocal;
	}

	if (isHeadless()) {
	    display = "";
	    return (isDisplayLocal = true);
	}

	String isRemote = (String)java.security.AccessController.doPrivileged(
            new sun.security.action.GetPropertyAction("sun.java2d.remote"));
	if (isRemote != null) {
	    display = "";
	    isDisplayLocal = isRemote.equals("false");
	    return isDisplayLocal;
	}
	
	int shm = checkShmExt();
	if (shm != -1) {
	    display = "";
	    isDisplayLocal = (shm == 1);
	    return isDisplayLocal;
	}
	// If XServer doesn't support ShMem extension, 
	// try the other way

	display = getDisplayString();

	int ind = display.indexOf(":");
	final String hostName = display.substring(0, ind);
	if (ind <= 0) { 
	    // ':0' case
	    return (isDisplayLocal = true);
	}

	Boolean result = (Boolean)java.security.AccessController.doPrivileged(
	    new java.security.PrivilegedAction() {
	    public Object run() {
		InetAddress remAddr[] = null;
		Enumeration locals = null;
		Enumeration interfaces = null;
		try {
		    interfaces = NetworkInterface.getNetworkInterfaces();
		    remAddr = InetAddress.getAllByName(hostName);
		    if (remAddr == null) {
			return Boolean.FALSE;
		    }
		} catch (UnknownHostException e) {
		    System.err.println("Unknown host: " + hostName);
		    return Boolean.FALSE;
		} catch (SocketException e1) {
		    System.err.println(e1.getMessage());
		    return Boolean.FALSE;
		}

		for (; interfaces.hasMoreElements();) {
		    locals = ((NetworkInterface)interfaces.nextElement()).getInetAddresses();
		    for (; locals.hasMoreElements();) {
			for (int i = 0; i < remAddr.length; i++) {
			    if (locals.nextElement().equals(remAddr[i])) {
				return Boolean.TRUE;
			    }
			}
		    }
		}
		return Boolean.FALSE;
	    }});
	return (isDisplayLocal = result.booleanValue());
    }

    /**
     * Gets a <code>PrinterJob</code> object suitable for the
     * the current platform.
     * @return    a <code>PrinterJob</code> object.
     * @see       java.awt.PrinterJob
     * @since     JDK1.2
     */
    public PrinterJob getPrinterJob() {
	SecurityManager security = System.getSecurityManager();
	if (security != null) {
	    security.checkPrintJobAccess();
	}

	return new PSPrinterJob();
    }

    protected String getBasePlatformFontPath(boolean noType1Font) {
	/* The base Solaris path for most West European & English locales is
	 * "/usr/openwin/lib/X11/fonts/TrueType". But here we return "", as
	 * the path is specified in the default font.properties using the
	 * appendedfontpath key, so we don't need to specify it in code.
	 */
	if (isSolaris) {
	    return "";
	} else {
	    return getPlatformFontPath(noType1Font);
	}
    }

    protected String getFontPropertyFD(String name) {
        return name;
    }

    // platform specific override
    // takes family name property in the following format:
    // 1) "-linotype-helvetica-medium-r-normal-sans-*-%d-*-*-p-*-iso8859-1"
    // and returns the name of the corresponding physical font

    protected String getFileNameFromPlatformName(String platName) {
        String fileName = null;
        String fontID = specificFontIDForName(platName);
       
        if (fontNameMap == null) {
	    fontNameMap = new Hashtable();
        }

        /* If the font filename has been explicitly assigned in the
         * font.properties file, use it. This avoids accessing
         * the wrong fonts on Linux, where different fonts (some
         * of which may not be usable by 2D) may share the same
         * specific font ID. It may also speed up the lookup.
         */
        fileName = super.getFileNameFromPlatformName(platName);
        if (fileName != null) {
	    if (fileName.startsWith("-")) {
		/* if it's headless, no xlfd should be used */
		if (isHeadless()) {
		    return null;
		}

	      /* If this is mapped to a native font (XLFD) which font isn't on
               * the X font path then we don't want to try to use it.
	       * This can happen with remote X display where the remote
	       * system's X font configuration doesn't match the expectations
	       * of the font.properties which are based on the client.
	       * The fileName value should key to an XLFD in the fontNameMap.
	       * If it isn't in the map then "switch" "adobe" for "sun"
	       * to see if there is an adobe equivalent and return that
	       * instead. If there's still none null is returned and some
	       * other font will need to be substituted.
	       */
	      if (fontNameMap.get(specificFontIDForName(fileName)) == null) {
                  fileName =
		      (String)fontNameMap.get(switchFontIDForName(fileName));
		  if (fileName == null) {
		      return null;
		  }
	      }
	    }
            fontNameMap.put (fontID,  fileName);            
            return fileName;
        }

        if (fontID != null) {
            fileName = (String)fontNameMap.get(fontID);
	    if (!registeredAllPaths &&
		(fileName == null || !fileName.startsWith("/"))) {
		if (debugMapping) {
		    System.out.println("** Registering all font paths");
		}
		String newPath = getPlatformFontPath(noType1Font);
		/* loadFonts needs to search the full path */
		fontPath = fontPath + File.pathSeparator + newPath;
		registerFontPaths(newPath);
		fileName = (String)fontNameMap.get(fontID);
	    }
            if (fileName == null) {
                fontID = switchFontIDForName(platName);
                fileName = (String)fontNameMap.get(fontID);
            }
	    /* here fileName will be null if we haven't seen the specified
	     * font, including as an XLFD. Currently to prevent this we
	     * list all X fonts via XListFonts and add them into the
	     * fontNameMap. What seems like a smarter idea is at this
	     * point if fileName is null to make an explicit X call to
	     * find this particular font. Then calling initNativeFonts()
	     * would not be necessary.
	     */
	       
        }
        return fileName;
    }

    // platform specific override
    // takes family name property in the following format:
    // 1) "-linotype-helvetica-medium-r-normal-sans-*-%d-*-*-p-*-iso8859-1"
    // and returns the name from the corresponding physical font

    protected String parseFamilyNameProperty(String name) {
        String retval = "default";
        String fontID = specificFontIDForName(name);

        if ((fontNameMap != null) && (fontID != null)) {
            String fileName = (String)fontNameMap.get(fontID);
            if (fileName == null) {
                fontID = switchFontIDForName(name);
                fileName = (String)fontNameMap.get(fontID);
            }
            if (fileName != null) {
                String fontName =
                    NativeFontWrapper.getFullNameByFileName(fileName);
                if (fontName != null) {
                    retval = fontName;
                }
            }
        }
        return retval;
    }
    
    // constants identifying XLFD and font ID fields
    private static final int FOUNDRY_FIELD = 1;
    private static final int FAMILY_NAME_FIELD = 2;
    private static final int WEIGHT_NAME_FIELD = 3;
    private static final int SLANT_FIELD = 4;
    private static final int SETWIDTH_NAME_FIELD = 5;
    private static final int ADD_STYLE_NAME_FIELD = 6;
    private static final int PIXEL_SIZE_FIELD = 7;
    private static final int POINT_SIZE_FIELD = 8;
    private static final int RESOLUTION_X_FIELD = 9;
    private static final int RESOLUTION_Y_FIELD = 10;
    private static final int SPACING_FIELD = 11;
    private static final int AVERAGE_WIDTH_FIELD = 12;
    private static final int CHARSET_REGISTRY_FIELD = 13;
    private static final int CHARSET_ENCODING_FIELD = 14;

    private String switchFontIDForName(String name) {
        String part1 = getXLFDField(name, FAMILY_NAME_FIELD);
        String part2 = getXLFDField(name, WEIGHT_NAME_FIELD);
        String part3 = getXLFDField(name, SLANT_FIELD);
        String part4 = getXLFDField(name, SETWIDTH_NAME_FIELD);
        String part5 = getXLFDField(name, CHARSET_REGISTRY_FIELD);
        String part6 = getXLFDField(name, CHARSET_ENCODING_FIELD);
        if (part3.equals("i")) {
            part3 = "o";
        } else if (part3.equals("o")) {
            part3 = "i";
        }
        //workaournd for #4471000
        if (part1.equals("itc zapfdingbats") 
            && part5.equals("sun")
            && part6.equals("fontspecific")){
            part5 = "adobe";
	}
        String retVal = "-" + part1 + "-" + part2 + "-" + part3 + "-" + part4 +
			"-" + part5 + "-" + part6;
	return (retVal.toLowerCase (Locale.ENGLISH));
    }

    private String specificFontIDForName(String name) {
        String part1 = getXLFDField(name, FAMILY_NAME_FIELD);
        String part2 = getXLFDField(name, WEIGHT_NAME_FIELD);
        String part3 = getXLFDField(name, SLANT_FIELD);
        String part4 = getXLFDField(name, SETWIDTH_NAME_FIELD);
        String part5 = getXLFDField(name, CHARSET_REGISTRY_FIELD);
        String part6 = getXLFDField(name, CHARSET_ENCODING_FIELD);
        String retVal = "-" + part1 + "-" + part2 + "-" + part3 + "-" + part4 +
			"-" + part5 + "-" + part6;
	return (retVal.toLowerCase (Locale.ENGLISH));
    }

    private String getXLFDField(String fontString, int field) {
        return getFontField(fontString, field);
    }
    
    private static final int[] fontIDFields = {
        0 /* unused */,
        FAMILY_NAME_FIELD, WEIGHT_NAME_FIELD, SLANT_FIELD,
        SETWIDTH_NAME_FIELD, CHARSET_REGISTRY_FIELD, CHARSET_ENCODING_FIELD
    };
    
    private String getFontIDField(String fontString, int field) {
        for (int fieldNum = 1; fieldNum < fontIDFields.length; fieldNum++) {
            if (fontIDFields[fieldNum] == field) {
                return getFontField(fontString, fieldNum);
            }
        }
        // if we get here, we were given an invalid field ID
        assert false;
        return null;
    }

    private String getFontField(String fontString, int fieldNum) {
        int beginIndex = 0;
        int endIndex = 0;
        while ((fieldNum-- > 0) && (beginIndex >= 0)) {
            beginIndex = fontString.indexOf("-", beginIndex)+1;
        }
        if (beginIndex == -1) {
            return "";
        }
        endIndex = fontString.indexOf("-", beginIndex);
        if (endIndex == -1) {
            return fontString.substring(beginIndex);
        } else {
            return fontString.substring(beginIndex, endIndex);
        }
    }

    // can have platform specific override
    protected void registerFontPath(String path) {
        final File fontDir = new File(path + File.separator + "fonts.dir");
	java.security.AccessController.doPrivileged(
				    new java.security.PrivilegedAction() {
	    public Object run() {
		try {
		    parseFontDir(fontDir);
		} catch (Exception e) {
		    // no relevant errors for application
		}
		return null;
	    }
	});
    }

    private native String [] getNativeFonts ();

    protected Vector getNativeNames(String fontFileName) {
        Vector nativeNames;
        if (xlfdMap == null ||
            ((nativeNames=(Vector)xlfdMap.get(fontFileName))==null)) {
            Vector v = new Vector();
             //v.add(fontFileName);
            return v;
        } else {
            return nativeNames;
	}
    }

    protected void registerNative (String fontName) {
	Vector v = new Vector(1, 1);
	v.addElement(fontName);
	registeredFonts.put (fontName, fontName);
	NativeFontWrapper.registerFonts (v, v.size(), v,
					 NativeFontWrapper.FONTFORMAT_NATIVE,
					 false);
    }

    private Vector nativeFonts;

    /* This method loads all the native fonts in to the maps.
     * Doesn't seem like it should be necessary do the full XListFonts() query
     * Just doing it on the specific font would be cheaper.
     */
    private void initNativeFonts () {

	if (nativeFonts != null || isHeadless()) {
	    return;
	}
	String [] xFonts = getNativeFonts ();
	if (xFonts != null) {
	    nativeFonts = new Vector ();
	    if (fontNameMap == null) {
		fontNameMap = new Hashtable();
	    }
	    if (xlfdMap == null) {
                xlfdMap = new Hashtable ();
	    }
  	    for (int i = 0; i < xFonts.length; i++) {
		String fontID = specificFontIDForName (xFonts [i]);
                String sVal = (String) fontNameMap.get (fontID);
		if (sVal == null) {
		   nativeFonts.addElement (xFonts [i]);
		   fontNameMap.put (fontID,  xFonts [i]);
                   Vector v = new Vector();
                   v.add(xFonts [i]);
                   xlfdMap.put (xFonts [i], v);
		}
	    }
	}
    }

    protected void addToFontPath(String theCompositeFontPath)
    {
      //      String theFontPath = super.fontPath;
      //      System.out.println ( "The super path = " + theFontPath );
      //      System.out.println("The composite path = " + theCompositeFontPath );
      //      if (theFontPath.indexOf(theCompositeFontPath) == -1)
      //	System.out.println ( "The path needs to be added to the fontpath" + theCompositeFontPath);
      NativeFontWrapper.setNativeFontPath(theCompositeFontPath);

    }  	

    // An X font spec (xlfd) includes an encoding. The same TrueType font file
    // may be referenced from different X font directories in font.dir files
    // to support use in multiple encodings by X apps.
    // So for the purposes of font properties logical fonts where AWT
    // heavyweights need to access the font via X APIs we need to ensure that
    // the directory for precisely the encodings needed by this are added to
    // the x font path. This requires that we note the platform names
    // specified in font.properties and use that to identify the
    // X font directory that contains a font.dir file for that platform name
    // and add it to the X font path (if display is local)
    // Here we make use of an already built map of xlfds to font locations
    // to add the font location to the set of those required to build the
    // x font path needed by AWT.
    // These are added to the x font path later.
    // All this is necessary because on Solaris the font.dir directories
    // may contain not real font files, but symbolic links to the actual
    // location but that location is not suitable for the x font path, since
    // it probably doesn't have a font.dir at all and certainly not one
    // with the required encodings
    protected void addPlatformNameForFontProperties(String platName) {
        if (fontPropDirs == null) {
	    fontPropDirs = new HashSet();
	}
	if (xFontDirsMap == null) {
	  return;
	}
	String fontID = specificFontIDForName(platName);
	String dirName = (String)xFontDirsMap.get(fontID);
	if (dirName != null && !fontPropDirs.contains(dirName)) {
	    fontPropDirs.add(dirName);
	}
	return;
    }

    protected void registerFontPropertiesFonts(TreeMap fPropFonts) {

	Object [] fonts = fPropFonts.keySet().toArray();

	for (int i=0; i<fonts.length; i++) {
	    String fontFileName = (String)fonts[i];
	    HashSet s = (HashSet)fPropFonts.get(fontFileName);
	    Vector natNames = getNativeNames(fontFileName);
	    // merge the platNames & nativeNames.
	    // both are forms of XLFD
	    // platnames are from font properties, nativeNames are from
	    // fonts.dir files. To compare them we need to convert to
	    // fontID format (see doc for fontNameMap variable).
	    // we "prefer" the platName forms because they are in a more
	    // ready to use form.
	    String[] platNames = (String[])s.toArray(new String[0]);
	    Vector platIDs = new Vector();
	    for (int j=0; j<platNames.length; j++) {
		platIDs.add(specificFontIDForName(platNames[j]));
	    }
	    Vector nativeNames = new Vector(s);
	    for (int j=0; j<natNames.size(); j++) {
		String name = (String)natNames.elementAt(j);
		String platID = specificFontIDForName(name); 
		if (!platIDs.contains(platID)) {
		    nativeNames.add(name);
		}
	    }
	    registerFontFile(fontFileName, nativeNames);
	}
	String [] fontdirs = (String[])fontPropDirs.toArray(new String[0]);
	// need to register these individually rather than by one call
	// to ensure that one bad directory doesn't cause all to be rejected
        for (int i=0; i<fontdirs.length; i++) {
	    if (debugMapping) {
		System.out.println("Add " + fontdirs[i] + " to X11 fontpath");
	    }
             addToFontPath(fontdirs[i]);
        }
	// now we no longer need some info so we can null out references
	// so the GC can reclaim the heap
	fontPropDirs = null;
	xFontDirsMap = null;
	
        
    }

    protected void registerFontFile(String fontFileName, Vector nativeNames) {
        // REMIND: case compare depends on platform
        if (registeredFonts.containsKey(fontFileName)) {
            return;
        }
        int fontFormat;
        if (new TTFilter().accept(null, fontFileName)) {
            fontFormat = NativeFontWrapper.FONTFORMAT_TRUETYPE;
        } else if (new T1Filter().accept(null, fontFileName)) {
            fontFormat = NativeFontWrapper.FONTFORMAT_TYPE1;
        } else if (new T2KFilter().accept(null, fontFileName)) {
            fontFormat = NativeFontWrapper.FONTFORMAT_T2K;
        } else {
	    registerNative (fontFileName);
            return;
        }

	// fontFileName should be absolute path - shouldn't search fontpath.

	File theFile =  new File(fontFileName);
	if (theFile.canRead()) {
	    Vector fontNames = new Vector(1, 1);
	    Vector platNames = new Vector(1, 1);
	    platNames.addElement(nativeNames);
	    String fullName = null;
	    try {
		fullName = theFile.getCanonicalPath();
		theFile = new File(fullName);
	    } catch (IOException e) {
		fullName = theFile.getAbsolutePath();
	    }
	    fontNames.addElement(fullName);
	    registeredFonts.put(fontFileName, fontFileName);
	    NativeFontWrapper.registerFonts(fontNames, fontNames.size(),
					    platNames, fontFormat, false);
        }
    }

    protected boolean registerNativeFonts () {
	if (!registerNativeFonts) {
	    return false;
	}

	if (nativeFonts != null) {
	    Vector v = new Vector (20, 10);
  	    for (int i = 0; i < nativeFonts.size (); i++) {
		String xlfd = (String) nativeFonts.elementAt (i);
		if (registeredFonts.containsKey (xlfd)) {
		    continue;
		}
		v.addElement (xlfd);
		registeredFonts.put (xlfd, xlfd);
	    }
	    NativeFontWrapper.registerFonts (v, v.size (), v,
				 NativeFontWrapper.FONTFORMAT_NATIVE, false);
	    return true;
	}

	return false;
    }

    protected void registerFontPaths(String pathName) {

	boolean retval = false;
        StringTokenizer parser = new StringTokenizer(pathName, 
                                                     File.pathSeparator);
	if (registeredPaths == null) {
	    registeredPaths = new Vector();
	}

        try {
            while (parser.hasMoreTokens()) {
                String newPath = parser.nextToken();
		if (newPath != null && !registeredPaths.contains(newPath)) {
		    registeredPaths.add(newPath);
		    registerFontPath(newPath);
		}
	    }
        } catch (NoSuchElementException e) {
            System.err.println(e);
        }
	if (loadNativeFonts) {
	    initNativeFonts ();
	}	
    }

    private void parseFontDir(File fontDir)
            throws FileNotFoundException, IOException {
        // file format:
        //  47
        //  Arial.ttf -monotype-arial-regular-r-normal--0-0-0-0-p-0-iso8859-1
        //  Arial-Bold.ttf -monotype-arial-bold-r-normal--0-0-0-0-p-0-iso8859-1
        //  ...
	String path = fontDir.getParent();
	if (debugMapping) {
	    System.out.println("ParseFontDir " + path);
	}
        if (fontDir.canRead()) {
            FileInputStream fs = new FileInputStream(fontDir);
	    BufferedInputStream bfs = new BufferedInputStream(fs, 8192);
            StreamTokenizer st = new StreamTokenizer((InputStream)bfs);
            st.eolIsSignificant(true);
            int ttype = st.nextToken();
            if (ttype == StreamTokenizer.TT_NUMBER) {
                int numEntries = (int)st.nval;
                ttype = st.nextToken();
                if (ttype == StreamTokenizer.TT_EOL) {
		    if (fontNameMap == null) {
			fontNameMap = new Hashtable();
		    }
		    if (xlfdMap == null) {
                        xlfdMap = new Hashtable();
		    }
		    if (xFontDirsMap == null) {
                        xFontDirsMap = new Hashtable();
		    }
                    st.resetSyntax();
                    st.wordChars(32, 127);
	            st.wordChars(128 + 32, 255);
	            st.whitespaceChars(0, 31);

                    for (int i=0; i < numEntries; i++) {
                        ttype = st.nextToken();
                        if (ttype == StreamTokenizer.TT_EOF) {
                            break;
                        }
                        if (ttype != StreamTokenizer.TT_WORD) {
                            //System.out.println("Invalid Syntax 1: font.dir");
                            break;
                        }
                        int breakPos = st.sval.indexOf(" ");
                        if (breakPos <= 0) {
                            //System.out.println("Invalid Syntax 2: font.dir");
                            break;
                        }
                        String fileName = st.sval.substring(0, breakPos);
			String fontPart = st.sval.substring(breakPos+1);
			
			if (debugMapping) {
			    System.out.println("file=" + fileName +
					       " xlfd=" + fontPart);
			}
                        String fontID = specificFontIDForName(fontPart);
			String sVal = (String) fontNameMap.get (fontID);
			if (debugMapping) {
			    System.out.println("fontID=" + fontID +
					       " sVal=" + sVal);
			}
			String fullPath = null;
			try {
			    File file = new File(path,fileName);
			    /* we may have a resolved symbolic link
			     * this becomes important for an xlfd we
			     * still need to know the location it was
			     * found to update the X server font path
			     * for use by AWT heavyweights - and when 2D
			     * wants to use the native rasteriser.
			     */
			    File f = file.getParentFile();
			    if (f != null) {
				xFontDirsMap.put(fontID, f.getAbsolutePath());
			    }
			    fullPath = file.getCanonicalPath();
			} catch (IOException e) {
			    fullPath = path + File.separator + fileName;
			}
			Vector xVal = (Vector) xlfdMap.get (fullPath);
			if (debugMapping) {
			    System.out.println("fullPath=" + fullPath +
					       " xVal=" + xVal);
			}
			if ((xVal == null || !xVal.contains(fontPart)) &&
                            (sVal == null) || !sVal.startsWith("/")) {
			    if (debugMapping) {
				System.out.println("Map fontID:"+fontID +
						   "to file:" + fullPath);
			    }	
                            fontNameMap.put(fontID, fullPath);
                            if (xVal == null) {
                                xVal = new Vector();
                                xlfdMap.put (fullPath, xVal);
                            }
                            xVal.add(fontPart);
			}

                        ttype = st.nextToken();
                        if (ttype != StreamTokenizer.TT_EOL) {
                            //System.out.println("Invalid Syntax 3: font.dir");
                            break;
                        }
                    }
                }
            }
        }
    }
    
    // overrides SunGraphicsEnvironment.addPlatformCompatibilityFileNames
    protected void addPlatformCompatibilityFileNames(Map registeredFileNames) {
        // For compatibility with old font.properties files, we want to
        // to make sure that we prefer the iso8859-1 variants of any TimesRoman,
        // Helvetica, or Courier fonts. We therefore register the files
        // containing such fonts first.
        if (fontNameMap == null || xlfdMap == null) {
            // not enough information to do the job, so just bail out
            return;
        }
        Iterator iterator = fontNameMap.keySet().iterator();
        while (iterator.hasNext()) {
            String fontID = (String) iterator.next();
            String fileName = (String) fontNameMap.get(fontID);
            if (registeredFileNames.get(fileName) != null ||
		fileName.startsWith("-")) {
                continue;
            }
            String family = getFontIDField(fontID, FAMILY_NAME_FIELD);
            String encoding = getFontIDField(fontID, CHARSET_REGISTRY_FIELD)
                    + "-" + getFontIDField(fontID, CHARSET_ENCODING_FIELD);
            if (isCompatibilityFont(family, encoding)) {
                HashSet set = new HashSet();
                List xlfdList = (List) xlfdMap.get(fileName);
                Iterator xlfdIterator = xlfdList.iterator();
                while (xlfdIterator.hasNext()) {
                    String xlfd = (String) xlfdIterator.next();
                    family = getXLFDField(xlfd, FAMILY_NAME_FIELD);
                    encoding = getXLFDField(xlfd, CHARSET_REGISTRY_FIELD)
                            + "-" + getXLFDField(xlfd, CHARSET_ENCODING_FIELD);
                    if (isCompatibilityFont(family, encoding)) {
                        set.add(xlfd);
                    }
                }
                registeredFileNames.put(fileName, set);
            }
        }
    }
    
    private boolean isCompatibilityFont(String family, String encoding) {
        return "iso8859-1".equals(encoding) &&
            ("timesroman".equals(family) ||
             "helvetica".equals(family) ||
             "courier".equals(family));
    }
        

    // Implements SunGraphicsEnvironment.createFontProperties.
    protected FontProperties createFontProperties() {
	return new MFontProperties();
    }

    private static native boolean pRunningXinerama();
    private static native Point getXineramaCenterPoint();

    /** 
     * Override for Xinerama case: call new Solaris API for getting the correct
     * centering point from the windowing system.
     */
    public Point getCenterPoint() {
        if (runningXinerama()) {
            Point p = getXineramaCenterPoint();
            if (p != null) {
                return p;
            }
        }
        return super.getCenterPoint();
    } 

    /**
     * Override for Xinerama case
     */
    public Rectangle getMaximumWindowBounds() {
        if (runningXinerama()) {
            return getXineramaWindowBounds();
        } else {
            return super.getMaximumWindowBounds();
        }
    }

    private synchronized boolean runningXinerama() {
        if (xinerState == null) {
            xinerState = new Boolean(pRunningXinerama());
        } 
        return xinerState.booleanValue();
    }

    /**
     * Return the bounds for a centered Window on a system running in Xinerama
     * mode.
     *
     * Calculations are based on the assumption of a perfectly rectangular
     * display area (display edges line up with one another, and displays
     * have consistent width and/or height).
     *
     * The bounds to return depend on the arrangement of displays and on where
     * Windows are to be centered.  There are two common situations:
     *
     * 1) The center point lies at the center of the combined area of all the
     *    displays.  In this case, the combined area of all displays is
     *    returned.
     *
     * 2) The center point lies at the center of a single display.  In this case
     *    the user most likely wants centered Windows to be constrained to that
     *    single display.  The boundaries of the one display are returned.
     *
     * It is possible for the center point to be at both the center of the
     * entire display space AND at the center of a single monitor (a square of
     * 9 monitors, for instance).  In this case, the entire display area is
     * returned.
     *
     * Because the center point is arbitrarily settable by the user, it could
     * fit neither of the cases above.  The fallback case is to simply return
     * the combined area for all screens.
     */
    protected Rectangle getXineramaWindowBounds() {
        Point center = getCenterPoint();
        Rectangle unionRect, tempRect;
        GraphicsDevice[] gds = getScreenDevices();
        Rectangle centerMonitorRect = null;
        int i;

        // if center point is at the center of all monitors
        // return union of all bounds
        //
        //  MM*MM     MMM       M
        //            M*M       *
        //            MMM       M

        // if center point is at center of a single monitor (but not of all
        // monitors)
        // return bounds of single monitor
        //
        // MMM         MM
        // MM*         *M

        // else, center is in some strange spot (such as on the border between
        // monitors), and we should just return the union of all monitors
        //
        // MM          MMM
        // MM          MMM

        unionRect = getUsableBounds(gds[0]);

        for (i = 0; i < gds.length; i++) {
            tempRect = getUsableBounds(gds[i]);
            if (centerMonitorRect == null &&
                // add a pixel or two for fudge-factor
                (tempRect.width / 2) + tempRect.x > center.x - 1 &&
                (tempRect.height / 2) + tempRect.y > center.y - 1 &&
                (tempRect.width / 2) + tempRect.x < center.x + 1 &&
                (tempRect.height / 2) + tempRect.y < center.y + 1) {
                centerMonitorRect = tempRect;
            }
            unionRect = unionRect.union(tempRect);
        }

        // first: check for center of all monitors (video wall)
        // add a pixel or two for fudge-factor
        if ((unionRect.width / 2) + unionRect.x > center.x - 1 &&
            (unionRect.height / 2) + unionRect.y > center.y - 1 &&
            (unionRect.width / 2) + unionRect.x < center.x + 1 &&
            (unionRect.height / 2) + unionRect.y < center.y + 1) {

            if (dbg.on) {
                dbg.println("Video Wall: center point is at center of all displays.");
            }
            return unionRect;
        }

        // next, check if at center of one monitor
        if (centerMonitorRect != null) {
            if (dbg.on) {
                dbg.println("Center point at center of a particular monitor, but not of the entire virtual display.");
            }
            return centerMonitorRect;
        }

        // otherwise, the center is at some weird spot: return unionRect
        if (dbg.on) {
            dbg.println("Center point is somewhere strange - return union of all bounds.");
        }
        return unionRect;
    }
}
