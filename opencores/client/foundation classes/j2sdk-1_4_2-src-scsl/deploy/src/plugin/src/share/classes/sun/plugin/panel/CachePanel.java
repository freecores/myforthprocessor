/*
 * @(#)CachePanel.java	1.28 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.panel;

import javax.swing.*;
import javax.swing.border.Border;
import javax.swing.event.*;
import javax.swing.text.*;
import javax.swing.filechooser.FileFilter;
import java.awt.*;
import java.awt.event.*;
import java.awt.Dimension;
import java.io.File;
import java.io.IOException;
import java.text.MessageFormat;
import java.text.NumberFormat;
import java.util.Hashtable;
import sun.plugin.util.*;
import sun.plugin.cache.JarCacheViewer;

/**
 * Panel to display JAR caching options
 */
public class CachePanel extends ActivatorSubPanel
                        implements  ActionListener {

    /**
     * Contruct a new cache panel
     */
    public CachePanel(final ConfigurationInfo model) {
        super(model);

        setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));

        // Add checkbox to enable and disable caching
        JPanel panel = new JPanel();
        panel.setLayout(new BoxLayout(panel, BoxLayout.X_AXIS));
        panel.setBorder(BorderFactory.createEmptyBorder(6, 6, 6, 4));
        enabled = new JCheckBox(mh.getMessage("enabled"));
        enabled.addActionListener(this);
        panel.add(enabled);
        panel.add(Box.createHorizontalGlue());

        // Create button to view cache
        view = new JButton(mh.getMessage("view"));
	view.setMnemonic(mh.getAcceleratorKey("view")); 
        view.addActionListener(this);
        panel.add(view);
        panel.add(Box.createRigidArea(new Dimension(5,0)));

        // Create button to clear cache
        clear = new JButton(mh.getMessage("clear"));
	clear.setMnemonic(mh.getAcceleratorKey("clear")); 
        clear.addActionListener(this);
        panel.add(clear);
        add(panel);

        // Create "Settings" panel and border
        JPanel settingsPanel = new JPanel();
        GridBagLayout gridBag = new GridBagLayout();
        GridBagConstraints c = new GridBagConstraints();
        settingsPanel.setLayout(gridBag);
	Border border = BorderFactory.createEtchedBorder();
	settingsPanel.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createEmptyBorder(4, 4, 4, 4),
				BorderFactory.createTitledBorder(border, mh.getMessage("settings"))));

        // Add a field for setting the location of the cache
        JLabel label = new JLabel(mh.getMessage("location"));
        c.anchor = GridBagConstraints.WEST;
        c.weighty = 3;
        c.fill = GridBagConstraints.NONE;
        c.insets = new Insets(5, 10, 5, 10);
        gridBag.setConstraints(label, c);
        settingsPanel.add(label);
        location = new LocationField();
        location.setDocument(new LocationDocument());
        c.weightx = 1;
        c.gridwidth = 3;
        c.insets = new Insets(5, 0, 5, 2);
        c.fill = GridBagConstraints.HORIZONTAL;
        gridBag.setConstraints(location, c);
        settingsPanel.add(location);
        choose = new JButton(UIManager.getIcon("FileView.directoryIcon"));
	choose.setMnemonic(mh.getAcceleratorKey("browseDirectory")); 
        choose.setMargin(new Insets(0, 2, 0, 2));
        choose.addActionListener(this);
        c.fill = GridBagConstraints.NONE;
        c.gridwidth = GridBagConstraints.REMAINDER;
        c.insets = new Insets(5, 2, 5, 10);
        c.weightx = 0;
        gridBag.setConstraints(choose, c);
        settingsPanel.add(choose);

        // Add radio buttons for cache size
        label = new JLabel(mh.getMessage("size"));
        c.weightx = 0;
        c.weighty = 0;
        c.gridwidth = 1;
        c.insets = new Insets(5, 10, 5, 10);
        gridBag.setConstraints(label, c);
        settingsPanel.add(label);
        ButtonGroup group = new ButtonGroup();
        unlimited = new JRadioButton(mh.getMessage("unlimited"));
        unlimited.addActionListener(this);
        unlimited.setMargin(new Insets(0, 0, 0, 0));
        c.weightx = 1;
        c.gridwidth = GridBagConstraints.REMAINDER;
        c.insets = new Insets(5, 0, 0, 0);
        gridBag.setConstraints(unlimited, c);
        group.add(unlimited);
        settingsPanel.add(unlimited);
        maximum = new JRadioButton(mh.getMessage("maximum"));
        maximum.setMargin(new Insets(0, 0, 0, 0));
        maximum.addActionListener(this);
        c.gridwidth = 1;
        c.gridx = 1;
        c.weightx = 0;
        c.insets = new Insets(0, 0, 10, 0);
        gridBag.setConstraints(maximum, c);
        group.add(maximum);
        settingsPanel.add(maximum);

        // Add a field for setting the size of the cache
        size = new JTextField(7);
        size.setDocument(new SizeDocument());
        c.gridx = GridBagConstraints.RELATIVE;
        gridBag.setConstraints(size, c);
        settingsPanel.add(size);
        String[] elements = {mh.getMessage("mb"),
                             mh.getMessage("kb"),
                             mh.getMessage("bytes")};
        units = new JComboBox(elements);
        units.addActionListener(this);
        c.gridwidth = 1;
        c.insets = new Insets(0, 2, 10, 10);
        gridBag.setConstraints(units, c);
        settingsPanel.add(units);

        // Add a slider for setting the compression level
        label = new JLabel(mh.getMessage("compression"));
        c.anchor = GridBagConstraints.NORTHWEST;
        c.gridy = 3;
        c.weightx = 0;
        c.weighty = 1;
        c.gridwidth = 1;
        c.insets = new Insets(5, 10, 5, 10);
        gridBag.setConstraints(label, c);
        settingsPanel.add(label);
        compression = new JSlider(0, 9, 0);
        compression.addChangeListener(new ChangeListener() {
            // Event handler for the compression slider
            public void stateChanged(ChangeEvent e) {
                if (e.getSource() == compression) {
                    model.setJARCacheCompression(compression.getValue());
                }
            }
        });
        Hashtable labels = new Hashtable(2);
        labels.put(new Integer(0),
                   new JLabel(mh.getMessage("no_compression")));
        labels.put(new Integer(9),
                   new JLabel(mh.getMessage("high_compression")));
        compression.setLabelTable(labels);
        compression.setPaintLabels(true);
        compression.setMajorTickSpacing(1);
        compression.setPaintTicks(true);
        compression.setSnapToTicks(true);
        c.weightx = 1;
        c.gridheight = 2;
        c.gridwidth = GridBagConstraints.REMAINDER;
        c.fill = GridBagConstraints.HORIZONTAL;
        c.insets = new Insets(5, 0, 5, 10);
        gridBag.setConstraints(compression, c);
        settingsPanel.add(compression);

        add(settingsPanel);
//        add(Box.createRigidArea(new java.awt.Dimension(10,10)));
        reset();
    }

    /**
     * ActionListener interface implementation.
     * Responds to user selection/deselection of the debug checkbox
     *
     * @param actionEvent
     */
    public void actionPerformed(ActionEvent e) {
	try {
	    if (e.getSource() == enabled) {
		boolean enable = enabled.isSelected();
		enableComponents(enable);
		model.setCacheEnabled(enable);
	    } else if (e.getSource() == unlimited) {
		model.setCacheSize("0");
	    } else if (e.getSource() == maximum) {
		updateSize();
	    } else if (e.getSource() == clear) {
		MessageFormat formatter =
		    new MessageFormat(mh.getMessage("erase.confirm.text"));
		String dir = model.getCacheLocation();
		String message = formatter.format(new Object[]{dir});
		String title = mh.getMessage("erase.confirm.caption");
		int erase = DialogFactory.showConfirmDialog(this, message, title);
		if (erase == 0) {
			String sep = File.separator;
		    deleteFiles(new File(dir + sep + "jar"));
		    deleteFiles(new File(dir + sep + "file"));
		}
	    } else if (e.getSource() == units) {
		maximum.setSelected(true);
		updateSize();
	    } else if (e.getSource() == choose) {
		JFileChooser chooser = new JFileChooser();
		chooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
		chooser.setDialogTitle(mh.getMessage("select_title"));
		chooser.setApproveButtonText(mh.getMessage("select"));
		String tooltip = mh.getMessage("select_tooltip");
		chooser.setApproveButtonToolTipText(tooltip);
		char mnemonic = mh.getMessage("select_mnemonic").charAt(0);
		chooser.setApproveButtonMnemonic(mnemonic);
		File dir = new File(model.getCacheLocation());
		chooser.setCurrentDirectory(dir);
		if (chooser.showDialog(this, null) ==
		    JFileChooser.APPROVE_OPTION) {
		    String file;
		    try {
			file = chooser.getSelectedFile().getCanonicalPath();
		    } catch (IOException ioe) {
			file = chooser.getSelectedFile().getPath();
		    }
		    model.setCacheLocation(file);
		    location.setText(file);
		}
	    } else if (e.getSource() == view) {
	       new JarCacheViewer(model.getCacheLocation());
	    }
	}catch(Exception exc) {
	    DialogFactory.showExceptionDialog(this, exc);
	}
    }

    // Enable or disable all components in the panel except the
    // enabled checkbox.
    private void enableComponents(boolean enable) {
        setEnabled(location, enable);
        setEnabled(choose, enable);
        setEnabled(unlimited, enable);
        setEnabled(maximum, enable);
        setEnabled(size, enable);
        setEnabled(units, enable);
        setEnabled(compression, enable);
        setEnabled(clear, enable);
	setEnabled(view, enable);
    }

    // Enable or disable a specific component
    private void setEnabled(JComponent component, boolean b) {
        component.setEnabled(b);
        component.repaint();
    }

    private void updateSize() {

        // Get the size from the size field
        String str = size.getText().trim();

        // If the size field is empty, set the size to zero
        if (str.equals("")) {
            str = "0";
        }

        // Make sure the size is in ASCII
        long val = Long.valueOf(str).longValue();
        str = Long.toString(val);

        // Get the units
        int index = units.getSelectedIndex();
        String[] list = {"m", "k", ""};
        str += list[index];

        // Set the size on the model
        model.setCacheSize(str);
    }

    // Reset the panel to the saved values
    void reset() {

        // Reset cache enabled status
        boolean enable = model.getCacheEnabled();
        enabled.setSelected(enable);
        enableComponents(enable);

        // Reset cache location
        location.setText(model.getCacheLocation());

        // Reset cache size
        long val;
        try {
            String str = model.getCacheSize().trim();
            int unitIndex = 0;
            if (str.endsWith("M") || str.endsWith("m")) {
                // Size is in MegaBytes
                str = str.substring(0, str.length() - 1);
                unitIndex = 0;
            } else if (str.endsWith("K") || str.endsWith("k")) {
                // Size is in KiloBytes
                str = str.substring(0, str.length() - 1);
                unitIndex = 1;
            } else {
                // Size is in bytes
                unitIndex = 2;
            }
            val = Long.valueOf(str).longValue();
            if (val == 0) {
                unlimited.setSelected(true);
            } else {
                maximum.setSelected(true);
                units.setSelectedIndex(unitIndex);
            }
        } catch (NumberFormatException e) {
            val = 50;
            maximum.setSelected(true);
            units.setSelectedIndex(0);
        }
        if (val != 0) {
            // Display the size according to the user's locale
            NumberFormat formatter = NumberFormat.getInstance();
            formatter.setGroupingUsed(false);
            size.setText(formatter.format(val));
        }

        // Reset compression
        compression.setValue(model.getJARCacheCompression());
    }

    // Delete all files from the cache directory.  
    private void deleteFiles(File directory)
    {
	try {
	    if (directory.exists() && directory.isDirectory()) {
		File[] list = directory.listFiles();
		for (int i = 0; i < list.length; i++) {
		    list[i].delete();
		}
	    }
	} catch (Throwable e) {
	    DialogFactory.showExceptionDialog(this, e);	    
	}
    }

    JCheckBox enabled = null;
    JTextField location = null;
    JButton choose = null;
    JRadioButton unlimited = null;
    JRadioButton maximum = null;
    JTextField size = null;
    JComboBox units = null;
    JSlider compression = null;
    JButton clear = null;
    JButton view = null;

    // A document that only accepts numbers.  This prevents users
    // from entering non-numeric characters in the cache size field.
    private class SizeDocument extends PlainDocument {
        public void insertString(int offset, String string,
                                 AttributeSet attributes)
                                 throws BadLocationException {
            if (isNumeric(string)) {
                super.insertString(offset, string, attributes);
                maximum.setSelected(true);
                updateSize();
            } else {
                Toolkit.getDefaultToolkit().beep();
            }
        }

        public void remove(int offset, int length)
            throws BadLocationException {
            super.remove(offset, length);
            maximum.setSelected(true);
            updateSize();
        }

        private boolean isNumeric(String s) {
            try {
                Long.valueOf(s);
            } catch (NumberFormatException e) {
                return false;
            }
            return true;
        }
    }

    // Document to update the JAR cache location
    private class LocationDocument extends PlainDocument {
        public void insertString(int offset, String string,
                                 AttributeSet attributes)
                                 throws BadLocationException {
            super.insertString(offset, string, attributes);
            model.setCacheLocation(location.getText());
        }

        public void remove(int offset, int length)
            throws BadLocationException {
            super.remove(offset, length);
            model.setCacheLocation(location.getText());
        }
    }

    // JTextField returns a preferred width that depends on the length of
    // the text in the field.  This means the panel will lay out differently
    // depending on how long the JAR cache directory name is.  We don't want
    // that.  Instead, we subclass JTextField and return a very small
    // preferred width.  This way, the layout manager will resize the field
    // to fit the available space in the panel, instead of resizing the
    // panel to fit the length of the text.
    private class LocationField extends JTextField {
        public Dimension getPreferredSize() {
            Dimension d = super.getPreferredSize();
            d.width = 10;
            return d;
        }

    }

    private MessageHandler mh = new MessageHandler("jarcache");
}
