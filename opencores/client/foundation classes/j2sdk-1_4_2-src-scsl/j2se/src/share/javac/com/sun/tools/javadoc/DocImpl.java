/**
 * @(#)DocImpl.java	1.29 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javadoc;
import com.sun.javadoc.*;

import java.io.InputStream;

import java.io.IOException;

import java.text.CollationKey;


/**
 * abstract base class of all Doc classes.  Doc item's are representations
 * of java language constructs (class, package, method,...) which have
 * comments and have been processed by this run of javadoc.  All Doc items
 * are unique, that is, they are == comparable.
 *
 * @since JDK1.2
 * @author Robert Field
 * @author Atul M Dambalkar
 * @author Neal Gafter (rewrite)
 */
abstract class DocImpl implements Doc, Comparable {

    /**
     * Doc environment
     */
    protected final DocEnv env;

    /**
     *  The complex comment object, lazily initialized.
     */
    private Comment comment;

    /**
     * The cached sort key, to take care of Natural Language Text sorting.
     */
    private CollationKey collationkey = null;

    /**
     *  Raw documentation string.
     */
    protected String documentation;

    /**
     * Constructor.
     */
    DocImpl(DocEnv env, String documentation) {
        super();
        this.documentation = documentation;
        this.env = env;
    }

    /**
      * So subclasses have the option to do lazy initialization of
      * "documentation" string.
      */
    String documentation() {
        if (documentation == null)
            documentation = "";
        return documentation;
    }

    /**
      * For lazy initialization of comment.
      */
    Comment comment() {
        if (comment == null) {
            comment = new Comment(this, documentation());
        }
        return comment;
    }

    /**
      * Return the text of the comment for this doc item.
      * TagImpls have been removed.
      */
    public String commentText() {
        return comment().commentText();
    }

    /**
      * Return all tags in this Doc item.
      *
      * @return an array of TagImpl containing all tags on this Doc item.
      */
    public Tag[] tags() {
        return comment().tags();
    }

    /**
      * Return tags of the specified kind in this Doc item.
      *
      * @param tagname name of the tag kind to search for.
      * @return an array of TagImpl containing all tags whose 'kind()'
      * matches 'tagname'.
      */
    public Tag[] tags(String tagname) {
        return comment().tags(tagname);
    }

    /**
      * Return the see also tags in this Doc item.
      *
      * @return an array of SeeTag containing all &#64see tags.
      */
    public SeeTag[] seeTags() {
        return comment().seeTags();
    }

    public Tag[] inlineTags() {
        return Comment.getInlineTags(this, commentText());
    }

    public Tag[] firstSentenceTags() {
        return Comment.firstSentenceTags(this, commentText());
    }

    /**
      * Utility for subclasses which read HTML documentation files.
      */
    String readHTMLDocumentation(InputStream input,
            String filename) throws IOException {
        int filesize = input.available();
        byte[] filecontents = new byte[filesize];
        input.read(filecontents, 0, filesize);
        input.close();
        String encoding = env.getEncoding();
        String rawDoc = (encoding != null) ? new String(filecontents, encoding) :
                new String(filecontents);
        String upper = null;
        int bodyIdx = rawDoc.indexOf("<body");
        if (bodyIdx == -1) {
            bodyIdx = rawDoc.indexOf("<BODY");
            if (bodyIdx == -1) {
                upper = rawDoc.toUpperCase();
                bodyIdx = upper.indexOf("<BODY");
                if (bodyIdx == -1) {
                    env.messager.error(SourcePositionImpl.make(filename, 0), "javadoc.Body_missing_from_html_file");
                    return "";
                }
            }
        }
        bodyIdx = rawDoc.indexOf('>', bodyIdx);
        if (bodyIdx == -1) {
            env.messager.error(SourcePositionImpl.make(filename, 0), "javadoc.Body_missing_from_html_file");
            return "";
        }
        ++bodyIdx;
        int endIdx = rawDoc.indexOf("</body", bodyIdx);
        if (endIdx == -1) {
            endIdx = rawDoc.indexOf("</BODY", bodyIdx);
            if (endIdx == -1) {
                if (upper == null) {
                    upper = rawDoc.toUpperCase();
                }
                endIdx = upper.indexOf("</BODY", bodyIdx);
                if (endIdx == -1) {
                    env.messager.error(SourcePositionImpl.make(filename, 0), "javadoc.End_body_missing_from_html_file");
                    return "";
                }
            }
        }
        return rawDoc.substring(bodyIdx, endIdx);
    }

    /**
      * Return the full unprocessed text of the comment.  Tags
      * are included as text.  Used mainly for store and retrieve
      * operations like internalization.
      */
    public String getRawCommentText() {
        return documentation();
    }

    /**
      * Set the full unprocessed text of the comment.  Tags
      * are included as text.  Used mainly for store and retrieve
      * operations like internalization.
      */
    public void setRawCommentText(String rawDocumentation) {
        documentation = rawDocumentation;
        comment = null;
    }

    /**
      * return a key for sorting.
      */
    CollationKey key() {
        if (collationkey == null) {
            collationkey = generateKey();
        }
        return collationkey;
    }

    /**
      * Generate a key for sorting.
      * <p>
      * Default is name().
      */
    CollationKey generateKey() {
        String k = name();
        return env.doclocale.collator.getCollationKey(k);
    }

    /**
      * Returns a string representation of this Doc item.
      */
    public String toString() {
        return qualifiedName();
    }

    /**
      * Returns the qualified string representation of this Doc item.
      *
      * @return the qualified name.
      */
    String toQualifiedString() {
        return toString();
    }

    /**
      * Returns the name of this Doc item.
      *
      * @return  the name
      */
    public abstract String name();

    /**
     * Returns the qualified name of this Doc item.
     *
     * @return  the name
     */
    public abstract String qualifiedName();

    /**
     * Compares this Object with the specified Object for order.  Returns a
     * negative integer, zero, or a positive integer as this Object is less
     * than, equal to, or greater than the given Object.
     * <p>
     * Included so that Doc item are java.lang.Comparable.
     *
     * @param   o the <code>Object</code> to be compared.
     * @return  a negative integer, zero, or a positive integer as this Object
     *		is less than, equal to, or greater than the given Object.
     * @exception ClassCastException the specified Object's type prevents it
     *		  from being compared to this Object.
     */
    public int compareTo(Object obj) {
        return key().compareTo(((DocImpl) obj).key());
    }

    /**
      * Is this Doc item a field?  False until overridden.
      *
      * @return true if it represents a field
      */
    public boolean isField() {
        return false;
    }

    /**
      * Is this Doc item a simple method (i.e. not a constructor)?  False
      * until overridden.
      *
      * @return true is it represents a method, false if it is anything
      * else, including constructor, field, class, ...
      */
    public boolean isMethod() {
        return false;
    }

    /**
      * Is this Doc item a constructor?  False until overridden.
      *
      * @return true is it represents a constructor
      */
    public boolean isConstructor() {
        return false;
    }

    /**
      * Is this Doc item a interface?  False until overridden.
      *
      * @return true is it represents a interface
      */
    public boolean isInterface() {
        return false;
    }

    /**
      * Is this Doc item a exception class?  False until overridden.
      *
      * @return true is it represents a exception
      */
    public boolean isException() {
        return false;
    }

    /**
      * Is this Doc item a error class?  False until overridden.
      *
      * @return true is it represents a error
      */
    public boolean isError() {
        return false;
    }

    /**
      * Is this Doc item a ordinary class (i.e. not an interface, exception,
      * or error)?  False until overridden.
      *
      * @return true is it represents a class
      */
    public boolean isOrdinaryClass() {
        return false;
    }

    /**
      * Is this Doc item a class.
      * Note: interfaces are not classes.
      * False until overridden.
      *
      * @return true is it represents a class
      */
    public boolean isClass() {
        return false;
    }

    /**
      *
      * return true if this Doc is include in the active set.
      */
    public abstract boolean isIncluded();

    /**
     * Return the source position of the entity, or null if
     * no position is available.
     */
    public SourcePosition position() {
        return null;
    }
}
