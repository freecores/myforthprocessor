/*
 * @(#)AppletViewerFactory.java	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * AppletViewerFactory.java
 */

package sun.applet;

import java.util.Hashtable;
import java.net.URL;
import java.awt.MenuBar;

public
interface AppletViewerFactory {
	public AppletViewer createAppletViewer(int x, int y, URL doc, Hashtable atts);
	public MenuBar getBaseMenuBar();
	public boolean isStandalone();
}
