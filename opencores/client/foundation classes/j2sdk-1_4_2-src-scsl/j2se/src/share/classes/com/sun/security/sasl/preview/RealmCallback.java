/*
 * @(#)RealmCallback.java	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.security.sasl.preview;

import javax.security.auth.callback.TextInputCallback;

/**
  * This callback is used by <tt>SaslClient</tt> and <tt>SaslServer</tt>
  * to retrieve realm information.
  *
  * @author Rosanna Lee
  * @author Rob Weltman
  */
public class RealmCallback extends TextInputCallback {
    /**
     * Constructs a <tt>RealmCallback</tt> with a prompt.
     * 
     * @param prompt The non-null prompt to use to request the realm information.
     * @throws IllegalArgumentException If <tt>prompt</tt> is null or 
     * the empty string.
     */
    public RealmCallback(String prompt) {
	super(prompt);
    }

    /**
     * Constructs a <tt>RealmCallback</tt> with a prompt and default
     * realm information.
     * 
     * @param prompt The non-null prompt to use to request the realm information.
     * @param defaultRealmInfo The non-null default realm information to use.
     * @throws IllegalArgumentException If <tt>prompt</tt> is null or 
     * the empty string, 
     * or if <tt>defaultRealm</tt> is empty or null.
     */
    public RealmCallback(String prompt, String defaultRealmInfo) {
	super(prompt, defaultRealmInfo);
    }
}
