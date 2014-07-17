/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)Main.java	1.9 03/01/23
 */

// The Main Entry point
package sun.tools.pack.verify;

import java.io.*;
import java.util.*;
import java.util.jar.*;

class Main {

    public static void main(String args[]) {
	Globals.getInstance();
	if (args == null || args.length < 2) {
	    System.err.println("Usage: Class_path_reference Class_path_to_compare [-O] [ -c ClassName] [ -l [Dirname|]LogfileName ]");
	    System.exit(1);
	}
	String refJarFile=null;
	String cmpJarFile=null;
	String specificClass=null;
	String logDirFileName=null;
	
	for (int i = 0 ; i < args.length ; i++ ) {
	    if (i == 0) {
		refJarFile = args[0];
		continue ;
	    }
	    if (i == 1) {
		cmpJarFile = args[1];
		continue;
	    }
	    
	    if ( args[i].startsWith("-O")) {
		Globals.setCheckJarClassOrdering(true);
	    }

	    if ( args[i].startsWith("-c") ) {
		i++;
		specificClass = args[i].trim();
	    }
	    if ( args[i].startsWith("-l") ) {
		i++;
		logDirFileName = args[i].trim();
	    }
	}

	Globals.openLog(logDirFileName);
	
	String f1 = new File(refJarFile).getAbsoluteFile().toString();
	String f2 = new File(cmpJarFile).getAbsoluteFile().toString();
	
	Globals.log("Logging to -> " + Globals.getLogFileName());
	Globals.log("Ref jar -> " + f1);
	Globals.log("Jar compared -> " + f2);
	
	System.out.println("Logging to -> " + Globals.getLogFileName());
	System.out.println("Ref jar -> " + f1);
	System.out.println("Jar compared ->" + f2);
	if (Globals.checkJarClassOrdering()) System.out.println("Check jar class ordering");
	if (specificClass == null) {
	    System.out.println("Comparing all entries");
	    JarFileCompare.jarCompare(refJarFile, cmpJarFile);
	    ClassCompare.compareClass(refJarFile, cmpJarFile, specificClass);
	} else {
	    System.out.println("Comparing class ->" + specificClass);
	    ClassCompare.compareClass(refJarFile, cmpJarFile, specificClass);
	}
    }
}






