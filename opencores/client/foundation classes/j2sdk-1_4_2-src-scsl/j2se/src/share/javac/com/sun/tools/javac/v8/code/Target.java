/**
 * @(#)Target.java	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javac.v8.code;
import com.sun.tools.javac.v8.util.*;


/**
 * The classfile version target.
 */
public final class Target {
    private static final Context.Key targetKey = new Context.Key();

    public static Target instance(Context context) {
        Target instance = (Target) context.get(targetKey);
        if (instance == null) {
            Options options = Options.instance(context);
            String targetString = (String) options.get("-target");
            if (targetString != null)
                instance = lookup(targetString);
            if (instance == null)
                instance = DEFAULT;
            context.put(targetKey, instance);
        }
        return instance;
    }
    private static Target MIN;

    public static Target MIN() {
        return MIN;
    }
    private static Target MAX;

    public static Target MAX() {
        return MAX;
    }
    private static int nextOrdinal = 0;
    private static final Hashtable tab = new Hashtable();
    public final int ordinal = nextOrdinal++;
    public final String name;
    public final int majorVersion;
    public final int minorVersion;

    private Target(String name, int majorVersion, int minorVersion) {
        super();
        this.name = name;
        this.majorVersion = majorVersion;
        this.minorVersion = minorVersion;
        tab.put(name, this);
        if (MIN == null)
            MIN = this;
        MAX = this;
    }
    public static final Target JDK1_1 = new Target("1.1", 45, 3);
    public static final Target JDK1_2 = new Target("1.2", 46, 0);
    public static final Target JDK1_3 = new Target("1.3", 47, 0);
    public static final Target JDK1_4 = new Target("1.4", 48, 0);
    public static final Target JDK1_4_1 = new Target("1.4.1", 48, 0);
    public static final Target JDK1_4_2 = new Target("1.4.2", 48, 0);
    public static final Target DEFAULT = JDK1_2;

    public static Target lookup(String name) {
        return (Target) tab.get(name);
    }

    /**
      * In -target 1.1 and earlier, the compiler is required to emit
      *  synthetic method definitions in abstract classes for interface
      *  methods that are not overridden.  We call them "Miranda" methods.
      */
    public boolean requiresIproxy() {
        return ordinal <= JDK1_1.ordinal;
    }

    /**
      * Begining in 1.4, we take advantage of the possibility of emitting
      *  code to initialize fields before calling the superclass constructor.
      *  This is allowed by the VM spec, but the verifier refused to allow
      *  it until 1.4.  This is necesary to translate some code involving
      *  inner classes.  See, for example, 4030374.
      */
    public boolean initializeFieldsBeforeSuper() {
        return ordinal >= JDK1_4.ordinal;
    }

    /**
      * Beginning with -target 1.2 we obey the JLS rules for binary
      *  compatibility, emitting as the qualifying type of a reference
      *  to a method or field the type of the qualifier.  In earlier
      *  targets we use as the qualifying type the class in which the
      *  member was found.  The following methods named
      *  *binaryCompatibility() indicate places where we vary from this
      *  general rule.
      */
    public boolean obeyBinaryCompatibility() {
        return ordinal >= JDK1_2.ordinal;
    }

    /**
      * In -target 1.4.1 and later, the compiler uses an array type as
      *  the qualifier for method calls (such as clone) where required by
      *  the language and VM spec.  Earlier versions of the compiler
      *  qualified them by Object.
      */
    public boolean arrayBinaryCompatibility() {
        return ordinal > JDK1_4.ordinal;
    }

    /**
      * Beginning after 1.2, we follow the binary compatibility rules for
      *  interface fields.  The 1.2 VMs had bugs handling interface fields
      *  when compiled using binary compatibility (see 4400598), so this is
      *  an accomodation to them.
      */
    public boolean interfaceFieldsBinaryCompatibility() {
        return ordinal > JDK1_2.ordinal;
    }

    /**
      * Beginning after -target 1.4, we follow the binary compatibility
      *  rules for interface methods that redefine Object methods.
      *  Earlier VMs had bugs handling such methods compiled using binary
      *  compatibility (see 4392595, 4398791, 4392595, 4400415).
      *  The VMs were fixed during or soon after 1.4.
      */
    public boolean interfaceObjectOverridesBinaryCompatibility() {
        return ordinal > JDK1_4.ordinal;
    }
}
