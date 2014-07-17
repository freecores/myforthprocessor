/*
 * @(#)MethodFinder.java	1.5 03/01/23
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
 * This class is useful for searching a method which has documentation 
 * comment and documentation tags. The method is searched in all the 
 * superclasses and interfaces(subsequently super-interfaces also) 
 * recursively.
 */
public abstract class MethodFinder {     

    abstract boolean isCorrectMethod(MethodDoc method);
    
    public MethodDoc search(ClassDoc cd, MethodDoc method) {
        MethodDoc meth = searchInterfaces(cd, method);
        if (meth != null) {
            return meth;
        }
        ClassDoc icd = cd.superclass();        
        if (icd != null) {
            meth = Util.findMethod(icd, method);
            if (meth != null) {
  	        if (isCorrectMethod(meth)) {
                    return meth;
                }
            }
            return search(icd, method);
        }
        return null;
    }

    public MethodDoc searchInterfaces(ClassDoc cd, MethodDoc method) {
        ClassDoc[] iin = cd.interfaces();
        for (int i = 0; i < iin.length; i++) {
            MethodDoc meth = Util.findMethod(iin[i], method);
            if (meth != null) {
	        if (isCorrectMethod(meth)) {
                    return meth;
                }
            }
        }
        for (int i = 0; i < iin.length; i++) {
            MethodDoc meth = searchInterfaces(iin[i], method);
            if (meth != null) {
                return meth;
            }
        }
        return null;
    }
}
