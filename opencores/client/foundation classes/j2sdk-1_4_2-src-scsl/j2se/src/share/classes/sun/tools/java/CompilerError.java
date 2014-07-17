/*
 * @(#)CompilerError.java	1.18 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.java;

/**
 * This exception is thrown when an internal compiler error occurs
 */

public
class CompilerError extends Error {
    Throwable e;

    /**
     * Constructor
     */
    public CompilerError(String msg) {
	super(msg);
	this.e = this;
    }

    /**
     * Create an exception given another exception.
     */
    public CompilerError(Exception e) {
	super(e.getMessage());
	this.e = e;
    }

    public void printStackTrace() {
	if (e == this)
	    super.printStackTrace();
	else
	    e.printStackTrace();
    }
}
