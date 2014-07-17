/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)JarFileCompare.java	1.7 03/01/23
 */

// The Main Entry point
package sun.tools.pack.verify;

import java.io.*;
import java.util.*;
import java.util.jar.*;

class JarFileCompare {
    
    private static VerifyTreeSet getVerifyTreeSet(String jarPath) {
	VerifyTreeSet vts = new VerifyTreeSet();
	try {
	    JarFile j = new JarFile(jarPath);
	    Enumeration e;
	    for (e = j.entries(); e.hasMoreElements();) {
		JarEntry je = (JarEntry) e.nextElement();
		if (!je.isDirectory()) { // totally ignore directories
		   vts.add(je.getName());
		}
	    }
	} catch (IOException ioe) {
	    ioe.printStackTrace();
	}
	return vts;
    }
    
    private static LinkedList getListOfClasses(String jarPath) {
	LinkedList l = new LinkedList();
	try {
	    JarFile j = new JarFile(jarPath);
	    Enumeration e;
	    for (e = j.entries(); e.hasMoreElements();) {
		JarEntry je = (JarEntry) e.nextElement();
		if (!je.isDirectory() && je.getName().endsWith(".class")) {
		   l.add(je.getName());
		}
	    }
	} catch (IOException ioe) {
	    ioe.printStackTrace();
	}
	return l;
    }
	
    
    private static void jarDirectoryCompare(String jarPath1, String jarPath2) {
	VerifyTreeSet vts1 = getVerifyTreeSet(jarPath1);
	VerifyTreeSet vts2 = getVerifyTreeSet(jarPath2);
	
	TreeSet diff1 = vts1.diff(vts2);
	if (diff1.size() > 0) {
	    Globals.log("Left has the following entries that right does not have");
	    Globals.log(diff1.toString());
	}
	TreeSet diff2 = vts2.diff(vts1);
	if (diff2.size() > 0) {
	    Globals.log("Right has the following entries that left does not have");
	    Globals.log(diff2.toString());
	}
	if (Globals.checkJarClassOrdering()) {
	    boolean error = false;
	    Globals.log("Checking Class Ordering");
	    LinkedList l1 = getListOfClasses(jarPath1);
	    LinkedList l2 = getListOfClasses(jarPath2);
	    if (l1.size() != l2.size()) {
		error = true;
		Globals.log("The number of classes differs");
		Globals.log("\t" + l1.size() + "<>" + l2.size());
	    }
	    
	    for (int i = 0 ; i < l1.size() ; i++) {
		String s1 = (String) l1.get(i);
		String s2 = (String) l2.get(i);
		if (s1.compareTo(s2) != 0 ) {
		    error = true;
		    Globals.log("Ordering differs at[" + i + "] = " + s1);
		    Globals.log("\t" + s2);
		}
	    }
	}
    }
    /**
     * Returns true if the two Streams are bit identical, and false if they
     * are not, no further diagnostics
     */
    private static boolean compareStreams(InputStream is1, InputStream is2) {
	BufferedInputStream bis1 = new BufferedInputStream(is1,4096);
	BufferedInputStream bis2 = new BufferedInputStream(is2,4096);
	int count = 0 ;
	try {
	    int i1, i2;
	    while ( (i1 = bis1.read()) > 0)  {
		count++;
		i2 = bis2.read();
		if (i1 != i2) return false;
	    }
	    return true;
	} catch (IOException ioe) {
	    ioe.printStackTrace();
	}
	return false;
    }
   /**
     * Given two jar files we compare and see if the jarfiles have all the
     * entries. The property ignoreJarDirectories is set to true by default
     * which means that Directory entries in a jar may be ignore.
     */
       
    public static void jarCompare(String jarPath1, String jarPath2) {
	jarDirectoryCompare(jarPath1, jarPath2);
	
	try {
	    JarFile jf1 = new JarFile(jarPath1);
	    JarFile jf2 = new JarFile(jarPath2);
	    
	    
	    int total_entries = 0;
	    int entries_checked = 0;
	    
	    for (Enumeration e = jf1.entries(); e.hasMoreElements();) {
		JarEntry je = (JarEntry) e.nextElement();
		if (!je.isDirectory() && !je.getName().endsWith(".class")) {
		   total_entries++;
		}
	    }

	    for (Enumeration e = jf1.entries(); e.hasMoreElements();) {
		JarEntry je = (JarEntry) e.nextElement();
		if (je.isDirectory()) continue ;  // Ignore directories
		if (!je.getName().endsWith(".class")) {
		    entries_checked++;
		    if (je.getName().compareTo("META-INF/MANIFEST.MF") == 0) {
			Manifest mf1 = new Manifest(jf1.getInputStream(je));
			Manifest mf2 = new Manifest(jf2.getInputStream(je));
			if (!mf1.equals(mf2)) {
			    Globals.log("Error: Manifests differ");
			    Globals.log("Manifest1");
			    Globals.log(mf1.getMainAttributes().entrySet().toString());
			    Globals.log("Manifest2");
			    Globals.log(mf2.getMainAttributes().entrySet().toString());
			}
		    } else {
			if (!compareStreams(jf1.getInputStream(je),jf2.getInputStream(je))) {
			    Globals.log("+++" + je.getName() + "+++");
			    Globals.log("Error: File:"  + je.getName() + " differs, use a diff util for further diagnostics");
			}
		    }
		}
	    }
	    Globals.log("Non-class entries checked/Total non-class entries = " + entries_checked + "/" + total_entries);
	} catch (IOException ioe) {
	    ioe.printStackTrace();
	}
    }
}


