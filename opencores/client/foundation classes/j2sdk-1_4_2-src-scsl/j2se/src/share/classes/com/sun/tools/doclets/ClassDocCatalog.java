/*
 * @(#)ClassDocCatalog.java 1.3 01/12/03
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets;

import com.sun.javadoc.*;
import java.util.*;

/**
 * This class acts as an artificial PackageDoc for classes specified
 * on the command line when running Javadoc.  For example, if you
 * specify several classes from package java.lang, this class will catalog
 * those classes so that we can retrieve all of the classes from a particular
 * package later.
 *
 * @author Jamie Ho
 * @since JDK 1.4
 */
 
 public class ClassDocCatalog {
 
     /**
      * Stores the set of packages that the classes specified on the command line
      * belong to.  Note that the default package is "".
      */
     private Set packageSet;
     
     
     /**
      * Stores all classes for each package
      */
     private Map allClasses;
     
     /**
      * Stores ordinary classes (excluding Exceptions and Errors) for each
      * package
      */
     private Map ordinaryClasses;
     
     /**
      * Stores exceptions for each package
      */
     private Map exceptions;
     
     /**
      * Stores errors for each package
      */
     private Map errors;
     
     /**
      * Stores interfaces for each package
      */
     private Map interfaces;
     
     /**
      * Construct a new ClassDocCatalog.
      *
      * @param classdocs the array of ClassDocs to catalog
      */
     public ClassDocCatalog (ClassDoc[] classdocs) {
         init();
         for (int i = 0; i < classdocs.length; i++) {
             addClassDoc(classdocs[i]);
         }
     }
     
     /**
      * Construct a new ClassDocCatalog.
      *
      */
     public ClassDocCatalog () {
         init();
     }
     
     private void init() {
         allClasses = new HashMap();
         ordinaryClasses = new HashMap();
         exceptions = new HashMap();
         errors = new HashMap();
         interfaces = new HashMap();
         packageSet = new HashSet();
     }
     
     /**
      * Add the given class to the catalog.
      * @param classdoc the ClassDoc to add to the catelog.
      */
      public void addClassDoc(ClassDoc classdoc) {
        if (classdoc == null) {
            return;
        }
        addClass(classdoc, allClasses);
        if (classdoc.isOrdinaryClass())
            addClass(classdoc, ordinaryClasses);
        if (classdoc.isException())
            addClass(classdoc, exceptions);
        if (classdoc.isError())
           addClass(classdoc, errors);
        if (classdoc.isInterface())
           addClass(classdoc, interfaces);
      }
      
      /**
       * Add the given class to the given map.
       * @param classdoc the ClassDoc to add to the catelog.
       * @param map the Map to add the ClassDoc to.
       */
      private void addClass(ClassDoc classdoc, Map map) {
          
          PackageDoc pkg = classdoc.containingPackage();
          if (pkg.isIncluded()) {
              //No need to catalog this class since it's package is
              //included on the command line
              return;
          }
          String key = pkg.name();
          Set s = (Set) map.get(key);
          if (s == null) {
              packageSet.add(key);
              s = new HashSet();
          }
          s.add(classdoc);
          map.put(key, s);
          
      }
      
      private ClassDoc[] getArray(Map m, String key) {
          Set s = (Set) m.get(key);
          if (s == null) {
              return new ClassDoc[] {};
          } else {
              return (ClassDoc[]) s.toArray(new ClassDoc[] {});
          }
      }
      
      /**
       * Return all of the classes specified on the command-line that
       * belong to the given package.
       * @param packageName the name of the package specified on the
       * command-line.
       */
      public ClassDoc[] allClasses(String packageName) {
          return getArray(allClasses, packageName);
      }
     
     /**
      * Return the array of package names that this catalog stores
      * ClassDocs for.
      */
     public String[] packageNames() {
         return (String[]) packageSet.toArray(new String[] {});
     }
     
     
      /**
       * Return all of the errors specified on the command-line
       * that belong to the given package.
       * @param packageName the name of the package specified on the
       * command-line.
       */
      public ClassDoc[] errors(String packageName) {
          return getArray(errors, packageName);
      }
      
      /**
       * Return all of the exceptions specified on the command-line
       * that belong to the given package.
       * @param packageName the name of the package specified on the
       * command-line.
       */
      public ClassDoc[] exceptions(String packageName) {
          return getArray(exceptions, packageName);
      }
      
      /**
       * Return all of the interfaces specified on the command-line
       * that belong to the given package.
       * @param packageName the name of the package specified on the
       * command-line.
       */
      public ClassDoc[] interfaces(String packageName) {
          return getArray(interfaces, packageName);
      }
      
      /**
       * Return all of the ordinary classes specified on the command-line
       * that belong to the given package.
       * @param packageName the name of the package specified on the
       * command-line.
       */
      public ClassDoc[] ordinaryClasses(String packageName) {
          return getArray(ordinaryClasses, packageName);
      }
}
