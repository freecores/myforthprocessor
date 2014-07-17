/*
 * @(#)VMModifiers.java	1.1 02/01/07
 *
 * Copyright 2003 Sun Microsystems, Inc. All Rights Reserved.
 * 
 * This software is the proprietary information of Sun Microsystems, Inc.  
 * Use is subject to license terms.
 * 
 */

package sun.jvm.hotspot.jdi;

import com.sun.jdi.*;

public interface VMModifiers
{
    int PUBLIC = 0x00000001;       /* visible to everyone */
    int PRIVATE = 0x00000002;      /* visible only to the defining class */
    int PROTECTED = 0x00000004;    /* visible to subclasses */
    int STATIC = 0x00000008;       /* instance variable is static */
    int FINAL = 0x00000010;        /* no further subclassing, overriding */
    int SYNCHRONIZED = 0x00000020; /* wrap method call in monitor lock */
    int VOLATILE = 0x00000040;     /* can cache in registers */
    int TRANSIENT = 0x00000080;    /* not persistant */
    int NATIVE = 0x00000100;       /* implemented in C */
    int INTERFACE = 0x00000200;    /* class is an interface */
    int ABSTRACT = 0x00000400;     /* no definition provided */
    int SYNTHETIC = 0xf0000000;    /* not in source code */
}
