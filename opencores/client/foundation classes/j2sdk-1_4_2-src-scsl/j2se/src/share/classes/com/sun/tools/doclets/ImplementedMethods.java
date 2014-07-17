/*
 * @(#)ImplementedMethods.java	1.8 03/01/23
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
 * For a given class method, build an array of interface methods which it
 * implements.
 *
 * @author Atul M Dambalkar
 */
public class ImplementedMethods {
    
    private List methlist = new ArrayList();
    private final ClassDoc classdoc;
    private final MethodDoc method;
    
    public ImplementedMethods(MethodDoc method) {
        this.method = method;
        classdoc = method.containingClass();
    }
    
    /**
     * Return the array of interface methods which the method passed in the
     * constructor is implementing. The search/build order is as follows:
     * <pre>
     * 1. Search in all the immediate interfaces which this method's class is
     *    implementing. Do it recursively for the superinterfaces as well.
     * 2. Traverse all the superclasses and search recursively in the
     *    interfaces which those superclasses implement.
     *</pre>
     *
     * @return MethodDoc[] Array of implemented methods.
     */
    public MethodDoc[] build() {
        ClassDoc icd = classdoc;
        while (icd != null) {
            buildImplementedMethodList(icd.interfaces());
            icd = icd.superclass();
        }
        return (MethodDoc[])methlist.toArray(new MethodDoc[methlist.size()]);
    }
    
    /**
     * Search for the method in the array of interfaces. If found check if it is
     * overridden by any other subinterface method which this class
     * implements. If it is not overidden, add it in the method list.
     * Do this recursively for all the extended interfaces for each interface
     * from the array passed.
     */
    private void buildImplementedMethodList(ClassDoc[] intfacs) {
        for (int i = 0; i < intfacs.length; i++) {
            MethodDoc found = Util.findMethod(intfacs[i], method);
            if (found != null) {
                removeOverriddenMethod(found);
                if (!overridingMethodFound(found)) {
                    methlist.add(found);
                }
            }
            ClassDoc[] iin = intfacs[i].interfaces();
            buildImplementedMethodList(iin);
        }
    }
    
    /**
     * Search in the method list and check if it contains a method which
     * is overridden by the method as parameter.  If found, remove the
     * overridden method from the method list.
     *
     * @param method Is this method overriding a method in the method list.
     */
    private void removeOverriddenMethod(MethodDoc method) {
        ClassDoc overriddenClass = method.overriddenClass();
        if (overriddenClass != null) {
            for (int i = 0; i < methlist.size(); i++) {
                ClassDoc cd = ((MethodDoc)methlist.get(i)).containingClass();
                if (cd == overriddenClass || overriddenClass.subclassOf(cd)) {
                    methlist.remove(i);  // remove overridden method
                    return;
                }
            }
        }
    }
    
    /**
     * Search in the already found methods' list and check if it contains
     * a method which is overriding the method parameter or is the method
     * parameter itself.
     *
     * @param method MethodDoc Method to be searched in the Method List for
     * an overriding method.
     */
    private boolean overridingMethodFound(MethodDoc method) {
        ClassDoc containingClass = method.containingClass();
        for (int i = 0; i < methlist.size(); i++) {
            MethodDoc listmethod = (MethodDoc)methlist.get(i);
            if (containingClass == listmethod.containingClass()) {
                // it's the same method.
                return true;
            }
            ClassDoc cd = listmethod.overriddenClass();
            if (cd == null) {
                continue;
            }
            if (cd == containingClass || cd.subclassOf(containingClass)) {
                return true;
            }
        }
        return false;
    }
}

