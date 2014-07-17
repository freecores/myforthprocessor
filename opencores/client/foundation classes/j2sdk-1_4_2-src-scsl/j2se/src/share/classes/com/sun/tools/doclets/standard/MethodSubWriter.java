/*
 * @(#)MethodSubWriter.java 1.50 01/12/03
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.standard;

import java.util.*;
import com.sun.tools.doclets.*;
import com.sun.tools.doclets.standard.tags.*;
import com.sun.javadoc.*;

/**
 *
 * @author Robert Field
 * @author Atul M Dambalkar
 */
public class MethodSubWriter extends ExecutableMemberSubWriter {
    
    ConfigurationStandard configuration;
    
    public MethodSubWriter(SubWriterHolderWriter writer, ClassDoc classdoc,
                           ConfigurationStandard configuration) {
        super(writer, classdoc);
        this.configuration = configuration;
        
    }
    
    public MethodSubWriter(SubWriterHolderWriter writer, ConfigurationStandard configuration) {
        super(writer);
        this.configuration = configuration;
    }
    
    public int getMemberKind() {
        return VisibleMemberMap.METHODS;
    }
    
    public void printSummaryLabel(ClassDoc cd) {
        writer.boldText("doclet.Method_Summary");
    }
    
    public void printSummaryAnchor(ClassDoc cd) {
        writer.anchor("method_summary");
    }
    
    public void printInheritedSummaryAnchor(ClassDoc cd) {
        writer.anchor("methods_inherited_from_class_" + configuration.getClassName(cd));
    }
    
    public void printInheritedSummaryLabel(ClassDoc cd) {
        String classlink = writer.getPreQualifiedClassLink(cd);
        writer.bold();
        writer.printText(cd.isClass()?
                             "doclet.Methods_Inherited_From_Class":
                             "doclet.Methods_Inherited_From_Interface",
                         classlink);
        writer.boldEnd();
    }
    
    protected void printSummaryType(ProgramElementDoc member) {
        MethodDoc meth = (MethodDoc)member;
        printModifierAndType(meth, meth.returnType());
    }
    
    protected void printOverridden(ClassDoc overridden, MethodDoc method) {
        if(configuration.nocomment){
            return;
        }
        String label = "doclet.Overrides";
        
        if (method != null) {
            if(overridden.isAbstract() && method.isAbstract()){
                //Abstract method is implemented from abstract class,
                //not overridden
                label = "doclet.Specified_By";
            }
            String overriddenclasslink = writer.codeText(writer.getClassLink(overridden));
            String methlink = "";
            String name = method.name();
            writer.dt();
            writer.boldText(label);
            writer.dd();
            methlink = writer.codeText(writer.getClassLink(overridden,
                                                           name + method.signature(),
                                                           name, false));
            writer.printText("doclet.in_class", methlink, overriddenclasslink);
        }
    }
    
    protected void printTags(ProgramElementDoc member) {
        if(configuration.nocomment){
            return;
        }
        MethodDoc method = (MethodDoc)member;
        ClassDoc[] intfacs = member.containingClass().interfaces();
        MethodDoc overriddenMethod = method.overriddenMethod();
        if (intfacs.length > 0 || (overriddenMethod != null && overriddenMethod.isIncluded())) {
            printTagsInfoHeader();
            printImplementsInfo(method);
            if (overriddenMethod != null && overriddenMethod.isIncluded()) {
                printOverridden(overriddenMethod.containingClass(), overriddenMethod);
            }
            printTagsInfoFooter();
        }
        configuration.tagletManager.checkTags(member, member.tags(), false);
        configuration.tagletManager.checkTags(member, member.inlineTags(), true);
        Taglet[] definedTags = configuration.tagletManager.getMethodCustomTags();
        Tag[][] tagsInDoc = new Tag[definedTags.length][];
        boolean found = false;
        for (int i = 0; i < definedTags.length; i++) {
            tagsInDoc[i] = method.tags(definedTags[i].getName());
        }
        printTagsInfoHeader();
        String output;
        for (int i = 0; i < definedTags.length; i++) {
            if (definedTags[i] instanceof SimpleTaglet) {
                output = ((SimpleTaglet) definedTags[i]).toString(tagsInDoc[i], writer);
            } else if (definedTags[i] instanceof AbstractExecutableMemberTaglet) {
                output = ((AbstractExecutableMemberTaglet) definedTags[i]).toString(method, writer);
            } else {
                output = definedTags[i].toString(tagsInDoc[i]);
            }
            if (output != null && output.length() > 0) {
                configuration.tagletManager.seenCustomTag(definedTags[i].getName());
                print(output);
            }
        }
        printTagsInfoFooter();
    }
    
    /**
     * Parse the &lt;Code&gt; tag and return the text.
     */
    protected String parseCodeTag(String tag){
        if(tag == null){
            return "";
        }
        
        String lc = tag.toLowerCase();
        int begin = lc.indexOf("<code>");
        int end = lc.indexOf("</code>");
        if(begin == -1 || end == -1 || end <= begin){
            return tag;
        } else {
            return tag.substring(begin + 6, end);
        }
    }
    
    protected void printTagsInfoHeader() {
        writer.dd();
        writer.dl();
    }
    
    protected void printTagsInfoFooter() {
        writer.dlEnd();
        writer.ddEnd();
    }
    
    protected void printImplementsInfo(MethodDoc method) {
        if(configuration.nocomment){
            return;
        }
        MethodDoc implementedMeth = implementedMethod(method);
        if (implementedMeth != null) {
            ClassDoc intfac = implementedMeth.containingClass();
            String methlink = "";
            String intfaclink = writer.codeText(
                writer.getClassLink(intfac));
            writer.dt();
            writer.boldText("doclet.Specified_By");
            writer.dd();
            methlink = writer.codeText(writer.getDocLink(implementedMeth,
                                                         implementedMeth.name()));
            writer.printText("doclet.in_interface", methlink, intfaclink);
        }
        
    }
    
    protected MethodDoc implementedMethod(MethodDoc method) {
        
        MethodDoc[] implementedMethods = (new ImplementedMethods(method)).build();
        
        if(implementedMethods != null && implementedMethods.length > 0){
            //Return nearest implemented method in class hierarchy
            return implementedMethods[0];
        } else {
            return null;
        }
    }
    
    protected void printSignature(ExecutableMemberDoc member) {
        writer.displayLength = 0;
        writer.pre();
        printModifiers(member);
        printReturnType((MethodDoc)member);
        if (configuration().genSrc) {
            writer.printSrcLink(member, member.name());
        } else {
            bold(member.name());
        }
        printParameters(member);
        printExceptions(member);
        writer.preEnd();
    }
    
    protected void printComment(ProgramElementDoc member) {
        if(configuration.nocomment){
            return;
        }
        if (member.inlineTags().length > 0) {
            writer.dd();
            writer.printInlineComment(member);
        } else {
            MethodDoc method = new CommentedMethodFinder().
                search(member.containingClass(),
                           (MethodDoc)member);
            printCommentFromCommentedMethod(method);
        }
    }
    
    protected void printCommentFromCommentedMethod(MethodDoc method) {
        if (method == null || configuration.nocomment) {
            return;
        }
        ClassDoc cd = method.containingClass();
        String classlink = writer.codeText(writer.getClassLink(cd));
        writer.dd();
        writer.boldText(cd.isClass()?
                            "doclet.Description_From_Class":
                            "doclet.Description_From_Interface",
                        classlink);
        writer.ddEnd();
        writer.dd();
        writer.printInlineComment(method);
    }
    
    public void printMembersSummary() {
        List members = new ArrayList(members(classdoc));
        if (members.size() > 0) {
            Collections.sort(members);
            printSummaryHeader(classdoc);
            for (int i = 0; i < members.size(); ++i) {
                MethodDoc member = (MethodDoc)members.get(i);
                boolean commentChanged = false;
                String prevRawComment = "";
                Tag[] tags = member.inlineTags();
                if (tags.length == 0) {
                    prevRawComment = member.getRawCommentText();
                    MethodDoc meth =
                        new CommentedMethodFinder().search(classdoc, member);
                    if (meth != null) { //set raw comment text for now.
                        member.setRawCommentText(meth.commentText());
                        commentChanged = true;
                    }
                }
                printSummaryMember(classdoc, member);
                if (commentChanged) {  // reset it to prevRawComment.
                    member.setRawCommentText(prevRawComment);
                }
            }
            printSummaryFooter(classdoc);
        }
    }
    
    protected void printReturnType(MethodDoc method) {
        Type type = method.returnType();
        if (type != null) {
            printTypeLink(type);
            print(' ');
        }
    }
    
    protected void printHeader(ClassDoc cd) {
        writer.anchor("method_detail");
        writer.printTableHeadingBackground(writer.
                                               getText("doclet.Method_Detail"));
    }
    
    protected void printNavSummaryLink(ClassDoc cd, boolean link) {
        if (link) {
            writer.printHyperLink("", (cd == null)?
                                      "method_summary":
                                      "methods_inherited_from_class_" +
                                      configuration.getClassName(cd),
                                  writer.getText("doclet.navMethod"));
        } else {
            writer.printText("doclet.navMethod");
        }
    }
    
    protected void printNavDetailLink(boolean link) {
        if (link) {
            writer.printHyperLink("", "method_detail",
                                  writer.getText("doclet.navMethod"));
        } else {
            writer.printText("doclet.navMethod");
        }
    }
}


