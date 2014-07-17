/*
 * @(#)Exception.java	1.30 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package java.lang;

/**
 * The class <code>Exception</code> and its subclasses are a form of
 * <code>Throwable</code> that indicates conditions that a reasonable
 * application might want to catch.
 *
 * @author  Frank Yellin
 * @version 1.30, 01/23/03
 * @see     java.lang.Error
 * @since   JDK1.0
 */
public class Exception //extends Throwable
{
    static final long serialVersionUID = -3387516993124229948L;
    private String message;

    /**
     * Constructs a new exception with <code>null</code> as its detail message.
     * The cause is not initialized, and may subsequently be initialized by a
     * call to {@link #initCause}.
     */
    public Exception() {
	this(null);
    }

    /**
     * Constructs a new exception with the specified detail message.  The
     * cause is not initialized, and may subsequently be initialized by
     * a call to {@link #initCause}.
     *
     * @param   message   the detail message. The detail message is saved for
     *          later retrieval by the {@link #getMessage()} method.
     */
    public Exception(String message) {
	this.message = message;
    }

    /**
     * Constructs a new exception with the specified detail message and
     * cause.  <p>Note that the detail message associated with
     * <code>cause</code> is <i>not</i> automatically incorporated in
     * this exception's detail message.
     *
     * @param  message the detail message (which is saved for later retrieval
     *         by the {@link #getMessage()} method).
     * @param  cause the cause (which is saved for later retrieval by the
     *         {@link #getCause()} method).  (A <tt>null</tt> value is
     *         permitted, and indicates that the cause is nonexistent or
     *         unknown.)
     * @since  1.4
    public Exception(String message, Throwable cause) {
        super(message, cause);
    }
*/

    /**
     * Constructs a new exception with the specified cause and a detail
     * message of <tt>(cause==null ? null : cause.toString())</tt> (which
     * typically contains the class and detail message of <tt>cause</tt>).
     * This constructor is useful for exceptions that are little more than
     * wrappers for other throwables (for example, {@link
     * java.security.PrivilegedActionException}).
     *
     * @param  cause the cause (which is saved for later retrieval by the
     *         {@link #getCause()} method).  (A <tt>null</tt> value is
     *         permitted, and indicates that the cause is nonexistent or
     *         unknown.)
     * @since  1.4
    public Exception(Throwable cause) {
        super(cause);
    }
*/
 /**
  * Returns the detail message string of this throwable.
  *
  * @return  the detail message string of this <tt>Throwable</tt> instance
  *          (which may be <tt>null</tt>).
  */
 public String getMessage() {
     return message;
 }

 /**
  * Creates a localized description of this throwable.
  * Subclasses may override this method in order to produce a
  * locale-specific message.  For subclasses that do not override this
  * method, the default implementation returns the same result as
  * <code>getMessage()</code>.
  *
  * @return  The localized description of this throwable.
  * @since   JDK1.1
  */
 public String getLocalizedMessage() {
     return getMessage();
 }

 /**
  * Returns a short description of this throwable.
  * If this <code>Throwable</code> object was created with a non-null detail
  * message string, then the result is the concatenation of three strings:
  * <ul>
  * <li>The name of the actual class of this object
  * <li>": " (a colon and a space)
  * <li>The result of the {@link #getMessage} method for this object
  * </ul>
  * If this <code>Throwable</code> object was created with a <tt>null</tt>
  * detail message string, then the name of the actual class of this object
  * is returned.
  *
  * @return a string representation of this throwable.
  */
 public String toString() {
     String s = super.toString();
     String message = getLocalizedMessage();
     return (message != null) ? (s + ": " + message) : s;
 }
}
