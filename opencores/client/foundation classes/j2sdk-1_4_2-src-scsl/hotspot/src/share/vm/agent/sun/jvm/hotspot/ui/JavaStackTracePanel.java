/*
 * @(#)JavaStackTracePanel.java	1.2 03/01/23 11:49:11
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.ui;

import java.awt.*;
import javax.swing.*;
import javax.swing.event.*;

import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.ui.classbrowser.*;

/** Provides Java stack trace of a Java Thread */

public class JavaStackTracePanel extends JPanel {
    private JSplitPane          splitPane;
    private JEditorPane         stackTraceEditor;
    private JEditorPane         contentEditor;
    private HTMLGenerator htmlGen = new HTMLGenerator();

    public JavaStackTracePanel() {
	initUI();
    }

    private void initUI() {
        setLayout(new BorderLayout());
        HyperlinkListener hyperListener = new HyperlinkListener() {
                         public void hyperlinkUpdate(HyperlinkEvent e) {
                            if (e.getEventType() == HyperlinkEvent.EventType.ACTIVATED) {
                               setContentText(htmlGen.genHTMLForHyperlink(e.getDescription()));
                            }
                         }
                      };

        stackTraceEditor = new JEditorPane();
        stackTraceEditor.setContentType("text/html");
        stackTraceEditor.setEditable(false);
        stackTraceEditor.addHyperlinkListener(hyperListener);

        contentEditor = new JEditorPane();
        contentEditor.setContentType("text/html");
        contentEditor.setEditable(false);
        contentEditor.addHyperlinkListener(hyperListener);

        JPanel topPanel = new JPanel();
        topPanel.setLayout(new BorderLayout());
        topPanel.add(new JScrollPane(stackTraceEditor), BorderLayout.CENTER);

        JPanel bottomPanel = new JPanel();
        bottomPanel.setLayout(new BorderLayout());
        bottomPanel.add(new JScrollPane(contentEditor), BorderLayout.CENTER);

        splitPane = new JSplitPane(JSplitPane.VERTICAL_SPLIT, topPanel, bottomPanel);
        splitPane.setDividerLocation(0);

        setLayout(new BorderLayout());
        add(splitPane, BorderLayout.CENTER);
    }

    public JavaStackTracePanel(final JavaThread thread) {
	initUI();
	setJavaThread(thread);
    }

    public void setJavaThread(final JavaThread thread) {
	setStackTraceText(htmlGen.genHTMLForJavaStackTrace(thread));
    }

    private void setStackTraceText(String text) {
	stackTraceEditor.setText(text);
        splitPane.setDividerLocation(0.5);
    }

    private void setContentText(String text) {
        contentEditor.setText(text);
        splitPane.setDividerLocation(0.5);
    }
}
