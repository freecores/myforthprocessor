/*
 * @(#)Anchor.java	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.javascript.navig;

import netscape.javascript.JSObject;
import netscape.javascript.JSException;



/** 
 * <p> Emulate the Anchor object in the JavaScript Document Object Model
 * in Navigator 3.x.
 * </p>
 */
public class Anchor extends sun.plugin.javascript.navig.JSObject {

    /**
     * <p> Construct a new Anchor object. 
     * </p>
     * 
     * @param instance Native plugin instance.
     * @param context Evaluation context.
     */
    protected Anchor(int instance, String context) {
	super(instance, context);
    }
}
