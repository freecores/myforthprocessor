/*
 * @(#)PrintDialog.java	1.24 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.print;

import java.awt.*;
import java.awt.event.*;
import java.awt.JobAttributes.*;
import java.awt.PageAttributes.*;
import java.io.File;
import java.net.URL;
import java.security.AccessController;
import java.security.PrivilegedAction;
import javax.swing.*;
import java.util.*;

 /**
  * A class which implements a cross-platform modal print dialog.
  *
  * @version     1.24, 01/23/03
  * @author      Amy Fowler
  * @author      David Mendenhall
  * @author      Michael Martak
  */
public class PrintDialog extends JDialog
    implements PrinterListUpdatable, ItemListener, ActionListener {

    private static String[] sortedPrinterList = null;

    private Frame parent;
    // The print control to invoke to create the print job from
    // dialog properties.
    private PrintControl printControl;

    private boolean okayedDialog = false;

    private String fileName = "";
    private int minPage = 0;
    private int maxPage = Integer.MAX_VALUE;

    //////////////// Print Dialog ////////////////
    private JTabbedPane tabs;
    private JPanel printPanel;
    private JLabel pagesLabel;
    private ButtonGroup selectionGroup;
    private JRadioButton selectionAllRB;
    private JRadioButton selectionRangeRB;
    private JPanel panelSelection;
    private NumericField selectionFromText;
    private JLabel selectionToLabel;
    private NumericField selectionToText;
    private JRadioButton selectionSelectionRB;
    private JLabel copiesLabel;
    private NumericField copiesText;
    private JCheckBox collateCB;
    private JLabel optionsLabel;
    private JCheckBox titleSheetCB;
    private JTextField titleSheetText;
    private JLabel commandsLabel;
    private JTextField commandsText;
    private JLabel printToLabel;
    private ButtonGroup printDestGroup;
    private JRadioButton printerRB;
    private JComboBox printerCmB;
    private JRadioButton fileRB;
    private JPanel spacingPanel1;
    private JPanel spacingPanel2;
    private JPanel propertiesPanel;
    private JLabel orientationLabel;
    private ButtonGroup orientationGroup;
    private JPanel portraitPanel;
    private JRadioButton portraitRB;
    private OrientationLabel portraitLabel;
    private JPanel landscapePanel;
    private JRadioButton landscapeRB;
    private OrientationLabel landscapeLabel;
    private JLabel colorLabel;
    private ButtonGroup colorGroup;
    private JRadioButton colorRB;
    private JRadioButton monochromeRB;
    private JLabel paperLabel;
    private JComboBox paperCmB;
    private JCheckBox duplexCB;
    private JCheckBox tumbleCB;
    private JLabel qualityLabel;
    private ButtonGroup qualityGroup;
    private JRadioButton highQualityRB;
    private JRadioButton normalQualityRB;
    private JRadioButton draftQualityRB;
    private JPanel spacingPanel3;
    private JPanel buttonPanel;
    private JPanel buttonLeftPanel;
    private JPanel buttonRightPanel;
    private JButton printButton;
    private JButton cancelButton;
    private JButton helpButton;
    private ImageIcon landscapeIcon;
    private ImageIcon portraitIcon;

    private static ResourceBundle messageRB;
    private static final MediaString[] ALL_SIZES =
        new MediaString[PrintControl.SIZES.length];
    private static final DefaultComboBoxModel ALL_SIZES_MODEL;

    private static final class MediaString {
        public final MediaType media;
        public final String localizedString;

        public MediaString(MediaType media) {
	    this.media = media;
	    localizedString = getMsg(media.toString());
	}
        public boolean equals(Object rhs) {
	    if (!(rhs instanceof MediaString)) {
	        return false;
	    } else {
	        return (media == ((MediaString)rhs).media);
	    }
	}
        public int hashCode() {
	    return media.hashCode();
	}
        public String toString() {
	    return localizedString;
	}
    }

    static {
        // Initialize resources
        initResource();
        // Generate localized size strings
        for (int i = 0; i < ALL_SIZES.length; i++) {
            ALL_SIZES[i] = new MediaString(PrintControl.SIZES[i]);
        }
	ALL_SIZES_MODEL = new DefaultComboBoxModel(ALL_SIZES);
    }

    /**
     * Initialize ResourceBundle
     */
    static void initResource() {
        java.security.AccessController.doPrivileged(
            new java.security.PrivilegedAction() {
                public Object run() {
                    try {
                        messageRB =
                        ResourceBundle.getBundle(
                            "sun.awt.print.resources.pdialog");
                        return null;
                    } catch (MissingResourceException e) {
                        throw new Error(
                        "Fatal: Resource for PrintDialog is missing.");
                    }
                }
            }
        );
    }

    /**
     * get message string from resource
     */
    static String getMsg(String key) {
        try {
            return messageRB.getString(key);
        } catch (MissingResourceException e) {
            throw new Error(
                "Fatal: Resource for PrintDialog is broken, There is no "
                + key + " key in resource");
        }
    }

    /**
     * get mnemonic char from resource
     */
    static char getMnemonic(String key) {
        String str = getMsg(key+".mnemonic");
        if (str != null && str.length() > 0) {
            return str.charAt(0);
        } else {
            return (char)0;
        }
    }

    /**
     * get message string from resource
     */
    static URL getImageResource(String key) {
        try {
            return PrintDialog.class.getResource("resources/" + key);
        } catch (MissingResourceException e) {
            throw new Error(
                "Fatal: Resource for PrintDialog is broken, There is no "
                + key + " key in resource");
        }
    }

    /**
     * Creates a modal cross-platform print dialog.
     */
    public PrintDialog(Frame parent, PrintControl printControl) {
        super(parent, getMsg("dialog.title"), true);
        this.parent = parent;
        this.printControl = printControl;
        setDefaultCloseOperation(HIDE_ON_CLOSE);

        // Set main layout
        getContentPane().setLayout(new BorderLayout(6,6));
        tabs = new JTabbedPane();
        getContentPane().add("Center",tabs);
        printPanel = new JPanel(new GridBagLayout());
        initPrintPanel();
        tabs.add(printPanel);
        propertiesPanel = new JPanel(new GridBagLayout());
        initPropertiesPanel();
        tabs.add(propertiesPanel);
        tabs.setSelectedIndex(0);
        tabs.setSelectedComponent(printPanel);
        tabs.setTitleAt(0, getMsg("tab.print"));
        tabs.setTitleAt(1, getMsg("tab.properties"));
        buttonPanel = new JPanel(new GridLayout(1,1,6,6));
        getContentPane().add("South",buttonPanel);
        buttonLeftPanel = new JPanel(new FlowLayout(FlowLayout.CENTER,5,5));
        buttonPanel.add(buttonLeftPanel);
        // 4425368 - Allow Print/Save buttons to expand to fit Asian languges, use FlowLayout instead of GridLayout.
        buttonRightPanel = new JPanel(new FlowLayout());
        buttonPanel.add(buttonRightPanel);
        buttonRightPanel.add(new JPanel()); // Temp. until help button works
        printButton = new JButton(getMsg("button.print"));
        printButton.setMnemonic(getMnemonic("button.print"));
        printButton.addActionListener(this);
        buttonRightPanel.add(printButton);
        cancelButton = new JButton(getMsg("button.cancel"));
        cancelButton.addActionListener(this);
        buttonRightPanel.add(cancelButton);
        helpButton = new JButton(getMsg("button.help"));
        helpButton.addActionListener(this);
        //buttonRightPanel.add(helpButton);
        // Adjust combo box sizes
        paperCmB.setPreferredSize(new Dimension(370, 20));
        printerCmB.setPreferredSize(new Dimension(100, 20));
        setResizable(false);
        getRootPane().setDefaultButton(printButton);
        setDefaultPrinter();
        pack();
        setLocationRelativeTo(parent);
    }

    /**
     * Adds component to dialog with specified gridbag constraints.
     */
    private void addComponent(JComponent comp, Container cont,
        int gridx, int gridy, int gridw, int gridh, int weightx,
        int weighty, int anchor, int fill,
        Insets insets) {
        GridBagConstraints c = new GridBagConstraints();
        c.gridx = gridx;
        c.gridy = gridy;
        c.gridwidth = gridw;
        c.gridheight = gridh;
        c.anchor = anchor;
        c.fill = fill;
        c.insets = insets;
        c.weightx = weightx;
        c.weighty = weighty;
        cont.add(comp,c);
    }

    private void initPrintPanel() {
        pagesLabel = new JLabel(getMsg("label.pages"));
        addComponent(pagesLabel, printPanel, 0, 0, 1, 1, 1, 1,
            GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
            new Insets(3, 6, 0, 0));
        selectionGroup = new ButtonGroup();
        selectionAllRB = new JRadioButton(getMsg("radiobutton.all"));
        selectionAllRB.setMnemonic(getMnemonic("radiobutton.all"));
        addComponent(selectionAllRB, printPanel, 1, 0, 1, 1, 1, 1,
            GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
            new Insets(3, 6, 0, 0));
        selectionGroup.add(selectionAllRB);
        selectionRangeRB = new JRadioButton(getMsg("radiobutton.range"));
        selectionRangeRB.setMnemonic(getMnemonic("radiobutton.range"));
        selectionRangeRB.addItemListener(this);
        addComponent(selectionRangeRB, printPanel, 1, 1, 1, 1, 1, 1,
            GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
            new Insets(0, 6, 0, 0));
            selectionGroup.add(selectionRangeRB);
        selectionSelectionRB = new JRadioButton(getMsg("radiobutton.selection"));
        selectionSelectionRB.setMnemonic(getMnemonic("radiobutton.selection"));
        addComponent(selectionSelectionRB, printPanel, 1, 2, 1, 1, 1, 1,
            GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
            new Insets(0, 6, 5, 0));
        selectionGroup.add(selectionSelectionRB);
        // Selection panel
        panelSelection = new JPanel(new GridLayout(1, 1, 0, 0));
        addComponent(panelSelection, printPanel, 2, 1, 1, 1, 0, 1,
            GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
            new Insets(0, 0, 0, 0));
        selectionFromText = new NumericField();
        selectionFromText.setEnabled(false);
        panelSelection.add(selectionFromText);
        selectionToLabel = new JLabel(getMsg("label.selection"), JLabel.CENTER);
        selectionToLabel.setForeground(selectionSelectionRB.getForeground());
        panelSelection.add(selectionToLabel);
        selectionToText = new NumericField();
        selectionToText.setEnabled(false);
        panelSelection.add(selectionToText);
        //
        copiesText = new NumericField();
        addComponent(copiesText, printPanel, 1, 3, 1, 1, 1, 1,
            GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
            new Insets(0, 6, 5, 5));
        copiesLabel = new JLabel(getMsg("label.copies"));
        copiesLabel.setDisplayedMnemonic(getMnemonic("label.copies"));
        copiesLabel.setLabelFor(copiesText);
        addComponent(copiesLabel, printPanel, 0, 3, 1, 1, 1, 1,
            GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
            new Insets(0, 6, 6, 0));
        collateCB = new JCheckBox(getMsg("checkbox.collate"));
        collateCB.setMnemonic(getMnemonic("checkbox.collate"));
        addComponent(collateCB, printPanel, 2, 3, 1, 1, 1, 1,
            GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
            new Insets(0, 0, 6, 0));
        optionsLabel = new JLabel(getMsg("label.options"));
        addComponent(optionsLabel, printPanel, 0, 4, 1, 1, 1, 1,
            GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
            new Insets(0, 6, 0, 0));
        titleSheetCB = new JCheckBox(getMsg("checkbox.titlesheet"));
        titleSheetCB.setMnemonic(getMnemonic("checkbox.titlesheet"));
        titleSheetCB.addItemListener(this);
        addComponent(titleSheetCB, printPanel, 1, 4, 1, 1, 1, 1,
            GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
            new Insets(0, 6, 0, 0));
        titleSheetText = new JTextField();
        addComponent(titleSheetText, printPanel, 2, 4, 3, 1, 3, 1,
            GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
            new Insets(0, 0, 0, 6));
        commandsText = new JTextField();
        addComponent(commandsText, printPanel, 2, 5, 3, 1, 3, 1,
            GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
            new Insets(0, 0, 5, 6));
        commandsLabel = new JLabel(getMsg("label.commands"));
        commandsLabel.setDisplayedMnemonic(getMnemonic("label.commands"));
        commandsLabel.setLabelFor(commandsText);
        addComponent(commandsLabel, printPanel, 1, 5, 1, 1, 1, 1,
            GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
            new Insets(0, 6, 5, 0));
        printToLabel = new JLabel(getMsg("label.printdest"));
        addComponent(printToLabel, printPanel, 0, 6, 1, 1, 1, 1,
            GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
            new Insets(0, 6, 0, 0));
        printDestGroup = new ButtonGroup();
        printerRB = new JRadioButton(getMsg("radiobutton.printer"));
        printerRB.setMnemonic(getMnemonic("radiobutton.printer"));
        printerRB.addItemListener(this);
        addComponent(printerRB, printPanel, 1, 6, 1, 1, 1, 1,
            GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
            new Insets(0, 6, 0, 0));
        printDestGroup.add(printerRB);
        printerCmB = new JComboBox();
        printerCmB.setEditable(true);
	printerCmB.addItemListener(this);
        printerCmB.setModel(new LoadingPrintersCBModel());
        addComponent(printerCmB, printPanel, 2, 6, 3, 1, 1, 1,
            GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
            new Insets(0, 0, 0, 6));
        fileRB = new JRadioButton(getMsg("radiobutton.file"));
        fileRB.setMnemonic(getMnemonic("radiobutton.file"));
        fileRB.addItemListener(this);
        addComponent(fileRB, printPanel, 1, 7, 1, 1, 1, 1,
            GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
            new Insets(0, 6, 5, 0));
        printDestGroup.add(fileRB);
        spacingPanel1 = new JPanel();
        addComponent(spacingPanel1, printPanel, 3, 1, 0, 1, 50, 1,
            GridBagConstraints.CENTER, GridBagConstraints.NONE,
            new Insets(0, 0, 0, 0));
        spacingPanel2 = new JPanel();
        addComponent(spacingPanel2, printPanel, 0, 8, 1, 2, 0, 80,
            GridBagConstraints.CENTER, GridBagConstraints.NONE,
            new Insets(0, 0, 0, 0));
    }

    private void initPropertiesPanel() {
        orientationLabel = new JLabel(getMsg("label.orientation"));
        addComponent(orientationLabel, propertiesPanel, 0, 0, 1, 1, 1, 0,
            GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
            new Insets(6, 6, 0, 0));
        orientationGroup = new ButtonGroup();
        // Portrait panel
        portraitPanel = new JPanel(new FlowLayout(FlowLayout.LEFT, 0, 0));
        addComponent(portraitPanel, propertiesPanel, 1, 0, 1, 1, 1, 0,
            GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
            new Insets(6, 1, 0, 0));
        portraitRB = new JRadioButton();
        portraitRB.setHorizontalAlignment(JRadioButton.CENTER);
        portraitPanel.add(portraitRB);
        orientationGroup.add(portraitRB);
        portraitLabel = new OrientationLabel("label.portrait", portraitRB);
        portraitLabel.setForeground(portraitRB.getForeground());
        portraitIcon = new ImageIcon(getImageResource("orientPortrait.gif"));
        portraitLabel.setIcon(portraitIcon);
        portraitPanel.add(portraitLabel);
        // Landscape panel
        landscapePanel = new JPanel(new FlowLayout(FlowLayout.LEFT, 0, 0));
        addComponent(landscapePanel, propertiesPanel, 1, 1, 1, 1, 0, 0,
            GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
            new Insets(0, 1, 0, 0));
        landscapeRB = new JRadioButton();
        landscapeRB.setHorizontalAlignment(JRadioButton.CENTER);
        landscapePanel.add(landscapeRB);
        orientationGroup.add(landscapeRB);
        landscapeLabel = new OrientationLabel("label.landscape", landscapeRB);
        landscapeLabel.setForeground(landscapeRB.getForeground());
        landscapeIcon = new ImageIcon(getImageResource("orientLandscape.gif"));
        landscapeLabel.setIcon(landscapeIcon);
        landscapePanel.add(landscapeLabel);
        //
        colorLabel = new JLabel(getMsg("label.color"));
        addComponent(colorLabel, propertiesPanel, 0, 2, 1, 1, 1, 0,
            GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
            new Insets(0, 6, 0, 0));
        colorGroup = new ButtonGroup();
        colorRB = new JRadioButton(getMsg("radiobutton.color"));
        colorRB.setMnemonic(getMnemonic("radiobutton.color"));
        addComponent(colorRB, propertiesPanel, 1, 2, 1, 1, 1, 0,
            GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
            new Insets(6, 6, 0, 0));
        colorGroup.add(colorRB);
        monochromeRB = new JRadioButton(getMsg("radiobutton.monochrome"));
        monochromeRB.setMnemonic(getMnemonic("radiobutton.monochrome"));
        addComponent(monochromeRB, propertiesPanel, 1, 3, 1, 1, 1, 0,
            GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
            new Insets(0, 6, 5, 0));
        colorGroup.add(monochromeRB);
        paperCmB = new JComboBox();
        paperCmB.setModel(ALL_SIZES_MODEL);
        addComponent(paperCmB, propertiesPanel, 1, 4, 2, 1, 60, 0,
            GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
            new Insets(0, 6, 0, 0));
        paperLabel = new JLabel(getMsg("label.paper"));
        paperLabel.setDisplayedMnemonic(getMnemonic("label.paper"));
        paperLabel.setLabelFor(paperCmB);
        addComponent(paperLabel, propertiesPanel, 0, 4, 1, 1, 1, 0,
            GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
            new Insets(0, 6, 0, 0));
        duplexCB = new JCheckBox(getMsg("checkbox.duplex"));
        duplexCB.setMnemonic(getMnemonic("checkbox.duplex"));
        duplexCB.addItemListener(this);
        addComponent(duplexCB, propertiesPanel, 1, 5, 1, 1, 0, 0,
            GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
            new Insets(0, 6, 0, 0));
        tumbleCB = new JCheckBox(getMsg("checkbox.tumble"));
        tumbleCB.setMnemonic(getMnemonic("checkbox.tumble"));
        addComponent(tumbleCB, propertiesPanel, 1, 6, 1, 1, 0, 0,
            GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
            new Insets(0, 22, 5, 0));
        qualityLabel = new JLabel(getMsg("label.quality"));
        addComponent(qualityLabel, propertiesPanel, 0, 7, 1, 1, 1, 0,
            GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
            new Insets(0, 6, 0, 0));
        qualityGroup = new ButtonGroup();
        highQualityRB = new JRadioButton(getMsg("radiobutton.highq"));
        highQualityRB.setMnemonic(getMnemonic("radiobutton.highq"));
        addComponent(highQualityRB, propertiesPanel, 1, 7, 1, 1, 1, 0,
            GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
            new Insets(0, 6, 0, 0));
        qualityGroup.add(highQualityRB);
        normalQualityRB = new JRadioButton(getMsg("radiobutton.normalq"));
        normalQualityRB.setMnemonic(getMnemonic("radiobutton.normalq"));
        addComponent(normalQualityRB, propertiesPanel, 1, 8, 1, 1, 1, 0,
            GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
            new Insets(0, 6, 0, 0));
        qualityGroup.add(normalQualityRB);
        draftQualityRB = new JRadioButton(getMsg("radiobutton.draftq"));
        draftQualityRB.setMnemonic(getMnemonic("radiobutton.draftq"));
        addComponent(draftQualityRB, propertiesPanel, 1, 9, 1, 1, 0, 0,
            GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
            new Insets(0, 6, 0, 0));
        qualityGroup.add(draftQualityRB);
        spacingPanel3 = new JPanel();
        addComponent(spacingPanel3, propertiesPanel, 3, 0, 1, 1, 100, 0,
            GridBagConstraints.CENTER, GridBagConstraints.NONE,
            new Insets(0, 0, 0, 0));
    }

    private void setDefaultPrinter() {
        String defaultPrinter = printControl.getDefaultPrinterName();
        if (sortedPrinterList == null) {
            if (defaultPrinter != null) {
                printerCmB.insertItemAt(defaultPrinter,0);
            }
        } else {
            printerCmB.setModel(
                new DefaultComboBoxModel(sortedPrinterList));
            if (defaultPrinter != null) {
                printerCmB.setSelectedItem(defaultPrinter);
            }
        }
	if (defaultPrinter != null) {
	    PrinterCapabilities caps =
	        new PrinterCapabilities(defaultPrinter);
	    updatePrinterCaps((printControl.getCapabilities(caps))
			      ? caps : null);
	} else {
	    updatePrinterCaps(null);
	}
    	printControl.getPrinterList(this); // returns immediately
    }

    /**
     * Display the print dialog.
     */
    public boolean display() {
        setVisible(true);

        return okayedDialog;
    }

    /**
     * Return the current print destination type.
     */
    public SidesType getSides() {
        if (duplexCB.isSelected()) {
            if (tumbleCB.isSelected()) {
                return SidesType.TWO_SIDED_SHORT_EDGE;
            } else {
                return SidesType.TWO_SIDED_LONG_EDGE;
            }
        } else {
            return SidesType.ONE_SIDED;
        }
    }
    public void setSides(SidesType sides) {
        boolean twoSided = (sides != SidesType.ONE_SIDED);
        duplexCB.setSelected(twoSided);
        if (twoSided) {
            tumbleCB.setSelected(sides == SidesType.TWO_SIDED_SHORT_EDGE);
        }
    }

    /**
     * Return the document title
     */
    public String getDocumentTitle() {
        if (titleSheetCB.isSelected()) {
            return titleSheetText.getText();
        } else {
            return null;
        }
    }
    public void setDocumentTitle(String title) {
        boolean hasTitle = (title != null);
        titleSheetCB.setSelected(hasTitle);
        if (hasTitle) {
            titleSheetText.setText(title);
        } else {
            titleSheetText.setText("");
        }
    }

    /**
     * Return the current print destination type.
     */
    public DestinationType getDestType() {
        if (printerRB.isSelected()) {
            return DestinationType.PRINTER;
        } else { // if (fileRB.isSelected())
            return DestinationType.FILE;
        }
    }
    public void setDestType(DestinationType destination) {
        if (destination == DestinationType.PRINTER) {
            printerRB.setSelected(true);
        } else { // if (destination == DestinationType.FILE)
            fileRB.setSelected(true);
        }
    }

    public String getDestString(DestinationType destination) {
        if (destination == DestinationType.PRINTER) {
            return printerCmB.getSelectedItem().toString();
        } else { // if (destination == DestinationType.FILE)
            return fileName;
        }
    }
    public void setDestString(DestinationType destination, String str) {
        if (destination == DestinationType.PRINTER) {
            printerCmB.setSelectedItem(str);
        } else { // if (destination == DestinationType.FILE)
            fileName = (str == null ? "" : str);
        }
    }

    public String getOptionsString() {
        return commandsText.getText();
    }
    public void setOptionsString(String options) {
        commandsText.setText(options);
    }

    public DefaultSelectionType getPrintRange() {
        if (selectionAllRB.isSelected()) {
            return DefaultSelectionType.ALL;
        } else if (selectionRangeRB.isSelected()) {
            return DefaultSelectionType.RANGE;
        } else { // if (selectionSelectionRB.isSelected())
            return DefaultSelectionType.SELECTION;
        }
    }
    public void setPrintRange(DefaultSelectionType defaultSelection) {
        if (defaultSelection == DefaultSelectionType.ALL) {
            selectionAllRB.setSelected(true);
        } else if (defaultSelection == DefaultSelectionType.RANGE) {
            selectionRangeRB.setSelected(true);
        } else { // if (defaultSelection == DefaultSelectionType.SELECTION) {
            selectionSelectionRB.setSelected(true);
        }
    }

    public void setMinMaxPage(int minPage, int maxPage) {
        if (this.minPage == minPage && this.maxPage == maxPage)
            return;

        this.minPage = minPage;
        this.maxPage = maxPage;

        if (getFromPage() < minPage)
            setFromPage(minPage);
        if (getToPage() > maxPage)
            setToPage(maxPage);
    }

    public int getFromPage() {
        try {
            return selectionFromText.getValue();
        } catch (NumberFormatException e) {
            return 0;
        }
    }
    public void setFromPage(int fromPage) {
        selectionFromText.setValue(fromPage);
    }

    public int getToPage() {
        try {
            return selectionToText.getValue();
        } catch (NumberFormatException e) {
            return 0;
        }
    }
    public void setToPage(int toPage) {
        selectionToText.setValue(toPage);
    }

    public int getCopies() {
        try {
            return copiesText.getValue();
        } catch (NumberFormatException e) {
            return 0;
        }
    }
    public void setCopies(int copies) {
        copiesText.setValue(copies);
    }

    public boolean getCollate() {
        return collateCB.isSelected();
    }
    public void setCollate(boolean collate) {
        collateCB.setSelected(collate);
    }

    public MediaType getMedia() {
        return ((MediaString)paperCmB.getSelectedItem()).media;
    }
    public void setMedia(MediaType media) {
        paperCmB.setSelectedItem(new MediaString(media));
    }

    public OrientationRequestedType getOrientation() {
        if (portraitRB.isSelected()) {
            return OrientationRequestedType.PORTRAIT;
        }
        else { // if (landscapeRB.isSelected())
            return OrientationRequestedType.LANDSCAPE;
        }
    }
    public void setOrientation(OrientationRequestedType orientation) {
        if (orientation == OrientationRequestedType.PORTRAIT) {
            portraitRB.setSelected(true);
        } else { // if (orientation == OrientationRequestedType.LANDSCAPE)
            landscapeRB.setSelected(true);
        }
    }

    public ColorType getColor() {
        if (colorRB.isSelected()) {
            return ColorType.COLOR;
        } else { // if (monochromeRB.isSelected()) {
            return ColorType.MONOCHROME;
        }
    }
    public void setColor(ColorType color) {
        if (color == ColorType.COLOR) {
            colorRB.setSelected(true);
        } else { // if (color == ColorType.MONOCHROME)
            monochromeRB.setSelected(true);
        }
    }

    public PrintQualityType getQuality() {
        if (highQualityRB.isSelected()) {
            return PrintQualityType.HIGH;
        } else if (normalQualityRB.isSelected()) {
            return PrintQualityType.NORMAL;
        } else { // if (draftQualityRB.isSelected())
            return PrintQualityType.DRAFT;
        }
    }
    public void setQuality(PrintQualityType printQuality) {
        if (printQuality == PrintQualityType.HIGH) {
            highQualityRB.setSelected(true);
        } else if (printQuality == PrintQualityType.NORMAL) {
            normalQualityRB.setSelected(true);
        } else { // if (printQuality == PrintQualityType.DRAFT)
            draftQualityRB.setSelected(true);
        }
    }

    private void updatePrinterCaps(PrinterCapabilities caps) {
        if (caps != null) {
	    long lcaps = caps.getCapabilities();
	    if ((lcaps & PrinterCapabilities.COLOR) != 0) {
	        colorRB.setEnabled(true);
		monochromeRB.setEnabled(true);
	    } else {
	        monochromeRB.setSelected(true);
		colorRB.setEnabled(false);
		monochromeRB.setEnabled(false);
	    }
	    if ((lcaps & PrinterCapabilities.DUPLEX) != 0) {
	        duplexCB.setEnabled(true);
		tumbleCB.setEnabled(duplexCB.isSelected());
	    } else {
	        tumbleCB.setSelected(false);
		duplexCB.setSelected(false);
		tumbleCB.setEnabled(false);
		duplexCB.setEnabled(false);
	    }
	    if ((lcaps & PrinterCapabilities.MEDIA) != 0) {
	        DefaultComboBoxModel model = (DefaultComboBoxModel)
		    caps.getUserData();
		if (model == null) {
		    int[] sizes = caps.getSizes();
		    MediaString[] strings = new MediaString[sizes.length];
		    for (int i = 0; i < sizes.length; i++) {
		        strings[i] =
			    new MediaString(PrintControl.SIZES[sizes[i]]);
		    }
		    model = new DefaultComboBoxModel(strings);
		    caps.setUserData(model);
		}
		paperCmB.setModel(model);
	    } else {
	        paperCmB.setModel(ALL_SIZES_MODEL);
	    }
	    setMedia(PrintControl.SIZES[printControl.getMediaAttrib()]);
	} else {
	    // use defaults
	    colorRB.setEnabled(true);
	    monochromeRB.setEnabled(true);

	    paperCmB.setModel(ALL_SIZES_MODEL);

	    duplexCB.setEnabled(true);
	    tumbleCB.setEnabled(duplexCB.isSelected());
	}
	duplexCB.repaint();
	tumbleCB.repaint();
	colorRB.repaint();
	monochromeRB.repaint();
	paperCmB.repaint();
    }

    public void updatePrinterList(String[] sortedNames) {
        if (sortedNames != null && sortedNames == this.sortedPrinterList) {
            return;
        }

        this.sortedPrinterList = sortedNames;
        EventQueue.invokeLater(new Runnable() {
            public void run() {
                Object text = printerCmB.getEditor().getItem();
		ComboBoxModel newModel = (sortedPrinterList != null)
		    ? new DefaultComboBoxModel(sortedPrinterList)
		    : new DefaultComboBoxModel();
		printerCmB.setModel(newModel);
                if (text != null) {
                    printerCmB.setSelectedItem(text);
                    printerCmB.getEditor().setItem(text);
                }
                printerCmB.repaint();
            }
        });
    }

    public void itemStateChanged(ItemEvent event) {
        Object source = event.getSource();
        if (source == selectionRangeRB) {
            boolean state = selectionRangeRB.isSelected();
            selectionFromText.setEnabled(state);
            selectionToText.setEnabled(state);
        } else if (source == titleSheetCB) {
            titleSheetText.setEnabled(titleSheetCB.isSelected());
        } else if ((source == printerRB) || (source == fileRB)) {
            boolean state = printerRB.isSelected();
            printerCmB.setEnabled(state);
            String key = (state ? "button.print" : "button.save");
            printButton.setText(getMsg(key));
            printButton.setMnemonic(getMnemonic(key));
        } else if (source == duplexCB) {
            tumbleCB.setEnabled(duplexCB.isSelected());
        } else /* if (source == printerCmB) */ {
	    String text = (String)printerCmB.getEditor().getItem();
	    if (text != null) {
	        PrinterCapabilities caps = new PrinterCapabilities(text);
		updatePrinterCaps((printControl.getCapabilities(caps))
				  ? caps : null);
	    } else {
	        updatePrinterCaps(null);
	    }
	}
    }

    public void actionPerformed(ActionEvent event) {
        Object source = event.getSource();
        if (source == printButton) {
            if (printerRB.isSelected()) {
                actionPrint();
            } else { // if (fileRB.isSelected)
                actionSave();
            }
        } else if (source == cancelButton) {
            setVisible(false);
        } else { // if (source == helpButton)
            // behavior not yet defined
        }
    }

    private void actionPrint() {
        if (getPrintRange() == DefaultSelectionType.RANGE) {
            int from = getFromPage();
            if (from < minPage || from > maxPage) {
                selectionFromText.selectAll();
                selectionFromText.requestFocus();
                Toolkit.getDefaultToolkit().beep();
                return;
            }

            int to = getToPage();
            if (to < minPage || to > maxPage || to < from) {
                selectionToText.selectAll();
                selectionToText.requestFocus();
                Toolkit.getDefaultToolkit().beep();
                return;
            }
        }

        if (getCopies() < 1) {
            copiesText.selectAll();
            copiesText.requestFocus();
            Toolkit.getDefaultToolkit().beep();
            return;
        }

	okayedDialog = true;
	setVisible(false);
    }

    public void setPrintToFileEnabled(boolean enable) {
        fileRB.setEnabled(enable);
    }

    private void actionSave() {
        SaveFileChooser chooser =
	    (SaveFileChooser)AccessController.doPrivileged(
                new PrivilegedAction() {
		    public Object run() {
                        return new SaveFileChooser(fileName);
		    }
	    });
        if (chooser.showSaveDialog(parent) == JFileChooser.APPROVE_OPTION) {
            fileName = chooser.getFileName();
            actionPrint();
        }
    }

    class SaveFileChooser extends JFileChooser {
        private JDialog dialog;
        private int returnValue = ERROR_OPTION;
        public SaveFileChooser(String filename) {
            super(filename);

	    setCurrentDirectory(new File(System.getProperty("user.dir", ".")));
	    setSelectedFile(new File(filename));
        }
        public int showDialog(Component parent, String approveButtonText) {
            if(approveButtonText != null) {
                setApproveButtonText(approveButtonText);
                setDialogType(CUSTOM_DIALOG);
            }

            String title = getDialogTitle();

            if(title == null) {
                title = getUI().getDialogTitle(this);
            }

            dialog = new JDialog((Frame)parent, title, true);
            Container contentPane = dialog.getContentPane();
            contentPane.setLayout(new BorderLayout());
            contentPane.add(this, BorderLayout.CENTER);

            dialog.pack();
            dialog.setLocationRelativeTo(parent);

            rescanCurrentDirectory();

            dialog.show();
            return returnValue;
        }
        public void approveSelection() {
            returnValue = APPROVE_OPTION;
            super.approveSelection();
            dialog.dispose();
        }
        public void cancelSelection() {
            returnValue = CANCEL_OPTION;
            super.cancelSelection();
            dialog.dispose();
        }
        public String getFileName() {
            return getSelectedFile().getAbsolutePath();
        }
    }

    class OrientationLabel extends JLabel
        implements ItemListener, FocusListener, MouseListener, ActionListener {
        public OrientationLabel(String text,JRadioButton buddy) {
            super(getMsg(text));
            char mnemonic = getMnemonic(text);
            setDisplayedMnemonic(mnemonic);
            setLabelFor(buddy);
            this.buddy = buddy;
            buddy.addItemListener(this);
            buddy.addFocusListener(this);
            addMouseListener(this);
    	    ActionMap map = SwingUtilities.getUIActionMap(this);
		    registerKeyboardAction(this, KeyStroke.getKeyStroke(mnemonic,
                ActionEvent.ALT_MASK, true), WHEN_FOCUSED);
        }

        public void reshape(int x, int y, int width, int height) {
            super.reshape(x, y, width+3, buddy.getHeight());
        }

        protected void paintComponent(Graphics g) {
            super.paintComponent(g);
            if (buddy.hasFocus() && buddy.isFocusPainted()) {
                Rectangle t = computeLayout(g);
                g.setColor(UIManager.getColor("RadioButton.focus"));
                g.drawRect(t.x,t.y-1,t.width+1,t.height+1);
            }
        }

        public boolean isFocusTraversable() { return false; }

        public void focusGained(FocusEvent e) {
            repaint();
        }
        public void focusLost(FocusEvent e) {
            repaint();
        }
        public void itemStateChanged(ItemEvent e) {
            repaint();
        }
        public void mouseClicked(MouseEvent e) {
            click();
        }
        public void mouseEntered(MouseEvent e) {
        }
        public void mouseExited(MouseEvent e) {
        }
        public void mousePressed(MouseEvent e) {
        }
        public void mouseReleased(MouseEvent e) {
        }
        public void actionPerformed(ActionEvent e) {
            click();
        }

        private void click() {
            buddy.requestFocus();
            buddy.doClick();
        }

        private Rectangle computeLayout(Graphics g) {
            Dimension size = getSize();
            Font f = getFont();
            g.setFont(f);
            FontMetrics fm = g.getFontMetrics();
            Rectangle viewRect = new Rectangle(getSize());
            Rectangle iconRect = new Rectangle();
            Rectangle textRect = new Rectangle();
            String text = SwingUtilities.layoutCompoundLabel(
                this, fm, getText(), getIcon(),
                getVerticalAlignment(), getHorizontalAlignment(),
                getVerticalTextPosition(), getHorizontalTextPosition(),
                viewRect, iconRect, textRect,
                ((Integer)UIManager.get("RadioButton.textIconGap")).intValue()
            );
            return textRect;
        }

        private JRadioButton buddy;
    }

    class LoadingPrintersCBModel extends DefaultComboBoxModel {
        public LoadingPrintersCBModel() {
            super(new String[] { getMsg("combobox.printers") });
        }
        public void setSelectedItem(Object anObject)
        {
            if (!getMsg("combobox.printers").equals(anObject)) {
                super.setSelectedItem(anObject);
            } else {
                // Do nothing
            }
        }
    }

    public static void main(String[] args) {
        JFrame frame = new JFrame();
        PrintDialog dlg = new PrintDialog(frame, null);
        dlg.addWindowListener(new WindowAdapter() {
            public void windowClosing(WindowEvent event) {
                System.exit(0);
            }
        });
        dlg.setVisible(true);
    }
}
