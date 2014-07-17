/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)ClassCompare.java	1.9 03/01/23
 */
package sun.tools.pack.verify;

import java.io.*;
import java.util.*;
import java.util.jar.*;

import com.sun.classinfo.*;
import com.sun.classinfo.parser.*;
import com.sun.classinfo.validator.*;
import com.sun.classinfo.classfile.*;
import com.sun.classinfo.classfile.exceptions.*;

/**
 * ClassCompare is the starting point for Comparing a) A Class between two Jar Files
 * or b) Iterate over all the classes in JarFile A and compare it with the same class
 * in the other. Normally the first Jar File will be the reference and the second is
 * the target.
 * The comparison Criteria is as follows:
 * 1. Compare all the meta data of a class ie.
 *    1. Magic number
 *    2. Version number
 *    3. Number of Fields
 *    4. Number of Methods
 *    5. Number of Interfaces.
 *    6. Number of Class Attributes
 *
 * 2. Field Compare
 *    1. Compare the Field String compare of:
 *       A. Access Flags.
 *       B. Type of Field
 *       C. Field Name
 *    2. Method Compare
 *       A.
 */


public class ClassCompare {
    
    private static ClassPath _cp1;
    private static ClassPath _cp2;
    private static Parser _parser;
    
    /*
     * Compares class or classes depending on classname
     */
    private static void compareClass0(String CtStr_classpath, String CtStr_classpathCompare,  String classname) {
	try {
	    _cp1 = PathFactory.createClassPath(CtStr_classpath);
	    _cp2 = PathFactory.createClassPath(CtStr_classpathCompare);
	     
	    int total_classes = _cp1.getNumberOfClasses();
	    int checked_classes = 0;
	    
	    _parser = new ClassFileParser();

	    ClassMetaData cmd1 = new ClassMetaData();
	    ClassMetaData cmd2 = new ClassMetaData();
	    ClassFileEntry e1;
	    
	    if (classname == null) { // iterate over all classes in reference classpath
		for (Iterator i = _cp1.getIterator(); i.hasNext(); ) {
		    checked_classes++;
		    e1 = (ClassFileEntry)i.next();
		    compareClass1(e1, cmd1, cmd2);
		}
	    } else {
		e1 = _cp1.get(classname);
		compareClass1(e1, cmd1, cmd2);
	    }
	    Globals.log("Class entries checked/Total class-entries =" + checked_classes + "/" + total_classes);
	} catch (IOException ioe) {
	    ioe.printStackTrace();
	} catch (ClassFileException cfe) {
	    cfe.printStackTrace();
	}
    }
    
    private static void compareClass1(ClassFileEntry e1, ClassMetaData cmd1, ClassMetaData cmd2)
    throws IOException {
	
	if (e1 == null) {
	    System.err.println("ClassFileEntry null....Exiting....");
	    System.exit(2);
	}
	
	Globals.log("+++"+e1.getFQName()+"+++");
	
	ClassFile cf1 = null;
	try {
	    cf1 = _parser.parse(e1);
	    cmd1.initClassMetaData(cf1, _cp1);
	    // System.out.println(cmd1.toString());
	} catch(ClassFileException cfe) {
	    Globals.log("Caught exception:");
	    Globals.log("\t" + cfe);
	}
	
	ClassFileEntry e2;
	
	ClassFile cf2 = null;
	try {
	    e2 = _cp2.get(e1.getFQName());
	    cf2 = _parser.parse(e2);
	    cmd2.initClassMetaData(cf2, _cp2);
	    // System.out.println(cmd2.toString());
	} catch (ClassFileException cfe) {
	    Globals.log("Caught exception: Class =" + e1.getFQName());
	    Globals.log("\t" + cfe);
	}
	
	if (cf1 != null && cf2 != null) {
	    
	    Globals.println("---------------------REFERENCE CLASS DUMP-------------------");
	    Globals.println(cf1.toString());
	    Globals.println("---------------------COMPARED CLASS DUMP-------------------");
	    Globals.println(cf1.toString());
	    Globals.println("---------------------END OF CLASS DUMP----------------------");
	    CPoolRef cpr = new CPoolRef(cf1);
	    cpr.getAllCPReferences(cf1);
	    Globals.println(cpr._usedCPRefs.toString());
	    cmd1.doClassMetaDataDiffs(cmd2);
	    
	    //Level2 Compare
	    FieldCompare fc = new FieldCompare();
	    fc.doFieldDiffs(cf1, cf2);
	    
	    MethodCompare mc = new MethodCompare();
	    mc.doMethodDiffs(cf1, cf2);
	    
	    ClassAttributeCompare cac = new ClassAttributeCompare(cpr);
	    cac.doAttributeDiffs(cf1, cf2);
	}
	
	Globals.log(" ");
	
	// Stomp on the references to conserve memory
	e1 = null;
	e2 = null;
	
    }
    /**
     * Given two Class Paths could be jars the first being a reference
     * will execute a series of comparisons on the classname  specified
     * The className could be null in which case it will iterate through
     * all the classes, otherwise it will compare one class and exit.
     */
    
    public static void compareClass(String classPath1, String classPath2, String className) {
	compareClass0(classPath1, classPath2, className);
    }
 
}




