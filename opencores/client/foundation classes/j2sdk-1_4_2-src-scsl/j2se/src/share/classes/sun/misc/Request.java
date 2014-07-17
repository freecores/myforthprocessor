/*
 * @(#)Request.java	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.misc;

/**
 * Requests are functor objects; that is, they provide part of the mechanism
 * for deferred function application.
 *
 * @version 	1.9, 01/23/03
 * @author	Steven B. Byrne
 */

abstract public class Request {
    /**
     * The main task of the Request object is to be exectuted from a request
     * queue. 
     */
    abstract public void execute();
}
