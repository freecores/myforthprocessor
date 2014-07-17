/*
 * @(#)GeneralUtilities.java	1.32 03/02/12
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.ui.general;

import com.sun.javaws.*;
import com.sun.javaws.debug.*;
import java.awt.*;
import java.awt.event.*;
import java.text.*;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.border.*;
import javax.swing.text.*;
import javax.swing.plaf.basic.*;


/**
 * @version 1.3 08/29/00
 */
public class GeneralUtilities {
    
    private static ImageIcon _jawsTitleImage;

    private static ImageIcon _jawsTitleVerImage;
    
    private static ImageIcon _sunLogoImage;

    private static ImageIcon _javaCupImage;;
    
    private static ImageIcon _dividerImage;
    
    private static ImageIcon _alertImage;

    private static ImageIcon _infoImage;

    private static ImageIcon _lockImage;

    private static ImageIcon _defaultImage;

    //private static Image _tileImage;
    private static int _tileWidth;
    private static int _tileHeight;

    public static final int INSTALL_NO  = 0;
    public static final int INSTALL_YES = 1;
    public static final int INSTALL_ASK = 2;

    public static ImageIcon getJawsTitleImage() {
	if (_jawsTitleImage == null) {
	    _jawsTitleImage = Resources.getIcon("jawsTitle.image");
        }
        return _jawsTitleImage;
    }

    public static ImageIcon getSunLogoImage() {
        if (_sunLogoImage == null) {
	    _sunLogoImage = Resources.getIcon("sunLogo.image");
        }
        return _sunLogoImage;
    }
    
    public static ImageIcon getJavaCupImage() {
	if (_javaCupImage == null) {
	    _javaCupImage = Resources.getIcon("javaCup.image");
        }
        return _javaCupImage;
    }

    public static ImageIcon getDividerImage() {
        if (_dividerImage == null) {
	    _dividerImage = Resources.getIcon("divider.image");
        }
        return _dividerImage;
    }
    
    public static ImageIcon getWarningIcon() {
        if (_alertImage == null) {
	    _alertImage = Resources.getIcon("alert.image");
        }
        return _alertImage;
    }

    public static ImageIcon getInfoIcon() {
        if (_infoImage == null) {
	    _infoImage = Resources.getIcon("info.image");
        }
        return _infoImage;
    }

    public static ImageIcon getLockIcon() {
        if (_lockImage == null) {
	    _lockImage = Resources.getIcon("lock.image");
        }
        return _lockImage;
    }

    public static ImageIcon getDefaultImage() {
	if (_defaultImage == null) {
	    _defaultImage = Resources.getIcon("default.image");
	}
	return _defaultImage;
    }

    public static JFrame createFrame(String title) {
	return new JFrame(title);
    }

    public static JDialog createDialog(Frame owner, String title, 
					boolean modal) {
	return new JDialog(owner, title, modal);
    }

    public static JTextPane addBoldText(JTextPane text, String message,
					Object[] args) {
	MessageFormat format = new MessageFormat("");
	int lastIndex = 0;
	int index;
	SimpleAttributeSet boldSAS = new SimpleAttributeSet();
        SimpleAttributeSet nSAS = new SimpleAttributeSet();
	
        if (text == null) {
	    text = new JTextPane() {
		public Dimension getPreferredSize() {
		    Dimension size = super.getPreferredSize();
		    
		    size.width = Math.min(400, size.width);
		    return size;
		}
	    };
        }
	
        Document doc = text.getDocument();
	
	StyleConstants.setBold(boldSAS, true);
        StyleConstants.setFontFamily(boldSAS, "SansSerif");
        StyleConstants.setFontFamily(nSAS, "SansSerif");
	while ((index = message.indexOf("{", lastIndex)) != -1) {
	    /* fix for bug in german and other locales where the
            ** resource used for message starts with: {0}
            */
            if (index == 0 || (index > 0 && message.charAt(index - 1) != '\'')) { 
		if (lastIndex != index) {
		    try {
			doc.insertString(doc.getLength(), message.substring
					     (lastIndex, index), nSAS);
		    } catch (BadLocationException ble) {}
		}
		
		int end = message.indexOf("}", index) + 1;
		
		format.applyPattern(message.substring(index, end));
		try {
		    doc.insertString(doc.getLength(), format.format(args),
				     boldSAS);
		} catch (BadLocationException ble) {}
		lastIndex = end;
	    } 
	}
	if (lastIndex < message.length()) {
	    try {
		doc.insertString(doc.getLength(), message.substring(lastIndex),
				 nSAS);
	    } catch (BadLocationException ble) {}
	}
        text.setOpaque(false);
        text.setEditable(false);
	return text;
    }
    
    private static void invokeOnEventDispatchingThread(Runnable r) {
        if (SwingUtilities.isEventDispatchThread()) {
	    r.run();
        }
        else {
	    try {
		SwingUtilities.invokeAndWait(r);
	    }
	    catch (java.lang.reflect.InvocationTargetException ite) {}
	    catch (InterruptedException ie) {}
        }
    }

    public static void showAboutDialog(Frame owner, String title) {

        String copyright_en = "     Copyright \u00a9 2003 Sun Microsystems, Inc.  All rights reserved.  Use is subject to license terms.  Third-party software, including font technology, is copyrighted and licensed from Sun suppliers.  Sun,  Sun Microsystems,  the Sun logo and  Java are trademarks or registered trademarks of Sun Microsystems, Inc. in the U.S. and other countries.  All SPARC trademarks are used under license and are trademarks or registered trademarks of SPARC International, Inc. in the U.S. and other countries.  Federal Acquisitions: Commercial Software - Government Users Subject to Standard License Terms and Conditions.  ";
	String seperator = "\n\n";
        String copyright_fr = "     Copyright \u00a9 2003 Sun Microsystems, Inc. Tous droits r\u00e9serv\u00e9s.  Distribu\u00e9 par des licences qui en restreignent l'utilisation.  Le logiciel d\u00e9tenu par des tiers, et qui comprend la technologie relative aux polices de caract\u00e8res, est prot\u00e9g\u00e9 par un copyright et licenci\u00e9 par des fournisseurs de Sun.  Sun,  Sun Microsystems,  le logo Sun et  Java sont des marques de fabrique ou des marques d\u00e9pos\u00e9es de Sun Microsystems, Inc. aux Etats-Unis et dans d'autres pays.  Toutes les marques SPARC sont utilis\u00e9es sous licence et sont des marques de fabrique ou des marques d\u00e9pos\u00e9es de SPARC International, Inc. aux Etats-Unis et dans d'autres pays.  ";

	final JDialog dialog = new JDialog(owner, title, true);
	Container parent = dialog.getContentPane();
	parent.setLayout(new BorderLayout());
	JTextArea text = new JTextArea();
	text.setEditable(false);
	text.setLineWrap(true);
	text.setWrapStyleWord(true);

	final JScrollPane sp = new JScrollPane (text,  
				JScrollPane.VERTICAL_SCROLLBAR_ALWAYS,
				JScrollPane.HORIZONTAL_SCROLLBAR_NEVER); 
	text.setText(copyright_en + seperator + copyright_fr);
	text.setBorder(BorderFactory.createEmptyBorder(6,6,6,6));

	JPanel bottomPanel = new JPanel(new BorderLayout());
	JPanel buttonPanel = new JPanel(new BorderLayout());

	JButton close = new JButton(
	    Resources.getString("aboutBox.closeButton"));
	buttonPanel.add(close, BorderLayout.SOUTH);
	bottomPanel.add(buttonPanel, BorderLayout.EAST);

        JPanel logoPanel = new JPanel(new BorderLayout());
        Border logoBorder = BorderFactory.createEmptyBorder(4,4,4,4);

        JLabel sunLogo = new JLabel(getSunLogoImage());
        sunLogo.setBorder(logoBorder);
        logoPanel.add(sunLogo, BorderLayout.WEST);

        JLabel cupLogo = new JLabel(getJavaCupImage());
        cupLogo.setBorder(logoBorder);
        logoPanel.add(cupLogo, BorderLayout.EAST);
 
        bottomPanel.add(logoPanel, BorderLayout.WEST);
        bottomPanel.setBorder(BorderFactory.createEmptyBorder(8,2,6,2));

	dialog.getRootPane().setDefaultButton(close);

	close.addActionListener(new ActionListener() {
	    public void actionPerformed(ActionEvent ev) {
	    	dialog.dispose();
	    }
	});
	close.requestFocus();
	dialog.getRootPane().setDefaultButton(close);

	JLabel versionLabel = new JLabel(
	    Resources.getString("aboutBox.versionLabel", Globals.getBuildID()),
	    SwingConstants.LEFT);
	versionLabel.setBorder(BorderFactory.createEmptyBorder(5,5,5,5));
	parent.add(versionLabel, BorderLayout.NORTH);
	parent.add(sp, BorderLayout.CENTER);
	parent.add(bottomPanel, BorderLayout.SOUTH);
	dialog.setBounds(0,0,400,400);
	GeneralUtilities.placeWindow(dialog);
//
// sp.getViewport().setViewPosition(new Point(0,0)); will work in 1.4, but in
// earlier jre's it needs to happen later to word, - this seems to do the trick:
//
        dialog.addWindowListener(new WindowAdapter() {
            public void windowOpened(WindowEvent e) {
                SwingUtilities.invokeLater(new Runnable() {
                    public void run() {
                        sp.getViewport().setViewPosition(new Point(0,0));
                    }
                });
            }
        });

	dialog.show();
    }

    public static String showInputDialog(Component owner, Object message, 
	String title, String initialString) {
	return (String) JOptionPane.showInputDialog(owner, 
		messageFormat(message), title, JOptionPane.PLAIN_MESSAGE, 
		getDefaultImage(), null, initialString);
    }

    public static void showMessageDialog(Component owner, Object message,
                                     String title, int messageType) {
	showOptionDialog(owner, message, title, 
			 JOptionPane.DEFAULT_OPTION, messageType);
    }

    public static int showOptionDialog(Component owner, Object message, 
			String title, int optionType,  int messageType) { 
        return showOptionDialog(owner, message, title, 
                optionType, messageType, null, null);
    }    

    public static int showLocalInstallDialog(Window owner, String title) {
        Object messages[] = new Object[2];
	JButton buttons[] = new JButton[4];

	buttons[0] = new JButton(Resources.getString("install.yesButton"));
	buttons[0].setMnemonic(Resources.getVKCode("install.yesMnemonic"));
	final JButton yesButton = buttons[0];

	buttons[1] = new JButton(Resources.getString("install.noButton"));
	buttons[1].setMnemonic(Resources.getVKCode("install.noMnemonic"));
	final JButton noButton = buttons[1];

	buttons[2] = new JButton(Resources.getString("install.askButton"));
	buttons[2].setMnemonic(Resources.getVKCode("install.askMnemonic"));

	buttons[3] = new JButton(Resources.getString("install.configButton"));
	buttons[3].setMnemonic(Resources.getVKCode("install.configMnemonic"));


	for (int i=0; i<3; i++) {
	    buttons[i].addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent ae) {
		    setReturn( (JButton) ae.getSource() );
                }
            });  
	}


        buttons[3].addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent ae) {

	        WhenInstallPanel radioPanel = new WhenInstallPanel();
                String dlgTitle = Resources.getString("install.configureTitle");
		Component owner = (Component) ae.getSource();
		while (owner != null && !(owner instanceof JDialog)) {
		    owner = owner.getParent();
		}

                int result = showOptionDialog(owner, radioPanel, dlgTitle,
                        JOptionPane.OK_CANCEL_OPTION, JOptionPane.PLAIN_MESSAGE,
                        null, null, false, null);
 
                switch (result) {
                    case 0:     // OK
                        ConfigProperties cp = ConfigProperties.getInstance();
                        int when = radioPanel.getWhen();
 
                        if (when == ConfigProperties.INSTALL_ALWAYS) {
                            cp.setWhenInstall(when);
                            setReturn(yesButton);
                        } else if (when == ConfigProperties.INSTALL_NEVER) {
                            cp.setWhenInstall(when);
                            setReturn(noButton);
                        } else {
                            cp.setWhenInstall(ConfigProperties.INSTALL_ASK);
                        }
                        synchronized(cp) {
			    cp.store();
			}
                        break;  // still INSTALL_ASK - continue
                    case 1:     // cancel - continue
                    default:
                        break;
                }
            }
        });  



        String dialogTitle = Resources.getString(
		 		"install.installTitle", title);
        messages[0] = Resources.getString("install.message0");
        messages[1] = Resources.getString("install.message1", title);

        int result = showOptionDialog(null, messages, dialogTitle, 
		 JOptionPane.DEFAULT_OPTION, JOptionPane.PLAIN_MESSAGE,
		 buttons, buttons[2], true, null);

        switch (result) {
            case 0: 
	        return INSTALL_YES;
            case 1:
	        return INSTALL_NO;
	    default:
	        return INSTALL_ASK;
        }
    }

    private static void setReturn(JButton b) {
        Component c = (Component) b;
	while (c != null) {
            c = c.getParent();
            if (c instanceof JOptionPane) {
                ((JOptionPane)c).setValue(b);
                break;
            }   
        }    
    }
    
    
    public static int showOptionDialog(Component owner, Object message, 
			String title, int optionType, int messageType, 
			Object[] options, Object initialValue) {
        return showOptionDialog(owner, message, title, optionType, messageType, 
	                options, initialValue, false, null);
    }

    public static int showOptionDialog(Component owner, Object message, 
			String title, int optionType, int messageType, 
			Object[] options, Object initialValue, Icon icon) {
        return showOptionDialog(owner, message, title, optionType, messageType, 
	                options, initialValue, false, icon);
    }

    private static int showOptionDialog(Component owner,
                        Object message, String title, int optionType,
                        int messageType, Object[] options,
                        Object initialValue, boolean localInstall, Icon icon) {

	if (icon == null) { 
	    icon = getDefaultImage(); 
	}

        JOptionPane pane = new JOptionPane(messageFormat(message), messageType,
		       optionType, icon, options, initialValue);
	
        final JDialog dialog = pane.createDialog(owner, title);
	
        pane.selectInitialValue();

        invokeOnEventDispatchingThread(new Runnable() {
	    public void run() {
		// Force a paint, so that components (like JTextPane) that
		// require a paint to invalidate themselves, will correctly
		// layout.
		Container parent = dialog.getContentPane();
		Graphics g = parent.getGraphics();
			
		try {
		    g.setClip(new Rectangle(0, 0, parent.getWidth(),
					    parent.getHeight()));
		    parent.paint(g);
		}
		finally {
		    g.dispose();
		}
		
		dialog.validate();
		dialog.setSize(dialog.getPreferredSize());
	    }
	});
	SplashScreen.hide();
	final Object lock = new Object();
	if (localInstall) {
            // give app a second first ...
            try {
                Thread.sleep(2000);
            } catch(Exception ioe) { 
                Debug.ignoredException(ioe);
            }
	    new Thread(new Runnable() {
                public void run() {
                    int seconds = 0;
		    do {
                        try {
                            Thread.sleep(1000);         // every  second 
			    synchronized (lock) {
			        Window[] w = dialog.getOwnedWindows();
                                if (dialog.isShowing() && 
			            ((w == null) || w.length == 0)) {
			            dialog.toFront();   // back to top.
			        }
			    }
                        } catch(Exception ioe) { 
			    Debug.ignoredException(ioe);
			}
  			seconds++;
                    } while ((seconds < 8) && dialog.isShowing());
		}
            }).start();
	}
        dialog.show();
	synchronized (lock) {
            dialog.dispose();
        }	
        Object selectedValue = pane.getValue();
	

        if(selectedValue == null)
	    return JOptionPane.CLOSED_OPTION;
        if(options == null) {
	    if(selectedValue instanceof Integer)
		return ((Integer)selectedValue).intValue();
	    return JOptionPane.CLOSED_OPTION;
        }
        for(int counter = 0, maxCounter = options.length;
	    counter < maxCounter; counter++) {
	    if(options[counter].equals(selectedValue))
		return counter;
        }
        return JOptionPane.CLOSED_OPTION;
    }

    private static Object messageFormat(Object message) {
        if (message instanceof String) {
	    return (Object) new PlainTextArea((String)message);
        } else if (message instanceof Object[]) {
            Object[] objs = (Object[]) message;
            Object[] newObjs = new Object[objs.length];
            for (int i=0; i<objs.length; i++) {
                if (objs[i] instanceof String) {
		    newObjs[i] = (Object) new PlainTextArea((String)objs[i]);
                } else {
                    newObjs[i] = objs[i];
                }      
            }   
            return newObjs;
        } else {
            return message;
        }
    } 

    private static class PlainTextArea extends JTextArea {
	PlainTextArea(String s) {
	    super(s);
	    setLineWrap(true);
	    setWrapStyleWord(true);
	    setOpaque(false);
	    setEditable(false);
	}       
	
	public boolean isFocusTraversable() {
	    return false;
	}
    }

    public static void placeWindow(Window window) {
	Window owner = window.getOwner();
	Rectangle screenBounds = new Rectangle(new Point(0,0), 
	                         Toolkit.getDefaultToolkit().getScreenSize());
	Rectangle winBounds = window.getBounds();
	Rectangle ownerBounds = (owner == null) ? 
					screenBounds : owner.getBounds();
	double goldenOffset = ownerBounds.height - (ownerBounds.height / 1.618);
	winBounds.x = ownerBounds.x + (ownerBounds.width - winBounds.width)/2;
	int computedOffset = (int) (goldenOffset - winBounds.height/2);

	// if the owner is smaller (less height) than the window this 
        // goldenMean offset computation results in computed offset < 0.
        // this causes dialog to obscure parent - make minimum of parents inset
        int minOffset = (owner == null) ? 0 : owner.getInsets().top;

	winBounds.y = ownerBounds.y + Math.max(computedOffset, minOffset);

	if ((winBounds.x + winBounds.width) > screenBounds.width) {
	    winBounds.x = Math.max(screenBounds.width - winBounds.width, 0);
	}
	if ((winBounds.y + winBounds.height) > screenBounds.height) {
	    winBounds.y = Math.max(screenBounds.height - winBounds.height, 0);
	}
	window.setBounds(winBounds);
    }
}


