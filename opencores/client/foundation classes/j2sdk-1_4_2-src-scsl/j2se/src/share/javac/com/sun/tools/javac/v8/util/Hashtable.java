/**
 * @(#)Hashtable.java	1.13 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8.util;

/**
 * A generic simplified version of java.util.Hashtable.
 */
public class Hashtable {

    private static class Entry {
        Object key;
        Object value;
        int hash;
        Entry next;

        Entry(Object key, Object value, int hash, Entry next) {
            super();
            this.key = key;
            this.value = value;
            this.hash = hash;
            this.next = next;
        }
    }
    private int hashSize;
    private int hashMask;
    private int limit;
    private int size;
    private Entry[] table;

    public Hashtable(int initialSize, float fillFactor) {
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

    public Hashtable(int initialSize) {
        this(initialSize, 0.75F);
    }

    public Hashtable() {
        this(32);
    }

    public static Hashtable make() {
        return new Hashtable();
    }

    private void dble() {
        hashSize = hashSize << 1;
        hashMask = hashSize - 1;
        limit = limit << 1;
        Entry[] oldtable = table;
        table = new Entry[hashSize];
        for (int i = 0; i < oldtable.length; i++) {
            for (Entry e = oldtable[i], next = null; e != null; e = next) {
                next = e.next;
                int ix = e.hash & hashMask;
                e.next = table[ix];
                table[ix] = e;
            }
        }
    }

    public Object get(Object key) {
        int hash = ((Object) key).hashCode();
        for (Entry e = table[hash & hashMask]; e != null; e = e.next) {
            if ((e.hash == hash) && ((Object) e.key).equals((Object) key)) {
                return e.value;
            }
        }
        return null;
    }

    public Object put(Object key, Object value) {
        int hash = ((Object) key).hashCode();
        for (Entry e = table[hash & hashMask]; e != null; e = e.next) {
            if ((e.hash == hash) && ((Object) e.key).equals((Object) key)) {
                Object oldvalue = e.value;
                e.value = value;
                return oldvalue;
            }
        }
        size++;
        if (size > limit)
            dble();
        int index = hash & hashMask;
        Entry e = new Entry(key, value, hash, table[index]);
        table[index] = e;
        return null;
    }

    public Object remove(Object key) {
        int hash = ((Object) key).hashCode();
        Entry prev = null;
        for (Entry e = table[hash & hashMask]; e != null; e = e.next) {
            if ((e.hash == hash) && ((Object) e.key).equals((Object) key)) {
                if (prev != null)
                    prev.next = e.next;
                else
                    table[hash & hashMask] = e.next;
                size--;
                return e.value;
            }
            prev = e;
        }
        return null;
    }

    public List keys() {
        ListBuffer result = new ListBuffer();
        for (int i = 0; i < table.length; i++) {
            for (Entry e = table[i]; e != null; e = e.next) {
                result.append(e.key);
            }
        }
        return result.toList();
    }

    public int size() {
        return size;
    }

    public void reset() {
        for (int i = 0; i < table.length; i++)
            table[i] = null;
        size = 0;
    }
}
