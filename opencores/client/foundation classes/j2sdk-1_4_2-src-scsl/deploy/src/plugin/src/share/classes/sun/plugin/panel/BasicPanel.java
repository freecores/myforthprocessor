/*
 * @(#)BasicPanel.java	1.20 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.panel;

/**
 * The Basic Panel shows the basic configuration information for the javaplugin
 * like console state, security attributes...
 *
 * @version 	1.3
 * @author	Jerome Dochez
 *
 */

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeEvent;
import javax.swing.border.*;

public class BasicPanel extends ActivatorSubPanel
			implements ActionListener{

    /**
     * Construct the panel, add widgets
     */
    BasicPanel(ConfigurationInfo model) {
	super(model);

	setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));

	// Add vertical pad
	Dimension vpad5 = new Dimension(1,5);

        JPanel first = new JPanel();
        first.setLayout(new BoxLayout(first, BoxLayout.X_AXIS));
        add(first);

        JPanel second = new JPanel();
        second.setLayout(new BoxLayout(second, BoxLayout.Y_AXIS));
        Border border = BorderFactory.createEtchedBorder();
        second.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createEmptyBorder(4, 4, 4, 4),
					       BorderFactory.createTitledBorder(border, mh.getMessage("java_console"))));

        JPanel consoleOptions = new JPanel();
        consoleOptions.setLayout(new BoxLayout(consoleOptions, BoxLayout.X_AXIS));
        second.add(Box.createGlue());
        show = new JRadioButton(mh.getMessage("show_console"));
        hide = new JRadioButton(mh.getMessage("hide_console"));
        hide.setSelected(true);
        nothing = new JRadioButton(mh.getMessage("no_console"));
        one = new ButtonGroup();
        one.add(show);
        one.add(hide);
        one.add(nothing);
        show.addActionListener(this);
        hide.addActionListener(this);
        nothing.addActionListener(this);
        JPanel btnConsoleOptions = new JPanel();
        btnConsoleOptions.setLayout(new BoxLayout(btnConsoleOptions, BoxLayout.Y_AXIS));
        btnConsoleOptions.add(show);
        btnConsoleOptions.add(hide);
        btnConsoleOptions.add(nothing);
        consoleOptions.add(Box.createRigidArea(new Dimension(15, 1)));
        consoleOptions.add(btnConsoleOptions);
        consoleOptions.add(Box.createGlue());
        second.add(consoleOptions);
        add(second);

        add(Box.createRigidArea(vpad5));

	if (model.isShowBrowserPanel()) {
	    JPanel third = new JPanel();
	    third.setLayout(new BoxLayout(third, BoxLayout.Y_AXIS));
	    //third.setBorder(BorderFactory.createEmptyBorder(8, 8, 8, 8));
	    JPanel sysTrayPanel = new JPanel();
	    sysTrayPanel.setLayout(new BoxLayout(sysTrayPanel, BoxLayout.X_AXIS));
	    showSysTray = new JCheckBox(mh.getMessage("show_systray"));
	    showSysTray.addActionListener(this);
	    sysTrayPanel.add(showSysTray);
	    sysTrayPanel.add(Box.createGlue());
	    third.add(sysTrayPanel);
	    add(third);
	    add(Box.createRigidArea(vpad5));
	}

        JPanel fourth = new JPanel();
        fourth.setLayout(new BoxLayout(fourth, BoxLayout.Y_AXIS));
        //fourth.setBorder(BorderFactory.createEmptyBorder(8, 8, 8, 8));
        JPanel exceptionPanel = new JPanel();
        exceptionPanel.setLayout(new BoxLayout(exceptionPanel, BoxLayout.X_AXIS));
	showException = new JCheckBox(mh.getMessage("show_exception"));
	showException.addActionListener(this);
        exceptionPanel.add(showException);
        exceptionPanel.add(Box.createGlue());
        fourth.add(exceptionPanel);
        add(fourth);

	// Add vertical pad
	add(Box.createRigidArea(new Dimension(1,10)));

	// Create the combobox panel
	add(Box.createGlue());

	// Add vertical pad
	add(Box.createRigidArea(new Dimension(1,10)));
	add(Box.createGlue());
	add(Box.createGlue());

        reset();
    }

    /**
     * Add a Combo box in the specified value, the items to be added
     * to the ComboBox are also specified
     *
     * @param panel where to place the ComboBox
     * @param title for the ComboBox
     * @param items items to add to the ComboBox
     * @param initialIndex
     */
    private JComboBox addComboBoxFor(JPanel panel, JLabel title, String[] items, int initialIndex) {

	panel.add(title);
	JComboBox combo = new JComboBox();

	for (int i=0;i<items.length;i++)
	    combo.addItem(items[i]);

	combo.setSelectedIndex(initialIndex);
	combo.addActionListener(this);
	panel.add(combo);
	return combo;
    }


    /**
     * ActionListener interface implementation. All fields in this
     * panel will generate this message when changed. We use the
     * internal UI state to save the user choices so we just reset
     * the hasChanged field of our model
     *
     * @param ActionEvent info about the event
     */
    public void actionPerformed(ActionEvent e) {
        if (e.getSource()==show)
            model.setConsoleEnabled("show"); // full size.
        if (e.getSource()==hide)
            model.setConsoleEnabled("hide"); // iconified/systemTray.
        if (e.getSource()==nothing)
            model.setConsoleEnabled("nothing"); // do not start Console.
	if (e.getSource()==showException)
	    model.setShowException(showException.isSelected());
	if (e.getSource()==showSysTray)
	    model.setShowSysTray(showSysTray.isSelected());
    }

    /**
     * Reset the UI state from the Model
     */
    public void reset() {
        String val=model.isConsoleEnabled();

        if(val.equalsIgnoreCase("show"))
            show.setSelected(true);
        if(val.equalsIgnoreCase("hide"))
            hide.setSelected(true);
        if(val.equalsIgnoreCase("nothing"))
            nothing.setSelected(true);

	if(showSysTray != null) {
	    showSysTray.setSelected(model.isSysTrayEnabled());
	}
	showException.setSelected(model.isShowExceptionEnabled());
    }

    private JCheckBox showException = null, showSysTray = null;
    private MessageHandler mh = new MessageHandler("basic");

    private JRadioButton show, hide, nothing;
    private ButtonGroup one;
}


