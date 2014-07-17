/*
 * @(#)InheritDocTaglet.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.standard.tags;

import com.sun.tools.doclets.standard.*;
import com.sun.tools.doclets.*;
import com.sun.javadoc.*;

/**
 * An inline Taglet representing {@inheritDoc}. This tag should only be used with
 * a method.  It is used to inherit documentation from overriden and implemented
 * methods.
 *
 * @author Jamie Ho
 * @since 1.4
 */

public class InheritDocTaglet extends AbstractInlineTaglet {
    

    /**
     * Construct a new InheritDocTaglet.
     */
    public InheritDocTaglet () {
        name = "inheritDoc";
    } 

    /**
     * Given the <code>Tag</code> representation of this custom
     * tag, return its string representation.
     * @param tag he <code>Tag</code> representation of this custom tag.
     * @param doc the field that holds this tag.
     * @param writer the HtmlStandardWriter used to write this inline tag.
     */
    public String toString(Tag tag, Doc doc, HtmlStandardWriter writer) {
        if (doc != null && writer != null && ! (doc instanceof ExecutableMemberDoc)) {
            writer.warning(doc.position(), "doclet.Warn_inline_taglet", "{@" + name + "}", "method");
        } 
        //The output of this tag is dependent on which other tag it was used with.
        //Simply return the name of this inline tag.  Other taglets such as ParamTaglet,
        //ReturnTaglet and ThrowsTaglet will see it and replace it with the appropriate text.
        return "{@" + name + "}";
    }    
}
