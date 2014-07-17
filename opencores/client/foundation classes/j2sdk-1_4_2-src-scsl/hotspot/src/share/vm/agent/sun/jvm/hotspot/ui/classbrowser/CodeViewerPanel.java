/*
 * @(#)CodeViewerPanel.java	1.3 03/01/23 11:50:01
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.ui.classbrowser;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;
import sun.jvm.hotspot.code.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.oops.*;
import sun.jvm.hotspot.runtime.*;

public class CodeViewerPanel extends JPanel {
   protected JEditorPane    contentEditor;
   protected JTextField     address;
   protected HTMLGenerator  htmlGen;

   public CodeViewerPanel() {
      htmlGen = new HTMLGenerator();
      contentEditor = new JEditorPane();

      HyperlinkListener hyperListener = new HyperlinkListener() {
                         public void hyperlinkUpdate(HyperlinkEvent e) {
                            if (e.getEventType() == HyperlinkEvent.EventType.ACTIVATED) {
                               String description = e.getDescription();
                               int equalToIndex = description.indexOf('=');
                               if (equalToIndex != -1) {
                                  String item = description.substring(0, equalToIndex);
                                  if (item.equals("pc") || item.equals("klass") || item.equals("method")) {
                                     address.setText(description.substring(equalToIndex + 1));
                                  }
                               }
                               contentEditor.setText(htmlGen.genHTMLForHyperlink(description));
                            }
                         }
                      };


      setLayout(new BorderLayout());

      JPanel topPanel = new JPanel();
      topPanel.setLayout(new BorderLayout());
      topPanel.add(new JLabel("Enter PC or methodOop/klassOop Address: "), BorderLayout.WEST);
      address = new JTextField();
      topPanel.add(address, BorderLayout.CENTER);

      JPanel bottomPanel = new JPanel();
      bottomPanel.setLayout(new GridLayout(1, 1));
      contentEditor = new JEditorPane();
      contentEditor.setEditable(false);
      contentEditor.addHyperlinkListener(hyperListener);
      contentEditor.setContentType("text/html");
      bottomPanel.add(new JScrollPane(contentEditor));

      add(topPanel, BorderLayout.NORTH);
      add(bottomPanel, BorderLayout.CENTER);

      address.addActionListener(new ActionListener() {
                                   public void actionPerformed(ActionEvent e) {
                                       viewAddress();
                                   }
                                });

   }

   private void viewAddress() {
      contentEditor.setText(htmlGen.genHTMLForAddress(address.getText()));
   }
}
