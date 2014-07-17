/*
 * @(#)Util.java	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets;  

import com.sun.javadoc.*;
import java.lang.*;
import java.util.*;
import java.io.*;

/**
 * Utilities Class for Doclets.
 *
 * @author Atul M Dambalkar
 */
public class Util {

    /**
     * Return array of class members whose documentation is to be generated.
     * If the member is deprecated do not include such a member in the 
     * returned array.
     *
     * @param  members             Array of members to choose from.
     * @return ProgramElementDoc[] Array of eligible members for whom 
     *                             documentation is getting generated.
     */
    public static ProgramElementDoc[] excludeDeprecatedMembers(
                                                ProgramElementDoc[] members) {
        return 
             toProgramElementDocArray(excludeDeprecatedMembersAsList(members));
    }

    /**
     * Return array of class members whose documentation is to be generated.
     * If the member is deprecated do not include such a member in the 
     * returned array.
     *
     * @param  members    Array of members to choose from.
     * @return List       List of eligible members for whom 
     *                    documentation is getting generated.
     */
    public static List excludeDeprecatedMembersAsList(
                                                ProgramElementDoc[] members) {
        List list = new ArrayList();
        for (int i = 0; i < members.length; i++) {
            if (members[i].tags("deprecated").length == 0) {
                list.add(members[i]);
            }
        }
        Collections.sort(list);
        return list;
    }

    /**
     * Return the list of ProgramElementDoc objects as Array. 
     */
    public static ProgramElementDoc[] toProgramElementDocArray(List list) {
        ProgramElementDoc[] pgmarr = new ProgramElementDoc[list.size()];
        for (int i = 0; i < list.size(); i++) {
            pgmarr[i] = (ProgramElementDoc)(list.get(i));
        } 
        return pgmarr; 
    }

    /**
     * Return true if a non-public member found in the given array.
     *
     * @param  members Array of members to look into.
     * @return boolean True if non-public member found, false otherwise.
     */
    public static boolean nonPublicMemberFound(ProgramElementDoc[] members) {
        for (int i = 0; i < members.length; i++) {
            if (!members[i].isPublic()) {
                return true;
            }
        }
        return false;
    }

    /**
     * Search for the given method in the given class.
     *
     * @param  cd        Class to search into.
     * @param  method    Method to be searched.
     * @return MethodDoc Method found, null otherwise.
     */
    public static MethodDoc findMethod(ClassDoc cd, MethodDoc method) {
        MethodDoc[] methods = cd.methods();
        for (int i = 0; i < methods.length; i++) {
            if (method.name().equals(methods[i].name()) &&
                   method.signature().equals(methods[i].signature())) {
                return methods[i];
            }
        }
        return null;
    }

    /**
     * According to the Java Language Specifications, all the outer classes
     * and static inner classes are core classes.
     */
    public static boolean isCoreClass(ClassDoc cd) {
        return cd.containingClass() == null || cd.isStatic();
    }

    public static boolean matches(ProgramElementDoc doc1, 
                                  ProgramElementDoc doc2) {
        if (doc1 instanceof ExecutableMemberDoc && 
                doc2 instanceof ExecutableMemberDoc) {
            ExecutableMemberDoc ed1 = (ExecutableMemberDoc)doc1;
            ExecutableMemberDoc ed2 = (ExecutableMemberDoc)doc2;
            return ed1.name().equals(ed2.name()) && 
                       ed1.signature().equals(ed2.signature());
        } else {
            return doc1.name().equals(doc2.name());
        }
    }

    /**
     * Copy source file to destination file.
     *
     * @throws SecurityException
     * @throws IOException
     */
    public static void copyFile(File destfile, File srcfile)
                         throws IOException {
        byte[] bytearr = new byte[512];
        int len = 0;
        FileInputStream input = new FileInputStream(srcfile);
        FileOutputStream output = new FileOutputStream(destfile);
        try {
            while ((len = input.read(bytearr)) != -1) {
                output.write(bytearr, 0, len);
            }
        } catch (FileNotFoundException exc) {
        } catch (SecurityException exc) {
        } finally {
            input.close();
            output.close();
        }
    }

    /**
     * Copy a file in the resources directory to the destination 
     * directory (if it is not there already).  If
     * <code>overwrite</code> is true and the destination file
     * already exists, overwrite it.
     *
     * @param configuration  Holds the destination directory and error message
     * @param resourcefile   The name of the resource file to copy
     * @param overwrite      A flag to indicate whether the file in the
     *                       destination directory will be overwritten if
     *                       it already exists.
     */
    public static void copyResourceFile(Configuration configuration, 
                                        String resourcefile, 
                                        boolean overwrite) {
        String destdir = configuration.destdirname;
        String destresourcesdir = destdir + "resources";
        DirectoryManager.createDirectory(configuration, destresourcesdir);
        File destfile = new File(destresourcesdir, resourcefile);
        if(destfile.exists() && (! overwrite)) return;
        try {
            
            InputStream in = Configuration.class.getResourceAsStream("resources/" + resourcefile);
            
            if(in==null) return;
            
            OutputStream out = new FileOutputStream(destfile);
            byte[] buf = new byte[2048];
            int n;
            while((n = in.read(buf))>0) out.write(buf,0,n);
            
            in.close();
            out.close();
        } catch(Throwable t) {}
    }

    /**
     * For the class return all implemented interfaces including the 
     * superinterfaces of the implementing interfaces, also iterate over for 
     * all the superclasses. For interface return all the extended interfaces
     * as well as superinterfaces for those extended interfaces. 
     *
     * @param  cd         Class or interface whose implemented or
     *                    super interfaces are sought.
     * @return ClassDoc[] Array of all the required interfaces.
     */
    public static ClassDoc[] getAllInterfaces(ClassDoc classdoc) {
        Set interfaces = new HashSet();
        buildInterfaces(interfaces, classdoc);
        if (classdoc.isClass()) {
            ClassDoc icd = classdoc.superclass();
            while (icd != null) {
                buildInterfaces(interfaces, icd);
                icd = icd.superclass();
            }
        }
        ClassDoc[] intarr = new ClassDoc[interfaces.size()];
        interfaces.toArray(intarr);
        Arrays.sort(intarr);
        return intarr;   
    }

    private static void buildInterfaces(Set interfaces, ClassDoc cd) {
        ClassDoc[] iin = cd.interfaces();
        for (int i = 0; i < iin.length; i++) {
            interfaces.add(iin[i]);
            buildInterfaces(interfaces, iin[i]);
        }
    }

    public static List asList(ProgramElementDoc[] members) {
        List list = new ArrayList();
        for (int i = 0; i < members.length; i++) {
            list.add(members[i]);
        }
        return list;
    }
}

