/*
 * @(#)WindowsPlatform.java	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.panel;

import sun.plugin.util.Trace;


class WindowsPlatform implements PlatformDependentInterface {

    public native void init();

    public native void onSave(ConfigurationInfo info);

    public native void onLoad(ConfigurationInfo info);

    public native boolean showURL(String url);

    static {
	try {
	    System.loadLibrary("RegUtils");
	} catch (Throwable e) {
	    Trace.printException(e);
	}
    }
}

