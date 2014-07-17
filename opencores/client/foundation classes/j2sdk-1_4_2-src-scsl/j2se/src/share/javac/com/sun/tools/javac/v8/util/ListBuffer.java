/**
 * @(#)ListBuffer.java	1.15 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8.util;

/**
 * A class for constructing lists by appending elements. Modelled after
 *  java.lang.StringBuffer.
 */
public class ListBuffer {

    /**
     * The list of elements of this buffer.
     */
    public List elems;

    /**
     * A pointer pointing to the last, sentinel element of `elems'.
     */
    public List last;

    /**
     * The number of element in this buffer.
     */
    public int count;

    /**
     * Has a list been created from this buffer yet?
     */
    public boolean shared;

    /**
     * Create a new initially empty list buffer.
     */
    public ListBuffer() {
        super();
        this.elems = new List();
        this.last = this.elems;
        count = 0;
        shared = false;
    }

    /**
      * Return the number of elements in this buffer.
      */
    public int length() {
        return count;
    }

    /**
      * Is buffer empty?
      */
    public boolean isEmpty() {
        return count == 0;
    }

    /**
      * Is buffer not empty?
      */
    public boolean nonEmpty() {
        return count != 0;
    }

    /**
      * Copy list and sets last.
      */
    private void copy() {
        List p = elems = new List(elems.head, elems.tail);
        while (true) {
            List tail = p.tail;
            if (tail == null)
                break;
            p = p.tail = new List(tail.head, tail.tail);
        }
        last = p;
        shared = false;
    }

    /**
      * Prepend an element to buffer.
      */
    public ListBuffer prepend(Object x) {
        elems = elems.prepend(x);
        count++;
        return this;
    }

    /**
      * Append an element to buffer.
      */
    public ListBuffer append(Object x) {
        if (shared)
            copy();
        last.head = x;
        last.tail = new List();
        last = last.tail;
        count++;
        return this;
    }

    /**
      * Append all elements in a list to buffer.
      */
    public ListBuffer appendList(List xs) {
        while (xs.nonEmpty()) {
            append(xs.head);
            xs = xs.tail;
        }
        return this;
    }

    /**
      * Append all elements in an array to buffer.
      */
    public ListBuffer appendArray(Object[] xs) {
        for (int i = 0; i < xs.length; i++) {
            append(xs[i]);
        }
        return this;
    }

    /**
      * Convert buffer to a list of all its elements.
      */
    public List toList() {
        shared = true;
        return elems;
    }

    /**
      * Does the list contain the specified element?
      */
    public boolean contains(Object x) {
        return elems.contains(x);
    }

    /**
      * Convert buffer to an array
      */
    public Object[] toArray(Object[] vec) {
        return elems.toArray(vec);
    }

    /**
      * The first element in this buffer.
      */
    public Object first() {
        return elems.head;
    }

    /**
      * Remove the first element in this buffer.
      */
    public void remove() {
        if (elems != last) {
            elems = elems.tail;
            count--;
        }
    }

    /**
      * Return first element in this buffer and remove
      */
    public Object next() {
        Object x = elems.head;
        remove();
        return x;
    }

    /**
      * An enumeration of all elements in this buffer.
      */
    public Enumeration elements() {
        return new Enumerator(elems, last);
    }

    static class Enumerator implements Enumeration {
        List elems;
        List last;

        Enumerator(List elems, List last) {
            super();
            this.elems = elems;
            this.last = last;
        }

        public boolean hasMoreElements() {
            return elems != last;
        }

        public Object nextElement() {
            Object elem = elems.head;
            elems = elems.tail;
            return elem;
        }
    }
}
