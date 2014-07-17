/*
 * @(#)VisibleMemberMap.java	1.8 03/01/23
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
 * Algorithm:
 *
 * @author Atul M Dambalkar
 */
public class VisibleMemberMap {
    
    public static final int INNERCLASSES = 0;
    public static final int FIELDS = 1;
    public static final int CONSTRUCTORS = 2;
    public static final int METHODS = 3;
    
    public static final String STARTLEVEL = "start";
    
    /**
     * List of ClassDoc objects for which ClassMembers objects are built.
     */
    private final List visibleClasses = new ArrayList();
    
    /**
     * Map for each member name on to a map which contains members with same
     * name-signature. The mapped map will contain mapping for each MemberDoc
     * onto it's respecive level string.
     */
    private final Map memberNameMap = new HashMap();
    
    /**
     * Map of class and it's ClassMembers object.
     */
    private final Map classMap = new HashMap();
    
    /**
     * Class whose visible members are requested.
     */
    private final ClassDoc classdoc;
    
    /**
     * Member kind: InnerClasses/Fields/Methods?
     */
    private final int kind;
    
    /**
     * Deprected members should be excluded or not?
     */
    private final boolean nodepr;
    
    public VisibleMemberMap(ClassDoc classdoc, int kind, boolean nodepr) {
        this.classdoc = classdoc;
        this.nodepr = nodepr;
        this.kind = kind;
        new ClassMembers(classdoc, STARTLEVEL).build();
    }
    
    public List getVisibleClassesList() {
        sort(visibleClasses);
        return visibleClasses;
    }
    
    public List getMembersFor(ClassDoc cd) {
        ClassMembers clmembers = (ClassMembers)(classMap.get(cd));
        if (clmembers == null) {
            return new ArrayList();
        }
        return clmembers.getMembers();
    }
    
    /**
     * Sort the given mixed list of classes and interfaces to a list of
     * classes followed by interfaces traversed. Don't sort alphabetically.
     */
    private void sort(List list) {
        List classes = new ArrayList();
        List interfaces = new ArrayList();
        for (int i = 0; i < list.size(); i++) {
            ClassDoc cd = (ClassDoc)list.get(i);
            if (cd.isClass()) {
                classes.add(cd);
            } else {
                interfaces.add(cd);
            }
        }
        list.clear();
        list.addAll(classes);
        list.addAll(interfaces);
    }
    
    private void fillMemberLevelMap(List list, String level) {
        for (int i = 0; i < list.size(); i++) {
            ProgramElementDoc member = ((ProgramElementDoc)list.get(i));
            String membername = getDocName(member);
            Map memberLevelMap = (Map)(memberNameMap.get(membername));
            if (memberLevelMap == null) {
                memberLevelMap = new HashMap();
                memberNameMap.put(membername, memberLevelMap);
            }
            memberLevelMap.put(member, level);
        }
    }
    
    /**
     * Return the name for each doc item. For example,
     * if the doc item is of type PackageDoc for "java.lang" return string
     * "java.lang", if the doc item is ClassDoc for "java.lang.Object" return
     * string "java.lang.Object", if the doc item is MethodDoc for method
     * "wait()" in java.lang.Object class, then return string wait()".
     *
     * @param  doc    Doc Item for which name to be returned.
     * @return String name string as described above.
     */
    protected static String getDocName(ProgramElementDoc doc) {
        if (doc.isMethod() || doc.isConstructor()) {
            return doc.name() + ((ExecutableMemberDoc)doc).signature();
        } else if (doc.isField()) {
            return doc.name();
        } else { // it's a class or interface
            String classOrIntName = doc.name();
            //Strip off the containing class name because we only want the member name.
            classOrIntName = classOrIntName.indexOf('.') != 0 ? classOrIntName.substring(classOrIntName.lastIndexOf('.'), classOrIntName.length()) : classOrIntName;
            return "clint" + classOrIntName;
        }
    }
    
    private class ClassMembers {
        
        /**
         * The mapping class, whose inherited members are put in the
         * {@link #members} list.
         */
        private ClassDoc mappingClass;
        
        /**
         * List of inherited members from the mapping class.
         */
        private List members = new ArrayList();
        
        /**
         * Level/Depth of inheritance.
         */
        private String level;
        
        /**
         * Return list of inherited members from mapping class.
         *
         * @return List Inherited members.
         */
        public List getMembers() {
            return members;
        }
        
        private ClassMembers(ClassDoc mappingClass, String level) {
            this.mappingClass = mappingClass;
            this.level = level;
            if (!classMap.containsKey(mappingClass)) {
                classMap.put(mappingClass, this);
                visibleClasses.add(mappingClass);
            }
            
        }
        
        private void build() {
            if (kind == CONSTRUCTORS) {
                addMembers(mappingClass);
            } else {
                mapClass(this);
            }
        }
        
        private void mapClass(ClassMembers clmembers) {
            if (checkAccess(mappingClass)) {
                clmembers = this;
            }
            clmembers.addMembers(mappingClass);
            ClassDoc[] interfaces = mappingClass.interfaces();
            for (int i = 0; i < interfaces.length; i++) {
                String locallevel = level + (i + 1);
                ClassMembers cm = new ClassMembers(interfaces[i], locallevel);
                cm.mapClass(clmembers);
            }
            if (mappingClass.isClass()) {
                ClassDoc superclass = mappingClass.superclass();
                if (superclass != null) {
                    ClassMembers cm = new ClassMembers(superclass,
                                                       level + "c");
                    cm.mapClass(clmembers);
                }
            }
        }
        
        private boolean checkAccess(ClassDoc cd) {
            return cd.isPublic() ||
                cd.isProtected() ||
                (cd.isPrivate() && cd.isPackagePrivate());
        }
        
        /**
         * Get all the valid members from the mapping class. Get the list of
         * members for the class to be included into(ctii), also get the level
         * string for ctii. If mapping class member is not already in the
         * inherited member list and if it is visible in the ctii and not
         * overridden, put such a member in the inherited member list.
         * Adjust member-level-map, class-map.
         */
        private void addMembers(ClassDoc fromClass) {
            List cdmembers = getClassMembers(fromClass, true);
            List incllist = new ArrayList();
            for (int i = 0; i < cdmembers.size(); i++) {
                ProgramElementDoc pgmelem =
                    (ProgramElementDoc)(cdmembers.get(i));
                if (!found(members, pgmelem) &&
                    isVisible(pgmelem) &&
                    !isOverridden(pgmelem, level)) {
                    incllist.add(pgmelem);
                }
            }
            members.addAll(incllist);
            fillMemberLevelMap(getClassMembers(fromClass, false), level);
        }
        
        /**
         * Is given doc item visible in given classdoc in terms fo inheritance?
         * The given doc item is visible in the given classdoc if it is public
         * or protected and if it is package-private if it's containing class
         * is in the same package as the given classdoc.
         */
        private boolean isVisible(ProgramElementDoc pgmdoc) {
            if (pgmdoc.containingClass() == mappingClass) {
                return true;
            } else if (pgmdoc.isPrivate()) {
                return false;
            } else if (pgmdoc.isPackagePrivate()) {
                return pgmdoc.containingClass().containingPackage() ==
                    mappingClass.containingPackage();
            }
            return true;
        }
        
        /**
         * Return all available class members.
         */
        private List getClassMembers(ClassDoc cd, boolean filter) {
            ProgramElementDoc[] members = null;
            switch (kind) {
                case INNERCLASSES:
                    members = cd.innerClasses(filter);
                    break;
                case FIELDS:
                    members = cd.fields(filter);
                    break;
                case CONSTRUCTORS:
                    members = cd.constructors();
                    break;
                case METHODS:
                    members = cd.methods(filter);
                    break;
                default:
                    members = new ProgramElementDoc[0];
            }
            if (nodepr) {
                return Util.excludeDeprecatedMembersAsList(members);
            }
            return Util.asList(members);
        }
        
        private boolean found(List list, ProgramElementDoc elem) {
            for (int i = 0; i < list.size(); i++) {
                ProgramElementDoc pgmelem = (ProgramElementDoc)list.get(i);
                if (Util.matches(pgmelem, elem)) {
                    return true;
                }
            }
            return false;
        }
        
        
        /**
         * Is member overridden? The member is overridden if it is found in the
         * same level hierarchy e.g. member at level "11" overrides member at
         * level "111".
         */
        private boolean isOverridden(ProgramElementDoc pgmdoc, String level) {
            Map memberLevelMap = (Map)(memberNameMap.get(getDocName(pgmdoc)));
            if (memberLevelMap == null) {
                return false;
            }
            String mappedlevel = null;
            Iterator iterator = memberLevelMap.values().iterator();
            while (iterator.hasNext()) {
                mappedlevel = (String)(iterator.next());
                if (mappedlevel.equals(STARTLEVEL) ||
                    level.startsWith(mappedlevel)) {
                    return true;
                }
            }
            return false;
        }
    }
}

