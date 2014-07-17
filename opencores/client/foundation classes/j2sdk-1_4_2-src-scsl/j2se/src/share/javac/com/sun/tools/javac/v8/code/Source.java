/**
 * @(#)Source.java	1.2 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All Rights Reserved.
 *
 * This software is the proprietary information of Sun Microsystems, Inc.
 * Use is subject to license terms.
 *
 */
package com.sun.tools.javac.v8.code;
import com.sun.tools.javac.v8.util.*;


/**
 * The source language version accepted.
 */
public class Source {
    private static final Context.Key sourceKey = new Context.Key();

    public static Source instance(Context context) {
        Source instance = (Source) context.get(sourceKey);
        if (instance == null) {
            Options options = Options.instance(context);
            String sourceString = (String) options.get("-source");
            if (sourceString != null)
                instance = lookup(sourceString);
            if (instance == null)
                instance = DEFAULT;
            context.put(sourceKey, instance);
        }
        return instance;
    }
    private static int nextOrdinal = 0;
    private static final Hashtable tab = new Hashtable();
    public final int ordinal = nextOrdinal++;
    public final String name;

    private Source(String name) {
        super();
        this.name = name;
        tab.put(name, this);
    }

    /**
      * 1.2 introduced strictfp.
      */
    public static final Source JDK1_2 = new Source("1.2");

    /**
     * 1.3 is the same language as 1.2.
     */
    public static final Source JDK1_3 = new Source("1.3");

    /**
     * 1.4 introduced assert.
     */
    public static final Source JDK1_4 = new Source("1.4");
    public static final Source DEFAULT = JDK1_3;

    public static Source lookup(String name) {
        return (Source) tab.get(name);
    }
}
