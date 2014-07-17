/*
 * @(#)RealmChoiceCallback.java	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.security.sasl.preview;

import javax.security.auth.callback.ChoiceCallback;

/**
  * This callback is used by <tt>SaslClient</tt> and <tt>SaslServer</tt>
  * to obtain a realm given a list of realm choices.
  *
  * @author Rosanna Lee
  * @author Rob Weltman
  */
public class RealmChoiceCallback extends ChoiceCallback {

    /**
     * Constructs a <tt>RealmChoiceCallback</tt> with a prompt, a list of
     * choices and a default choice.
     * 
     * @param prompt the non-null prompt to use to request the realm.
     * @param choices the non-null list of realms to choose from.
     * @param defaultChoice the choice to be used as the default choice
     * when the list of choices is displayed. It is an index into
     * the <tt>choices</tt> arary.
     * @param multiple true if multiple choices allowed; false otherwise
     * @throws IllegalArgumentException If <tt>prompt</tt> is null or the empty string,
     * if <tt>choices</tt> has a length of 0, if any element from
     * <tt>choices</tt> is null or empty, or if <tt>defaultChoice</tt> 
     * does not fall within the array boundary of <tt>choices</tt>
     */
    public RealmChoiceCallback(String prompt, String[]choices, 
	int defaultChoice, boolean multiple) {
	super(prompt, choices, defaultChoice, multiple);
    }
}
