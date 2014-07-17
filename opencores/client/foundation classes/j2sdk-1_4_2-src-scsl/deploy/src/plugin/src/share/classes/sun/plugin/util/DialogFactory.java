/*
 * @(#)DialogFactory.java	1.74 03/03/06
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.util;

import java.awt.Component;
import java.awt.BorderLayout;
import java.awt.FlowLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Toolkit;
import java.awt.Cursor;
import java.awt.Graphics;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.event.ActionListener;
import java.awt.event.MouseListener;
import java.awt.event.KeyEvent;
import java.awt.event.MouseEvent;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Array;
import java.net.URL;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.security.PrivilegedExceptionAction;
import javax.swing.BorderFactory;
import javax.swing.JComponent;
import javax.swing.Icon;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JOptionPane;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.SwingUtilities;
import javax.swing.LookAndFeel;
import javax.swing.border.Border;
import javax.swing.border.EmptyBorder;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.awt.Dialog;
import sun.plugin.resources.ResourceHandler;


/**
* DialogFactory provides confirmation, input, exception and message dialogboxes
*
* @version 1.0 
* @Author Devananda Jayaraman
**/

public class DialogFactory 
{
    // Dialog message type
    public static final int ERROR_MESSAGE = 1;
    public static final int INFORMATION_MESSAGE = 2;
    public static final int WARNING_MESSAGE = 3;
    public static final int QUESTION_MESSAGE = 4;
    public static final int PLAIN_MESSAGE = 5;

    // Dialog title    
    private static String confirmDialogTitle;
    private static String inputDialogTitle;
    private static String messageDialogTitle;
    private static String exceptionDialogTitle;
    private static String optionDialogTitle;
    private static String aboutDialogTitle;
    private static String javaHomeLink;    

    private static final String LEGAL_NOTE_BASE = "sun/plugin/util/legal";

    static
    {
	confirmDialogTitle = ResourceHandler.getMessage("usability.confirmDialogTitle");
	inputDialogTitle = ResourceHandler.getMessage("usability.inputDialogTitle");
	messageDialogTitle = ResourceHandler.getMessage("usability.messageDialogTitle");
	exceptionDialogTitle = ResourceHandler.getMessage("usability.exceptionDialogTitle");
	optionDialogTitle = ResourceHandler.getMessage("usability.optionDialogTitle");
	aboutDialogTitle = ResourceHandler.getMessage("usability.aboutDialogTitle");
	javaHomeLink = ResourceHandler.getMessage("usability.java.home.link");
    }
    
    // Loads the icon to be used in the dialog
    public static Icon loadIcon()
    {
	Icon icon = null;
	try
	{
	    icon = (Icon) AccessController.doPrivileged(
			new PrivilegedAction() 
			{
			    public Object run() 
			    {
				URL url = ClassLoader.getSystemResource("sun/plugin/util/logo.jpg");

				ImageIcon imageIcon = new ImageIcon(url);

				return imageIcon;
			    }
			});
	}
	catch(Throwable e)
	{
	    e.printStackTrace();
	}
	
	return icon;
    }

    public static void showAboutPluginDialog() {
	try {
	    PluginSysUtil.execute(new PluginSysAction() {
		public Object execute() throws Exception {
		    showAboutPluginDialogImpl();
		    return null;
		}});
	}
	catch(Exception e) {
	    e.printStackTrace();
	    // should never happen
	    assert(false);
	}
    }

    private static void showAboutPluginDialogImpl()
    {  
	AccessController.doPrivileged(
	    new PrivilegedAction() 
	    {
		public Object run() 
		{
		    LookAndFeel lookAndFeel = null;

		    try
		    {
			// Change theme
			lookAndFeel = PluginUIManager.setTheme();

    			JOptionPane pane = new JOptionPane();
			pane.setMessageType(JOptionPane.PLAIN_MESSAGE);
			JButton btnClose = new JButton(ResourceHandler.getMessage("about.option.close"));
			btnClose.setMnemonic(java.awt.event.KeyEvent.VK_C);
			pane.setOptions(new Object[] {btnClose});
			pane.setWantsInput(false);
			pane.setBorder(BorderFactory.createEmptyBorder(4, 4, 4, 4));
			pane.setInitialValue(btnClose);
			showDialog(pane, null, aboutDialogTitle, createInfoPane(), false);

			return null;
		    }
		    finally
		    {
			// Restore theme
			PluginUIManager.restoreTheme(lookAndFeel);
		    }
		}
	    });
    }
    
    private static native void launchLink(String link);

    private static Cursor handCursor = new Cursor(Cursor.HAND_CURSOR); 

    static class LinkMouseListener implements MouseListener {
	private Color	txtColor =null;
	private Cursor	lblCursor = null;
	private JLabel	label;
	
	public LinkMouseListener(JLabel label) {
	    this.label = label;
	}    

	public void mouseClicked(MouseEvent e) {
	    final String url = label.getText();
	    Thread t = new Thread(new Runnable() {
		public void run() {
		    launchLink(url);
		}
	    });
	    t.start();
	}
	 
	public void mouseEntered(MouseEvent e) {
	    txtColor = label.getForeground();
	    lblCursor = label.getCursor();

	    label.setForeground(Color.RED);
	    label.setCursor(handCursor);
	}

	public void mouseExited(MouseEvent e) {
	    if(txtColor != null)
	        label.setForeground(txtColor);

	    if(lblCursor != null)
	        label.setCursor(lblCursor);
	}
	
	public void mousePressed(MouseEvent e) {
	
	} 
	public void mouseReleased(MouseEvent e) {
	}
    }

    static class VersionLabel extends JLabel{
	private static String VERSION;
	static {
	    VERSION = ResourceHandler.getMessage("about.java.version") + " " +
		System.getProperty("java.version");
	}

	public VersionLabel(Icon image){
	    super(image);
	    int h = image.getIconHeight();
	    int w = image.getIconWidth();
	    setSize(w, h);
	}

	public void paint(Graphics g){
	    super.paint(g);
	    g.setFont(new Font("SunSans", Font.PLAIN, 11));
	    g.setColor(new Color(8979191));
	    g.drawString(VERSION, 15, 20);
	}
    }


    private static JPanel createInfoPane() {
	JPanel pane = new JPanel(new BorderLayout());
	pane.setForeground(Color.WHITE);
	pane.setBackground(Color.WHITE);
	pane.setBorder(BorderFactory.createLineBorder(Color.BLACK));
	try {
	    URL url = ClassLoader.getSystemResource("sun/plugin/util/about_top.jpg");
	    ImageIcon imageTopIcon = new ImageIcon(url);
	    JTextArea textInfo = new JTextArea(ResourceHandler.getMessage("about.legal.note"), 0, 1);
	    textInfo.setEditable(false);
	    textInfo.setLineWrap(true);
	    textInfo.setWrapStyleWord(true);
	    textInfo.setBorder(BorderFactory.createEmptyBorder(0, 13, 0, 4));
	    textInfo.invalidate();
	    url = ClassLoader.getSystemResource("sun/plugin/util/about_bottom.jpg");
	    ImageIcon imageBottomIcon = new ImageIcon(url);
	    JLabel topImage = new VersionLabel(imageTopIcon);
	    topImage.setBorder(BorderFactory.createEmptyBorder(0, 0, 0, 0));
	    topImage.setOpaque(false);
	    int imgWidth = imageTopIcon.getIconWidth();

	    pane.add(BorderLayout.NORTH, topImage);
	    pane.add(BorderLayout.CENTER, textInfo);

	    JPanel bottomPane = new JPanel(new FlowLayout(FlowLayout.LEFT));
	    bottomPane.setBorder(BorderFactory.createEmptyBorder(0, 0, 0, 0));
	    JLabel promptLabel = new JLabel(ResourceHandler.getMessage("about.prompt.info"));
	    JLabel linkLabel = new JLabel(ResourceHandler.getMessage("about.home.link"));

	    linkLabel.addMouseListener(new LinkMouseListener(linkLabel));
	    JLabel javaIcon = new JLabel(imageBottomIcon); 
	    bottomPane.add(javaIcon);
	    JPanel labelPane = new JPanel();
	    labelPane.setForeground(Color.WHITE);
	    labelPane.setBackground(Color.WHITE);
	    promptLabel.setOpaque(false);
	    linkLabel.setOpaque(false);
	    linkLabel.setForeground(Color.BLUE);
	    int javaIconWidth = imageBottomIcon.getIconWidth();
	    Font font = linkLabel.getFont();
	    FontMetrics fm = linkLabel.getFontMetrics(font);

	    int linkWidth = fm.stringWidth(linkLabel.getText());
	    int linkHeight = fm.getMaxDescent() + fm.getMaxAscent();
	    font = promptLabel.getFont();
	    fm = promptLabel.getFontMetrics(font);
	    int promptWidth = fm.stringWidth(promptLabel.getText());

	    labelPane.add(promptLabel, BorderLayout.NORTH);
	    labelPane.add(linkLabel, BorderLayout.SOUTH);

	    int offset = 25;
	    if(imgWidth > (javaIconWidth + linkWidth + promptWidth)) {
		labelPane.setLayout(new FlowLayout(FlowLayout.LEFT));
		labelPane.add(promptLabel);
		labelPane.add(linkLabel);
	    } else {
		offset += linkHeight;
		labelPane.setLayout(new BorderLayout());
		labelPane.add(promptLabel, BorderLayout.NORTH);
		labelPane.add(linkLabel, BorderLayout.SOUTH);
	    }

	    JPanel layoutPane = new JPanel(new BorderLayout());
	    layoutPane.setBackground(Color.WHITE);
	    layoutPane.setLayout(new javax.swing.BoxLayout(layoutPane, javax.swing.BoxLayout.Y_AXIS));
	    layoutPane.add(javax.swing.Box.createVerticalStrut(imageBottomIcon.getIconHeight() - linkLabel.getFont().getSize() - offset));

	    layoutPane.add(labelPane);
	    bottomPane.add(layoutPane);
	    bottomPane.setBackground(Color.WHITE);
	    pane.add(BorderLayout.SOUTH, bottomPane);
	} catch(Exception e) {
//	    e.printStackTrace();
	}
		
	return pane;
    } 

    public static int showConfirmDialog(Object message)
    {
	return showConfirmDialog(null, message, confirmDialogTitle);
    }

    public static int showConfirmDialog(Component parentComponent, Object message)
    {
	return showConfirmDialog(parentComponent, message, confirmDialogTitle);
    }

    public static String showInputDialog(Object message)
    {
	return showInputDialog(null, message, inputDialogTitle);
    }

    public static String showInputDialog(Component parentComponent, Object message)
    {
	return showInputDialog(parentComponent, message, inputDialogTitle);
    }

    public static void showInformationDialog(Object message)
    {
	showInformationDialog(null, message, messageDialogTitle);
    }

    public static void showInformationDialog(Component parentComponent, Object message)
    {
	showInformationDialog(parentComponent, message, messageDialogTitle);
    }

    public static void showErrorDialog(String message)
    {
	showErrorDialog(null, message, messageDialogTitle);
    }

    public static void showErrorDialog(Component parentComponent, String message)
    {
	showErrorDialog(parentComponent, message, messageDialogTitle);
    }

    public static void showExceptionDialog(Throwable ex)
    {
	showExceptionDialog(null, ex, ex.toString(), exceptionDialogTitle);
    }

    public static void showExceptionDialog(Component parentComponent, Throwable ex)
    {
	showExceptionDialog(parentComponent, ex, ex.toString(), exceptionDialogTitle);
    }

    public static int showConfirmDialog(Object message, String title)
    {
	return showConfirmDialog(null, message, title);
    }

    
    public static int showConfirmDialog(final Component parentComponent, final Object message, final String title) {
	try {
	    return ((Integer)PluginSysUtil.execute(new PluginSysAction() {
		public Object execute() throws Exception {
		    return new Integer(showConfirmDialogImpl(parentComponent, message, title));
		}})).intValue();
	}
	catch(Exception e) {
	    // should never happen
	    assert(false);
	    return 0;
	}
    }

    private static int showConfirmDialogImpl(Component parentComponent, Object message, String title)
    {
	LookAndFeel lookAndFeel = null;

	try
	{
	    // Change theme
	    lookAndFeel = PluginUIManager.setTheme();

	    JButton yes = new JButton(ResourceHandler.getMessage("usability.confirm.yes")); 
	    JButton no = new JButton(ResourceHandler.getMessage("usability.confirm.no")); 
	    yes.setMnemonic(ResourceHandler.getAcceleratorKey("usability.confirm.yes")); 
	    no.setMnemonic(ResourceHandler.getAcceleratorKey("usability.confirm.no")); 

	    Object[] options = { yes, no };

	    if(title == null)
		title = confirmDialogTitle;

	    // Popup YES/NO dialog, 
	    return showOptionDialog(parentComponent, QUESTION_MESSAGE, message, title, options, options[0]);
	}
	finally
	{
	    // Restore theme
	    PluginUIManager.restoreTheme(lookAndFeel);
	}
    }

    public static int showOptionDialog(Object message, String title, Object[] options, Object initValue)
    {
	return showOptionDialog(null, message, title, options, initValue);
    }


    public static int showOptionDialog(Component parentComponent, Object message, String title, Object[] options, Object initValue)
    {
	// Popup YES/NO dialog, 
	return showOptionDialog(parentComponent, PLAIN_MESSAGE, message, title, options, initValue);
    }

    public static int showOptionDialog(int messageType, Object message, String title, Object[] options, Object initValue)
    {
	return showOptionDialog(null, messageType, message, title, options, initValue);
    }

    public static int showOptionDialog(final Component parentComponent, final int messageType, 
	final Object message, final String title, final Object[] options, final Object initValue) {
	try {
	    return ((Integer)PluginSysUtil.execute(new PluginSysAction() {
		public Object execute() throws Exception {
		    return new Integer(showOptionDialogImpl(parentComponent, messageType, message, 
		    	title, options, initValue, false));
	    }})).intValue();
	}
	catch(Exception e) {
	    // should never happen
	    assert(false);
	    return 0;
	}
    }
    // Fix for BugId: 4803342 we load the dialog without displaying it so as to prevent a
    // dead lock in the class loader.
    public static void preLoadDialog(int messageType, Object message, String title, Object[] options, Object initValue) {
	showOptionDialogImpl(null, messageType, message, title, options, options[0], true);
    }
    
    private static int showOptionDialogImpl(Component parentComponent, int messageType, Object message, String title, Object[] options, Object initValue, boolean preload)
    {
	LookAndFeel lookAndFeel = null;

	try
	{
	    // Change theme
	    lookAndFeel = PluginUIManager.setTheme();

	    int selection = -1;

    	    JOptionPane pane = new JOptionPane();

	    switch (messageType)
	    {
		case DialogFactory.ERROR_MESSAGE: 
		    pane.setMessageType(JOptionPane.ERROR_MESSAGE);
		    break;
		case DialogFactory.INFORMATION_MESSAGE: 
		    pane.setMessageType(JOptionPane.INFORMATION_MESSAGE);
		    break;
		case DialogFactory.WARNING_MESSAGE: 
		    pane.setMessageType(JOptionPane.WARNING_MESSAGE);
		    break;
		case DialogFactory.QUESTION_MESSAGE: 
		    pane.setMessageType(JOptionPane.QUESTION_MESSAGE);
		    break;
		default:
    //		pane.setIcon(loadIcon());
		    break;
	    }

	    pane.setOptions(options);
	    pane.setInitialValue(initValue);
	    pane.setWantsInput(false);

	    Object msg = extractMessage(pane, message);
	    
	    if(title == null)
		title = optionDialogTitle;

	    if(showDialog(pane, parentComponent, title, msg, true, preload) == true)
	    {
		Object selectedValue = pane.getValue();
		if(selectedValue != null)
		{
		    for (int i=0; i < options.length; i++)
		    {
			if (options[i].equals(selectedValue))
			{
			    selection = i;
			    break;
			}
		    }
		
		    Trace.msgPrintln("dialogfactory.user.selected", new Object[] {new Integer(selection)});
		}
	    }

	    return selection;
	}
	finally
	{
	    // Restore theme
	    PluginUIManager.restoreTheme(lookAndFeel);
	}
    }

    public static String showInputDialog(Object message, String title)
    {
	return showInputDialog(null, message, title);
    }

    public static String showInputDialog(final Component parentComponent, final Object message, final String title) {

	try {
	    return (String)PluginSysUtil.execute(new PluginSysAction() {
		public Object execute() throws Exception {
		    return showInputDialogImpl(parentComponent, message, title);
		}});
	}
	catch(Exception e) {
	    // should never happen
	    assert(false);
	    return null;
	}
    }

    private static String showInputDialogImpl(Component parentComponent, Object message, String title)
    {
	LookAndFeel lookAndFeel = null;
	
	try
	{
	    // Change theme
	    lookAndFeel = PluginUIManager.setTheme();

	    String selection = null;
    	    JOptionPane pane = new JOptionPane();
	    pane.setMessageType(JOptionPane.QUESTION_MESSAGE);
	    pane.setOptionType(JOptionPane.OK_CANCEL_OPTION);
	    pane.setWantsInput(true);

	    Object msg = extractMessage(pane, message);

	    if (title == null)
		title = inputDialogTitle;

	    if(showDialog(pane, parentComponent, title, msg) == true)
	    {
		Object selectedValue = pane.getInputValue();
		if(selectedValue != null)
		{
    		    if(selectedValue instanceof String)
			selection = selectedValue.toString();
		
	    	    Trace.msgPrintln("dialogfactory.user.typed", new Object[] {selection});
		}
	    }

	    return selection;
	}
	finally
	{
	    // Restore theme
	    PluginUIManager.restoreTheme(lookAndFeel);
	}
    }

    private static void showMessageDialog(int messageType, Object message, String title, boolean fModal)    
    {
	showMessageDialog(null, messageType, message, title, fModal);    
    }

    private static void showMessageDialog(final Component parentComponent, final int messageType, 

	final Object message, final String title, final boolean fModal) {
	try {
	    PluginSysUtil.execute(new PluginSysAction() {
		public Object execute() throws Exception {
		    showMessageDialogImpl(parentComponent, messageType, message, title, fModal);
		    return null;
		}});
	}
	catch(Exception e) {
	    // should never happen
	    assert(false);
	}
    }


    private static void showMessageDialogImpl(Component parentComponent, int messageType, Object message, String title, boolean fModal)    
    {
	LookAndFeel lookAndFeel = null;
	
	try
	{
	    // Change theme
	    lookAndFeel = PluginUIManager.setTheme();

    	    JOptionPane pane = new JOptionPane();

	    switch (messageType)
	    {
		case DialogFactory.ERROR_MESSAGE: 
		    pane.setMessageType(JOptionPane.ERROR_MESSAGE);
		    break;
		case DialogFactory.INFORMATION_MESSAGE: 
		    pane.setMessageType(JOptionPane.INFORMATION_MESSAGE);
		    break;
		case DialogFactory.WARNING_MESSAGE: 
		    pane.setMessageType(JOptionPane.WARNING_MESSAGE);
		    break;
		case DialogFactory.QUESTION_MESSAGE: 
		    pane.setMessageType(JOptionPane.QUESTION_MESSAGE);
		    break;
		default:
    //		pane.setIcon(loadIcon());
		    break;
	    }

	    pane.setOptionType(JOptionPane.DEFAULT_OPTION);
	    pane.setWantsInput(false);

	    Object msg = extractMessage(pane, message);

	    if (title == null)
		title = messageDialogTitle;

	    showDialog(pane, parentComponent, title, msg, fModal);
	}
	finally
	{
	    // Restore theme
	    PluginUIManager.restoreTheme(lookAndFeel);
	}
    }

    public static void showInformationDialog(Object message, String title)    
    {
	showInformationDialog(null, message, title);
    }

    public static void showInformationDialog(Component parentComponent, Object message, String title)    
    {
	showMessageDialog(parentComponent, INFORMATION_MESSAGE, message, title, true);
    }

    public static void showErrorDialog(String message, String title)    
    {
	showErrorDialog(null, message, title);
    }

    public static void showErrorDialog(Component parentComponent, String message, String title)    
    {
	showMessageDialog(parentComponent, ERROR_MESSAGE, message, title, true);
    }

    public static void showExceptionDialog(Throwable ex, String message)    
    {
	showExceptionDialog(ex, message, exceptionDialogTitle);
    }

    public static void showExceptionDialog(Component parentComponent, Throwable ex, String message)    
    {
	showExceptionDialog(parentComponent, ex, message, exceptionDialogTitle);
    }

    public static void showExceptionDialog(Throwable e, String message, String title)    
    {
	showExceptionDialog(null, e, message, title);
    }


    public static void showExceptionDialog(final Component parentComponent, final Throwable e, final String message, final String title) {
	try {
	    PluginSysUtil.execute(new PluginSysAction() {
		public Object execute() throws Exception {
		    showExceptionDialogImpl(parentComponent, e, message, title);
		    return null;
		}});
	}
	catch(Exception err) {
	    // should never happen
	    assert(false);
	}
    }


    private static void showExceptionDialogImpl(Component parentComponent, Throwable e, String message, String title)    
    {
	LookAndFeel lookAndFeel = null;
	
	try
	{
	    // Change theme
	    lookAndFeel = PluginUIManager.setTheme();

	    JOptionPane pane = new JOptionPane();
	    pane.setMessageType(JOptionPane.ERROR_MESSAGE);
	    pane.setOptionType(JOptionPane.DEFAULT_OPTION);
	    pane.setWantsInput(false);

	    final Component ex = (Component) extractMessage(pane, e);

	    // Create morePanel which only contains a "More" button
	    final JPanel morePanel = new JPanel();
	    JButton moreButton = new JButton(ResourceHandler.getMessage("usability.moreInfo")); 
	    moreButton.setMnemonic(ResourceHandler.getAcceleratorKey("usability.moreInfo")); 
	    morePanel.setLayout(new FlowLayout(FlowLayout.RIGHT));
	    morePanel.add(moreButton);

	    // Create lessPanel which contains a "Less" button and the exception message
	    final JPanel lessPanel = new JPanel();
	    JButton lessButton = new JButton(ResourceHandler.getMessage("usability.lessInfo")); 
	    lessButton.setMnemonic(ResourceHandler.getAcceleratorKey("usability.lessInfo")); 
	    JPanel buttonPanel = new JPanel();
	    buttonPanel.setLayout(new FlowLayout(FlowLayout.RIGHT));
	    buttonPanel.add(lessButton);
	    lessPanel.setLayout(new BorderLayout());
	    lessPanel.add(buttonPanel, BorderLayout.CENTER);
	    lessPanel.add(ex, BorderLayout.SOUTH);

	    // containerPanel either contains morePanel or lessPanel
	    final JPanel containerPanel = new JPanel();
	    containerPanel.setLayout(new BorderLayout());
	    containerPanel.add(morePanel, BorderLayout.CENTER);


	    // If "More Details" is pressed, dd exception info
	    moreButton.addActionListener(new ActionListener()
	    {
		public void actionPerformed(ActionEvent e)
		{
		    Component source = (Component) e.getSource();
		    JDialog dialog = null;

		    // Iterate through container to find JDialog
		    while (source.getParent() != null)
		    {
			if (source instanceof JDialog)
			    dialog = (JDialog) source;

			source = source.getParent();
		    }

		    if (dialog != null)
		    {
			dialog.setVisible(false);
			dialog.setResizable(true);

			// Replace button
			containerPanel.remove(morePanel);
			containerPanel.add(lessPanel, BorderLayout.CENTER);

			// Force dialog to layout new component
			dialog.doLayout();
			dialog.pack();

			dialog.setResizable(false);
			dialog.setVisible(true);
		    }
		}
	    });

	    // If "Less Details" is pressed, remove exception info
	    lessButton.addActionListener(new ActionListener()
	    {
		public void actionPerformed(ActionEvent e)
		{
		    Component source = (Component) e.getSource();
		    JDialog dialog = null;

		    // Iterate through container to find JDialog
		    while (source.getParent() != null)
		    {
			if (source instanceof JDialog)
			    dialog = (JDialog) source;

			source = source.getParent();
		    }

		    if (dialog != null)
		    {
			dialog.setVisible(false);
			dialog.setResizable(true);

			// Replace button
			containerPanel.remove(lessPanel);
			containerPanel.add(morePanel, BorderLayout.CENTER);

			// Force dialog to layout new component
			dialog.doLayout();
			dialog.pack();

			dialog.setResizable(false);
			dialog.setVisible(true);
		    }
		}
	    });


	    Object[] msgs = new Object[2];
	    msgs[0] = extractMessage(pane, message);
	    msgs[1] = containerPanel;

	    if (title == null)
		title = exceptionDialogTitle;

	    showDialog(pane, parentComponent, title, msgs);
	}
	finally
	{
	    // Restore theme
	    PluginUIManager.restoreTheme(lookAndFeel);
	}
    }


    private static boolean showDialog(JOptionPane pane, Component parentComponent, String title, Object msg)
    {
	return showDialog(pane, parentComponent, title, msg, true, false);
    }
    
    private static boolean showDialog(JOptionPane pane, Component parentComponent, String title, Object msg, boolean fModal)
    {
	return showDialog(pane, parentComponent, title, msg, fModal, false);
    }
    
    private static boolean showDialog(final JOptionPane pane, final Component parentComponent, 
				      final String title, final Object msg, final boolean fModal, final boolean preload)
    {
      boolean dialogNoError = true;
	try {
	    pane.setValue(null);

	    Runnable work = new Runnable()
	    {
		public void run()
		{
			// The app may have a different decorated state for JDialog, 
			// so it is important to set it and restore it if necessary.
			//

			// Store decorated state
			// boolean isDecorated = JDialog.isDefaultLookAndFeelDecorated();

			// Set decorated state to true
			// if (isDecorated != true)
			//    JDialog.setDefaultLookAndFeelDecorated(true);

			// Create dialog
		        final JDialog dialog = pane.createDialog(parentComponent, title); 

                        // add DefaultActionListener for option buttons that does not have action defined
                        Object[] options = pane.getOptions();

                        if (null != options) {
                           DefaultActionListener lsr = new DefaultActionListener(dialog, pane);
                           JButton btn;

                           for (int index = 0; index < options.length; index ++) {
                               if (options[index] instanceof JButton) {
                                  btn = (JButton)options[index];
				
				  // Remove the old DefaultActionListener which were not destroyed. 
				  // Fix bug 4660451
				  Object[] listeners = btn.getActionListeners();
				  for (int i=0; i<listeners.length; i++) {
				      if (listeners[i] instanceof DefaultActionListener)
					 btn.removeActionListener((DefaultActionListener)listeners[i]);
				  }
				
				  // If there is no ActionListener for the button, add one.
				  if (btn.getActionListeners().length == 0)
				     btn.addActionListener(lsr);
                               }
                           }
                         }

			pane.setMessage(msg);
			dialog.pack();

			// Call method to fix swing bug.
			fixSwingLayoutBug(msg);	
			dialog.pack();

			dialog.setResizable(false); 
			dialog.setModal(fModal);

			Dimension dlgDimension = dialog.getSize();
			Dimension scrndimension = Toolkit.getDefaultToolkit().getScreenSize();
			dlgDimension.width += 30;	    
			dialog.setLocation((scrndimension.width-dlgDimension.width)/2,
					      (scrndimension.height-dlgDimension.height)/2);	


			// Show dialog
		    	if (preload == false) {
			    dialog.show();
			    dialog.pack();
			    dialog.toFront();
			}

			// Restore decorated state to true
			//if (isDecorated != true)
			//   JDialog.setDefaultLookAndFeelDecorated(isDecorated);
		    }
		};
		if ( (preload == true) || SwingUtilities.isEventDispatchThread())
		{
			// If we are already in event dispatch thread or called from a classloader, just call it.
			work.run();	
		}
		else
		{
		    if (fModal)
		    {
			SwingUtilities.invokeAndWait(work);
		    }
		    else
		    {
			SwingUtilities.invokeLater(work);
		    }
		}
	}
	catch(Exception e)
	{
	    e.printStackTrace();
	    dialogNoError = false;
	}

	return dialogNoError;
    }


    /**
     * <P> Extract message from the object and formatted it.
     * </P>
     *
     * @param message Object
     * @param formatted object
     */
    private static Object extractMessage(JOptionPane pane, Throwable message)
    {
	return formatExceptionMessage(pane, message);
    }


    /**
     * <P> Extract message from the object and formatted it.
     * </P>
     *
     * @param message Object
     * @param formatted object
     */
    private static Object extractMessage(JOptionPane pane, String message)
    {
	return formatStringMessage(pane, message);
    }


    /**
     * <P> Extract message from the object and formatted it.
     * </P>
     *
     * @param message Object
     * @param formatted object
     */
    private static Object extractMessage(JOptionPane pane, Object[] messages)
    {
	Object[] result = null;

	if (messages != null)
	{
	    result = new Object[messages.length];

	    for (int i=0; i < messages.length; i++)
		result[i] = extractMessage(pane, messages[i]);
	}

	return result;
    }


    /**
     * <P> Extract message from the object and formatted it.
     * </P>
     *
     * @param message Object
     * @param formatted object
     */
    private static Object extractMessage(JOptionPane pane, Object message)
    {
	if (message instanceof Object[])
	    return extractMessage(pane, (Object[]) message);
	else if (message instanceof String)
	    return extractMessage(pane, (String) message);
	else if (message instanceof Exception)
	    return extractMessage(pane, (Exception) message);
	else
    	    return message;
    }

    private static Object formatStringMessage(JOptionPane pane, String in)
    {
	assert in != null;

	if (in == null)
	    in = "null";

	int i = in.indexOf("</html>");

	if (i == -1)
	{
	    // It doesn't contain HTML text

	    JTextArea text = new JTextArea();
	    text.setColumns(40);
	    text.setAutoscrolls(true);
	    text.setEditable(false);
	    text.setLineWrap(true);
	    text.setWrapStyleWord(true);
	    text.setBackground(pane.getBackground());
	    text.setText(in);
	    return text;
	}
	else
	{
	    // Contains <html> and </html>
	    //
	    if ((i + 7) == in.length())
	    {
		// Contains <html> and </html> only
		return new JLabel(in);
	    }
	    else
	    {
		// Some strings follows </html>
    		Object[] msgs = new Object[2];

		msgs[0] = new JLabel(in.substring(0, i + 7));
		msgs[1] = formatStringMessage(pane, in.substring(i + 7));

		return msgs;
	    }
	}
    }

    private static Object formatExceptionMessage(JOptionPane pane, Throwable e)
    {
	StringWriter byt = new StringWriter();
	PrintWriter print = new PrintWriter(byt);
	e.printStackTrace(print);

	JTextArea text = new JTextArea("" + byt.toString());
	text.setColumns(40);
	text.setRows(10);
	text.setEditable(false);

	JScrollPane spane = new JScrollPane( text ); 
	text.setBackground(pane.getBackground());

	return spane;
    }


    /**
     * Fix Swing Layout bug.
     */ 
    private static void fixSwingLayoutBug(Object msg)
    {
        if (msg == null)
	{
           return;
	}
        else if (msg instanceof JTextArea)
        {
           JTextArea text = (JTextArea) msg;
           text.getUI().getPreferredSize(text);
           return;
        }
        else if (msg.getClass().isArray())
        {
           int len = Array.getLength(msg);

           for (int i=0; i<len; i++)
	   {
               fixSwingLayoutBug(Array.get(msg, i));
	   }
        }
    }

}

class DefaultActionListener implements ActionListener{
 
     private Dialog dlg;
     private JOptionPane pane;
     
     /** Creates new DefaultActionListener */
     protected DefaultActionListener(Dialog dlg, JOptionPane pane) {
         this.dlg = dlg;
         this.pane = pane;
     }
     
     /**
      * Invoked when an action occurs.
      */
     public void actionPerformed(ActionEvent e) {
         pane.setValue(e.getSource());
         dlg.hide();
     }
}



