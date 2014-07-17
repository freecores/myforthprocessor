/*
 * @(#)PrintStatusDialog.java	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.print;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

/** 
 * A dialog for displaying print status messages to the user.
 *
 * @version 	1.6 01/23/03
 * @author 	Amy Fowler
 * @author      David Mendenhall
 */
public class PrintStatusDialog extends JDialog implements ActionListener {
    JPanel buttonPanel;
    String command;

    PrintStatusDialog(Frame f, String title, String msg) {
        super(f, title, true);
        getContentPane().setLayout(new BorderLayout());

        JLabel l = new JLabel(msg);
        getContentPane().add("Center", l);

        buttonPanel = new JPanel();
        buttonPanel.setLayout(new FlowLayout());
        getContentPane().add("South", buttonPanel);
	setResizable(false);
    }

    public PrintStatusDialog(Frame f, String title, String msg,
                             String command) {
        this(f, title, msg);
        JButton b = new JButton(command);
        b.setActionCommand(command);
        buttonPanel.add(b);
        b.addActionListener(this);

        pack();
    }

    public PrintStatusDialog(Frame f, String title, String msg,
                             String command1, String command2) {
        this(f, title, msg, command1);
        JButton b = new JButton(command2);
        b.setActionCommand(command2);
        buttonPanel.add(b);
        b.addActionListener(this);

        pack();
    }

    PrintStatusDialog(Dialog d, String title, String msg) {
        super(d, title, true);
        getContentPane().setLayout(new BorderLayout());

        JLabel l = new JLabel(msg);
        getContentPane().add("Center", l);

        buttonPanel = new JPanel();
        buttonPanel.setLayout(new FlowLayout());
        getContentPane().add("South", buttonPanel);
	setResizable(false);
    }

    public PrintStatusDialog(Dialog d, String title, String msg,
                             String command) {
        this(d, title, msg);
        JButton b = new JButton(command);
        b.setActionCommand(command);
        buttonPanel.add(b);
        b.addActionListener(this);

        pack();
    }

    public PrintStatusDialog(Dialog d, String title, String msg,
                             String command1, String command2) {
        this(d, title, msg, command1);
        JButton b = new JButton(command2);
        b.setActionCommand(command2);
        buttonPanel.add(b);
        b.addActionListener(this);

        pack();
    }

    public void actionPerformed(ActionEvent e) {
        command = e.getActionCommand();
        dispose();
    }

    public String getCommand() {
        return command;
    }
}
