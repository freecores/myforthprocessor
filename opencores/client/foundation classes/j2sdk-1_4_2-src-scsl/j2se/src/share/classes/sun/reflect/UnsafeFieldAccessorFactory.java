/*
 * @(#)UnsafeFieldAccessorFactory.java	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.reflect;

import java.lang.reflect.Field;
import java.lang.reflect.Modifier;

class UnsafeFieldAccessorFactory {
    static FieldAccessor newFieldAccessor(Field field) {
        Class type = field.getType();
        if (Modifier.isStatic(field.getModifiers())) {
            // This code path does not guarantee that the field's
            // declaring class has been initialized, but it must be
            // before performing reflective operations.
            UnsafeFieldAccessorImpl.unsafe.ensureClassInitialized(field.getDeclaringClass());

            if (type == Boolean.TYPE) {
                return new UnsafeStaticBooleanFieldAccessorImpl(field);
            } else if (type == Byte.TYPE) {
                return new UnsafeStaticByteFieldAccessorImpl(field);
            } else if (type == Short.TYPE) {
                return new UnsafeStaticShortFieldAccessorImpl(field);
            } else if (type == Character.TYPE) {
                return new UnsafeStaticCharacterFieldAccessorImpl(field);
            } else if (type == Integer.TYPE) {
                return new UnsafeStaticIntegerFieldAccessorImpl(field);
            } else if (type == Long.TYPE) {
                return new UnsafeStaticLongFieldAccessorImpl(field);
            } else if (type == Float.TYPE) {
                return new UnsafeStaticFloatFieldAccessorImpl(field);
            } else if (type == Double.TYPE) {
                return new UnsafeStaticDoubleFieldAccessorImpl(field);
            } else {
                return new UnsafeStaticObjectFieldAccessorImpl(field);
            }
        } else {
            if (type == Boolean.TYPE) {
                return new UnsafeBooleanFieldAccessorImpl(field);
            } else if (type == Byte.TYPE) {
                return new UnsafeByteFieldAccessorImpl(field);
            } else if (type == Short.TYPE) {
                return new UnsafeShortFieldAccessorImpl(field);
            } else if (type == Character.TYPE) {
                return new UnsafeCharacterFieldAccessorImpl(field);
            } else if (type == Integer.TYPE) {
                return new UnsafeIntegerFieldAccessorImpl(field);
            } else if (type == Long.TYPE) {
                return new UnsafeLongFieldAccessorImpl(field);
            } else if (type == Float.TYPE) {
                return new UnsafeFloatFieldAccessorImpl(field);
            } else if (type == Double.TYPE) {
                return new UnsafeDoubleFieldAccessorImpl(field);
            } else {
                return new UnsafeObjectFieldAccessorImpl(field);
            }
        }
    }
}
