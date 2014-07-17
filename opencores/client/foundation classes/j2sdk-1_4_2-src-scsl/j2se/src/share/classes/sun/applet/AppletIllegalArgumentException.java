/*
 * @(#)AppletIllegalArgumentException.java	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.applet;

/**
 * An applet security exception.
 *
 * @version 	03/01/23
 * @author 	Arthur van Hoff
 */
public 
class AppletIllegalArgumentException extends IllegalArgumentException {
    private String key = null;

    public AppletIllegalArgumentException(String key) {
        super(key);
        this.key = key;

    }

    public String getLocalizedMessage() {
        return amh.getMessage(key);
    }

    private static AppletMessageHandler amh = new AppletMessageHandler("appletillegalargumentexception");
}
