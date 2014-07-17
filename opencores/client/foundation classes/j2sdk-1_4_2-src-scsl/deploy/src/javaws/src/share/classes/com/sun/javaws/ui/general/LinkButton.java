/*
 * @(#)LinkButton.java	1.8 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.ui.general;

import com.sun.javaws.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.border.*;

public class LinkButton extends JButton {

    private Color _linkColor;
    private Color _armedColor;

    public LinkButton() {
        super();
        _linkColor = Resources.getColor("player.homePageButton.color");
        _armedColor= Resources.getColor("player.homePageButton.armedColor");
	setBorder(BorderFactory.createEmptyBorder(1,1,1,1));
        setForeground(_linkColor);
	setFocusPainted(true);
        setContentAreaFilled(false);
	setCursor(Cursor.getPredefinedCursor(Cursor.HAND_CURSOR));
    }
    public void paintComponent(Graphics g) {
        if (getModel().isArmed() && getModel().isPressed()) {
            g.setColor(_armedColor);
            setForeground(_armedColor);
        } else {
            g.setColor(_linkColor);
            setForeground(_linkColor);
        }
        super.paintComponent(g);
        if (isEnabled()) {
	    String text = getText();
	    if (text != null) {
                FontMetrics fm = g.getFontMetrics();
                int width = fm.stringWidth(getText());
                Insets ins = getInsets();
                int height = getHeight();
                int x = ins.left;
                int y = height - (ins.bottom + fm.getDescent());
                g.drawLine(x,y,x+width,y);
	    }
        }
    }
}






