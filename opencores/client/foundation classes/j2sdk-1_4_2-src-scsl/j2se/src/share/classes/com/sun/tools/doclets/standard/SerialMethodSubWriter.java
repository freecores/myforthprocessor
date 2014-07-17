/*
 * @(#)SerialMethodSubWriter.java	1.28 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.standard;

import com.sun.javadoc.*;
import com.sun.tools.doclets.*;
import com.sun.tools.doclets.standard.tags.*;
import java.util.*;

/**
 * Generate serialized form for Serializable/Externalizable methods.
 * Documentation denoted by the <code>serialData</code> tag is processed.
 *
 * @author Joe Fialli
 */
public class SerialMethodSubWriter extends MethodSubWriter {
    public SerialMethodSubWriter(SubWriterHolderWriter writer,
                                 ClassDoc classdoc, ConfigurationStandard configuration) {
        super(writer, classdoc, configuration);
    }
    
    public List members(ClassDoc cd) {
        return Util.asList(cd.serializationMethods());
    }
    
    protected void printHeader(ClassDoc cd) {
        writer.anchor("serialized_methods");
        writer.printTableHeadingBackground(writer.getText("doclet.Serialized_Form_methods"));
        
        // Specify if Class is Serializable or Externalizable.
        
        writer.p();
        
        if (cd.isSerializable() && !cd.isExternalizable()) {
            if (members(cd).size() == 0) {
                String msg =
                    writer.getText("doclet.Serializable_no_customization");
                writer.print(msg);
                writer.p();
            }
        }
    }
    
    protected void printMember(ClassDoc cd, ProgramElementDoc member) {
        ExecutableMemberDoc emd = (ExecutableMemberDoc)member;
        String name = emd.name();
        printHead(emd);
        printFullComment(emd);
    }
    
    protected void printSerialDataTag(Doc doc, Tag[] serialData) {
        if (serialData != null && serialData.length > 0) {
            writer.dt();
            writer.boldText("doclet.SerialData");
            writer.dd();
            for (int i = 0; i < serialData.length; i++)
                writer.printInlineComment(doc, serialData[i]);
        }
    }
    
    /**
     * Print comments, See tags and serialData for SerialMethods.
     */
    protected void printTags(ProgramElementDoc member) {
        MethodDoc method = (MethodDoc)member;
        Tag[] serialData = method.tags("serialData");
        Tag[] sinces = method.tags("since");
        SeeTag[] sees = method.seeTags();
        ThrowsTag[] throwTags = method.throwsTags();
        String output;
        if (throwTags.length + serialData.length + sees.length + sinces.length > 0) {
            writer.dd();
            writer.dl();
            printSerialDataTag(member, serialData);
            output = (new ThrowsTaglet()).toString(method, writer);
            if (output != null && output.length() > 0) {
                writer.print(output);
            }
            writer.printSinceTag(method);
            output = (new SeeTaglet()).toString(method, writer);
            if (output != null && output.length() > 0) {
                writer.print(output);
            }
            writer.dlEnd();
            writer.ddEnd();
        } else {
            if (method.name().compareTo("writeExternal") == 0) {
                serialWarning(member.position(), "doclet.MissingSerialDataTag",
                              method.containingClass().qualifiedName(),
                              method.name());
            }
        }
    }
    
    /**
     * Print header even if there are no serializable methods.
     */
    public void printMembers() {
        if (members(classdoc).size() > 0) {
            super.printMembers();
        }
    }
    
    public void buildVisibleMemberMap() {
        // Do nothing.
    }
    
    protected void printTypeLinkNoDimension(Type type) {
        ClassDoc cd = type.asClassDoc();
        if (cd == null || cd.isPackagePrivate()) {
            print(type.typeName());
        } else {
            writer.printClassLink(cd);
        }
    }
}



