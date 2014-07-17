/*
 * @(#)ConstantFieldSubWriter.java	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.standard;

import com.sun.tools.doclets.*;
import com.sun.javadoc.*;
import java.util.*;

/**
 * Print the table of constants.
 *
 * @author Jamie Ho
 * @since 1.4
 */
public class ConstantFieldSubWriter extends FieldSubWriter {
    
    /**
     * The map used to get the visible variables.
     */
    public VisibleMemberMap visibleMemberMap = null;
    
    /**
     * The writer to output the constants information.
     */
    SubWriterHolderWriter writer;
    
    /**
     * The classdoc that we are examining constants for.
     */
    ClassDoc classdoc;
    
    /**
     * Construct a ConstantFieldSubWriter.
     * @param writer the writer to output the constants information.
     * @param classdoc the classdoc that we are examining constants for.
     */
    public ConstantFieldSubWriter(SubWriterHolderWriter writer, ClassDoc classdoc) {
        super(writer, classdoc);
        this.writer = writer;
        this.classdoc = classdoc;
        visibleMemberMap = new VisibleMemberMap(classdoc, getMemberKind(),
                                                writer.configuration().nodeprecated);
    }
    
    /**
     * Print the table of constants.
     */
    public void printMembersSummary() {
        List members = new ArrayList(members(classdoc));
        if (members.size() > 0) {
            Collections.sort(members);
            for (int i = 0; i < members.size(); ++i) {
                printSummaryMember((FieldDoc)(members.get(i)));
            }
        }
    }
    
    /**
     * Print the row in the constant table for the given member.
     * @param member the member to print information for.
     */
    protected void printSummaryMember(FieldDoc member) {
        writer.trBgcolorStyle("white", "TableRowColor");
        writer.anchor(member.qualifiedName());
        printTypeColumn(member);
        printNameColumn(member);
        printValue(member);
        writer.trEnd();
    }
    
    /**
     * Print the type of the given member in a column.
     * @param member the member to print information for.
     */
    protected void printTypeColumn(FieldDoc member) {
        writer.tdAlign("right");
        writer.font("-1");
        writer.code();
        StringTokenizer mods = new StringTokenizer(member.modifiers());
        while(mods.hasMoreTokens()) {
            writer.print(mods.nextToken() + "&nbsp;");
        }
        printTypeLink(member.type());
        writer.codeEnd();
        writer.fontEnd();
        writer.tdEnd();
    }
    
    /**
     * Print the name of the given member in a column.
     * @param member the member to print information for.
     */
    protected void printNameColumn(FieldDoc member) {
        writer.tdAlign("left");
        writer.code();
        String name = member.name();
        writer.printClassLink(classdoc, name, name, false);
        writer.codeEnd();
        writer.tdEnd();
    }
    
    /**
     * Print the constant value of the given member in a column.
     * @param member the member to print information for.
     */
    protected void printValue(FieldDoc member) {
        writer.tdAlign("right");
        writer.code();
        writer.print(member.constantValueExpression());
        writer.codeEnd();
        writer.tdEnd();
    }
    
    /**
     * Return the list of visible constant fields for the given classdoc.
     * @param cd the classdoc to examine.
     * @return the list of visible constant fields for the given classdoc.
     */
    public List members(ClassDoc cd) {
        List l = visibleMemberMap.getMembersFor(cd);
        Iterator iter;
        
        if(l != null){
            iter = l.iterator();
        } else {
            return null;
        }
        List inclList = new LinkedList();
        FieldDoc member;
        while(iter.hasNext()){
            member = (FieldDoc)iter.next();
            if(member.constantValue() != null){
                inclList.add(member);
            }
        }
        return inclList;
    }
}


