/*
 * @(#)FindInHeapPanel.java	1.3 03/01/23 11:48:51
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.ui;

import java.io.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.text.*;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.oops.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.utilities.*;

/** Finds a given (Address) value in the heap. Only intended for use
    in a debugging system. */

public class FindInHeapPanel extends JPanel {
  private RawHeapVisitor   iterator;
  private long             addressSize;
  private long             usedSize;
  private long             iterated;
  private Address          value;
  private ProgressBarPanel progressBar;
  private JTextField       addressField;
  private JButton          findButton;
  private JTextArea        textArea;

  public FindInHeapPanel() {
    super();

    setLayout(new BorderLayout());

    JPanel topPanel = new JPanel();
    topPanel.setLayout(new BoxLayout(topPanel, BoxLayout.Y_AXIS));

    JPanel panel = new JPanel();
    panel.setLayout(new BoxLayout(panel, BoxLayout.X_AXIS));
    panel.add(new JLabel("Address to search for:"));

    addressField = new JTextField(30);
    panel.add(addressField);

    iterator = new RawHeapVisitor() {
        public void prologue(long used) {
          usedSize = used;
          iterated = 0;
          addressSize = VM.getVM().getAddressSize();
          clearResultWindow();
        }
        
        public void visitAddress(Address addr) {
          Address val = addr.getAddressAt(0);
          if (AddressOps.equal(val, value)) {
            reportResult(addr);
          }
          iterated += addressSize;
          updateProgressBar();
        }

        public void epilogue() {
          iterated = 0;
          updateProgressBar();
          findButton.setEnabled(true);
        }
      };

    findButton = new JButton("Find");
    ActionListener listener = new ActionListener() {
        public void actionPerformed(ActionEvent e) {
          clearResultWindow();
          // Parse text
          try {
            Address val = VM.getVM().getDebugger().parseAddress(addressField.getText());
            value = val;

            findButton.setEnabled(false);

            java.lang.Thread t = new java.lang.Thread(new Runnable() {
                public void run() {
                  try {
                    VM.getVM().getObjectHeap().iterateRaw(iterator);
                  } finally {
                    SwingUtilities.invokeLater(new Runnable() {
                        public void run() {
                          findButton.setEnabled(true);
                        }
                      });
                  }
                }
              });
            t.start();
          } catch (Exception ex) {
            addToResultWindow("Error parsing address");
          }
        }
      };
    panel.add(findButton);
    findButton.addActionListener(listener);
    addressField.addActionListener(listener);
    topPanel.add(panel);

    progressBar = new ProgressBarPanel(ProgressBarPanel.HORIZONTAL, "Search progress:");
    topPanel.add(progressBar);

    add(topPanel, BorderLayout.NORTH);

    textArea = new JTextArea();
    JScrollPane scroller = new JScrollPane(textArea);
    add(scroller, BorderLayout.CENTER);
  }

  private void reportResult(Address addr) {
    addToResultWindow("Value " + value + " found at address " + addr);
  }

  private void clearResultWindow() {
    SwingUtilities.invokeLater(new Runnable() {
        public void run() {
          Document d = textArea.getDocument();
          try {
            d.remove(0, d.getLength());
          } catch (BadLocationException e) {
          }
        }
      });
  }

  private void addToResultWindow(String s) {
    textArea.append(s);
    textArea.append("\n");
  }

  private void updateProgressBar() {
    SwingUtilities.invokeLater(new Runnable() {
        public void run() {
          progressBar.setValue((double) iterated / (double) usedSize);
        }
      });
  }
}
