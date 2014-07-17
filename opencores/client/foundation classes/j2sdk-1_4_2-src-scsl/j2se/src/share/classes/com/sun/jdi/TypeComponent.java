/*
 * @(#)TypeComponent.java	1.20 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jdi;

/**
 * An entity declared within a user defined
 * type (class or interface).
 * This interface is the root of the type
 * component hierarchy which 
 * includes {@link Field} and {@link Method}. 
 * Type components of the same name declared in different classes 
 * (including those related by inheritance) have different 
 * TypeComponent objects.
 * TypeComponents can be used alone to retrieve static information
 * about their declaration, or can be used in conjunction with a 
 * {@link ReferenceType} or {@link ObjectReference} to access values
 * or invoke, as applicable.
 *
 * @author Robert Field
 * @author Gordon Hirsch
 * @author James McIlree
 * @since  1.3
 */
public interface TypeComponent extends Mirror, Accessible {

    /**
     * Gets the name of this type component. 
     * <P> 
     * Note: for fields, this is the field name; for methods,
     * this is the method name; for constructors, this is &lt;init&gt;;
     * for static initializers, this is &lt;clinit&gt;.
     *
     * @return a string containing the name.
     */
    String name();

    /**
     * Gets the JNI-style signature for this type component. The 
     * signature is encoded type information as defined
     * in the JNI documentation. It is a convenient, compact format for 
     * for manipulating type information internally, not necessarily
     * for display to an end user. See {@link Field#typeName} and 
     * {@link Method#returnTypeName} for ways to help get a more readable
     * representation of the type.
     *
     * @see <a href="doc-files/signature.html">Type Signatures</a>
     * @return a string containing the signature
     */   
    String signature();
    
    /**
     * Returns the type in which this component was declared. The
     * returned {@link ReferenceType} mirrors either a class or an 
     * interface in the target VM.
     *
     * @return a {@link ReferenceType} for the type that declared
     * this type component.
     */    
    ReferenceType declaringType();

    /**
     * Determines if this TypeComponent is static.
     * Return value is undefined for constructors and static initializers.
     *
     * @return <code>true</code> if this type component was declared
     * static; false otherwise.
     */
    boolean isStatic(); 

    /**
     * Determines if this TypeComponent is final.
     * Return value is undefined for constructors and static initializers.
     *
     * @return <code>true</code> if this type component was declared
     * final; false otherwise.
     */
    boolean isFinal();
    
    /**
     * Determines if this TypeComponent is synthetic. Synthetic members
     * are generated by the compiler and are not present in the source
     * code for the containing class.
     * <p>
     * Not all target VMs support this query. See
     * {@link VirtualMachine#canGetSyntheticAttribute} to determine if the 
     * operation is supported.
     *
     * @return <code>true</code> if this type component is synthetic; 
     * <code>false</code> otherwise.
     * @throws java.lang.UnsupportedOperationException if the target
     * VM cannot provide information on synthetic attributes.
     */
    boolean isSynthetic();
}

