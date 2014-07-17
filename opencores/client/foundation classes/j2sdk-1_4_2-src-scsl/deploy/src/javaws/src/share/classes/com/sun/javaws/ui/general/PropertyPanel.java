/*
 * @(#)PropertyPanel.java	1.4 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.ui.general;

import javax.swing.JPanel;
import java.awt.*;

 public class PropertyPanel extends JPanel implements Subcontroller {
	private boolean started;

	public PropertyPanel() {
	    super();
	}

	public void start() {
	    if (!started) {
		started = true;
		createPropertyControls();
	    }
	}
	
	public void stop() {
	}
	
	public Component getComponent() {
	    return PropertyPanel.this;
	}
	
	public void apply() {
	}
	
	public void revert() {
	}
	
	protected void createPropertyControls() {
	}



        protected void addGridBagComponent(Container parent,
                                           Component c, int gridx, int gridy,
                                           int width, int height,
                                           double weightx, double weighty,
                                           int anchor, int fill,
                                           Insets insets, int ipadx,
                                           int ipady) {
            GridBagHelper.addGridBagComponent(parent, c, gridx, gridy,
                                              width, height, weightx,
                                              weighty, anchor, fill,
                                              insets, ipadx, ipady);
        }
 }
