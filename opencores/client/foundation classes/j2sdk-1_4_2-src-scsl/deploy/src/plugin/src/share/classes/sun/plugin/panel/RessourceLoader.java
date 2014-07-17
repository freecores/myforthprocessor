/*
 * @(#)RessourceLoader.java	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.panel;

import javax.swing.ImageIcon;

public interface RessourceLoader {

	public ImageIcon loadImageIcon(String fileName, String description);
}

