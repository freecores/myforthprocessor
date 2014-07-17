/*
 * @(#)ClassUtils.java	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


/*****************************************************************************/
/*                    Copyright (c) IBM Corporation 1998                     */
/*                                                                           */
/* IBM Confidential                                          ClassUtils.java */
/*                                                                           */
/* OCO Source Materials                                                      */
/*                                                                           */
/* (C) Copyright IBM Corp. 1998                                              */
/*                                                                           */
/* The source code for this program is not published or otherwise            */
/* divested of its trade secrets, irrespective of what has been              */
/* deposited with the U.S. Copyright Office.                                 */
/*                                                                           */
/*****************************************************************************/

package sun.rmi.rmic;

import java.io.File;
import sun.tools.java.ClassPath;
import sun.tools.java.ClassFile;
import java.io.IOException;
import sun.tools.java.Identifier;
import java.util.StringTokenizer;

/**
 * ClassUtils provides miscellaneous static utility methods related to
 * classes and their packages.
 * @author Bryan Atsatt
 */

public class ClassUtils implements sun.rmi.rmic.Constants {

    /**
     * Return the directory that should be used for output for a given
     * class.
     * @param theClass The fully qualified name of the class.
     * @param rootDir The directory to use as the root of the
     * package heirarchy.  May be null, in which case the current
     * ClassPath is searched to find the directory in which to create
     * the output file.  If that search fails (most likely because the
     * package directory lives in a zip or jar file rather than the
     * file system), and the class has no package, the current working
     * directory (user.dir) is used.  If the search fails and the class
     * has a package, an error message is generated and an InternalError
     * is thrown to terminate the compile.
     */
	public static File getOutputDirectoryFor(   Identifier theClass,
	                                            File rootDir,
	                                            BatchEnvironment env) {
        
        File outputDir = null;
        String className = theClass.getFlatName().toString().replace('.', SIGC_INNERCLASS);    		
		String qualifiedClassName = className;
 	    String packagePath = null;
 	    String packageName = theClass.getQualifier().toString();
		
		if (packageName.length() > 0) {
    		qualifiedClassName = packageName + "." + className;
 	        packagePath = packageName.replace('.', File.separatorChar);
 	    }

        // Do we have a root directory?
        
		if (rootDir != null) {
		    
            // Yes, do we have a package name?
            
            if (packagePath != null) {
        	    
    		    // Yes, so use it as the root. Open the directory...
    		    
    		    outputDir = new File(rootDir, packagePath);
    		    
    		    // Make sure the directory exists...
    		    
                ensureDirectory(outputDir,env);
                
        	} else {
        	    
        	    // Default package, so use root as output dir...
        	    
        		outputDir = rootDir;
            }		    
		} else {
		    
		    // No root directory. Try to find the package directory...

            outputDir = packageDirectory(qualifiedClassName,env.getClassPath(),false);
	    
	        // Did we find it?
	        
            if (outputDir == null) {
                
                // No. Get the current working directory...
                
                String workingDirPath = System.getProperty("user.dir");
                File workingDir = new File(workingDirPath);
                
                // Do we have a package name?
                
                if (packagePath == null) {
                    
                    // No, so use working directory...
           
                    outputDir = workingDir;
                    
                } else {
                    
                    // Yes, is the current working directory in
                    // the classpath?
                    
                    if (directoryInPath(workingDirPath,env.getClassPath().toString())) {
                    
                        // Yes, so use working directory as the root...
                        
      		            outputDir = new File(workingDir, packagePath);
            		    
    		            // Make sure the directory exists...
            		    
    		            ensureDirectory(outputDir,env);
    		      
                    } else {
                    
                        // Yes, so error...
                        
                        env.error(0, "rmic.no.output.dir", qualifiedClassName);
                        throw new InternalError();
                    }
                }
            }
	    }

	    // Finally, return the directory...
	    
	    return outputDir;
	}
 
    private static void ensureDirectory (File dir, BatchEnvironment env) {
    	if (!dir.exists()) {
            dir.mkdirs();
            if (!dir.exists()) {
                env.error(0,"rmic.cannot.create.dir",dir.getAbsolutePath());
                throw new InternalError();
            }
    	}
    }
    
    /**
     * Return the directory which contains a given class (either .java or .class).
     * Uses the current system classpath.
     * @param className Fully qualified class name.
     * @param requireFile True if .class or .java file must be found. False if
     * ok to return a directory which does not contain file.
     * @return the directory or null if none found (or zipped).
     */
	public static File packageDirectory (String className, boolean requireFile) {
        ClassPath path = new ClassPath(System.getProperty("java.class.path"));
        File result = packageDirectory(className,path,requireFile);
        try {
            path.close();
        } catch (IOException e) {}
        
        return result;
	}
	
    /**
     * Return the directory which contains a given class (either .java or .class).
     * @param className Fully qualified class name.
     * @param path the class path.
     * @param requireFile True if .class or .java file must be found. False if
     * ok to return a directory which does not contain file.
     * @return the directory or null if none found (or zipped).
     */
	public static File packageDirectory (String className, ClassPath path, boolean requireFile) {
        
        // Try binary first, then source, then directory...
        
        File result = packageDirectory(className,path,".class");
        if (result == null) {
            result = packageDirectory(className,path,".java");
            if (result == null && !requireFile) {
                int i = className.lastIndexOf('.');
                if (i >= 0) {
                    String packageName = className.substring(0,i);
                    ClassFile cls = path.getDirectory(packageName.replace('.',File.separatorChar));
                    if (cls != null && ! cls.isZipped()) {
                		result = new File(cls.getPath());
                    }
                }
            }
        }
        return result;
	}
	
	private static boolean directoryInPath(String dirPath, String path) {
        if (!dirPath.endsWith(File.separator)) {
            dirPath = dirPath + File.separator;
        }
        StringTokenizer st = new StringTokenizer(path,"\t\n\r"+File.pathSeparator);
        while (st.hasMoreTokens()) {
            String entry = st.nextToken();
            if (!entry.endsWith(".zip") &&
                !entry.endsWith(".jar")) {
                 
                if (entry.equals(".")) {
                    return true;
                } else {
                    if (!entry.endsWith(File.separator)) {
                        entry = entry + File.separator;  
                    }
                    if (entry.equalsIgnoreCase(dirPath)) {
                        return true;   
                    }
                }
            }
        }
      
		return false;
	}
	
    private static File packageDirectory (String className, ClassPath path, String fileExt) {
        
        ClassFile cls = path.getFile(className.replace('.',File.separatorChar) + fileExt);

        if (cls != null && ! cls.isZipped()) {
    		File file = new File(cls.getPath());
    		File dir = new File(file.getParent());
    		return dir;
        }

		return null;
    }
}

