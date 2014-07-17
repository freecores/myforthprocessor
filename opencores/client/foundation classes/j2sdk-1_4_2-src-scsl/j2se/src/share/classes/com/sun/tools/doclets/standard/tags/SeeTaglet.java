/*
 * @(#)SeeTaglet.java   1.6 01/12/03
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.standard.tags;

import com.sun.tools.doclets.standard.*;
import com.sun.tools.doclets.*;
import com.sun.javadoc.*;
import java.util.*;

/**
 * A taglet that represents the @see tag.
 * @author Jamie Ho
 * @since 1.4
 */
public class SeeTaglet extends AbstractExecutableMemberTaglet{
    
    public SeeTaglet() {
        name = "see";
    }
    
    /**
     * Return true if this <code>Taglet</code>
     * is used in field documentation.
     * @return true if this <code>Taglet</code>
     * is used in field documentation and false
     * otherwise.
     */
    public boolean inField() {
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
        return true;
    }
    
    /**
     * Return true if this <code>Taglet</code>
     * is used in package documentation.
     * @return true if this <code>Taglet</code>
     * is used in package documentation and false
     * otherwise.
     */
    public boolean inPackage() {
        return true;
    }
    
    /**
     * Return true if this <code>Taglet</code>
     * is used in type documentation (classes or interfaces).
     * @return true if this <code>Taglet</code>
     * is used in type documentation and false
     * otherwise.
     */
    public boolean inType() {
        return true;
    }
    
    /**
     * Given an array of <code>Tag</code>s representing this custom
     * tag, return its string representation.
     * @param tags the array of <code>Tag</code>s representing of this custom tag.
     * @param holder the ExecutableMemberDoc that holds this tag.
     * @param writer the HtmlStandardWriter that will write this tag.
     * @return the string representation of this <code>Tag</code>.
     */
    public String toString(Doc holder, HtmlStandardWriter writer) {
        SeeTag[] tags = holder.seeTags();
        String result = "";
        MethodDoc inheritedMethod;
        if (tags.length == 0 &&
            holder instanceof MethodDoc &&
                (inheritedMethod = getInheritedMethodDoc((MethodDoc) holder)) != null) {
            tags = inheritedMethod.seeTags();
        }
        if (tags.length > 0) {
            result = addHeader(result, writer);
            for (int i = 0; i < tags.length; ++i) {
                if (i > 0) {
                    result += ", \n";
                }
                result += writer.seeTagToString(tags[i]);
            }
        }
        if (holder.isField() && ((FieldDoc)holder).constantValue() != null) {
            //Automatically add link to constant values page for constant fields.
            result = addHeader(result, writer);
            result += writer.getHyperLink(writer.relativepath + writer.configuration.CONSTANTS_FILE_NAME
                                              + "#" + ((FieldDoc) holder).qualifiedName(),
                                          writer.getText("doclet.Constants_Summary"));
        }
        if (holder.isClass() && ((ClassDoc)holder).isSerializable()) {
            //Automatically add link to serialized form page for serializable classes.
            if (!(writer.serialInclude(holder) &&
                      writer.serialInclude(((ClassDoc)holder).containingPackage()))) {
                return result.equals("") ? null : result;
            }
            result = addHeader(result, writer);
            result += writer.getHyperLink(writer.relativepath + "serialized-form.html",
                ((ClassDoc)holder).qualifiedName(), writer.getText("doclet.Serialized_Form"),
                false, "", "", "");
        }
        return result.equals("") ? null : result;
    }
    
    private String addHeader(String result, HtmlStandardWriter writer) {
        if (result != null && result.length() > 0) {
            return result + ", \n";
        } else {
            return "<DT><B>" + writer.getText("doclet.See_Also") + "</B><DD>";
        }
    }
}

