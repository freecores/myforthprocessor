/*
 * @(#)ExecutableMemberSubWriter.java	1.35 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.standard;

import com.sun.tools.doclets.*;
import com.sun.javadoc.*;
import java.util.*;

/**
 * Print method and constructor info.
 *
 * @author Robert Field
 * @author Atul M Dambalkar
 */
public abstract class ExecutableMemberSubWriter extends AbstractSubWriter {

    public ExecutableMemberSubWriter(SubWriterHolderWriter writer,
                                     ClassDoc classdoc) {
        super(writer, classdoc);
    }

    public ExecutableMemberSubWriter(SubWriterHolderWriter writer) {
        super(writer);
    }

    protected void printSignature(ExecutableMemberDoc member) {
        writer.displayLength = 0;
    writer.pre();
    printModifiers(member);
    if (configuration().genSrc &&
            member.position().line() != classdoc.position().line()) {
            writer.printSrcLink(member, member.name());
        } else {
            bold(member.name());
        }
    printParameters(member);
    printExceptions(member);
    writer.preEnd();
    }
      
    protected void printDeprecatedLink(ProgramElementDoc member) {
        ExecutableMemberDoc emd = (ExecutableMemberDoc)member;
        writer.printClassLink(emd.containingClass(),
                              emd.name() + emd.signature(),
                              emd.qualifiedName() + emd.flatSignature());
    }
  
    protected void printSummaryLink(ClassDoc cd, ProgramElementDoc member) {
        ExecutableMemberDoc emd = (ExecutableMemberDoc)member;
        String name = emd.name();
    writer.bold();
    writer.printClassLink(cd, name + emd.signature(), name, false);
    writer.boldEnd();
        writer.displayLength = name.length();
    printParameters(emd);
    }
 
    protected void printInheritedSummaryLink(ClassDoc cd,
                                             ProgramElementDoc member) {
        ExecutableMemberDoc emd = (ExecutableMemberDoc)member;
        String name = emd.name();
    writer.printClassLink(cd, name + emd.signature(), name, false);
    }

    protected void printParam(Parameter param) {
        printTypedName(param.type(), param.name());
    }

    protected void printParameters(ExecutableMemberDoc member) {
        int paramstart = 0;
        print('(');
        Parameter[] params = member.parameters();
        String indent = makeSpace(writer.displayLength);
        if (configuration().genSrc) {
            //add spaces to offset indentation changes caused by link.
            indent+= makeSpace(member.name().length());
        }
        while (paramstart < params.length) {
            Parameter param = params[paramstart++];
            if (!param.name().startsWith("this$")) {
                printParam(param);
                break;
            }
        }

        for (int i = paramstart; i < params.length; i++) {
            writer.print(',');
            writer.print('\n');
            writer.print(indent);
            printParam(params[i]);
        }
        writer.print(')');
    }

    protected void printExceptions(ExecutableMemberDoc member) {
        ClassDoc[] except = member.thrownExceptions();
        if(except.length > 0) {
            int retlen = getReturnTypeLength(member);
            String indent = makeSpace(modifierString(member).length() +
                                  member.name().length() + retlen - 4);
            writer.print('\n');
            writer.print(indent);
            writer.print("throws ");
            indent += "       ";
            writer.printClassLink(except[0]);
            for(int i = 1; i < except.length; i++) {
                writer.print(",\n");
                writer.print(indent);
                writer.printClassLink(except[i]);
            }
        }
    }

    protected int getReturnTypeLength(ExecutableMemberDoc member) {
        if (member instanceof MethodDoc) {
            MethodDoc method = (MethodDoc)member;
            Type rettype = method.returnType();
            ClassDoc cd = rettype.asClassDoc();
            if (cd == null) {
                return rettype.typeName().length() +
                       rettype.dimension().length();
            } else {
                return rettype.dimension().length() +
                       ((writer.isCrossClassIncluded(cd))?
                            cd.name().length(): cd.qualifiedName().length());
            }
        } else {   // it's a constructordoc
            return -1;
        }
    }

    protected ClassDoc implementsMethodInIntfac(MethodDoc method,
                                                ClassDoc[] intfacs) {
        for (int i = 0; i < intfacs.length; i++) {
            MethodDoc[] methods = intfacs[i].methods();
            if (methods.length > 0) {
                for (int j = 0; j < methods.length; j++) {
                    if (methods[j].name().equals(method.name()) &&
                          methods[j].signature().equals(method.signature())) {
                        return intfacs[i];
                    }
                }
            }
        }
        return null;
    }

    protected String name(ProgramElementDoc member) {
        return member.name() + "()";
    }

    protected void printBodyHtmlEnd(ClassDoc cd) {
    }

    protected void printMember(ProgramElementDoc member) {
        ExecutableMemberDoc emd = (ExecutableMemberDoc)member;
        String name = emd.name();
        writer.anchor(name + emd.signature());
        printHead(emd);
        printSignature(emd);
        printFullComment(emd);
    }
}
    
    
