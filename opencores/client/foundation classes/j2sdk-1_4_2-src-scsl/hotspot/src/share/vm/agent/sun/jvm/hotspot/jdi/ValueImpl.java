/*
 * @(#)ValueImpl.java	1.2 02/03/03
 *
 * Copyright 2003 Sun Microsystems, Inc. All Rights Reserved.
 * 
 * This software is the proprietary information of Sun Microsystems, Inc.  
 * Use is subject to license terms.
 * 
 */

package sun.jvm.hotspot.jdi;

import com.sun.jdi.*;

abstract class ValueImpl extends MirrorImpl {
    // implements Value //fixme jjh:

    ValueImpl(VirtualMachine aVm) {
        super(aVm);
    }

    // type() is in the subclasses

}


