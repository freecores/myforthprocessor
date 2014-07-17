/*
 * @(#)ReturnTaglet.java	1.7 03/01/23
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
 * A taglet that represents the @return tag.
 * @author Jamie Ho
 * @since 1.4
 */
public class ReturnTaglet extends AbstractExecutableMemberTaglet{
    
    public ReturnTaglet() {
        name = "return";
    }
    
    /**
     * Return true if this <code>Taglet</code>
     * is used in constructor documentation.
     * @return true if this <code>Taglet</code>
     * is used in constructor documentation and false
     * otherwise.
     */
    public boolean inConstructor() {
        return false;
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
        Tag[] tags = holder.tags(name);
        String result = "";
        MethodDoc inheritedMethod;
        
        if (tags.length == 0 &&
                holder instanceof MethodDoc &&
                (inheritedMethod = getInheritedMethodDoc((MethodDoc) holder)) != null) {
            tags = inheritedMethod.tags("return");
        }
        if (tags.length > 0) {
            result += "\n<DT>" + "<B>" + writer.getText("doclet.Returns") + "</B>" + "<DD>";
            result += writer.commentTagsToString(null, tags[0].inlineTags(), false, false);
            if (holder instanceof MethodDoc ) {
                result = writer.replaceInheritDoc((MethodDoc) holder, tags[0], result);
            }
        }
        return result.equals("") ? null : result;
    }
}
