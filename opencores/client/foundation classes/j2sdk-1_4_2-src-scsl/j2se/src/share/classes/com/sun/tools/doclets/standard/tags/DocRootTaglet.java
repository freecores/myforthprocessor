/*
 * @(#)DocRootTaglet.java	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.standard.tags;

import com.sun.tools.doclets.standard.*;
import com.sun.tools.doclets.*;
import com.sun.javadoc.*;

/**
 * An inline Taglet representing {&#064;docRoot}.  This taglet is 
 * used to get the relative path to the document's root output 
 * directory.
 *
 * @author Jamie Ho
 * @author Doug Kramer
 * @since 1.4
 */

public class DocRootTaglet extends AbstractInlineTaglet {
    

    /**
     * Construct a new DocRootTaglet.
     */
    public DocRootTaglet() {
        name = "docRoot";
    } 

    /**
     * Given the <code>Tag</code> representation of this custom
     * tag, return its string representation.
     * @param tag     the <code>Tag</code> representation of this custom tag.
     * @param doc     the doc that holds this tag.
     * @param writer  the HtmlStandardWriter used to write this inline tag.
     */
    public String toString(Tag tag, Doc doc, HtmlStandardWriter writer) {

        return writer.relativepathNoSlash;
    }
}
