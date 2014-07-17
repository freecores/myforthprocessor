/*
 * @(#)ParamTaglet.java	1.10 03/01/23
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
 * A taglet that represents the @param tag.
 * @author Jamie Ho
 * @since 1.4
 */
public class ParamTaglet extends AbstractExecutableMemberTaglet{
    
    public ParamTaglet() {
        name = "param";
    }
    
    /**
     * Given an array of <code>Parameter</code>s, return
     * a name/rank number map.  If the array is null, then
     * null is returned.
     * @param params The array of <code>Parameter</code>s to check.
     * @return a name-rank number map.
     */
    private Map getRankMap(Parameter[] params){
        if (params == null) {
            return null;
        }
        HashMap result = new HashMap();
        for (int i = 0; i < params.length; i++) {
            result.put(params[i].name(), new Integer(i));
        }
        return result;
    }
    
    /**
     * Given an array of <code>Parameter</code>s, return
     * a rank/name number map.  If the array is null, then
     * null is returned.
     * @param params The array of <code>Parameter</code>s to check.
     * @return a rank/name number map.
     */
    private Map getNameMap(Parameter[] params){
        if (params == null) {
            return null;
        }
        HashMap result = new HashMap();
        for (int i = 0; i < params.length; i++) {
            result.put(new Integer(i), params[i].name());
        }
        return result;
    }
    
    /**
     * Given an array of <code>ParamTag</code>s,return its string representation.
     * @param tags the array of <code>ParamTag</code>s representing of this custom tag.
     * @param holder the ExecutableMemberDoc that holds this tag.
     * @param writer the HtmlStandardWriter that will write this tag.
     * @return the string representation of these <code>ParamTag</code>s.
     */
    public String toString(Doc holder, HtmlStandardWriter writer) {
        ExecutableMemberDoc execHolder = (ExecutableMemberDoc) holder;
        Map nameMap = getNameMap(execHolder.parameters());
        ParamTag[] tags = execHolder.paramTags();
        String result = "";
        MethodDoc inheritedMethod;
        HashSet alreadyDocumented = new HashSet();
        if (tags.length > 0) {
            result += paramTagsToString(execHolder,
                                        execHolder.paramTags(),
                                        writer,
                                        getRankMap(execHolder.parameters()),
                                        nameMap,
                                        alreadyDocumented,
                                        false);
        }
        if (holder instanceof MethodDoc &&
                (inheritedMethod = getInheritedMethodDoc((MethodDoc) holder)) != null) {
            //Inherit param tags
            result += paramTagsToString(execHolder,
                                        inheritedMethod.paramTags(),
                                        writer,
                                        getRankMap(inheritedMethod.parameters()),
                                        nameMap,
                                        alreadyDocumented,
                                        true);
        }
        return result.equals("") ? null : result;
    }
    
    /**
     * Given an array of <code>Tag</code>s representing this custom
     * tag, return its string representation.
     * @param holder the ExecutableMemberDoc that holds this tag.
     * @param paramTags the array of <code>ParamTag</code>s to convert.
     * @param writer the HtmlStandardWriter that will write this tag.
     * @param alreadyDocumented the set of exceptions that have already
     *        been documented.
     * @param rankMap a {@link java.util.Map} which holds ordering
     *                    information about the parameters.
     * @param nameMap a {@link java.util.Map} which holds a mapping
     *                of a rank of a parameter to its name.  This is
     *                used to ensure that the right name is used
     *                when parameter documentation is inherited.
     * @param isInheriting true if we are inheriting param docs.
     * @return the string representation of this <code>Tag</code>.
     */
    protected String paramTagsToString(ExecutableMemberDoc holder,
                                       ParamTag[] paramTags,
                                       HtmlStandardWriter writer,
                                       Map rankMap,
                                       Map nameMap,
                                       Set alreadyDocumented, 
                                       boolean isInheriting) {
        String result = "";
        if (paramTags.length == 0
            && holder instanceof MethodDoc
            && holder.commentText().indexOf("{@inheritDoc}") != -1) {
            MethodDoc omd = ((MethodDoc) holder).overriddenMethod();
            if (omd != null) {
                paramTags = omd.paramTags();
            }
        }
        if (paramTags.length > 0) {
            if (holder instanceof MethodDoc
                && holder.commentText().indexOf("{@inheritDoc}") != -1) {
                MethodDoc omd = ((MethodDoc) holder).overriddenMethod();
                if (omd != null) {
                    paramTags = omd.paramTags();
                }
            }
            String name;
            Integer rank;
            for (int i = 0; i < paramTags.length; ++i) {
                ParamTag pt = paramTags[i];
                rank = (Integer) rankMap.get(name = pt.parameterName());
                if (rank == null) {
                    if (! isInheriting) {
                        writer.warning(pt.position(), "doclet.Parameters_warn", pt.parameterName());
                    }
                    continue;
                }
                if (alreadyDocumented.contains(rank)) {
                    if (! isInheriting) {
                        writer.warning(pt.position(), "doclet.Parameters_dup_warn", pt.parameterName());
                    }
                    continue;
                }
                if (alreadyDocumented.size() == 0) {
                    writer.dt();
                    writer.boldText("doclet.Parameters");
                }
                result += "<DD><CODE>" + nameMap.get(rank) + "</CODE>";
                result += " - " + writer.commentTagsToString(null, pt.inlineTags(), false, false);
                if (holder instanceof MethodDoc) {
                    result = writer.replaceInheritDoc((MethodDoc) holder, pt, result);
                }
                alreadyDocumented.add(rank);
            }
        }
        return result;
    }
}

