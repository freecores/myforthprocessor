/**
 * @(#)List.java	1.17 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8.util;

/**
 * A class for generic linked lists. Links are supposed to be
 *  immutable, the only exception being the incremental construction of
 *  lists via ListBuffers.  List is the main container class in
 *  GJC. Most data structures and algorthms in GJC use lists rather
 *  than arrays.
 *
 *  Lists are always trailed by a sentinel element, whose head and tail
 *  are both null.
 */
public final class List {

    /**
     * The first element of the list, supposed to be immutable.
     */
    public Object head;

    /**
     * The remainder of the list except for its first element, supposed
     *  to be immutable.
     */
    public List tail;

    /**
     * Construct a list given its head and tail.
     */
    public List(Object head, List tail) {
        super();
        this.tail = tail;
        this.head = head;
    }

    /**
      * Construct an empty list.
      */
    public List() {
        this(null, null);
    }

    /**
      * Construct an empty list.
      */
    public static List make() {
        return new List();
    }

    /**
      * Construct a list consisting of given element.
      */
    public static List make(Object x1) {
        return new List(x1, new List());
    }

    /**
      * Construct a list consisting of given elements.
      */
    public static List make(Object x1, Object x2) {
        return new List(x1, new List(x2, new List()));
    }

    /**
      * Construct a list consisting of given elements.
      */
    public static List make(Object x1, Object x2, Object x3) {
        return new List(x1, new List(x2, new List(x3, new List())));
    }

    /**
      * Construct a list consisting all elements of given array.
      */
    public static List make(Object[] vec) {
        List xs = new List();
        for (int i = vec.length - 1; i >= 0; i--)
            xs = new List(vec[i], xs);
        return xs;
    }

    /**
      * Construct a list consisting of a given number of identical elements.
      *  @param len    The number of elements in the list.
      *  @param init   The value of each element.
      */
    public static List make(int len, Object init) {
        List l = new List();
        for (int i = 0; i < len; i++)
            l = new List(init, l);
        return l;
    }

    /**
      * Does list have no elements?
      */
    public boolean isEmpty() {
        return tail == null;
    }

    /**
      * Does list have elements?
      */
    public boolean nonEmpty() {
        return tail != null;
    }

    /**
      * Return the number of elements in this list.
      */
    public int length() {
        List l = this;
        int len = 0;
        while (l.tail != null) {
            l = l.tail;
            len++;
        }
        return len;
    }

    /**
      * Prepend given element to front of list, forming and returning
      *  a new list.
      */
    public List prepend(Object x) {
        return new List(x, this);
    }

    /**
      * Prepend given list of elements to front of list, forming and returning
      *  a new list.
      */
    public List prependList(List xs) {
        if (this.isEmpty())
            return xs;
        if (xs.isEmpty())
            return this;
        List result = this;
        xs = xs.reverse();
        while (xs.nonEmpty()) {
            List h = xs;
            xs = xs.tail;
            h.tail = result;
            result = h;
        }
        return result;
    }

    /**
      * Reverse list, forming and returning a new list.
      */
    public List reverse() {
        List rev = new List();
        for (List l = this; l.nonEmpty(); l = l.tail)
            rev = new List(l.head, rev);
        return rev;
    }

    /**
      * Append given element at length, forming and returning
      *  a new list.
      */
    public List append(Object x) {
        return make(x).prependList(this);
    }

    /**
      * Append given list at length, forming and returning
      *  a new list.
      */
    public List appendList(List x) {
        return x.prependList(this);
    }

    /**
      * Copy successive elements of this list into given vector until
      *  list is exhausted or end of vector is reached.
      */
    public Object[] toArray(Object[] vec) {
        int i = 0;
        List l = this;
        while (l.nonEmpty() && i < vec.length) {
            vec[i] = l.head;
            l = l.tail;
            i++;
        }
        return vec;
    }

    /**
      * Form a string listing all elements with given separator character.
      */
    public String toString(String sep) {
        if (isEmpty()) {
            return "";
        } else {
            StringBuffer buf = new StringBuffer();
            buf.append(((Object) head).toString());
            for (List l = tail; l.nonEmpty(); l = l.tail) {
                buf.append(sep);
                buf.append(((Object) l.head).toString());
            }
            return buf.toString();
        }
    }

    /**
      * Form a string listing all elements with comma as the separator character.
      */
    public String toString() {
        return toString(",");
    }

    /**
      * Compute a hash code, overrides Object
      */
    public int hashCode() {
        List l = this;
        int h = 0;
        while (l.tail != null) {
            h = h * 41 + (head != null ? head.hashCode() : 0);
            l = l.tail;
        }
        return h;
    }

    /**
      * Is this list the same as other list?
      */
    public boolean equals(Object other) {
        return other instanceof List && equals(this, (List) other);
    }

    /**
      * Are the two lists the same?
      */
    public static boolean equals(List xs, List ys) {
        while (xs.tail != null && ys.tail != null) {
            if (xs.head == null) {
                if (ys.head != null)
                    return false;
            } else {
                if (!xs.head.equals(ys.head))
                    return false;
            }
            xs = xs.tail;
            ys = ys.tail;
        }
        return xs.tail == null && ys.tail == null;
    }

    /**
      * Does the list contain the specified element?
      */
    public boolean contains(Object x) {
        List l = this;
        while (l.tail != null) {
            if (x == null) {
                if (l.head == null)
                    return true;
            } else {
                if (x.equals(l.head))
                    return true;
            }
            l = l.tail;
        }
        return false;
    }

    /**
      * The last element in the list, if any, or null.
      */
    public Object last() {
        Object last = null;
        List t = this;
        while (t.tail != null) {
            last = t.head;
            t = t.tail;
        }
        return last;
    }
}
