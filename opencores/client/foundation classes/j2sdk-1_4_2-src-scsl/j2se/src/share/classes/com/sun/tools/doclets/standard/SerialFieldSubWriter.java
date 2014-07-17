/*
 * @(#)SerialFieldSubWriter.java	1.25 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.standard;

import com.sun.javadoc.*;
import com.sun.tools.doclets.*;
import java.util.*;

/**
 * Generate serialized form for serializable fields.
 * Documentation denoted by the tags <code>serial</code> and
 * <code>serialField<\code> is processed.
 *
 * @author Joe Fialli
 */
public class SerialFieldSubWriter extends FieldSubWriter {
    ProgramElementDoc[] members = null;
    
    public SerialFieldSubWriter(SubWriterHolderWriter writer,
                                ClassDoc classdoc) {
        super(writer, classdoc);
    }
    
    public List members(ClassDoc cd) {
        return Util.asList(cd.serializableFields());
    }
    
    void printSignature(MemberDoc member) {
        FieldDoc field = (FieldDoc)member;
        printHead(member);
        writer.pre();
        printTypeLink(field.type());
        print(' ');
        bold(field.name());
        writer.preEnd();
    }
    
    protected void printTypeLinkNoDimension(Type type) {
        ClassDoc cd = type.asClassDoc();
        //Linking to package private classes in serialized for causes
        //broken links.  Don't link to them.
        if (cd == null || cd.isPackagePrivate()) {
            print(type.typeName());
        } else {
            writer.printClassLink(cd);
        }
    }
    
    protected void printHeader(ClassDoc cd) {
        writer.anchor("serializedForm");
        printSerializableClassComment(cd);
        writer.printTableHeadingBackground(writer.getText("doclet.Serialized_Form_fields"));
    }
    
    /**
     * javadoc comments for "serialPersistentFields" is considered
     * as serializable class comments, not field comments.
     */
    private void printSerializableClassComment(ClassDoc cd) {
        if (cd.definesSerializableFields()) {
            FieldDoc serialPersistentFields =
                (FieldDoc)(members(classdoc).get(0));
            String comment = serialPersistentFields.commentText();
            if (comment.length() > 0) {
                writer.printTableHeadingBackground(writer.getText("doclet.Serialized_Form_class"));
                printFullComment(serialPersistentFields);
            }
        }
    }
    
    protected void printBodyHtmlEnd(ClassDoc cd) {
    }
    
    /**
     * Print a default Serializable field or
     * print all Serializable fields documented by
     * <code>serialField</code> tags.
     */
    protected void printMember(ProgramElementDoc member) {
        FieldDoc field = (FieldDoc)member;
        ClassDoc cd = field.containingClass();
        if (cd.definesSerializableFields()) {
            
            // Process Serializable Fields specified as array of ObjectStreamFields.
            //Print a member for each serialField tag.
            //(There should be one serialField tag per ObjectStreamField element.)
            SerialFieldTag[] tags = field.serialFieldTags();
            Arrays.sort(tags);
            for (int i = 0; i < tags.length; i++) {
                if (i > 0) {
                    writer.printMemberHeader();
                }
                writer.h3();
                writer.print(tags[i].fieldName());
                writer.h3End();
                printSignature(tags[i]);
                printComment(tags[i]);
                writer.printMemberFooter();
            }
        } else {
            
            // Process default Serializable field.
            if ((field.tags("serial").length == 0) && ! field.isSynthetic()) {
                serialWarning(field.position(), "doclet.MissingSerialTag", cd.qualifiedName(),
                              field.name());
            }
            printSignature(field);
            printFullComment(field);
        }
    }
    
    /*
     * Print comment for a default Serializable field.
     *
     * If serial tag has an optional comment, append the
     * comment to the javadoc comment for the field.
     */
    protected void printComment(ProgramElementDoc member) {
        if (member.inlineTags().length > 0) {
            writer.dd();
            writer.printInlineComment(member);
        }
        
        Tag[] tags = member.tags("serial");
        if (tags.length > 0) {
            writer.dt();
            writer.dd();
            writer.printInlineComment(member, tags[0]);
        }
    }
    
    /* Methods for processing serialField fieldName fieldType description.*/
    void printSignature(SerialFieldTag sftag) {
        writer.pre();
        ClassDoc fieldTypeDoc = sftag.fieldTypeDoc();
        if (fieldTypeDoc != null) {
            writer.printClassLink(fieldTypeDoc);
        } else {
            writer.print(sftag.fieldType());
        }
        print(' ');
        bold(sftag.fieldName());
        writer.preEnd();
    }
    
    private void printComment(SerialFieldTag sftag) {
        writer.dl();
        writer.dd();
        writer.print(sftag.description());
        writer.dlEnd();
    }
    
    /* The methods following this comment have no meaning for SerialFields.*/
    protected void printDeprecatedLink(ProgramElementDoc member) {
    }
    
    public void printSummaryLabel(ClassDoc cd) {
    }
    
    public void printInheritedSummaryLabel(ClassDoc cd) {
    }
    
    protected void printSummaryLink(ClassDoc cd, ProgramElementDoc member) {
    }
    
    protected void printInheritedSummaryLink(ClassDoc cd,
                                             ProgramElementDoc member) {
    }
    
    protected void printSummaryType(ProgramElementDoc member) {
    }
    
    public void buildVisibleMemberMap() {
        // Do nothing.
    }
}


