/*
 * @(#)SaslException.java	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.security.sasl.preview;

/**
 * This class represents an error that has occurred when using SASL.
 *
 * @author Rosanna Lee
 * @author Rob Weltman
 */
public class SaslException extends java.io.IOException {
    /**
     * The possibly null root cause exception.
     * @serial
     */
    private Throwable exception;

    /**
     * Constructs a new instance of <tt>SaslException</tt>.
     * The root exception and the detailed message are null.
     */
    public SaslException () {
	super();
    }

    /**
     * Constructs a new instance of <tt>SaslException</tt> with a detailed message.
     * The root exception is null.
     * @param detail A possibly null string containing details of the exception.
     *
     * @see java.lang.Throwable#getMessage
     */
    public SaslException (String detail) {
	super(detail);
    }

    /**
     * Constructs a new instance of <tt>SaslException</tt> with a detailed message
     * and a root exception.
     * For example, a SaslException might result from a problem with
     * the callback handler, which might throw a NoSuchCallbackException if
     * it does not support the requested callback, or throw an IOException
     * if it had problems obtaining data for the callback. The
     * SaslException's root exception would be then be the exception thrown
     * by the callback handler.
     *
     * @param detail A possibly null string containing details of the exception.
     * @param ex A possibly null root exception that caused this exception.
     *
     * @see java.lang.Throwable#getMessage
     * @see #getCause
     */
    public SaslException (String detail, Throwable ex) {
	super(detail);
	exception = ex;
    }

    /**
     * Returns the cause of this throwable or <code>null</code> if the
     * cause is nonexistent or unknown.  (The cause is the throwable that
     * caused this throwable to get thrown.)
     *
     * @return The possibly null exception that caused this exception.
     */
    public Throwable getCause() {
	return exception;
    }

    /**
     * Prints this exception's stack trace to <tt>System.err</tt>.
     * If this exception has a root exception; the stack trace of the
     * root exception is also printed to <tt>System.err</tt>.
     */
    public void printStackTrace() {
	printStackTrace(System.err);
    }

    /**
     * Prints this exception's stack trace to a print stream.
     * If this exception has a root exception; the stack trace of the
     * root exception is also printed to the print stream.
     * @param ps The non-null print stream to which to print.
     */
    public void printStackTrace(java.io.PrintStream ps) {
	if (exception != null) {
	    synchronized (ps) {
		super.printStackTrace(ps);
		ps.print("Caused by: ");
		exception.printStackTrace(ps);
	    }
	} else {
	    super.printStackTrace(ps);
	}
    }

    /**
     * Prints this exception's stack trace to a print writer.
     * If this exception has a root exception; the stack trace of the
     * root exception is also printed to the print writer.
     * @param ps The non-null print writer to which to print.
     */
    public void printStackTrace(java.io.PrintWriter pw) {
	if (exception != null) {
	    synchronized (pw) {
		super.printStackTrace(pw);
		pw.print("Caused by: ");
		exception.printStackTrace(pw);
	    }
	} else {
	    super.printStackTrace(pw);
	}
    }

    /**
     * Returns the string representation of this exception.
     * The string representation contains
     * this exception's class name, its detailed messsage, and if
     * it has a root exception, the string representation of the root
     * exception. This string representation
     * is meant for debugging and not meant to be interpreted
     * programmatically.
     * @return The non-null string representation of this exception.
     * @see java.lang.Throwable#getMessage
     */
    public String toString() {
	String answer = super.toString();
	if (exception != null && exception != this) {
	    answer += " [Caused by " + exception.toString() + "]";
	}
	return answer;
    }
}
