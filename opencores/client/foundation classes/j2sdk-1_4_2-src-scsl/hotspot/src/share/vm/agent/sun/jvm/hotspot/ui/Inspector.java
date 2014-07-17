/* 
 * @(#)Inspector.java	1.5 03/01/23 11:49:04
 * 
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.ui;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.oops.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.ui.tree.*;
import sun.jvm.hotspot.utilities.*;

/** This class implements tree-browsing functionality of a particular
    SimpleTreeNode, and is only designed to be used in a debugging
    system. It uses a SimpleTreeModel internally. */

public class Inspector extends JPanel {
  private JTree tree;
  private SimpleTreeModel model;

  // UI widgets we need permanent handles to
  private JTextField addressField;
  private JLabel statusLabel;

  public Inspector() {
    model = new SimpleTreeModel();
    tree = new JTree(model);

    setLayout(new BorderLayout());
    Box hbox = Box.createHorizontalBox();
    hbox.add(new JLabel("Address: "));
    addressField = new JTextField(20);
    hbox.add(addressField);
    statusLabel = new JLabel();
    hbox.add(statusLabel);
    add(hbox, BorderLayout.NORTH);
    
    addressField.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
          try {
            Address a = VM.getVM().getDebugger().parseAddress(addressField.getText());
            int max_searches = 1000; 
            int searches = 0; 
            int offset = 0; 
            Oop oop = null; 
            if (a != null) { 
              OopHandle handle = a.addOffsetToAsOopHandle(0); 
              while (searches < max_searches) { 
                searches++; 
                if (RobustOopDeterminator.oopLooksValid(handle)) { 
                  try { 
                    oop = VM.getVM().getObjectHeap().newOop(handle); 
                    addressField.setText(handle.toString()); 
                    break; 
                  } catch (RuntimeException ex) { 
                    ex.printStackTrace(); 
                  } 
                } 
                offset -= 8; 
                handle = a.addOffsetToAsOopHandle(offset); 
              } 
            } 
            if (oop == null) { 
              statusLabel.setText("<bad oop>");
              return;
            } 
            statusLabel.setText("");
            setRoot(new OopTreeNodeAdapter(oop, null));
          }
          catch (NumberFormatException ex) {
            statusLabel.setText("<parse error>");
          }
          catch (AddressException ex) {
            ex.printStackTrace();
            statusLabel.setText("<bad address>");
          }
          catch (Exception ex) {
            ex.printStackTrace();
            statusLabel.setText("<error constructing oop>");
          }
        }
      });

    JScrollPane scrollPane = new JScrollPane(tree);
    // Let's see what happens if we let the parent deal with resizing the panel
    add(scrollPane, BorderLayout.CENTER);
  }
  
  public Inspector(SimpleTreeNode root) {
    this();
    setRoot(root);
  }

  public void setRoot(SimpleTreeNode root) {
    model.setRoot(root);

    //    tree.invalidate();
    //    tree.validate();
    //    repaint();
    // FIXME: invalidate? How to get to redraw? Will I have to make
    // tree listeners work?
  }
}
