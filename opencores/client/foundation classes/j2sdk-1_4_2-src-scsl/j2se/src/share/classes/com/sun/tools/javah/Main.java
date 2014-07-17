/*
 * @(#)Main.java	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 

package com.sun.tools.javah;


import java.io.*;

/**
 * Javah generates support files for native methods.
 * Parse commandline options & Invokes javadoc to execute those commands. 
 *
 * @author Sucheta Dambalkar
 */
public class Main{ 
    
    /*
     * Parse arguments given for javah to give proper error messages.
     */
    public static void main(String args[]){

	// by default use the old implementation, unless we see -Xnew
    checkXnew:
	{
	    for (int i=0; i<args.length; i++) {
		if (args[i].equals("-Xnew")) {
		    // get rid of -Xnew
		    String[] newjhargs = new String[args.length-1];
		    for (int k=0, l=0; k < args.length; k++)
			if (k != i) newjhargs[l++] = args[k];
		    args = newjhargs;
		    break checkXnew;
		}
	    }
	    com.sun.tools.javah.oldjavah.Main.main(args);
	    return;
	}

	if (args.length == 0) {
	    Util.usage(1);
	}
	for ( int i = 0; i < args.length; i++) {
	    if (args[i].equals("-o")) {
		i++;
		if(i >= args.length){
		    Util.usage(1);
		}else if(args[i].charAt(0) == '-'){
		    Util.error("no.outputfile.specified");
		}else if((i+1) >= args.length){
		    Util.error("no.classes.specified");
		}
	    } else if (args[i].equals("-d")) {
		i++;
		if(i >= args.length){
		    Util.usage(1);
		}else if(args[i].charAt(0) == '-')  {
		    Util.error("no.outputdir.specified");
		}else if((i+1) >= args.length){
		    Util.error("no.classes.specified");
		}
	    } else if (args[i].equals("-td")) {
		/* Ignored.  Generate tmp files to memory. */
		i++;
		if (i == args.length)
		    Util.usage(1);
	    } else if (args[i].equals("-stubs")) {
		if((i+1) >= args.length){
		    Util.error("no.classes.specified");
		}
	    } else if (args[i].equals("-v") || args[i].equals("-verbose")) {
		if((i+1) >= args.length){
		    Util.error("no.classes.specified");
		}
	    } else if ((args[i].equals("-help")) || (args[i].equals("--help")) 
		       || (args[i].equals("-?")) || (args[i].equals("-h"))) {
		Util.usage(0);
	    } else if (args[i].equals("-trace")) {
		System.err.println(Util.getText("tracing.not.supported"));
	    } else if (args[i].equals("-version")) {
		if((i+1) >= args.length){
		    Util.version();
		}
	    } else if (args[i].equals("-jni")) {
		if((i+1) >= args.length){
		    Util.error("no.classes.specified");
		}
	    } else if (args[i].equals("-force")) {
		if((i+1) >= args.length){
		    Util.error("no.classes.specified");
		}
	    } else if (args[i].equals("-old")) {
		if((i+1) >= args.length){
		    Util.error("no.classes.specified");
		}
	    } else if (args[i].equals("-Xold")) {
		String[] oldjhargs = new String[(args.length)-1];
		int l = 0;
		for(int k = 0; k < args.length; k++){
		    if(!(args[k].equals("-Xold"))){
			oldjhargs[l] = args[k];
			l++;
		    }
		}
		com.sun.tools.javah.oldjavah.Main.main(oldjhargs);
		return;
	    } else if (args[i].equals("-Xllni")) {
		if((i+1) >= args.length){
		    Util.error("no.classes.specified");
		}
	    } else if (args[i].equals("-llni")) {
		if((i+1) >= args.length){
		    Util.error("no.classes.specified");
		}
	    } else if (args[i].equals("-llniDouble")) {
		if((i+1) >= args.length){
		    Util.error("no.classes.specified");
		}
	    } else if (args[i].equals("-classpath")) {
		i++;
		if(i >= args.length){
		    Util.usage(1);
		}else if(args[i].charAt(0) == '-') {
		    Util.error("no.classpath.specified");
		}else if((i+1) >= args.length){
		    Util.error("no.classes.specified");
		}
	    } else if (args[i].equals("-bootclasspath")) {
		i++;
		if(i >= args.length){
		    Util.usage(1);
		}else if(args[i].charAt(0) == '-'){
		    Util.error("no.bootclasspath.specified");
		}else if((i+1) >= args.length){
		    Util.error("no.classes.specified");
		}
	    } else if (args[i].charAt(0) == '-') {
		Util.error("unknown.option", args[i], null, true);
		
	    } else {
		//break; /* The rest must be classes. */
	    }
	}
	
	/* Invoke javadoc */
	
	String[] javadocargs = new String[args.length + 2];
	int i = 0;
	
	for(; i < args.length; i++) {
	    javadocargs[i] = args[i];
	}
	
	javadocargs[i] = "-private";
	i++;
	javadocargs[i] = "-Xclasses";
	
	com.sun.tools.javadoc.Main.execute("javadoc", "com.sun.tools.javah.MainDoclet", javadocargs);
    }
}

























































