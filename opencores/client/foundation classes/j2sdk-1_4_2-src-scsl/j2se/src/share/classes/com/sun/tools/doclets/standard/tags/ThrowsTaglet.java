/*
 * @(#)ThrowsTaglet.java	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.standard.tags;

import com.sun.tools.doclets.standard.*;
import com.sun.tools.doclets.*;
import com.sun.javadoc.*;
import java.util.*;

/**
 * A taglet that represents the @throws tag.
 * @author Jamie Ho
 * @since 1.4
 */
public class ThrowsTaglet extends AbstractExecutableMemberTaglet{
    
    public ThrowsTaglet() {
        name = "throws";
    }
    
    /**
     * Given an array of <code>Tag</code>s representing this custom
     * tag, return its string representation. If possible, inherit the throws tags
     * for undocumented declared exceptions.
     * @param tags the array of <code>ThrowsTag</code>s representing of this custom tag.
     * @param holder the ExecutableMemberDoc that holds this tag.
     * @param writer the HtmlStandardWriter that will write this tag.
     * @return the string representation of this <code>Tag</code>.
     */
    public String toString(Doc holder, HtmlStandardWriter writer) {
        ExecutableMemberDoc execHolder = (ExecutableMemberDoc) holder;
        ThrowsTag[] tags = execHolder.throwsTags();
        String result = "";
        MethodDoc inheritedMethod;
        HashSet alreadyDocumented = new HashSet();
        if (tags.length > 0) {
            result += throwsTagsToString(execHolder,
                                         execHolder.throwsTags(),
                                         writer,
                                         alreadyDocumented,
                                         true);
        }
        ClassDoc[] declaredExceptionCDs = execHolder.thrownExceptions();
        if (holder instanceof MethodDoc &&
                (inheritedMethod = getInheritedMethodDoc((MethodDoc) holder)) != null) {
            //Inherit @throws tags only for declared exceptions.
            HashSet declaredExceptions = new HashSet();
            
            //Save all declared exceptions
            for (int j = 0; j < declaredExceptionCDs.length; j++) {
                declaredExceptions.add(declaredExceptionCDs[j]);
            }
            //Inherit @throws tags for declared exceptions
            HashSet declaredExceptionTags = new HashSet();
            ThrowsTag[] throwsTags = inheritedMethod.throwsTags();
            for (int j = 0; j < throwsTags.length; j++) {
                if (declaredExceptions.contains(throwsTags[j].exception())) {
                    declaredExceptionTags.add(throwsTags[j]);
                }
            }
            result += throwsTagsToString(execHolder,
                                        (ThrowsTag[]) declaredExceptionTags.toArray(new ThrowsTag[] {}),
                                         writer,
                                         alreadyDocumented,
                                         false);
        }
        
        //Add links to the exceptions declared but not documented.
        for (int i = 0; i < declaredExceptionCDs.length; i++) {
            if (declaredExceptionCDs[i] != null &&
                ! alreadyDocumented.contains(declaredExceptionCDs[i].name()) &&
                ! alreadyDocumented.contains(declaredExceptionCDs[i].qualifiedName())) {
                if (alreadyDocumented.size() == 0) {
                    result += getHeader(writer);
                }
                result += "\n<DD>";
                result += writer.codeText(writer.getClassLink(declaredExceptionCDs[i]));
                alreadyDocumented.add(declaredExceptionCDs[i].name());
            }
        }
        
        return result.equals("") ? null : result;
    }
    
    /**
     * Returns the header for this tag.
     * @param writer the writer object to read the header from.
     * @return the header for this tag.
     */
    private String getHeader(HtmlStandardWriter writer) {
        return "\n<DT>" + "<B>" + writer.getText("doclet.Throws") + "</B>";
    }
    
    /**
     * Given an array of <code>Tag</code>s representing this custom
     * tag, return its string representation.
     * @param holder the ExecutableMemberDoc that holds this tag.
     * @param throwsTags the array of <code>ThrowsTag</code>s to convert.
     * @param writer the HtmlStandardWriter that will write this tag.
     * @param alreadyDocumented the set of exceptions that have already
     *        been documented.
     * @param allowDups True if we allow duplicate throws tags to be documented.
     * @return the string representation of this <code>Tag</code>.
     */
    protected String throwsTagsToString(ExecutableMemberDoc holder,
                                        ThrowsTag[] throwTags,
                                        HtmlStandardWriter writer,
                                        Set alreadyDocumented,
                                        boolean allowDups) {
        String result = "";
        if (throwTags.length > 0) {
            for (int i = 0; i < throwTags.length; ++i) {
                ThrowsTag tt = throwTags[i];
                ClassDoc cd = tt.exception();
                if ((!allowDups) && (alreadyDocumented.contains(tt.exceptionName()) ||
                                         (cd != null && alreadyDocumented.contains(cd.qualifiedName())))) {
                    continue;
                }
                if (alreadyDocumented.size() == 0) {
                    result += getHeader(writer);
                }
                result += "\n<DD>";
                result += cd == null ? writer.codeText(tt.exceptionName()) :
                    writer.codeText(writer.getClassLink(cd));
                String text = writer.commentTagsToString(null, tt.inlineTags(), false, false);
                if (holder instanceof MethodDoc) {
                    text = writer.replaceInheritDoc((MethodDoc) holder, tt, text);
                }
                if (text != null && text.length() > 0) {
                    result += " - " + text;
                }
                alreadyDocumented.add(cd != null ? cd.qualifiedName() : tt.exceptionName());
            }
        }
        return result;
    }
}
