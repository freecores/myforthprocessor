/*
 * @(#)MessageHandler.java	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.panel;

/**
 * Class to internationalize messages in the Activator ControlPanel.
 *
 * Each of the main classes of the ControlPanel creates an instance
 * of this using its own baseKey, such as "proxy".
 *
 * It then uses calls on (for example) getMessage() with a subKey
 * to locate specific localized strings.  The baseKey and the subKey
 * are combined as "baseKey.subKey" and resolved in the locale
 * specific ResourceBundle.
 *
 * @author Graham Hamilton
 *
 */

import java.util.ResourceBundle;

class MessageHandler {

    private String baseKey;
    private static ResourceBundle rb;

    MessageHandler(String baseKey) {
	this.baseKey = baseKey;
        try {
	    if (rb == null) {
                rb = ResourceBundle.getBundle("sun.plugin.resources.ControlPanel");
	    }
        } catch (java.util.MissingResourceException ex) {
	    // This "should never happen"
	    String mess = "Activator ControlPanel no locale info: " + ex;
            System.err.println(mess);
	    throw new Error(mess);
        }
    }

    String getMessage(String subKey) {
	String key = baseKey + "." + subKey;
        try {
	    return rb.getString(key);
        } catch (java.util.MissingResourceException ex) {
	    return key;
	}
    }

    int getAcceleratorKey(String subKey) {
        String key = baseKey + "." + subKey + ".acceleratorKey";
        Integer val = (Integer)rb.getObject(key);
        return val.intValue();
    }

}
