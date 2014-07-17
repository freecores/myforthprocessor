/*
 * @(#)PluggablePanel.java	1.2 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.ui.prefs;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;

/**
 * PluggablePanel provides something similiar to what most wizard controls
 * contain. It has a set of buttons (text and enabled can be
 * changed), a title, and an area to plugin in your own component.
 *
 * @version 1.9 11/29/01
 */
public class PluggablePanel extends JPanel {
    protected JButton[] buttons;
    private int activeButtonCount;
    private JLabel titleLabel;
    private Component component;
    /** ActionListener added to the JButtons, will invoke
     * <code>buttonPressed</code>.
     */
    private ActionListener buttonActionListener;
    /** Constraints for the component, used in the add method. */
    private Object componentCons;
    /** Parent component the buttons are added to. */
    private Container buttonParent;

    public PluggablePanel() {
	this(false);
    }

    public PluggablePanel(boolean wantsBorders) {
	super(new GridBagLayout());
	buttons = null;
	buttonActionListener = createButtonActionListener();
	createWidgets();
	layoutContainer(wantsBorders);
    }

    public void setTitle(String title) {
	titleLabel.setText(title);
    }

    public void setComponent(Component component) {
	if (this.component != null) {
	    remove(this.component);
	}
	this.component = component;
	if (this.component != null) {
	    add(this.component, componentCons);
	}
	revalidate();
	repaint();
    }

    public void setButtons(String[] labels) {
	setButtons(labels, null);
    }

    public void setButtons(String[] labels, boolean[] enabled) {
	// Create new buttons if necessary
	int oldLength = (buttons == null) ? 0 : buttons.length;
	int newLength = (labels == null) ? 0 : labels.length;
	if (newLength > oldLength) {
	    JButton[] newButtons = new JButton[newLength];
	    if (oldLength > 0) {
		System.arraycopy(buttons, 0, newButtons, 0, oldLength);
	    }
	    buttons = newButtons;
	    for (int counter = oldLength; counter < newLength; counter++) {
		buttons[counter] = createButton();
		buttons[counter].addActionListener(buttonActionListener);
	    }
	}

	// Reset the state of the buttons
	for (int counter = 0; counter < newLength; counter++) {
	    setButtonTitle(counter, labels[counter]);
	    setButtonEnabled(counter, (enabled == null) ? true :
			     enabled[counter]);
	}

	// And add/remove any new buttons.
	int oldActiveCount = activeButtonCount;
	activeButtonCount = newLength;
	if (activeButtonCount > oldActiveCount) {
	    for (int counter = oldActiveCount; counter < activeButtonCount;
		 counter++) {
		addButtonAt(counter);
	    }
	}
	else if (activeButtonCount < oldActiveCount) {
	    for (int counter = oldActiveCount - 1;
		     counter >= activeButtonCount; counter--) {
		removeButtonAt(counter);
	    }
	}
    }

    public void setButtonTitle(int index, String title) {
	buttons[index].setText(title);
    }

    public void setButtonEnabled(int index, boolean enable) {
	buttons[index].setEnabled(enable);
    }

    protected void buttonPressed(int index) {
    }

    private void removeButtonAt(int index) {
	Component[] kids = buttonParent.getComponents();
	for (int counter = kids.length - 1; counter >= 0; counter--) {
	    if (kids[counter] == buttons[index]) {
		buttonParent.remove(counter);
		if (counter > 0) {
		    buttonParent.remove(counter - 1);
		}
		return;
	    }
	}
    }

    private void addButtonAt(int index) {
	if (index > 0) {
	    buttonParent.add(Box.createHorizontalStrut(5));
	}
	buttonParent.add(buttons[index]);
    }

    private ActionListener createButtonActionListener() {
	return new ActionListener() {
	    public void actionPerformed(ActionEvent ae) {
		Object source = ae.getSource();
		for (int counter = 0; counter < activeButtonCount; counter++) {
		    if (source == buttons[counter]) {
			buttonPressed(counter);
		    }
		}
	    }
	};
    }

    private JButton createButton() {
	return new JButton();
    }

    private void createWidgets() {
	titleLabel = new JLabel();
	titleLabel.setFont(titleLabel.getFont().deriveFont(Font.ITALIC, 18f));
    }

    private void layoutContainer(boolean wantsBorders) {
	GridBagConstraints cons = new GridBagConstraints();
	cons.gridx = cons.gridy = 0;
	cons.weightx = cons.weighty = 0;
	cons.fill = GridBagConstraints.NONE;
	cons.anchor = GridBagConstraints.WEST;
	cons.insets = new Insets(5, 2, 5, 2);
	add(titleLabel, cons);

	if (wantsBorders) {
	    cons.gridy++;
	    cons.weightx = 1;
	    cons.fill = GridBagConstraints.HORIZONTAL;
	    cons.insets.top = 0;
	    JPanel panel = new JPanel();
	    panel.setBorder(BottomBevelBorder.getSharedBottomBevelBorder());
	    cons.insets.left = cons.insets.right = 0;
	    add(panel, cons);

	    cons.gridy += 2;
	    panel = new JPanel();
	    panel.setBorder(BottomBevelBorder.getSharedBottomBevelBorder());
	    add(panel, cons);
	}

	if (wantsBorders) {
	    cons.gridy++;
	}
	else {
	    cons.gridy = 2;
	}
	cons.insets.left = cons.insets.right = 5;
	cons.weightx = 0;
	cons.fill = GridBagConstraints.NONE;
	cons.anchor = GridBagConstraints.EAST;
	buttonParent = Box.createHorizontalBox();
	add(buttonParent, cons);

	if (wantsBorders) {
	    cons.gridy = 2;
	}
	else {
	    cons.gridy = 1;
	}
	cons.fill = GridBagConstraints.BOTH;
	cons.weightx = cons.weighty = 1;
	componentCons = cons;
    }
}
