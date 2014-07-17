/*
 * @(#)LineBorder.java	1.6 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.ui.general;

import java.awt.*;
import javax.swing.SwingConstants;
import javax.swing.border.Border;

/**
 * @version 1.6 01/23/03
 */
public class LineBorder implements Border {
    private Color _color;
    private int _thickness;
    private int _sides;

    public LineBorder(Color color, int thickness, int sides) {
        _color = color;
        _thickness = thickness;
        _sides = sides;
    }

    public void paintBorder(Component c, Graphics g, int x, int y, int width,
                            int height) {
        g.setColor(_color);
        if ((_sides & (1 << SwingConstants.TOP)) != 0) {
            g.fillRect(x, y, width, _thickness);
        }
        if ((_sides & (1 << SwingConstants.LEFT)) != 0) {
            g.fillRect(x, y, _thickness, height);
        }
        if ((_sides & (1 << SwingConstants.BOTTOM)) != 0) {
            g.fillRect(x, y + height - _thickness, width, _thickness);
        }
        if ((_sides & (1 << SwingConstants.RIGHT)) != 0) {
            g.fillRect(x + width - _thickness, y, _thickness, height);
        }
    }

    public Insets getBorderInsets(Component c) {
        Insets insets = new Insets(0, 0, 0, 0);

        if ((_sides & (1 << SwingConstants.TOP)) != 0) {
            insets.top = _thickness;
        }
        if ((_sides & (1 << SwingConstants.LEFT)) != 0) {
            insets.left = _thickness;
        }
        if ((_sides & (1 << SwingConstants.BOTTOM)) != 0) {
            insets.bottom = _thickness;
        }
        if ((_sides & (1 << SwingConstants.RIGHT)) != 0) {
            insets.right = _thickness;
        }
        return insets;
    }

    public boolean isBorderOpaque() {
        return true;
    }
}
