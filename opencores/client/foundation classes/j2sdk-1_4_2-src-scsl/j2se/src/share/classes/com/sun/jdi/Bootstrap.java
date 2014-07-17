/*
 * @(#)Bootstrap.java	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jdi;

/**
 * Initial class that provides access to the default implementation 
 * of JDI interfaces. A debugger application uses this class to access the 
 * single instance of the {@link VirtualMachineManager} interface.
 *
 * @author Gordon Hirsch
 * @since  1.3
 */
public class Bootstrap extends Object {

    static private VirtualMachineManager virtualMachineManager = null;

    static public synchronized VirtualMachineManager virtualMachineManager() {
        if (virtualMachineManager == null) {
            virtualMachineManager = new com.sun.tools.jdi.VirtualMachineManagerImpl();
        }
        return virtualMachineManager;
    }
}
