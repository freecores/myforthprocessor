/**
 * @(#)ParamTagImpl.java	1.24 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javadoc;
import com.sun.javadoc.*;


/**
 * Represents an @param documentation tag.
 * The parses and stores the name and comment parts of the
 * method/constructor parameter tag.
 *
 * @author Robert Field
 *
 */
class ParamTagImpl extends TagImpl implements ParamTag {
    private final String parameterName;
    private final String parameterComment;

    ParamTagImpl(DocImpl holder, String name, String text) {
        super(holder, name, text);
        String[] sa = divideAtWhite();
        parameterName = sa[0];
        parameterComment = sa[1];
    }

    /**
      * Return the parameter name.
      */
    public String parameterName() {
        return parameterName;
    }

    /**
      * Return the parameter comment.
      */
    public String parameterComment() {
        return parameterComment;
    }

    /**
      * Return the kind of this tag.
      */
    public String kind() {
        return "@param";
    }

    /**
      * convert this object to a string.
      */
    public String toString() {
        return name + ":" + text;
    }

    /**
      * For the parameter comment with embedded @link tags return the array of
      * TagImpls consisting of SeeTagImpl(s) and text containing TagImpl(s).
      *
      * @return TagImpl[] Array of tags with inline SeeTagImpls.
      * @see TagImpl#inlineTagImpls()
      * @see ThrowsTagImpl#inlineTagImpls()
      */
    public Tag[] inlineTags() {
        return Comment.getInlineTags(holder, parameterComment);
    }
}
