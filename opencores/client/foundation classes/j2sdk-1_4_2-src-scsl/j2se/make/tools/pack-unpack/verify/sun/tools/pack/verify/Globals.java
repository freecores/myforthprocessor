/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)Globals.java	1.11 03/01/23
 */

/*
 * A collection of useful global utilities commonly used.
 */
package sun.tools.pack.verify;

import java.io.*;
import java.util.*;


public class Globals {
    
    private static PrintWriter _pw = null;
    
    private static String _logFileName = null;
    
    private static final String DEFAULT_LOG_FILE = "verifier.log";
    
    private static boolean _verbose = true;
    
    private static boolean _ignoreJarDirectories = true;
    
    private static boolean _checkJarClassOrdering = false;
    
    // Ignore Deprecated, SourceFile and Synthetic
    private static boolean _ignoreKnownAttributes = true;
    
    private static Globals _instance = null;
    
    public static Globals getInstance() {
	if (_instance == null) {
	    _instance = new Globals();
	    _verbose = (System.getProperty("sun.tools.pack.verify.verbose") == null) ? false : true;
	    _ignoreJarDirectories = System.getProperty("ignoreJarDirectories") == null ? false : true;
	}
	return _instance;
    }
    
    public static boolean ignoreKnownAttributes() {
	return _ignoreKnownAttributes;
    }
    
    public static boolean ignoreJarDirectories() {
	return _ignoreJarDirectories;
    }
    
    public static void setCheckJarClassOrdering(boolean flag) {
	_checkJarClassOrdering = flag;
    }
    
    public static boolean checkJarClassOrdering() {
	return _checkJarClassOrdering;
    }
    public static void print(String s) {
	if (_verbose) System.out.print(s);
    }
    
    public static void println(String s) {
	if (_verbose) System.out.println(s);
    }
    
    public static void log(String s) {
	_pw.println(s);
    }
    
    public static void lognoln(String s) {
	_pw.print(s);
    }
    
    private static PrintWriter openFile(String fileName) {
	//Lets create the directory if it does not exist.
	File f = new File(fileName) ;
	File baseDir = f.getParentFile();
	if (baseDir != null && baseDir.exists() == false) {
	    baseDir.mkdirs();
	}
	try {
	    return new PrintWriter(new FileWriter(f), true);
	} catch (Exception e) {
	    e.printStackTrace();
	}
	return null;
    }
    
    static void printPropsToLog() {
	log("Log started " + new Date(System.currentTimeMillis()));
	lognoln(System.getProperty("java.vm.version"));
	log("\t" + System.getProperty("java.vm.name"));
	
	log("System properties");
	log("\tjava.home="+System.getProperty("java.home"));
	log("\tjava.class.version="+System.getProperty("java.class.version"));
	log("\tjava.class.path="+System.getProperty("java.class.path"));
	log("\tjava.ext.dirs="+System.getProperty("java.ext.dirs"));
	log("\tos.name="+System.getProperty("os.name"));
	log("\tos.arch="+System.getProperty("os.arch"));
	log("\tos.version="+System.getProperty("os.version"));
	log("\tuser.name="+System.getProperty("user.name"));
	log("\tuser.home="+System.getProperty("user.home"));
	log("\tuser.dir="+System.getProperty("user.dir"));
	log("\tLocale.getDefault="+Locale.getDefault());
	log("System properties end");
    }
    
    static void openLog(String s) {
	_logFileName = (s != null) ? s : "." + File.separator + DEFAULT_LOG_FILE;
	_logFileName = (new File(_logFileName).isDirectory()) ?
	    _logFileName + File.separator + DEFAULT_LOG_FILE : _logFileName;
	_pw = openFile(_logFileName);
	printPropsToLog();
    }
    
    static String getLogFileName() {
	return _logFileName;
    }
    
    public static void diffCharData(String s1, String s2) {
	boolean diff = false;
	char[] c1 = s1.toCharArray();
	char[] c2 = s2.toCharArray();
	if (c1.length != c2.length) {
	    diff = true;
	    Globals.log("Length differs: " + (c1.length - c2.length));
	}
	// Take the smaller of the two arrays to prevent Array...Exception
	int minlen = (c1.length < c2.length) ? c1.length : c2.length ;
	for (int i = 0 ; i < c1.length ; i++) {
	    if (c1[i] != c2[i]) {
		diff = true;
		Globals.lognoln("\t idx[" + i + "] 0x" + Integer.toHexString(c1[i]) + "<>" + "0x" + Integer.toHexString(c2[i]));
		Globals.log(" -> "  + c1[i] + "<>" + c2[i]);
	    }
	}
    }
    
    public static void diffByteData(String s1, String s2) {
	boolean diff = false;
	byte[] b1 = s1.getBytes();
	byte[] b2 = s2.getBytes();
	
	if (b1.length != b2.length) {
	    diff = true;
	    //(+) b1 is greater, (-) b2 is greater
	    Globals.log("Length differs diff: " + (b1.length - b2.length));
	}
	// Take the smaller of the two array to prevent Array...Exception
	int minlen = (b1.length < b2.length) ? b1.length : b2.length ;
	for (int i = 0 ; i < b1.length ; i++) {
	    if (b1[i] != b2[i]) {
		diff = true;
		Globals.log("\t" + "idx[" + i + "] 0x" + Integer.toHexString(b1[i]) + "<>" + "0x" + Integer.toHexString(b2[i]));
	    }
	}
    }
    
    protected static void dumpToHex(String s) {
	try {
	    dumpToHex(s.getBytes("UTF-8"));
	} catch (UnsupportedEncodingException uce) {
	    uce.printStackTrace();
	}
    }
    
    protected static void dumpToHex(byte[] buffer) {
	int linecount=0;
	byte[] b = new byte[16];
	for (int i = 0 ; i < buffer.length  ; i += 16 ) {
	    if (buffer.length - i  > 16) {
		System.arraycopy(buffer,i,b,0,16);
		print16Bytes(b,linecount);
		linecount += 16;
	    } else {
		System.arraycopy(buffer,i,b,0,buffer.length - i);
		for (int n = buffer.length - (i+1);n < 16; n++) b[n]=0;
		print16Bytes(b,linecount);
		linecount += 16;
	    }
	}
	Globals.log("-----------------------------------------------------------------");
    }
    
    
    private static void print16Bytes(byte[] buffer, int linecount) {
	final int MAX = 4;
	Globals.lognoln(paddedHexString(linecount,4) + " ");
	
	for (int i = 0 ; i < buffer.length ;  i += 2) {
	    int iOut = pack2Bytes2Int(buffer[i], buffer[i + 1]);
	    Globals.lognoln(paddedHexString(iOut,4) + " ");
	}
	
	Globals.lognoln("| ");
	
	StringBuffer sb = new StringBuffer(new String(buffer)) ;
	
	for (int i = 0 ; i < buffer.length ; i++) {
	    if (Character.isISOControl(sb.charAt(i))) {
		sb.setCharAt(i,'.');
	    }
	}
	
	Globals.log(sb.toString());
    }
    
    private static int pack2Bytes2Int(byte b1, byte b2) {
	int out = 0x0;
	out += b1;
	out <<= 8;
	out &= 0x0000ffff;
	out |= 0x000000ff & b2;
	
	return out ;
    }
    private static String paddedHexString(int n, int max) {
	
	char[] c  = Integer.toHexString(n).toCharArray();
	char[] out = new char[max];
	
	for (int i = 0 ; i < max ; i++) out[i]='0';
	int offset = (max - c.length < 0) ? 0 : max - c.length;
	for (int i = 0 ; i < c.length ; i++) out[offset+i] =  c[i];
	
	return new String(out);
    }
    
  
}


