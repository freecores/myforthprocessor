/*
 * @(#)BottomBevelBorder.java	1.2 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.ui.prefs;

import java.awt.*;
import javax.swing.border.*;

/**
 * @version 1.8 11/29/01
 */
public class BottomBevelBorder implements Border {
    private static BottomBevelBorder sharedBorder;

    public static BottomBevelBorder getSharedBottomBevelBorder() {
	if (sharedBorder == null) {
	    sharedBorder = new BottomBevelBorder();
	}
	return sharedBorder;
    }


    public void paintBorder(Component c, Graphics g, int x, int y,
			    int width, int height) {
	g.setColor(c.getBackground().darker().darker().darker());
	g.drawLine(0, height - 3, width, height - 3);
	g.setColor(c.getBackground().darker().darker());
	g.drawLine(0, height - 2, width, height - 2);
	g.setColor(c.getBackground().darker());
	g.drawLine(0, height - 1, width, height - 1);
    }
    public Insets getBorderInsets(Component c) {
	return new Insets(0, 0, 3, 0);
    }
    public boolean isBorderOpaque() {
	return true;
    }
}
