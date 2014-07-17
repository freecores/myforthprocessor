/*
 * @(#)ClassTypeImpl.java	1.2 02/03/03
 * Copyright 2003 Sun Microsystems, Inc. All Rights Reserved.
 * 
 * This software is the proprietary information of Sun Microsystems, Inc.  
 * Use is subject to license terms.
 * 
 */

package sun.jvm.hotspot.jdi;

import com.sun.jdi.*;
import sun.jvm.hotspot.oops.InstanceKlass;

import java.util.*;

public class ClassTypeImpl extends ReferenceTypeImpl
    implements ClassType
{
    private boolean cachedSuperclass = false;
    private ClassType superclass = null;
    private Map lineMapper = null;
    private int lastLine = -1;
    private List interfaces = null;

    protected ClassTypeImpl(VirtualMachine aVm, InstanceKlass aRef) {
        super(aVm, aRef);
    }

    public ClassType superclass() {
        InstanceKlass kk = (InstanceKlass)((InstanceKlass)saObject).getSuper();
        if (kk == null) {
            return null;
        }
        return new ClassTypeImpl(vm, kk);
    }

    public List interfaces()  {
        if (interfaces == null) {
            interfaces = getInterfaces();
        }
        return interfaces;
    }

    void addInterfaces(List list) {
        List immediate = interfaces();
        list.addAll(interfaces());

        Iterator iter = immediate.iterator();
        while (iter.hasNext()) {
            InterfaceTypeImpl interfaze = (InterfaceTypeImpl)iter.next();
            interfaze.addSuperinterfaces(list);
        }

        ClassTypeImpl superclass = (ClassTypeImpl)superclass();
        if (superclass != null) {
            superclass.addInterfaces(list);
        }
    }

    public List allInterfaces()  {
        List all = new ArrayList();
        addInterfaces(all);
        return all;
    }

    public List subclasses() {
        List all = vm.allClasses();
        List subs = new ArrayList();
        Iterator iter = all.iterator();
        while (iter.hasNext()) {
            ReferenceType refType = (ReferenceType)iter.next();
            if (refType instanceof ClassType) {
                ClassType clazz = (ClassType)refType;
                ClassType superclass = clazz.superclass();
                if ((superclass != null) && superclass.equals(this)) {
                    subs.add(refType);
                }
            }
        }
        return subs;
    }

    public Method concreteMethodByName(String name, String signature)  {
       List methods = visibleMethods();
       Method method = null;
       Iterator iter = methods.iterator();
       while (iter.hasNext()) {
           Method candidate = (Method)iter.next();
           if (candidate.name().equals(name) &&
               candidate.signature().equals(signature) &&
               !candidate.isAbstract()) {

               method = candidate;
               break;
           }
       }
       return method;
   }

   public List allMethods() {
        ArrayList list = new ArrayList(methods());

        ClassType clazz = superclass();
        while (clazz != null) {
            list.addAll(clazz.methods());
            clazz = clazz.superclass();
        }

        /*
         * Avoid duplicate checking on each method by iterating through 
         * duplicate-free allInterfaces() rather than recursing
         */
        Iterator iter = allInterfaces().iterator();
        while (iter.hasNext()) {
            InterfaceType interfaze = (InterfaceType)iter.next();
            list.addAll(interfaze.methods());
        } 

        return list;
    }

    List inheritedTypes() {
        List inherited = new ArrayList(interfaces());
        if (superclass() != null) {
            inherited.add(0, superclass()); /* insert at front */
        }
        return inherited;
    }

    public void setValue(Field field, Value value) 
        throws InvalidTypeException, ClassNotLoadedException {
        throw new UnsupportedOperationException("Not Read Only: ClassType.setValue(...)");
    }


    public Value invokeMethod(ThreadReference threadIntf, Method methodIntf, 
                              List arguments, int options) 
                                   throws InvalidTypeException,
                                          ClassNotLoadedException,
                                          IncompatibleThreadStateException,
                                          InvocationException {
        throw new UnsupportedOperationException("Not Read Only: ClassType.invokeMethod(...)");
    }

    public ObjectReference newInstance(ThreadReference threadIntf, 
                                       Method methodIntf, 
                                       List arguments, int options) 
                                   throws InvalidTypeException,
                                          ClassNotLoadedException,
                                          IncompatibleThreadStateException,
                                          InvocationException {
        throw new UnsupportedOperationException("Not Read Only: ClassType.newInstance(...)");
    }

    void addVisibleMethods(Map methodMap) {
        /*
         * Add methods from 
         * parent types first, so that the methods in this class will
         * overwrite them in the hash table
         */

        //System.out.println("jj: CTI: aVM for : " + this);
        Iterator iter = interfaces().iterator();
        while (iter.hasNext()) {
            InterfaceTypeImpl interfaze = (InterfaceTypeImpl)iter.next();
            interfaze.addVisibleMethods(methodMap);
        }

        ClassTypeImpl clazz = (ClassTypeImpl)superclass();
        if (clazz != null) {
            clazz.addVisibleMethods(methodMap);
        }

        addToMethodMap(methodMap, methods());
    }

    boolean isAssignableTo(ReferenceType type) {
        ClassTypeImpl superclazz = (ClassTypeImpl)superclass();
        if (this.equals(type)) {
            return true;
        } else if ((superclazz != null) && superclazz.isAssignableTo(type)) {
            return true;
        } else {
            List interfaces = interfaces();
            Iterator iter = interfaces.iterator();
            while (iter.hasNext()) {
                InterfaceTypeImpl interfaze = (InterfaceTypeImpl)iter.next();
                if (interfaze.isAssignableTo(type)) {
                    return true;
                }
            }
            return false;
        }
    }

    public String toString() {
       return "class " + name() + " (" + loaderString() + ")";
    }
}
