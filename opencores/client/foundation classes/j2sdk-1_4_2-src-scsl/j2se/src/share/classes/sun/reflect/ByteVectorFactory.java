/*
 * @(#)ByteVectorFactory.java	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.reflect;

class ByteVectorFactory {
    static ByteVector create() {
        return new ByteVectorImpl();
    }

    static ByteVector create(int sz) {
        return new ByteVectorImpl(sz);
    }
}
