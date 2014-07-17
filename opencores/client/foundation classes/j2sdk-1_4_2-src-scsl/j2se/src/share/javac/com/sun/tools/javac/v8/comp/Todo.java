/**
 * @(#)Todo.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8.comp;
import com.sun.tools.javac.v8.util.*;


/**
 * A queue of all as yet unattributed classes.
 */
public class Todo extends ListBuffer {

    /**
     * The context key for the todo list.
     */
    private static final Context.Key todoKey = new Context.Key();

    /**
     * Get the Todo instance for this context.
     */
    public static Todo instance(Context context) {
        Todo instance = (Todo) context.get(todoKey);
        if (instance == null)
            instance = new Todo(context);
        return instance;
    }

    /**
      * Create a new todo list.
      */
    private Todo(Context context) {
        super();
        context.put(todoKey, this);
    }

    /*synthetic*/ public Object next() {
        return super.next();
    }

    /*synthetic*/ public Object first() {
        return super.first();
    }

    /*synthetic*/ public Object[] toArray(Object[] x0) {
        return super.toArray((Env[]) x0);
    }

    /*synthetic*/ public boolean contains(Object x0) {
        return super.contains((Env) x0);
    }

    /*synthetic*/ public ListBuffer appendArray(Object[] x0) {
        return super.appendArray((Env[]) x0);
    }

    /*synthetic*/ public ListBuffer append(Object x0) {
        return super.append((Env) x0);
    }

    /*synthetic*/ public ListBuffer prepend(Object x0) {
        return super.prepend((Env) x0);
    }
}
