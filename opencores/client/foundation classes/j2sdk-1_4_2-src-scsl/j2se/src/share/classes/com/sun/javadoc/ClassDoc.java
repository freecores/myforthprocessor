/*
 * @(#)ClassDoc.java	1.15 02/10/06
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javadoc;


/**
 * Represents a java class or interface and provides access to 
 * information about the class, the class's comment and tags, and the
 * members of the class.  A ClassDoc only exists if it was
 * processed in this run of javadoc.  References to classes
 * which may or may not have been processed in this run are
 * referred to using Type (which can be converted to ClassDoc,
 * if possible).
 *
 * @see Type
 *
 * @since JDK1.2
 * @author Kaiyang Liu (original)
 * @author Robert Field (rewrite)
 */
public interface ClassDoc extends ProgramElementDoc, Type {

    /**
     * Return true if this class is abstract.  Return true
     * for all interfaces.
     */
    boolean isAbstract();

    /**
     * Return true if this class implements or interface extends
     * <code>java.io.Serializable</code>.
     *
     * Since <code>java.io.Externalizable</code> extends
     * <code>java.io.Serializable</code>,
     * Externalizable objects are also Serializable.
     */
    boolean isSerializable();

    /**
     * Return true if this class implements or interface extends
     * <code>java.io.Externalizable</code>.
     */
    boolean isExternalizable();

    /**
     * Return the serialization methods for this class or
     * interface.
     *
     * @return an array of MethodDoc objects that represents
     *         the serialization methods for this class or interface.
     */
    MethodDoc[] serializationMethods();

    /**
     * Return the Serializable fields of this class or interface.
     * <p>
     * Return either a list of default fields documented by
     * <code>serial</code> tag<br>
     * or return a single <code>FieldDoc</code> for
     * <code>serialPersistentField</code> member.
     * There should be a <code>serialField</code> tag for
     * each Serializable field defined by an <code>ObjectStreamField</code>
     * array component of <code>serialPersistentField</code>.
     *
     * @return an array of <code>FieldDoc</code> objects for the Serializable 
     *         fields of this class or interface.
     *
     * @see #definesSerializableFields()
     * @see SerialFieldTag
     */
    FieldDoc[] serializableFields();

    /**
     *  Return true if Serializable fields are explicitly defined with
     *  the special class member <code>serialPersistentFields</code>.
     *
     * @see #serializableFields()
     * @see SerialFieldTag
     */
    boolean definesSerializableFields();

    /**
     * Return the superclass of this class.  Return null is this is an
     * interface.
     *
     * @return the ClassDoc for the superclass of this class, null if
     *         there is no superclass.
     */
    ClassDoc superclass();

    /**
     * Test whether this class is a subclass of the specified class.
     * If this is an interface, return false for all classes except 
     * <code>java.lang.Object</code> (we must keep this unexpected
     * behavior for compatibility reasons).
     *
     * @param cd the candidate superclass.
     * @return true if cd is a superclass of this class.
     */
    boolean subclassOf(ClassDoc cd);

    /**
     * Return interfaces implemented by this class or interfaces extended
     * by this interface. Includes only directly-declared interfaces, not
     * inherited interfaces.
     * Return an empty array if there are no interfaces.
     *
     * @return An array of ClassDoc objects representing the interfaces.
     */
    ClassDoc[] interfaces();

    /**
     * Return
     * <a href="{@docRoot}/com/sun/javadoc/package-summary.html#included">included</a>
     * fields in this class or interface.
     *
     * @return an array of FieldDoc objects representing the included
     *         fields in this class or interface.
     */
    FieldDoc[] fields();
    
    /**
     * Return fields in this class or interface, filtered to the specified
     * <a href="{@docRoot}/com/sun/javadoc/package-summary.html#included">access 
     * modifier option</a>
     *
     * @param filter Specify true to filter according to the specified access
     *               modifier option.
     *               Specify false to include all fields regardless of
     *               access modifier option.
     * @return       an array of FieldDoc objects representing the included
     *               fields in this class or interface.
     */
    FieldDoc[] fields(boolean filter);

    /**
     * Return 
     * <a href="{@docRoot}/com/sun/javadoc/package-summary.html#included">included</a>
     * methods in this class or interface.
     * Same as <code>methods(true)</code>.
     *
     * @return an array of MethodDoc objects representing the included
     *         methods in this class or interface.  Does not include constructors.
     */
    MethodDoc[] methods();
    
    /**
     * Return methods in this class or interface, filtered to the specified
     * <a href="{@docRoot}/com/sun/javadoc/package-summary.html#included">access 
     * modifier option</a>.  Does not include constructors.
     *
     * @param filter Specify true to filter according to the specified access
     *               modifier option.
     *               Specify false to include all methods regardless of
     *               access modifier option.
     * @return       an array of MethodDoc objects representing the included
     *               methods in this class or interface.
     */
    MethodDoc[] methods(boolean filter);

    /**
     * Return
     * <a href="{@docRoot}/com/sun/javadoc/package-summary.html#included">included</a>
     * constructors in this class.  An array containing the default
     * no-arg constructor is returned if no other constructors exist.
     * Return empty array if this is an interface.
     *
     * @return an array of ConstructorDoc objects representing the included
     *         constructors in this class.
     */
    ConstructorDoc[] constructors();
    
    /**
     * Return constructors in this class, filtered to the specified
     * <a href="{@docRoot}/com/sun/javadoc/package-summary.html#included">access 
     * modifier option</a>.  Return an array containing the default
     * no-arg constructor if no other constructors exist.
     *
     * @param filter Specify true to filter according to the specified access
     *               modifier option.
     *               Specify false to include all constructors regardless of
     *               access modifier option.
     * @return       an array of ConstructorDoc objects representing the included
     *               constructors in this class.
     */
    ConstructorDoc[] constructors(boolean filter);


    /**
     * Return 
     * <a href="{@docRoot}/com/sun/javadoc/package-summary.html#included">included</a>
     * nested classes and interfaces within this class or interface.
     * This includes both static and non-static nested classes.
     * (This method should have been named <code>nestedClasses()</code>, 
     * as inner classes are technically non-static.)  Anonymous and local classes
     * or interfaces are not included.
     *
     * @return an array of ClassDoc objects representing the included classes
     *         and interfaces defined in this class or interface. 
     */
    ClassDoc[] innerClasses();
    
    /**
     * Return nested classes and interfaces within this class or interface
     * filtered to the specified 
     * <a href="{@docRoot}/com/sun/javadoc/package-summary.html#included">access 
     * modifier option</a>.
     * This includes both static and non-static nested classes.
     * Anonymous and local classes are not included.
     *
     * @param filter Specify true to filter according to the specified access
     *               modifier option.
     *               Specify false to include all nested classes regardless of
     *               access modifier option.
     * @return       a filtered array of ClassDoc objects representing the included
     *               classes and interfaces defined in this class or interface.
     */
    ClassDoc[] innerClasses(boolean filter);
    
    /**
     * Find the specified class or interface within the context of this class doc.
     * Search order: 1) qualified name, 2) nested in this class or interface,
     * 3) in this package, 4) in the class imports, 5) in the package imports.
     * Return the ClassDoc if found, null if not found.
     */
    ClassDoc findClass(String className);

    /**
     * Get the list of classes and interfaces declared as imported.
     * These are called "single-type-import declarations" in the 
     * Java Language Specification.
     *
     * @return an array of ClassDoc representing the imported classes.
     */
    ClassDoc[] importedClasses();

    /**
     * Get the list of packages declared as imported.
     * These are called "type-import-on-demand declarations" in the 
     * Java Language Specification.
     *
     * @return an array of PackageDoc representing the imported packages.
     */
    PackageDoc[] importedPackages();
}


