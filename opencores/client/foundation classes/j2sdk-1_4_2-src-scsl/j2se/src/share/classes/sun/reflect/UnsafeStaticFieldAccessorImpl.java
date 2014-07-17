/*
 * @(#)UnsafeStaticFieldAccessorImpl.java	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.reflect;

import java.lang.reflect.Field;
import java.lang.reflect.Modifier;
import java.security.AccessController;
import sun.misc.Unsafe;

/** Base class for sun.misc.Unsafe-based FieldAccessors for static
    fields. The observation is that there are only nine types of
    fields from the standpoint of reflection code: the eight primitive
    types and Object. Using class Unsafe instead of generated
    bytecodes saves memory and loading time for the
    dynamically-generated FieldAccessors. */

abstract class UnsafeStaticFieldAccessorImpl extends UnsafeFieldAccessorImpl {
    protected Object base; // base 

    UnsafeStaticFieldAccessorImpl(Field field) {
        super(field);
        base = unsafe.staticFieldBase(field);
    }
}
