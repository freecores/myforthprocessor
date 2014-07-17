/*
 * @(#)ExecutableMemberDoc.java	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javadoc;

/**
 * Represents a method or constructor of a java class.
 *
 * @since JDK1.2
 * @author Robert Field
 */
public interface ExecutableMemberDoc extends MemberDoc {

    /**
     * Return exceptions this method or constructor throws.
     *
     * @return an array of Type[] representing the exceptions
     * thrown by this method.
     */
    ClassDoc[] thrownExceptions();

    /**
     * Return true if this method is native
     */
    boolean isNative();

    /**
     * Return true if this method is synchronized
     */
    boolean isSynchronized();

    /**
     * Get argument information.
     *
     * @see Parameter
     *
     * @return an array of Parameter, one element per argument
     * in the order the arguments are present.
     */
    Parameter[] parameters();

    /**
     * Return the throws tags in this method.
     *
     * @return an array of ThrowTag containing all <code>&#64exception</code>
     * and <code>&#64throws</code> tags.
     */
    ThrowsTag[] throwsTags();

    /**
     * Return the param tags in this method.
     *
     * @return an array of ParamTag containing all <code>&#64param</code> tags.
     */
    ParamTag[] paramTags();

    /**
     * Get the signature. It is the parameter list, type is qualified.
     *      For instance, for a method <code>mymethod(String x, int y)</code>,
     *      it will return <code>(java.lang.String,int)</code>.
     */
    String signature();

    /**
     * get flat signature.  all types are not qualified.
     *      return a String, which is the flat signiture of this member.
     *      It is the parameter list, type is not qualified.
     *      For instance, for a method <code>mymethod(String x, int y)</code>,
     *      it will return <code>(String, int)</code>.
     */
    String flatSignature();
}
