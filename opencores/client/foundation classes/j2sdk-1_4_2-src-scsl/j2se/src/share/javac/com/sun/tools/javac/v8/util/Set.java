/**
 * @(#)Set.java	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8.util;

/**
 * A generic class for sets, modelled after Hashtable.
 */
public class Set {

    private static class Entry {
        Object key;
        int hash;
        Entry next;

        Entry(Object key, int hash, Entry next) {
            super();
            this.key = key;
            this.hash = hash;
            this.next = next;
        }
    }
    private int hashSize;
    private int hashMask;
    private int limit;
    private int size;
    private Entry[] table;

    public Set(int initialSize, float fillFactor) {
        super();
        int hashSize = 1;
        while (hashSize < initialSize)
            hashSize = hashSize << 1;
        this.hashSize = hashSize;
        this.hashMask = hashSize - 1;
        this.limit = (int)(hashSize * fillFactor);
        this.size = 0;
        table = new Entry[hashSize];
    }

    public Set(int initialSize) {
        this(initialSize, 0.75F);
    }

    public Set() {
        this(32);
    }

    public static Set make() {
        return new Set();
    }

    private void dble() {
        hashSize = hashSize << 1;
        hashMask = hashSize - 1;
        limit = limit << 1;
        Entry[] oldtable = table;
        table = new Entry[hashSize];
        for (int i = 0; i < oldtable.length; i++) {
            for (Entry e = oldtable[i], next = null; e != null; e = next) {
                int ix = e.hash & hashMask;
                next = e.next;
                e.next = table[ix];
                table[ix] = e;
            }
        }
    }

    public boolean contains(Object key) {
        int hash = ((Object) key).hashCode();
        for (Entry e = table[hash & hashMask]; e != null; e = e.next) {
            if ((e.hash == hash) && ((Object) e.key).equals((Object) key)) {
                return true;
            }
        }
        return false;
    }

    public boolean put(Object key) {
        int hash = ((Object) key).hashCode();
        for (Entry e = table[hash & hashMask]; e != null; e = e.next) {
            if ((e.hash == hash) && ((Object) e.key).equals((Object) key)) {
                return true;
            }
        }
        size++;
        if (size > limit)
            dble();
        int index = hash & hashMask;
        Entry e = new Entry(key, hash, table[index]);
        table[index] = e;
        return false;
    }

    public void reset() {
        for (int i = 0; i < table.length; i++)
            table[i] = null;
        size = 0;
    }
}
