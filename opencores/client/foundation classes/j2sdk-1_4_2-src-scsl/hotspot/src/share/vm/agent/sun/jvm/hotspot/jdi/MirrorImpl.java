/*
 * @(#)MirrorImpl.java	1.2 02/03/03
 *
 * Copyright 2003 Sun Microsystems, Inc. All Rights Reserved.
 * 
 * This software is the proprietary information of Sun Microsystems, Inc.  
 * Use is subject to license terms.
 * 
 */

package sun.jvm.hotspot.jdi;

import com.sun.jdi.*;

import sun.jvm.hotspot.oops.Klass;
import sun.jvm.hotspot.oops.Instance;

import java.util.Collection;
import java.util.Iterator;

abstract class MirrorImpl extends Object implements Mirror {


    protected VirtualMachineImpl vm;
    VirtualMachineImpl jjvm;     //fix me jjh ; javac won't allow a call to 
                                 // virtualMachine() from ThreadReferenceImpl.java 
                                 // for some reason that I am too dumb to grok.


    MirrorImpl(VirtualMachine aVm) {
        super();

        // Yes, its a bit of a hack. But by doing it this
        // way, this is the only place we have to change
        // typing to substitute a new impl.
        vm = (VirtualMachineImpl)aVm;
        jjvm = vm;
    }

    MirrorImpl(VirtualMachine aVm, Klass klass) { //fixme jjh:
        super();
        vm = (VirtualMachineImpl)aVm;
        jjvm = vm;
    }

    MirrorImpl(VirtualMachine aVm, Instance inst) { //fixme jjh:
        super();
        vm = (VirtualMachineImpl)aVm;
        jjvm = vm;
    }

    public VirtualMachine virtualMachine() {
        return vm;
    }

    public boolean equals(Object obj) {
        if ((obj != null) && (obj instanceof Mirror)) {
            Mirror other = (Mirror)obj;
            return vm.equals(other.virtualMachine());
        } else {
            return false;
        }
    }

    public int hashCode() {
        return 89;  //jjh fixme
    }
}
