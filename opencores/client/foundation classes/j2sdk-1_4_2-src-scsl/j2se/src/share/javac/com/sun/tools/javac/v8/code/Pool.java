/**
 * @(#)Pool.java	1.13 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8.code;
import com.sun.tools.javac.v8.util.*;

import com.sun.tools.javac.v8.code.Symbol.*;


/**
 * An internal structure that corresponds to the constant pool of a classfile.
 */
public class Pool {
    public static final int MAX_ENTRIES = 65535;
    public static final int MAX_STRING_LENGTH = 65535;

    /**
     * Index of next constant to be entered.
     */
    int pp;

    /**
     * The initial pool buffer.
     */
    Object[] pool;

    /**
     * A hashtable containing all constants in the pool.
     */
    Hashtable indices;

    /**
     * Construct a pool with given number of elements and element array.
     */
    public Pool(int pp, Object[] pool) {
        super();
        this.pp = pp;
        this.pool = pool;
        this.indices = new Hashtable(pool.length);
        for (int i = 1; i < pp; i++) {
            if (pool[i] != null)
                indices.put(pool[i], new Integer(i));
        }
    }

    /**
      * Construct an empty pool.
      */
    public Pool() {
        this(1, new Object[64]);
    }

    /**
      * Return the number of entries in the constant pool.
      */
    public int numEntries() {
        return pp;
    }

    /**
      * Remove everything from this pool.
      */
    public void reset() {
        pp = 1;
        indices.reset();
    }

    /**
      * Double pool buffer in size.
      */
    private void doublePool() {
        Object[] newpool = new Object[pool.length * 2];
        System.arraycopy(pool, 0, newpool, 0, pool.length);
        pool = newpool;
    }

    /**
      * Place an object in the pool, unless it is already there.
      *  If object is a symbol also enter its owner unless the owner is a
      *  package.  Return the object's index in the pool.
      */
    public int put(Object value) {
        if (value instanceof MethodSymbol)
            value = new Method((MethodSymbol) value);
        else if (value instanceof VarSymbol)
            value = new Variable((VarSymbol) value);
        Integer index = (Integer) indices.get(value);
        if (index == null) {
            index = new Integer(pp);
            indices.put(value, index);
            if (pp == pool.length)
                doublePool();
            pool[pp++] = value;
            if (value instanceof Long || value instanceof Double) {
                if (pp == pool.length)
                    doublePool();
                pool[pp++] = null;
            }
        }
        return index.intValue();
    }

    /**
      * Return the given object's index in the pool,
      *  or -1 if object is not in there.
      */
    public int get(Object o) {
        Integer n = (Integer) indices.get(o);
        return n == null ? -1 : n.intValue();
    }

    static class Method {
        MethodSymbol m;

        Method(MethodSymbol m) {
            super();
            this.m = m;
        }

        public boolean equals(Object other) {
            if (!(other instanceof Method))
                return false;
            MethodSymbol o = ((Method) other).m;
            return o.name == m.name && o.owner == m.owner && o.type.equals(m.type);
        }

        public int hashCode() {
            return m.name.hashCode() * 33 + m.owner.hashCode() * 9 +
                    m.type.hashCode();
        }
    }

    static class Variable {
        VarSymbol v;

        Variable(VarSymbol v) {
            super();
            this.v = v;
        }

        public boolean equals(Object other) {
            if (!(other instanceof Variable))
                return false;
            VarSymbol o = ((Variable) other).v;
            return o.name == v.name && o.owner == v.owner && o.type.equals(v.type);
        }

        public int hashCode() {
            return v.name.hashCode() * 33 + v.owner.hashCode() * 9 +
                    v.type.hashCode();
        }
    }
}
