/**
 * @(#)SerialFieldTagImpl.java	1.18 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javadoc;
import com.sun.javadoc.*;


/**
 * Documents a Serializable field defined by an ObjectStreamField.
 * <pre>
 * The class parses and stores the three serialField tag parameters:
 *
 * - field name
 * - field type name
 *      (fully-qualified or visible from the current import context)
 * - description of the valid values for the field
 *
 * </pre>
 * This tag is only allowed in the javadoc for the special member
 * serialPersistentFields.
 *
 * @author Joe Fialli
 * @author Neal Gafter
 *
 * @see java.io.ObjectStreamField
 */
class SerialFieldTagImpl extends TagImpl implements SerialFieldTag, Comparable {
    private String fieldName;
    private String fieldType;
    private String description;
    private ClassDoc containingClass;
    private ClassDoc fieldTypeDoc;
    private FieldDocImpl matchingField;

    SerialFieldTagImpl(DocImpl holder, String name, String text) {
        super(holder, name, text);
        parseSerialFieldString();
        if (holder instanceof MemberDoc) {
            containingClass = ((MemberDocImpl) holder).containingClass();
        }
    }

    private void parseSerialFieldString() {
        int len = text.length();
        int inx = 0;
        for (; inx < len && Character.isWhitespace(text.charAt(inx)); inx++)
            ;
        int first = inx;
        int last = inx;
        if (!Character.isJavaIdentifierStart(text.charAt(inx))) {
            docenv().warning(holder, "tag.serialField.illegal_character",
                    "" + text.charAt(inx), text);
            return;
        }
        for (; inx < len && Character.isJavaIdentifierPart(text.charAt(inx)); inx++)
            ;
        if (inx < len && !Character.isWhitespace(text.charAt(inx))) {
            docenv().warning(holder, "tag.serialField.illegal_character",
                    "" + text.charAt(inx), text);
            return;
        }
        last = inx;
        fieldName = text.substring(first, last);
        for (; inx < len && Character.isWhitespace(text.charAt(inx)); inx++)
            ;
        first = inx;
        last = inx;
        for (; inx < len && !Character.isWhitespace(text.charAt(inx)); inx++)
            ;
        if (inx < len && !Character.isWhitespace(text.charAt(inx))) {
            docenv().warning(holder, "tag.serialField.illegal_character",
                    "" + text.charAt(inx), text);
            return;
        }
        last = inx;
        fieldType = text.substring(first, last);
        for (; inx < len && Character.isWhitespace(text.charAt(inx)); inx++)
            ;
        description = text.substring(inx);
    }

    /**
      * return a key for sorting.
      */
    String key() {
        return fieldName;
    }

    void mapToFieldDocImpl(FieldDocImpl fd) {
        matchingField = fd;
    }

    /**
      * Return the serialziable field name.
      */
    public String fieldName() {
        return fieldName;
    }

    /**
      * Return the field type string.
      */
    public String fieldType() {
        return fieldType;
    }

    /**
      * Return the ClassDocImpl for field type.
      *
      * @returns null if no ClassDocImpl for field type is visible from
      *          containingClass context.
      */
    public ClassDoc fieldTypeDoc() {
        if (fieldTypeDoc == null && containingClass != null) {
            fieldTypeDoc = containingClass.findClass(fieldType);
        }
        return fieldTypeDoc;
    }

    /**
      * Return the corresponding FieldDocImpl for this SerialFieldTagImpl.
      *
      * @returns null if no matching FieldDocImpl.
      */
    FieldDocImpl getMatchingField() {
        return matchingField;
    }

    /**
      * Return the field comment. If there is no serialField comment, return
      * javadoc comment of corresponding FieldDocImpl.
      */
    public String description() {
        if (description.length() == 0 && matchingField != null) {
            Comment comment = matchingField.comment();
            if (comment != null) {
                return comment.commentText();
            }
        }
        return description;
    }

    /**
      * Return the kind of this tag.
      */
    public String kind() {
        return "@serialField";
    }

    /**
      * Convert this object to a string.
      */
    public String toString() {
        return name + ":" + text;
    }

    /**
      * Compares this Object with the specified Object for order.  Returns a
      * negative integer, zero, or a positive integer as this Object is less
      * than, equal to, or greater than the given Object.
      * <p>
      * Included to make SerialFieldTagImpl items java.lang.Comparable.
      *
      * @param   obj the <code>Object</code> to be compared.
      * @return  a negative integer, zero, or a positive integer as this Object
      *		is less than, equal to, or greater than the given Object.
      * @exception ClassCastException the specified Object's type prevents it
      *		  from being compared to this Object.
      * @since JDK1.2
      */
    public int compareTo(Object obj) {
        return key().compareTo(((SerialFieldTagImpl) obj).key());
    }
}
