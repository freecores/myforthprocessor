/*
 * @(#)ValueTaglet.java	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.standard.tags;

import com.sun.tools.doclets.standard.*;
import com.sun.tools.doclets.*;
import com.sun.javadoc.*;

/**
 * An inline Taglet representing {@value}. This tag should only be used with
 * constant fields that have a value.  It is used to access the value of constant
 * fields.
 *
 * @author Jamie Ho
 * @since 1.4
 */

public class ValueTaglet extends AbstractInlineTaglet {
    

    /**
     * Construct a new ValueTaglet.
     */
    public ValueTaglet() {
        name = "value";
    } 

    /**
     * Given the <code>Tag</code> representation of this custom
     * tag, return its string representation.
     * @param tag he <code>Tag</code> representation of this custom tag.
     * @param doc the field that holds this tag.
     * @param writer the HtmlStandardWriter used to write this inline tag.
     */
    public String toString(Tag tag, Doc doc, HtmlStandardWriter writer) {
        FieldDoc field;
        if (! (doc instanceof FieldDoc) || (field = (FieldDoc) doc).constantValue() == null) {
            writer.warning(doc.position(), 
                "doclet.Warn_inline_taglet", "{@" + name + "}", "constant field");
            //Don't replace the {@value} tag with anything.  Simply return it.
            return "{@" + name + "}";
        } else {
            return field.constantValueExpression();
        }
    }    
}
