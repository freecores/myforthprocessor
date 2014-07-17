/*
 * @(#)AbstractSubWriter.java   1.53 01/12/03
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.standard;

import com.sun.tools.doclets.*;
import com.sun.tools.doclets.standard.tags.*;
import com.sun.javadoc.*;
import java.util.*;
import java.lang.reflect.Modifier;

/**
 *
 * @author Robert Field
 * @author Atul M Dambalkar
 */
public abstract class AbstractSubWriter {

    protected boolean printedSummaryHeader = false;
    protected final SubWriterHolderWriter writer;
    protected final ClassDoc classdoc;
    public final boolean nodepr;

    public VisibleMemberMap visibleMemberMap = null;
    public List visibleClasses = null;

    public AbstractSubWriter(SubWriterHolderWriter writer,
                             ClassDoc classdoc) {
        this.writer = writer;
        this.nodepr = configuration().nodeprecated;
        this.classdoc = classdoc;
        if (classdoc != null) {
            buildVisibleMemberMap();
        }
    }

    public AbstractSubWriter(SubWriterHolderWriter writer) {
        this(writer, null);
    }

    /*** abstracts ***/

    public abstract int getMemberKind();

    public abstract void printSummaryLabel(ClassDoc cd);

    public abstract void printInheritedSummaryLabel(ClassDoc cd);

    public abstract void printSummaryAnchor(ClassDoc cd);

    public abstract void printInheritedSummaryAnchor(ClassDoc cd);

    protected abstract void printSummaryType(ProgramElementDoc member);

    protected abstract void printSummaryLink(ClassDoc cd,
                                             ProgramElementDoc member);

    protected abstract void printInheritedSummaryLink(ClassDoc cd,
                                                     ProgramElementDoc member);

    protected abstract void printHeader(ClassDoc cd);

    protected abstract void printBodyHtmlEnd(ClassDoc cd);

    protected abstract void printMember(ProgramElementDoc elem);

    protected abstract void printDeprecatedLink(ProgramElementDoc member);

    protected abstract void printNavSummaryLink(ClassDoc cd, boolean link);

    protected abstract void printNavDetailLink(boolean link);

    /***  ***/

    protected void print(String str) {
        writer.print(str);
        writer.displayLength += str.length();
    }

    protected void print(char ch) {
        writer.print(ch);
        writer.displayLength++;
    }

    protected void bold(String str) {
        writer.bold(str);
        writer.displayLength += str.length();
    }

    protected void printTypeLinkNoDimension(Type type) {
        ClassDoc cd = type.asClassDoc();
    if (cd == null) {
        print(type.typeName());
    } else {
        writer.printClassLink(cd);
    }
    }

    protected void printTypeLink(Type type) {
        printTypeLinkNoDimension(type);
        print(type.dimension());
    }

    /**
     * Return a string describing the access modifier flags.
     * Don't include native or synchronized.
     *
     * The modifier names are returned in canonical order, as
     * specified by <em>The Java Language Specification</em>.
     */
    protected String modifierString(MemberDoc member) {
        int ms = member.modifierSpecifier();
        int no = Modifier.NATIVE | Modifier.SYNCHRONIZED;
    return Modifier.toString(ms & ~no);
    }

    protected String typeString(MemberDoc member) {
        String type = "";
        if (member instanceof MethodDoc) {
            type = ((MethodDoc)member).returnType().toString();
        } else if (member instanceof FieldDoc) {
            type = ((FieldDoc)member).type().toString();
        }
        return type;
    }
 
    protected void printModifiers(MemberDoc member) {
        String mod;
        mod = modifierString(member);
        if(mod.length() > 0) {
            print(mod);
            print(' ');
        }
    }

    protected void printTypedName(Type type, String name) {
        if (type != null) {
            printTypeLink(type);
        }
        if(name.length() > 0) {
            writer.space();
            writer.print(name);
        }
    }

    protected String makeSpace(int len) {
        if (len <= 0) {
            return "";
        }
        StringBuffer sb = new StringBuffer(len);
        for(int i = 0; i < len; i++) {
            sb.append(' ');
    }
        return sb.toString();
    }

    /**
     * Print 'static' if static and type link.
     */
    protected void printStaticAndType(boolean isStatic, Type type) {
        writer.printTypeSummaryHeader();
        if (isStatic) {
            print("static");
        }
        writer.space();
        if (type != null) {
            printTypeLink(type);
        }
        writer.printTypeSummaryFooter();
    }

    protected void printModifierAndType(ProgramElementDoc member, Type type) {
        writer.printTypeSummaryHeader();
        printModifier(member);
        if (type == null) {
        if (member.isClass()) {
                print("class");
            } else {
            print("interface");
            }
        } else {
            printTypeLink(type);
        }
        writer.printTypeSummaryFooter();
    }

    protected void printModifier(ProgramElementDoc member) {
        if (member.isProtected()) {
            print("protected ");
        } else if (member.isPrivate()) {
            print("private ");
        } else if (!member.isPublic()) { // Package private
            writer.printText("doclet.Package_private");
            print(" ");
        }
        if (member.isMethod() && ((MethodDoc)member).isAbstract()) {
            print("abstract ");
        }
        if (member.isStatic()) {
            print("static");
        }
        writer.space();
    }

    protected void printComment(ProgramElementDoc member) {
        if (member.inlineTags().length > 0) {
            writer.dd();
            writer.printInlineComment(member);
        }
    }

    protected void printTags(ProgramElementDoc member) {
        ConfigurationStandard configuration = configuration();
        TagletManager tagletManager = configuration.tagletManager;
        tagletManager.checkTags(member, member.tags(), false);
        tagletManager.checkTags(member, member.inlineTags(), true);
        Taglet[] definedTags;
        if (member instanceof FieldDoc) {
            definedTags = configuration().tagletManager.getFieldCustomTags();
        } else {
            definedTags = configuration().tagletManager.getMethodCustomTags();
        }
        Tag[][] tagsInDoc = new Tag[definedTags.length][];
        for (int i = 0; i < definedTags.length; i++) {
            tagsInDoc[i] = member.tags(definedTags[i].getName());
        }
        writer.dl();
        String output;
        for (int i = 0; i < definedTags.length; i++) {
            if (definedTags[i] instanceof SimpleTaglet) {
                output = ((SimpleTaglet) definedTags[i]).toString(tagsInDoc[i], writer);
            } else if (definedTags[i] instanceof SeeTaglet) {
                output = ((SeeTaglet) definedTags[i]).toString(member, writer);
            } else {
                output = definedTags[i].toString(tagsInDoc[i]);
            }
            if (output != null && output.length() > 0) {
                tagletManager.seenCustomTag(definedTags[i].getName());
                print(output);
            }
        }
        writer.dlEnd();
    }

    protected String name(ProgramElementDoc member) {
        return member.name();
    }

    protected void printDeprecated(ProgramElementDoc member) {
        Tag[] deprs = member.tags("deprecated");
        if (deprs.length > 0) {
        writer.dd();
            writer.boldText("doclet.Deprecated");
            writer.space();
            writer.printInlineDeprecatedComment(member, deprs[0]);
            writer.p();
        } else {
            printDeprecatedClassComment(member);
        }
    }

    protected void printDeprecatedClassComment(ProgramElementDoc member) {
        Tag[] deprs = member.containingClass().tags("deprecated");
        if (deprs.length > 0) {
            writer.dd();
            writer.boldText("doclet.Deprecated");
            writer.space();
        }
    }
  
    protected void printHead(MemberDoc member) {
        writer.h3();
        writer.print(member.name());
        writer.h3End();
    }

    protected void printFullComment(ProgramElementDoc member) {
        if(configuration().nocomment){
            return;
        }
        writer.dl();
        printDeprecated(member);
        printCommentAndTags(member);
        writer.dlEnd();
    }

    protected void printCommentAndTags(ProgramElementDoc member) {
        printComment(member);
        printTags(member);
    }

    /**
     * Forward to containing writer
     */
    public void printSummaryHeader(ClassDoc cd) {
        printedSummaryHeader = true;
        writer.printSummaryHeader(this, cd);
    }

    /**
     * Forward to containing writer
     */
    public void printInheritedSummaryHeader(ClassDoc cd) {
        writer.printInheritedSummaryHeader(this, cd);
    }

    /**
     * Forward to containing writer
     */
    public void printInheritedSummaryFooter(ClassDoc cd) {
        writer.printInheritedSummaryFooter(this, cd);
    }

    /**
     * Forward to containing writer
     */
    public void printSummaryFooter(ClassDoc cd) {
        writer.printSummaryFooter(this, cd);
    }

    /**
     * Forward to containing writer
     */
    public void printSummaryMember(ClassDoc cd, ProgramElementDoc member) {
        writer.printSummaryMember(this, cd, member);
    }

    /**
     * Forward to containing writer
     */
    public void printInheritedSummaryMember(ClassDoc cd,
                                            ProgramElementDoc member) {
        writer.printInheritedSummaryMember(this, cd, member);
    }

    public void printMembersSummary() {
        List members = new ArrayList(members(classdoc));
        if (members.size() > 0) {
            printedSummaryHeader = true;
            printSummaryHeader(classdoc);
            Collections.sort(members);
            for (int i = 0; i < members.size(); ++i) {
                printSummaryMember(classdoc,
                                   (ProgramElementDoc)(members.get(i)));
            }
            printSummaryFooter(classdoc);
        }
    }

   /**
    * Return true if the given <code>ProgramElement</code> is inherited
    * by the class that is being documented.
    *
    * @param ped The code>ProgramElement</code> being checked.
    * return true if the code>ProgramElement</code> is being inherited and
    * false otherwise.
    */
    protected boolean isInherited(ProgramElementDoc ped){
        if(ped.isPrivate() || (ped.isPackagePrivate() &&
            ! ped.containingPackage().equals(classdoc.containingPackage()))){
            return false;
        }
        return true;
    }

    public void printInheritedMembersSummary() {
        for (int i = 0; i < visibleClasses.size(); i++) {
            ClassDoc inhclass = (ClassDoc)(visibleClasses.get(i));
            if (inhclass == classdoc) {
                continue;
            }
            List inhmembers = new ArrayList(members(inhclass));
            if (inhmembers.size() > 0) {
                if(! (printedSummaryHeader || inhmembers.get(0) instanceof MethodDoc)){
                    printSummaryHeader(classdoc);
                    printSummaryFooter(classdoc);
                    printedSummaryHeader = true;
                }
                Collections.sort(inhmembers);
                printInheritedSummaryHeader(inhclass);
                boolean printedPrev = false;
                ProgramElementDoc ped = (ProgramElementDoc)(inhmembers.get(0));
                if(isInherited(ped)){
                    printInheritedSummaryMember(inhclass, ped);
                    printedPrev = true;
                }
                
                for (int j = 1; j < inhmembers.size(); ++j) {
                   ped = (ProgramElementDoc)(inhmembers.get(j));
                   if(isInherited(ped)){
                         if(printedPrev){
                             print(", ");
                         }
                         printInheritedSummaryMember(inhclass, ped);
                         printedPrev = true;
                   }
                }
                printInheritedSummaryFooter(inhclass);
            }
        }
    }
 
    public void printMembers() {
        List members = members(classdoc);
        if (members.size() > 0) {
            printHeader(classdoc);
            for (int i = 0; i < members.size(); ++i) {
                if (i > 0) {
                    writer.printMemberHeader();
                }
                writer.println("");
                printMember((ProgramElementDoc)(members.get(i)));
                writer.printMemberFooter();
            }
            printBodyHtmlEnd(classdoc);
        }
    }

    /**
     * Generate the code for listing the deprecated APIs. Create the table
     * format for listing the API. Call methods from the sub-class to complete
     * the generation.
     */
    protected void printDeprecatedAPI(List deprmembers, String headingKey) {
        if (deprmembers.size() > 0) {
            writer.tableIndexSummary();
            writer.tableHeaderStart("#CCCCFF");
            writer.boldText(headingKey);
            writer.tableHeaderEnd();
            for (int i = 0; i < deprmembers.size(); i++) {
                ProgramElementDoc member =(ProgramElementDoc)deprmembers.get(i);
                ClassDoc cd = member.containingClass();
                writer.trBgcolorStyle("white", "TableRowColor");
                writer.summaryRow(0);
                printDeprecatedLink(member);
                writer.br();
                writer.printNbsps();
                writer.printInlineDeprecatedComment(member, member.tags("deprecated")[0]);
                writer.space();
                writer.summaryRowEnd();
                writer.trEnd();
            }
            writer.tableEnd();
            writer.space();
            writer.p();
        }
    }

    /**
     * Print use info.
     */
    protected void printUseInfo(Object mems, String heading) {
        if (mems == null) {
            return;
        }
        List members = (List)mems;
        if (members.size() > 0) {
            writer.tableIndexSummary();
            writer.tableUseInfoHeaderStart("#CCCCFF");
            writer.print(heading);
            writer.tableHeaderEnd();
            for (Iterator it = members.iterator(); it.hasNext(); ) {
                ProgramElementDoc pgmdoc = (ProgramElementDoc)it.next();
                ClassDoc cd = pgmdoc.containingClass();

                writer.printSummaryLinkType(this, pgmdoc);
                if (cd != null && !(pgmdoc instanceof ConstructorDoc)
                               && !(pgmdoc instanceof ClassDoc)) {
                    // Add class context
                    writer.bold(cd.name() + ".");
                }
                printSummaryLink(cd, pgmdoc);
                writer.printSummaryLinkComment(this, pgmdoc);
            }
            writer.tableEnd();
            writer.space();
            writer.p();
        }
    }

    protected void navSummaryLink() {
        List members = members(classdoc);
        if (members.size() > 0) {
            printNavSummaryLink(null, true);
            return;
        } else {
            ClassDoc icd = classdoc.superclass();
            while (icd != null) {
                List inhmembers = members(icd);
                if (inhmembers.size() > 0) {
                    printNavSummaryLink(icd, true);
                    return;
                }
                icd = icd.superclass();
            }
        }
        printNavSummaryLink(null, false);
    }
   
    protected void navDetailLink() {
        List members = visibleMemberMap.getMembersFor(classdoc);
        printNavDetailLink(members.size() > 0? true: false);
    }

    protected void serialWarning(SourcePosition pos, String key, String a1, String a2) {
        if (configuration().serialwarn) {
            msg().warning(pos, key, a1, a2);
        }
    }

    public void buildVisibleMemberMap() {
        visibleMemberMap = new VisibleMemberMap(classdoc, getMemberKind(),
                                                nodepr);
        visibleClasses = visibleMemberMap.getVisibleClassesList();
    }

    public ProgramElementDoc[] eligibleMembers(ProgramElementDoc[] members) {
        return nodepr? Util.excludeDeprecatedMembers(members): members;
    }

    public List members(ClassDoc cd) {
        return visibleMemberMap.getMembersFor(cd);
    }

    public ConfigurationStandard configuration() {
        return writer.configuration;
    }

    public MessageRetriever msg() {
        return writer.msg(true);
    }
}
    
    
