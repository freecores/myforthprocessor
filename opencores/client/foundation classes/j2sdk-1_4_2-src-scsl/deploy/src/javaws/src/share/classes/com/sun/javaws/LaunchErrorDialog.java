/*
 * @(#)LaunchErrorDialog.java	1.28 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws;

import javax.swing.*;
import javax.swing.border.*;
import javax.swing.event.*;
import javax.swing.table.*;
import javax.swing.text.*;
import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.util.*;
import java.security.GeneralSecurityException;
import com.sun.javaws.debug.Debug;
import com.sun.javaws.Resources;
import com.sun.javaws.ui.console.Console;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.javaws.jnl.InformationDesc;
import com.sun.javaws.exceptions.*;
import com.sun.javaws.ui.general.GeneralUtilities;
import com.sun.javaws.debug.Globals;

public class LaunchErrorDialog extends JDialog {
    
    private LaunchErrorDialog(Frame owner, Throwable exception) {
        super(owner, true);
        
        JNLPException jnlpException = null;
        if (exception instanceof JNLPException) {
	    jnlpException = (JNLPException)exception;
        }
        
        JTabbedPane tabPane = new JTabbedPane();
        getContentPane().setLayout(new BorderLayout());
        getContentPane().add("Center", tabPane);

	tabPane.setBorder(BorderFactory.createEmptyBorder(4,4,4,4));
        
        String errorKind = getErrorCategory(exception);
        setTitle(Resources.getString("launcherrordialog.title", errorKind));
        
        String title = getLaunchDescTitle();
        String vendor = getLaunchDescVendor();
        String msg= Resources.getString("launcherrordialog.errorintro");
        if (title != null) {
	    msg += Resources.getString("launcherrordialog.errortitle", title);
        }
        if (vendor != null) {
	    msg += Resources.getString("launcherrordialog.errorvendor", vendor);
        }
        msg += Resources.getString("launcherrordialog.errorcategory", errorKind);
        msg += getErrorDescription(exception);
        
        // Pane 1: General
        JTextArea tp1 = new JTextArea();
        tp1.setEditable(false);
        tp1.setLineWrap(true);
        tp1.setText(msg);
        tabPane.add(Resources.getString("launcherrordialog.generalTab"), new JScrollPane(tp1));
        
        
        // Get source for LaunchFile that got error, and the one that for the application (if
        // they are different)
        String errorLaunchDescSource = null;
        String mainLaunchDescSource = null;
        
        // Get LaunchDesc for JNLP file that caused the error
        if (jnlpException != null) {
	    errorLaunchDescSource = jnlpException.getLaunchDescSource();
	    if (errorLaunchDescSource == null) {
		LaunchDesc ld = jnlpException.getDefaultLaunchDesc();
		if (ld != null) {
			errorLaunchDescSource = ld.getSource();
		}
	    }
        } else {
	    // Just get the default one
	    if (JNLPException.getDefaultLaunchDesc() != null) {
		errorLaunchDescSource = JNLPException.getDefaultLaunchDesc().getSource();
	    }
        }
        // Get the main LaunchDesc
        if (JNLPException.getDefaultLaunchDesc() != null) {
	    mainLaunchDescSource = JNLPException.getDefaultLaunchDesc().getSource();
        }
        
        // Only show main if it is different than the error one
        if (mainLaunchDescSource != null && mainLaunchDescSource.equals(errorLaunchDescSource)) {
	    mainLaunchDescSource = null;
        }
        
        // Pane 2: Launch File w/ error
        if (errorLaunchDescSource != null) {
	    JTextArea tp12 = new JTextArea();
	    tp12.setEditable(false);
	    tp12.setLineWrap(true);
	    tp12.setText(errorLaunchDescSource);
	    tabPane.add(Resources.getString("launcherrordialog.jnlpTab"), new JScrollPane(tp12));
        }
        
        // Pane 3: Main Launch File if different
        if (mainLaunchDescSource != null) {
	    JTextArea tp12 = new JTextArea();
	    tp12.setEditable(false);
	    tp12.setLineWrap(true);
	    tp12.setText(mainLaunchDescSource);
	    tabPane.add(Resources.getString("launcherrordialog.jnlpMainTab"), new JScrollPane(tp12));
        }
        
        // Pane 4: Exception
        if (exception != null) {
	    JTextArea tp3 = new JTextArea();
	    tp3.setEditable(false);
	    tp3.setLineWrap(true);
	    tp3.setWrapStyleWord(false);
	    StringWriter sw = new StringWriter();
	    PrintWriter pw = new PrintWriter(sw);
	    exception.printStackTrace(pw);
	    tp3.setText(sw.toString());
	    tabPane.add(Resources.getString("launcherrordialog.exceptionTab"),
			new JScrollPane(tp3));
        }
        
        // Pane 5: Wrapped Exception
        if (jnlpException != null && jnlpException.getWrappedException() != null) {
	    JTextArea tp3 = new JTextArea();
	    tp3.setEditable(false);
	    tp3.setLineWrap(true);
	    tp3.setWrapStyleWord(false);
	    StringWriter sw = new StringWriter();
	    PrintWriter pw = new PrintWriter(sw);
	    jnlpException.getWrappedException().printStackTrace(pw);
	    tp3.setText(sw.toString());
	    tabPane.add(Resources.getString("launcherrordialog.wrappedExceptionTab"),
			new JScrollPane(tp3));
        }
	// Pane 6: Console
	Document doc = Console.consoleErrorExit();
	if (doc != null) {
	    tabPane.add(Resources.getString("launcherrordialog.consoleTab"),
                        new JScrollPane(new JTextArea(doc)));
	}
        // Add Abort button
        JButton abortButton = new JButton(
	    Resources.getString("launcherrordialog.abort"));
	abortButton.setMnemonic(
	    Resources.getVKCode("launcherrordialog.abortMnemonic"));
        Box box = new Box(BoxLayout.X_AXIS);
        box.add(Box.createHorizontalGlue());
        box.add(abortButton);
        box.add(Box.createHorizontalGlue());
        getContentPane().add("South", box);
        getRootPane().setDefaultButton(abortButton);

        abortButton.addActionListener(new ActionListener() {
		    public void actionPerformed(ActionEvent e) {
			Main.systemExit(-1); 
		    }
		});
        
        addWindowListener(new WindowAdapter() {
		    public void windowClosing(WindowEvent e) {
			Main.systemExit(-1); 
		    }
		});
        pack();
        setSize(450, 300);
        
        // Center the window
        Rectangle size = getBounds();
        Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
        size.width = Math.min(screenSize.width, size.width);
        size.height = Math.min(screenSize.height, size.height);
        setBounds(
		     (screenSize.width - size.width) / 2,
		     (screenSize.height - size.height) / 2,
		     (size.width), (size.height));
    }
    
    /** Shows an error message for the given exception. The error box gets
     *  automatically configured based on the exception. The error dialog
     *  knows how to handle JNLPException's and its subtype.
     */
    public static void show(Frame owner, Throwable e) {
	showWarning(owner, e);
        Main.systemExit(-1); 
    }

    public static void showWarning(Frame owner, Throwable e) {
        SplashScreen.hide(); // Just in case
        
	if (Globals.RedirectErrors) {
	    // Just print details to System.err (this is mostly for debugging and testing)
	    System.err.println("#### Java Web Start Error:");
	    System.err.println("#### " + e.getMessage());
	} else if (wantsDetails(owner, e)) {
	    LaunchErrorDialog led = new LaunchErrorDialog(owner, e);
	    led.show();
        }
    }
    
    /** Compute the broad describtion of the exception */
    static private String getErrorCategory(Throwable e) {
        String errorKind = Resources.getString("launch.error.category.unexpected");
        // Comput brief message
        if (e instanceof JNLPException) {
	    // JNLPExceptions are easy - the know there category already
	    JNLPException je = (JNLPException)e;
	    errorKind = je.getCategory();
        } else if (e instanceof SecurityException || e instanceof GeneralSecurityException) {
	    errorKind = Resources.getString("launch.error.category.security");
        } else if (e instanceof java.lang.OutOfMemoryError) {
	    errorKind = Resources.getString("launch.error.category.memory");
	}
        return errorKind;
    }
    
    /** Compute the detailed describtion of the exception */
    static private String getErrorDescription(Throwable e) {
        // The error messages should already be localized. We should probably handle
        // non-JNLP exceptions in a different way
        String errorDescription = e.getMessage();
        if (errorDescription == null) {
	    // Create generic error messaage based on exception type
	    errorDescription = Resources.getString("launcherrordialog.genericerror", e.getClass().getName());
        }
        return errorDescription;
    }
    
    private static String getLaunchDescTitle() {
        LaunchDesc ld = JNLPException.getDefaultLaunchDesc();
        return (ld == null) ? null : ld.getInformation().getTitle();
    }
    
    private static String getLaunchDescVendor() {
        LaunchDesc ld = JNLPException.getDefaultLaunchDesc();
        return (ld == null) ? null : ld.getInformation().getVendor();
    }
    
    
    /**
     * Shows the user that we were unable to launch the specified application.
     * Will return true if the user wants details on what went wrong.
     */
    private static boolean wantsDetails(Frame owner, Throwable exception) {
        String message = null;
	String errorKind = getErrorCategory(exception);
        
	if (exception instanceof JNLPException) {
	    message = ((JNLPException) exception).getBriefMessage();
	}
	if (message == null) {
            if (getLaunchDescTitle() == null) {
	        message = Resources.getString
		    ("launcherrordialog.brief.message");
            } else {
	        message = Resources.getString
		("launcherrordialog.brief.messageKnown", getLaunchDescTitle());
            }
	}
        String[] options = new String[] {
		Resources.getString("launcherrordialog.brief.ok"),
	    Resources.getString("launcherrordialog.brief.details")
        };
        int retValue = GeneralUtilities.showOptionDialog (
		owner, message, 
		Resources.getString("launcherrordialog.brief.title", errorKind),
		JOptionPane.DEFAULT_OPTION, JOptionPane.ERROR_MESSAGE,
		options, options[0]);
        if (retValue == 1) {
	    return true;
        }
        return false;
    }
}

