/*
 * @(#)ValueContainer.java	1.1 02/03/04
 * Copyright 2003 Sun Microsystems, Inc. All Rights Reserved.
 * 
 * This software is the proprietary information of Sun Microsystems, Inc.  
 * Use is subject to license terms.
 * 
 */

package sun.jvm.hotspot.jdi;

import com.sun.jdi.*;

/*
 * This interface allows us to pass fields, variables, and 
 * array components through the same interfaces. This currently allows
 * more common code for type checking. In the future we could use it for
 * more. 
 */
interface ValueContainer {
    Type type() throws ClassNotLoadedException;
    Type findType(String signature) throws ClassNotLoadedException;
    String typeName();
    String signature();
}


