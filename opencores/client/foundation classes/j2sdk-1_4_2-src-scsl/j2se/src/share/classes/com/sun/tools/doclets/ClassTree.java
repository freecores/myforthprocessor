/*
 * @(#)ClassTree.java	1.19 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets;

import com.sun.javadoc.*;
import java.io.*;
import java.lang.*;
import java.util.*;

/**
 * Build Class Hierarchy for all the Classes. This class builds the Class
 * Tree and the Interface Tree separately.
 *
 * @see java.util.HashMap
 * @see java.util.List
 * @see com.sun.javadoc.Type
 * @see com.sun.javadoc.ClassDoc
 * @author Atul M Dambalkar
 */
public class ClassTree {
    
    /**
     * List of baseclasses. Contains only java.lang.Object. Can be used to get
     * the mapped listing of sub-classes.
     */
    private List baseclasses = new ArrayList();

    /**
    * Mapping for each Class with their SubClasses
    */
    private Map subclasses = new HashMap();

    /**
     * List of base-interfaces. Contains list of all the interfaces who do not
     * have super-interfaces. Can be used to get the mapped listing of
     * sub-interfaces.
     */
    private List baseinterfaces = new ArrayList();

    /**
    * Mapping for each Interface with their SubInterfaces
    */
    private Map subinterfaces = new HashMap();

    /**
    * Mapping for each Interface with classes who implement it.
    */
    private Map implementingclasses = new HashMap();

    /**
     * Constructor. Build the Tree using the Root of this Javadoc run.
     *
     * @param configuration the configuration of the doclet.
     * @param noDeprecated Don't add deprecated classes in the class tree, if
     * true.
     */
    public ClassTree(Configuration configuration, boolean noDeprecated) {
        configuration.message.notice("doclet.Building_Tree");
        buildTree(configuration.root.classes(), noDeprecated);
    }
    
    /**
     * Constructor. Build the Tree using the Root of this Javadoc run.
     *
     * @param root Root of the Document.
     * @param noDeprecated Don't add deprecated classes in the class tree, if
     * true.
     */
    public ClassTree(RootDoc root, boolean noDeprecated) {
        buildTree(root.classes(), noDeprecated);
    }

    /**
     * Constructor. Build the tree for the given array of classes.
     *
     * @param classes Array of classes.
     * @param noDeprecated Don't add deprecated classes in the class tree, if
     * true.
     */
    public ClassTree(ClassDoc[] classes, boolean noDeprecated) {
        buildTree(classes, noDeprecated);
    }
        
    /**
     * Generate mapping for the sub-classes for every class in this run.
     * Return the sub-class list for java.lang.Object which will be having
     * sub-class listing for itself and also for each sub-class itself will
     * have their own sub-class lists.
     *
     * @param classes all the classes in this run.
     * @param noDeprecated Don't add deprecated classes in the class tree, if
     * true.
     */
    private void buildTree(ClassDoc[] classes, boolean noDeprecated) {
        for (int i = 0; i < classes.length; i++) {
            if (noDeprecated && classes[i].tags("deprecated").length > 0) {
                continue;
            }
            if (classes[i].isClass()) {
                processClass(classes[i]);
            } else {    // this is an interface.
                processInterface(classes[i]);
                List list  = (List)implementingclasses.get(classes[i]);
                if (list != null) {
                    Collections.sort(list);
                }
            }
        }

        Collections.sort(baseinterfaces);
        for (Iterator it = subinterfaces.values().iterator(); it.hasNext(); ) {
            Collections.sort((List)it.next());
        }
        for (Iterator it = subclasses.values().iterator(); it.hasNext(); ) {
            Collections.sort((List)it.next());
        }
    }

    /**
     * For the class passed map it to it's own sub-class listing.
     * For the Class passed, get the super class,
     * if superclass is non null, (it is not "java.lang.Object")
     *    get the "value" from the hashmap for this key Class
     *    if entry not found create one and get that.
     *    add this Class as a sub class in the list
     *    Recurse till hits java.lang.Object Null SuperClass.
     *
     * @param cd class for which sub-class mapping to be generated.
     */
    private void processClass(ClassDoc cd) {
        ClassDoc superclass = cd.superclass();
        if (superclass != null) {
            if (!add(subclasses, superclass, cd)) {
                return;
            } else {
                processClass(superclass);      // Recurse
            }
        } else {     // cd is java.lang.Object, add it once to the list
            if (!baseclasses.contains(cd)) {
                baseclasses.add(cd);
            }
        }
        ClassDoc[] intfacs = cd.interfaces();
        for (int i = 0; i < intfacs.length; i++) {
            add(implementingclasses, intfacs[i], cd);
        }
    }

    /**
     * For the interface passed get the interfaces which it extends, and then
     * put this interface in the sub-interface list of those interfaces. Do it
     * recursively. If a interface doesn't have super-interface just attach
     * that interface in the list of all the baseinterfaces.
     *
     * @param cd Interface under consideration.
     */
    private void processInterface(ClassDoc cd) {
        ClassDoc[] intfacs = cd.interfaces();
        if (intfacs.length > 0) {
            for (int i = 0; i < intfacs.length; i++) {
                if (!add(subinterfaces, intfacs[i], cd)) {
                    return;
                } else {
                    processInterface(intfacs[i]);   // Recurse
                }
            }
        } else {
            // we need to add all the interfaces who do not have
            // super-interfaces to baseinterfaces list to traverse them
            if (!baseinterfaces.contains(cd)) {
                baseinterfaces.add(cd);
            }
        }
    }

    /**
     * Adjust the Class Tree. Add the class interface  in to it's super-class'
     * or super-interface's sub-interface list.
     *
     * @param map the entire map.
     * @param superclass java.lang.Object or the super-interface.
     * @param cd sub-interface to be mapped.
     * @returns boolean true if class added, false if class already processed.
     */
    private boolean add(Map map, ClassDoc superclass, ClassDoc cd) {
        List list = (List)map.get(superclass);
        if (list == null) {
            list = new ArrayList();
            map.put(superclass, list);
        }
        if (list.contains(cd)) {
            return false;
        } else {
            list.add(cd);
        }
        return true;
    }

    /**
     * From the map return the list of sub-classes or sub-interfaces. If list
     * is null create a new one and return it.
     *
     * @param map The entire map.
     * @param cd class for which the sub-class list is requested.
     * @returns List Sub-Class list for the class passed.
     */
    private List get(Map map, ClassDoc cd) {
        List list = (List)map.get(cd);
        if (list == null) {
            return new ArrayList();
        }
        return list;
    }

    /**
     *  Return the sub-class list for the class passed.
     *
     * @param cd class whose sub-class list is required.
     */
    public List subclasses(ClassDoc cd) {
        return get(subclasses, cd);
    }

    /**
     *  Return the sub-interface list for the interface passed.
     *
     * @param cd interface whose sub-interface list is required.
     */
    public List subinterfaces(ClassDoc cd) {
        return get(subinterfaces, cd);
    }

    /**
     *  Return the list of classes which implement the interface passed.
     *
     * @param cd interface whose implementing-classes list is required.
     */
    public List implementingclasses(ClassDoc cd) {
        List result = get(implementingclasses, cd);
        List subinterfaces = allSubs(cd);
        
        //If class x implements a subinterface of cd, then it follows
        //that class x implements cd.
        Iterator implementingClassesIter, subInterfacesIter = subinterfaces.listIterator();
        ClassDoc c;
        while(subInterfacesIter.hasNext()){
            implementingClassesIter = implementingclasses((ClassDoc)
                    subInterfacesIter.next()).listIterator();
            while(implementingClassesIter.hasNext()){
                c = (ClassDoc)implementingClassesIter.next();
                if(! result.contains(c)){
                    result.add(c);
                }
            }
        }
        return result;
    }

    /**
     *  Return the sub-class/interface list for the class/interface passed.
     *
     * @param cd class/interface whose sub-class/interface list is required.
     */
    public List subs(ClassDoc cd) {
        return get(cd.isInterface()? subinterfaces: subclasses, cd);
    }

    /**
     * Return a list of all direct or indirect, sub-classes and subinterfaces
     * of the ClassDoc argument.
     *
     * @param cd ClassDoc whose sub-classes or sub-interfaces are requested.
     */
    public List allSubs(ClassDoc cd) {
        List list = get(cd.isInterface()? subinterfaces: subclasses, cd);
        for (int i = 0; i < list.size(); i++) {
            cd = (ClassDoc)list.get(i);
            List tlist = get(cd.isInterface()? subinterfaces: subclasses, cd);
            for (int j = 0; j < tlist.size(); j++) {
                ClassDoc tcd = (ClassDoc)tlist.get(j);
                if (!list.contains(tcd)) {
                    list.add(tcd);
                }
            }
        }
        Collections.sort(list);
        return list;
    }

    /**
     *  Return the base-classes list. This will have only one element namely
     *  thw classdoc for java.lang.Object, since this is the base class for all
     *  classes.
     */
    public List baseclasses() {
        return baseclasses;
    }

    /**
     *  Return the list of base interfaces. This is the list of interfaces
     *  which do not have super-interface.
     */
    public List baseinterfaces() {
        return baseinterfaces;
    }
}
