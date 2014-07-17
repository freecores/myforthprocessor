/*
 * @(#)AbstractExecutableMemberTaglet.java	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.standard.tags;

import com.sun.tools.doclets.*;
import com.sun.tools.doclets.standard.*;
import com.sun.javadoc.*;

/**
 * An abstract class for Taglets in
 * <code>ExecutableMembers</code> in the standard doclet.
 * @author Jamie Ho
 * @since 1.4
 */
public abstract class AbstractExecutableMemberTaglet implements Taglet{
    
    protected String name = "Default";
    
    /**
     * Return true if this <code>Taglet</code>
     * is used in constructor documentation.
     * @return true if this <code>Taglet</code>
     * is used in constructor documentation and false
     * otherwise.
     */
    public boolean inConstructor() {
        return true;
    }
    
    /**
     * Return true if this <code>Taglet</code>
     * is used in field documentation.
     * @return true if this <code>Taglet</code>
     * is used in field documentation and false
     * otherwise.
     */
    public boolean inField() {
        return false;
    }
    
    /**
     * Return true if this <code>Taglet</code>
     * is used in method documentation.
     * @return true if this <code>Taglet</code>
     * is used in method documentation and false
     * otherwise.
     */
    public boolean inMethod() {
        return true;
    }
    
    /**
     * Return true if this <code>Taglet</code>
     * is used in overview documentation.
     * @return true if this <code>Taglet</code>
     * is used in method documentation and false
     * otherwise.
     */
    public boolean inOverview() {
        return false;
    }
    
    /**
     * Return true if this <code>Taglet</code>
     * is used in package documentation.
     * @return true if this <code>Taglet</code>
     * is used in package documentation and false
     * otherwise.
     */
    public boolean inPackage() {
        return false;
    }
    
    /**
     * Return true if this <code>Taglet</code>
     * is used in type documentation (classes or interfaces).
     * @return true if this <code>Taglet</code>
     * is used in type documentation and false
     * otherwise.
     */
    public boolean inType() {
        return false;
    }
    
    /**
     * Return true if this <code>Taglet</code>
     * is an inline tag.
     * @return true if this <code>Taglet</code>
     * is an inline tag and false otherwise.
     */
    public boolean isInlineTag() {
        return false;
    }
    
    /**
     * Return the name of this custom tag.
     * @return the name of this custom tag.
     */
    public String getName() {
        return name;
    }
    
    /**
     * This method always returns "" because this <code>Taglet</code> requires more
     * information to convert a Tag to a String.
     * @param tag the <code>Tag</code> representation of this custom tag.
     * @return an empty String.
     */
    public String toString(Tag tag) {
        return "";
    }
    
    /**
     * This method always returns "" because this <code>Taglet</code>  requires more
     * information to convert a Tag to a String.
     * @param tags an array of <code>Tag</code>s representing of this custom tag.
     * @return an empty String.
     */
    public String toString(Tag[] tags){
        return "";
    }
    
    /**
     * Given a method, find a method that it overrides or implements
     * so that we can inherit documentation.
     * @param method the method to inherit documentation from.
     * @return an overriden or implemented method that we can inherit
     * documentation from.  Return if null no such method is found.
     */
    
    protected MethodDoc getInheritedMethodDoc(MethodDoc method){
        MethodDoc omd = method.overriddenMethod();
        if (omd == null) {
            //This method does not override any method in the superclass.
            //Let's try to find a method that it implements from an interface.
            MethodDoc[] implementedMethods = (new ImplementedMethods(method)).build();
            omd = implementedMethods != null && implementedMethods.length > 0 ?
                implementedMethods[0] : null;
        }
        return omd;
    }
    
    public abstract String toString(Doc holder, HtmlStandardWriter writer);
}
