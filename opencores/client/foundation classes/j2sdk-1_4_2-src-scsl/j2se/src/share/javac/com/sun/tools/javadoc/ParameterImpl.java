/**
 * @(#)ParameterImpl.java	1.19 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javadoc;
import com.sun.javadoc.*;

import com.sun.tools.javac.v8.code.Type;


/**
 * ParameterImpl information.
 * This includes a parameter type and parameter name.
 *
 * @author Kaiyang Liu (original)
 * @author Robert Field (rewrite)
 */
class ParameterImpl implements Parameter {

    /**
     * Parameter type information.
     */
    private final com.sun.javadoc.Type type;

    /**
     * Parameter local name.
     */
    private final String name;

    /**
     * Constructor of paramter info class.
     */
    ParameterImpl(DocEnv env, Type type, String name) {
        super();
        this.type = TypeMaker.getType(env, type);
        this.name = name;
    }

    /**
      * Get the type of this parameter.
      */
    public com.sun.javadoc.Type type() {
        return type;
    }

    /**
      * Get local name of this parameter.
      * For example if parameter is the short 'index', returns "index".
      */
    public String name() {
        return name;
    }

    /**
      * Get type name of this parameter.
      * For example if parameter is the short 'index', returns "short".
      */
    public String typeName() {
        return type.toString();
    }

    /**
      * Returns a string representation of the class.
      * <p>
      * For example if parameter is the short 'index', returns "short index".
      *
      * @return type name and parameter name of this parameter.
      */
    public String toString() {
        return type.toString() + " " + name;
    }
}
