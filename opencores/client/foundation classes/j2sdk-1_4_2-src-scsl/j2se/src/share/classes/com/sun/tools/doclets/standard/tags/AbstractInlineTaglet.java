/*
 * @(#)AbstractInlineTaglet.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.standard.tags;

import com.sun.tools.doclets.standard.*;
import com.sun.tools.doclets.*;
import com.sun.javadoc.*;
import java.util.Map;

/**
 * An abstract inline taglet that outputs HTML.
 *
 * @author Jamie Ho
 * @since 1.4
 */

public abstract class AbstractInlineTaglet implements Taglet {
    
    /**
     * The name of this inline tag.
     */
    protected String name;
    
    /**
     * Return the name of this custom tag.
     */
    public String getName() {
        return name;
    }
    
    /**
     * Will return false since this is an inline tag.
     * @return false since this is an inline tag.
     */
    public boolean inField() {
        return false;
    }
    
    /**
     * Will return false since this is an inline tag.
     * @return false since this is an inline tag.
     */
    public boolean inConstructor() {
        return false;
    }
    
    /**
     * Will return false since this is an inline tag.
     * @return false since this is an inline tag.
     */
    public boolean inMethod() {
        return false;
    }
    
    /**
     * Will return false since this is an inline tag.
     * @return false since this is an inline tag.
     */
    public boolean inOverview() {
        return false;
    }

    /**
     * Will return false since this is an inline tag.
     * @return false since this is an inline tag.
     */
    public boolean inPackage() {
        return false;
    }

    /**
     * Will return false since this is an inline tag.
     * @return false since this is an inline tag.
     */
    public boolean inType() {
        return false;
    }
    
    /**
     * Will return true since this is an inline tag.
     * @return true since this is an inline tag.
     */
    
    public boolean isInlineTag() {
        return true;
    }
    
    /**
     * This method should not be called because this Taglet requires more
     * information to output to generate output.
     * @param tag a tag representing this inline tag.
     * @return null since this Taglet requires more
     * information to output to generate output.
     */
    public String toString(Tag tag) {
        return null;
    }
    
    /**
     * This method should not be called since this taglet does not support
     * the conversion of arrays on inline tags to a string.
     * @param tags the array of <code>Tag</code>s representing of this custom tag.
     * @return null since this taglet does not support
     * the conversion of arrays on inline tags to a string.
     */
    public String toString(Tag[] tags) {
        return null;
    }

    /**   
     * Given the <code>Tag</code> representation of this custom
     * tag, return its string representation.
     * @param tag he <code>Tag</code> representation of this custom tag.
     * @param doc the field that holds this tag.
     * @param writer the HtmlStandardWriter used to write this inline tag.
     */
    public abstract String toString(Tag tag, Doc doc, HtmlStandardWriter writer);

}
