/*
 * @(#)AppletIOException.java	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.applet;

import java.io.IOException;

/**
 * An applet IO exception.
 *
 * @version 	03/01/23
 * @author 	Koji Uno
 */
public 
class AppletIOException extends IOException {
    private String key = null;
    private Object msgobj = null;

    public AppletIOException(String key) {
        super(key);
        this.key = key;
        
    }
    public AppletIOException(String key, Object arg) {
        this(key);
        msgobj = arg;
    }

    public String getLocalizedMessage() {
        if( msgobj != null)
            return amh.getMessage(key, msgobj);
        else
            return amh.getMessage(key);
    }

    private static AppletMessageHandler amh = new AppletMessageHandler("appletioexception");

}
