/**
 * @(#)Messager.java	1.26 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.javadoc;
import java.io.PrintWriter;

import java.text.MessageFormat;

import java.util.ResourceBundle;

import java.util.MissingResourceException;

import com.sun.javadoc.*;

import com.sun.tools.javac.v8.util.Context;

import com.sun.tools.javac.v8.util.Log;


/**
 * Utility for integrating with javadoc tools and for localization.
 * Handle Resources. Access to error and warning counts.
 * Message formatting.
 * <br>
 * Also provides implementation for DocErrorReporter.
 *
 * @see java.util.ResourceBundle
 * @see java.text.MessageFormat
 * @author Neal Gafter (rewrite)
 */
public class Messager extends Log implements DocErrorReporter {

    /**
     * The context key for the log.
     */
    private static final Context.Key messagerKey = new Context.Key();

    /**
     * Get the current messager, which is also the compiler log.
     */
    public static Messager instance0(Context context) {
        Messager instance = (Messager) context.get(messagerKey);
        if (instance == null)
            throw new InternalError("no messager instance!");
        return instance;
    }

    public class ExitJavadoc extends Error {

        public ExitJavadoc() {
            super();
        }
    }
    private final String programName;
    private ResourceBundle messageRB = null;

    /**
     * The default writer for diagnostics
     */
    static final PrintWriter defaultErrWriter = new PrintWriter(System.err);
    static final PrintWriter defaultWarnWriter = new PrintWriter(System.err);
    static final PrintWriter defaultNoticeWriter = new PrintWriter(System.out);

    /**
     * Constructor
     * @param programName  Name of the program (for error messages).
     */
    protected Messager(Context context, String programName) {
        this(context, programName, defaultErrWriter, defaultWarnWriter,
                defaultNoticeWriter);
    }

    /**
      * Constructor
      * @param programName  Name of the program (for error messages).
      * @param errWriter    Stream for error messages
      * @param warnWriter   Stream for warnings
      * @param noticeWriter Stream for other messages
      */
    protected Messager(Context context, String programName,
            PrintWriter errWriter, PrintWriter warnWriter, PrintWriter noticeWriter) {
        super(context, errWriter, warnWriter, noticeWriter);
        context.put(messagerKey, this);
        this.programName = programName;
    }

    /**
      * Reset resource bundle, eg. locale has changed.
      */
    public void reset() {
        messageRB = null;
    }

    /**
      * Get string from ResourceBundle, initialize ResourceBundle
      * if needed.
      */
    private String getString(String key) {
        ResourceBundle messageRB = this.messageRB;
        if (messageRB == null) {
            try {
                this.messageRB = messageRB = ResourceBundle.getBundle("com.sun.tools.javadoc.resources.javadoc");
            } catch (MissingResourceException e) {
                throw new Error("Fatal: Resource for javadoc is missing");
            }
        }
        return messageRB.getString(key);
    }

    /**
      * get and format message string from resource
      *
      * @param key selects message from resource
      */
    String getText(String key) {
        return getText(key, (String) null);
    }

    /**
      * get and format message string from resource
      *
      * @param key selects message from resource
      * @param a1 first argument
      */
    String getText(String key, String a1) {
        return getText(key, a1, null);
    }

    /**
      * get and format message string from resource
      *
      * @param key selects message from resource
      * @param a1 first argument
      * @param a2 second argument
      */
    String getText(String key, String a1, String a2) {
        return getText(key, a1, a2, null);
    }

    /**
      * get and format message string from resource
      *
      * @param key selects message from resource
      * @param a1 first argument
      * @param a2 second argument
      * @param a3 third argument
      */
    String getText(String key, String a1, String a2, String a3) {
        return getText(key, a1, a2, a3, null);
    }

    /**
      * get and format message string from resource
      *
      * @param key selects message from resource
      * @param a1 first argument
      * @param a2 second argument
      * @param a3 third argument
      * @param a4 fourth argument
      */
    String getText(String key, String a1, String a2, String a3, String a4) {
        try {
            String message = getString(key);
            String[] args = new String[4];
            args[0] = a1;
            args[1] = a2;
            args[2] = a3;
            args[3] = a4;
            return MessageFormat.format(message, args);
        } catch (MissingResourceException e) {
            return "********** Resource for javadoc is broken. There is no " +
                    key + " key in resource.";
        }
    }

    /**
      * Print error message, increment error count.
      * Part of DocErrorReporter.
      *
      * @param msg message to print
      */
    public void printError(String msg) {
        printError(null, msg);
    }

    /**
      * Print error message, increment error count.
      * Part of DocErrorReporter.
      *
      * @param pos the position where the error occurs
      * @param msg message to print
      */
    public void printError(SourcePosition pos, String msg) {
        String prefix = (pos == null) ? programName : pos.toString();
        errWriter.println(prefix + ": " + msg);
        errWriter.flush();
        prompt();
        nerrors++;
    }

    /**
      * Print warning message, increment warning count.
      * Part of DocErrorReporter.
      *
      * @param msg message to print
      */
    public void printWarning(String msg) {
        printWarning(null, msg);
    }

    /**
      * Print warning message, increment warning count.
      * Part of DocErrorReporter.
      *
      * @param pos the position where the error occurs
      * @param msg message to print
      */
    public void printWarning(SourcePosition pos, String msg) {
        String prefix = (pos == null) ? programName : pos.toString();
        warnWriter.println(prefix + ": warning - " + msg);
        warnWriter.flush();
        nwarnings++;
    }

    /**
      * Print a message.
      * Part of DocErrorReporter.
      *
      * @param msg message to print
      */
    public void printNotice(String msg) {
        printNotice(null, msg);
    }

    /**
      * Print a message.
      * Part of DocErrorReporter.
      *
      * @param pos the position where the error occurs
      * @param msg message to print
      */
    public void printNotice(SourcePosition pos, String msg) {
        if (pos == null)
            noticeWriter.println(msg);
        else
            noticeWriter.println(pos + ": " + msg);
        noticeWriter.flush();
    }

    /**
      * Print error message, increment error count.
      *
      * @param key selects message from resource
      */
    public void error(SourcePosition pos, String key) {
        printError(pos, getText(key));
    }

    /**
      * Print error message, increment error count.
      *
      * @param key selects message from resource
      * @param a1 first argument
      */
    public void error(SourcePosition pos, String key, String a1) {
        printError(pos, getText(key, a1));
    }

    /**
      * Print error message, increment error count.
      *
      * @param key selects message from resource
      * @param a1 first argument
      * @param a2 second argument
      */
    public void error(SourcePosition pos, String key, String a1, String a2) {
        printError(pos, getText(key, a1, a2));
    }

    /**
      * Print error message, increment error count.
      *
      * @param key selects message from resource
      * @param a1 first argument
      * @param a2 second argument
      * @param a3 third argument
      */
    public void error(SourcePosition pos, String key, String a1, String a2,
            String a3) {
        printError(pos, getText(key, a1, a2, a3));
    }

    /**
      * Print warning message, increment warning count.
      *
      * @param key selects message from resource
      */
    public void warning(SourcePosition pos, String key) {
        printWarning(pos, getText(key));
    }

    /**
      * Print warning message, increment warning count.
      *
      * @param key selects message from resource
      * @param a1 first argument
      */
    public void warning(SourcePosition pos, String key, String a1) {
        printWarning(pos, getText(key, a1));
    }

    /**
      * Print warning message, increment warning count.
      *
      * @param key selects message from resource
      * @param a1 first argument
      * @param a2 second argument
      */
    public void warning(SourcePosition pos, String key, String a1, String a2) {
        printWarning(pos, getText(key, a1, a2));
    }

    /**
      * Print warning message, increment warning count.
      *
      * @param key selects message from resource
      * @param a1 first argument
      * @param a2 second argument
      * @param a3 third argument
      */
    public void warning(SourcePosition pos, String key, String a1, String a2,
            String a3) {
        printWarning(pos, getText(key, a1, a2, a3));
    }

    /**
      * Print warning message, increment warning count.
      *
      * @param key selects message from resource
      * @param a1 first argument
      * @param a2 second argument
      * @param a3 third argument
      */
    public void warning(SourcePosition pos, String key, String a1, String a2,
            String a3, String a4) {
        printWarning(pos, getText(key, a1, a2, a3, a4));
    }

    /**
      * Print a message.
      *
      * @param key selects message from resource
      */
    public void notice(String key) {
        printNotice(getText(key));
    }

    /**
      * Print a message.
      *
      * @param key selects message from resource
      * @param a1 first argument
      */
    public void notice(String key, String a1) {
        printNotice(getText(key, a1));
    }

    /**
      * Print a message.
      *
      * @param key selects message from resource
      * @param a1 first argument
      * @param a2 second argument
      */
    public void notice(String key, String a1, String a2) {
        printNotice(getText(key, a1, a2));
    }

    /**
      * Print a message.
      *
      * @param key selects message from resource
      * @param a1 first argument
      * @param a2 second argument
      * @param a3 third argument
      */
    public void notice(String key, String a1, String a2, String a3) {
        printNotice(getText(key, a1, a2, a3));
    }

    /**
      * Return total number of errors, including those recorded
      * in the compilation log.
      */
    public int nerrors() {
        return nerrors;
    }

    /**
      * Return total number of warnings, including those recorded
      * in the compilation log.
      */
    public int nwarnings() {
        return nwarnings;
    }

    /**
      * Print exit message.
      */
    public void exitNotice() {
        int nerrors = nerrors();
        int nwarnings = nwarnings();
        if (nerrors > 0) {
            notice((nerrors > 1) ? "main.errors" : "main.error", "" + nerrors);
        }
        if (nwarnings > 0) {
            notice((nwarnings > 1) ? "main.warnings" : "main.warning",
                    "" + nwarnings);
        }
    }

    /**
      * Force program exit, e.g., from a fatal error.
      * <p>
      * TODO: This method does not really belong here.
      */
    public void exit() {
        throw new ExitJavadoc();
    }
}
