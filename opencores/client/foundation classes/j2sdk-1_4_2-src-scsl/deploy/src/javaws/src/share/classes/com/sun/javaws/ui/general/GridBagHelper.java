/*
 * @(#)GridBagHelper.java	1.6 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.ui.general;

import java.awt.*;

/**
 * @version 1.6 01/23/03
 */
public class GridBagHelper {
    private static GridBagConstraints _tempCons = new GridBagConstraints();
    private static Insets _emptyInsets = new Insets(0, 0, 0, 0);

    public static void addGridBagComponent(Container parent,
                                           Component c, int gridx, int gridy,
                                           int width, int height,
                                           double weightx, double weighty,
                                           int anchor, int fill,
                                           Insets insets, int ipadx,
                                           int ipady) {
        _tempCons.gridx = gridx;
        _tempCons.gridy = gridy;
        _tempCons.gridwidth = width;
        _tempCons.gridheight = height;
        _tempCons.weightx = weightx;
        _tempCons.weighty = weighty;
        _tempCons.anchor = anchor;
        _tempCons.fill = fill;
        _tempCons.insets = (insets == null) ? _emptyInsets : insets;
        _tempCons.ipadx = ipadx;
        _tempCons.ipady = ipady;
        parent.add(c, _tempCons);
    }
}
