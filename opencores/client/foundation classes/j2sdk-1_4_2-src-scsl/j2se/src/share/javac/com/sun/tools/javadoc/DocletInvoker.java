/**
 * @(#)DocletInvoker.java	1.17 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javadoc;
import com.sun.javadoc.*;

import com.sun.tools.javac.v8.util.List;

import java.net.*;

import java.lang.OutOfMemoryError;

import java.lang.reflect.Method;

import java.lang.reflect.Modifier;

import java.lang.reflect.InvocationTargetException;

import java.io.File;

import java.io.IOException;

import java.util.StringTokenizer;


/**
 * Class creates, controls and invokes doclets.
 * @author Neal Gafter (rewrite)
 */
public class DocletInvoker {
    private final Class docletClass;
    private final String docletClassName;
    private final ClassLoader appClassLoader;
    private final Messager messager;

    private static class DocletInvokeException extends Exception {

        private DocletInvokeException() {
            super();
        }
    }

    private String appendPath(String path1, String path2) {
        if (path1 == null || path1.length() == 0) {
            return path2 == null ? "." : path2;
        } else if (path2 == null || path2.length() == 0) {
            return path1;
        } else {
            return path1 + File.pathSeparator + path2;
        }
    }

    public DocletInvoker(Messager messager, String docletClassName,
            String docletPath) {
        super();
        this.messager = messager;
        this.docletClassName = docletClassName;
        String cpString = null;
        cpString = appendPath(System.getProperty("env.class.path"), cpString);
        cpString = appendPath(System.getProperty("java.class.path"), cpString);
        cpString = appendPath(docletPath, cpString);
        URL[] urls = pathToURLs(cpString);
        appClassLoader = new URLClassLoader(urls);
        Class dc = null;
        try {
            dc = appClassLoader.loadClass(docletClassName);
        } catch (ClassNotFoundException exc) {
            messager.error(null, "main.doclet_class_not_found", docletClassName);
            messager.exit();
        }
        docletClass = dc;
    }

    /**
      * Generate documentation here.  Return true on success.
      */
    public boolean start(RootDoc root) {
        Object retVal;
        String methodName = "start";
        Class[] paramTypes = new Class[1];
        Object[] params = new Object[1];
        try {
            paramTypes[0] = Class.forName("com.sun.javadoc.RootDoc");
        } catch (ClassNotFoundException exc) {
            return false;
        }
        params[0] = root;
        try {
            retVal = invoke(methodName, null, paramTypes, params);
        } catch (DocletInvokeException exc) {
            return false;
        }
        if (retVal instanceof Boolean) {
            return ((Boolean) retVal).booleanValue();
        } else {
            messager.error(null, "main.must_return_boolean", docletClassName,
                    methodName);
            return false;
        }
    }

    /**
      * Check for doclet added options here. Zero return means
      * option not known.  Positive value indicates number of
      * arguments to option.  Negative value means error occurred.
      */
    public int optionLength(String option) {
        Object retVal;
        String methodName = "optionLength";
        Class[] paramTypes = new Class[1];
        Object[] params = new Object[1];
        paramTypes[0] = option.getClass();
        params[0] = option;
        try {
            retVal = invoke(methodName, new Integer(0), paramTypes, params);
        } catch (DocletInvokeException exc) {
            return -1;
        }
        if (retVal instanceof Integer) {
            return ((Integer) retVal).intValue();
        } else {
            messager.error(null, "main.must_return_int", docletClassName, methodName);
            return -1;
        }
    }

    /**
      * Let doclet check that all options are OK. Returning true means
      * options are OK.  If method does not exist, assume true.
      */
    public boolean validOptions(List optlist) {
        Object retVal;
        String[][] options =
                (String[][]) optlist.toArray(new String[optlist.length()][]);
        String methodName = "validOptions";
        DocErrorReporter reporter = messager;
        Class[] paramTypes = new Class[2];
        Object[] params = new Object[2];
        paramTypes[0] = options.getClass();
        try {
            paramTypes[1] = Class.forName("com.sun.javadoc.DocErrorReporter");
        } catch (ClassNotFoundException exc) {
            return false;
        }
        params[0] = options;
        params[1] = reporter;
        try {
            retVal = invoke(methodName, Boolean.TRUE, paramTypes, params);
        } catch (DocletInvokeException exc) {
            return false;
        }
        if (retVal instanceof Boolean) {
            return ((Boolean) retVal).booleanValue();
        } else {
            messager.error(null, "main.must_return_boolean", docletClassName,
                    methodName);
            return false;
        }
    }

    /**
      * Utility method for calling doclet functionality
      */
    private Object invoke(String methodName, Object returnValueIfNonExistent,
            Class[] paramTypes, Object[] params) throws DocletInvokeException {
        Method meth;
        try {
            meth = docletClass.getMethod(methodName, paramTypes);
        } catch (NoSuchMethodException exc) {
            if (returnValueIfNonExistent == null) {
                messager.error(null, "main.doclet_method_not_found",
                        docletClassName, methodName);
                throw new DocletInvokeException();
            } else {
                return returnValueIfNonExistent;
            }
        }
        catch (SecurityException exc) {
            messager.error(null, "main.doclet_method_not_accessible",
                    docletClassName, methodName);
            throw new DocletInvokeException();
        }
        if (!Modifier.isStatic(meth.getModifiers())) {
            messager.error(null, "main.doclet_method_must_be_static",
                    docletClassName, methodName);
            throw new DocletInvokeException();
        }
        try {
            Thread.currentThread().setContextClassLoader(appClassLoader);
            return meth.invoke(null, params);
        } catch (IllegalArgumentException exc) {
            messager.error(null, "main.internal_error_exception_thrown",
                    docletClassName, methodName, exc.toString());
            throw new DocletInvokeException();
        }
        catch (IllegalAccessException exc) {
            messager.error(null, "main.doclet_method_not_accessible",
                    docletClassName, methodName);
            throw new DocletInvokeException();
        }
        catch (NullPointerException exc) {
            messager.error(null, "main.internal_error_exception_thrown",
                    docletClassName, methodName, exc.toString());
            throw new DocletInvokeException();
        }
        catch (InvocationTargetException exc) {
            Throwable err = exc.getTargetException();
            if (err instanceof java.lang.OutOfMemoryError) {
                messager.error(null, "main.out.of.memory");
            } else {
                messager.error(null, "main.exception_thrown", docletClassName,
                        methodName, exc.toString());
                exc.getTargetException().printStackTrace();
            }
            throw new DocletInvokeException();
        }
    }

    /**
      * Utility method for converting a search path string to an array
      * of directory and JAR file URLs.
      *
      * @param path the search path string
      * @return the resulting array of directory and JAR file URLs
      */
    static URL[] pathToURLs(String path) {
        StringTokenizer st = new StringTokenizer(path, File.pathSeparator);
        URL[] urls = new URL[st.countTokens()];
        int count = 0;
        while (st.hasMoreTokens()) {
            URL url = fileToURL(new File(st.nextToken()));
            if (url != null) {
                urls[count++] = url;
            }
        }
        if (urls.length != count) {
            URL[] tmp = new URL[count];
            System.arraycopy(urls, 0, tmp, 0, count);
            urls = tmp;
        }
        return urls;
    }

    /**
      * Returns the directory or JAR file URL corresponding to the specified
      * local file name.
      *
      * @param file the File object
      * @return the resulting directory or JAR file URL, or null if unknown
      */
    static URL fileToURL(File file) {
        String name;
        try {
            name = file.getCanonicalPath();
        } catch (IOException e) {
            name = file.getAbsolutePath();
        }
        name = name.replace(File.separatorChar, '/');
        if (!name.startsWith("/")) {
            name = "/" + name;
        }
        if (!file.isFile()) {
            name = name + "/";
        }
        try {
            return new URL("file", "", name);
        } catch (MalformedURLException e) {
            throw new IllegalArgumentException("file");
        }
    }
}
