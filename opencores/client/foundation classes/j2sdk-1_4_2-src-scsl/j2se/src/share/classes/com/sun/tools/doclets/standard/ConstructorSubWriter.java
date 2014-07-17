/*
 * @(#)ConstructorSubWriter.java	1.33 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package com.sun.tools.doclets.standard;

import com.sun.tools.doclets.*;
import com.sun.tools.doclets.standard.tags.*;
import com.sun.javadoc.*;
import java.util.*;

/**
 *
 * @author Robert Field
 * @author Atul M Dambalkar
 */
public class ConstructorSubWriter extends ExecutableMemberSubWriter {
    
    protected boolean foundNonPublicMember = false;
    
    public ConstructorSubWriter(SubWriterHolderWriter writer,
                                ClassDoc classdoc) {
        super(writer, classdoc);
        checkForNonPublicMembers(visibleMemberMap.getMembersFor(classdoc));
    }
    
    public ConstructorSubWriter(SubWriterHolderWriter writer) {
        super(writer);
    }
    
    public void printSummaryLabel(ClassDoc cd) {
        writer.boldText("doclet.Constructor_Summary");
    }
    
    public void printSummaryAnchor(ClassDoc cd) {
        writer.anchor("constructor_summary");
    }
    
    public void printInheritedSummaryAnchor(ClassDoc cd) {
    }   // no such
    
    public void printInheritedSummaryLabel(ClassDoc cd) {
        // no such
    }
    
    public int getMemberKind() {
        return VisibleMemberMap.CONSTRUCTORS;
    }
    
    protected void printSummaryType(ProgramElementDoc member) {
        if (foundNonPublicMember) {
            writer.printTypeSummaryHeader();
            if (member.isProtected()) {
                print("protected ");
            } else if (member.isPrivate()) {
                print("private ");
            } else if (member.isPublic()) {
                writer.space();
            } else {
                writer.printText("doclet.Package_private");
            }
            writer.printTypeSummaryFooter();
        }
    }
    
    protected void printTags(ProgramElementDoc member) {
        ConfigurationStandard configuration = configuration();
        configuration.tagletManager.checkTags(member, member.tags(), false);
        configuration.tagletManager.checkTags(member, member.inlineTags(), true);
        ConstructorDoc constructor = (ConstructorDoc) member;
        Taglet[] definedTags = configuration().tagletManager.getConstructorCustomTags();
        Tag[][] tagsInDoc = new Tag[definedTags.length][];
        for (int i = 0; i < definedTags.length; i++) {
            tagsInDoc[i] = constructor.tags(definedTags[i].getName());
        }
        String output;
        for (int i = 0; i < definedTags.length; i++) {
            if (tagsInDoc[i].length == 0) {
                output = null;
            } else {
                if (definedTags[i] instanceof SimpleTaglet) {
                    output = ((SimpleTaglet) definedTags[i]).toString(tagsInDoc[i], writer);
                } else if (definedTags[i] instanceof AbstractExecutableMemberTaglet) {
                    output = ((AbstractExecutableMemberTaglet) definedTags[i]).toString(constructor, writer);
                } else {
                    output = definedTags[i].toString(tagsInDoc[i]);
                }
            }
            if (output != null) {
                configuration.tagletManager.seenCustomTag(definedTags[i].getName());
                print(output);
            }
        }
    }
    
    protected void printHeader(ClassDoc cd) {
        writer.anchor("constructor_detail");
        writer.printTableHeadingBackground(writer.
                                               getText("doclet.Constructor_Detail"));
    }
    
    protected void navSummaryLink() {
        printNavSummaryLink(classdoc,
                            visibleMemberMap.getMembersFor(classdoc).size() > 0? true: false);
    }
    
    protected void printNavSummaryLink(ClassDoc cd, boolean link) {
        if (link) {
            writer.printHyperLink("", "constructor_summary",
                                  writer.getText("doclet.navConstructor"));
        } else {
            writer.printText("doclet.navConstructor");
        }
    }
    
    protected void printNavDetailLink(boolean link) {
        if (link) {
            writer.printHyperLink("", "constructor_detail",
                                  writer.getText("doclet.navConstructor"));
        } else {
            writer.printText("doclet.navConstructor");
        }
    }
    
    protected void checkForNonPublicMembers(List members) {
        for (int i = 0; i < members.size(); i++) {
            if (!foundNonPublicMember) {
                if (!((ProgramElementDoc)(members.get(i))).isPublic()) {
                    foundNonPublicMember = true;
                    break;
                }
            }
        }
    }
}


