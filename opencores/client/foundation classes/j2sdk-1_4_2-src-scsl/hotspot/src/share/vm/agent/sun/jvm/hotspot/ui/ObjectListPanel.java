/* 
 * @(#)ObjectListPanel.java	1.8 03/01/23 11:49:23
 * 
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.ui;

import java.io.*;
import java.util.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.table.*;

import sun.jvm.hotspot.oops.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.utilities.*;

/** Lists objects along with their types */

public class ObjectListPanel extends JPanel {
  private AbstractTableModel dataModel;
  private JTable             table;
  private java.util.List     elements;
  private LivenessAnalysis.ProgressThunk thunk;
  private Map                liveness;
  private java.util.List     listeners;
  private boolean            checkedForArrays;
  private boolean            hasArrays;
  private int                numColumns;
  // For changing the text of the "Compute Liveness" button
  private JButton            livenessButton;
  private ActionListener     livenessButtonListener;
  private static final String showLivenessText = "Show Liveness";
  private ReversePtrs        revPtrs;

  /** Takes a List<Oop> in constructor, an optional
      LivenessAnalysis.ProgressThunk used if computing liveness, and
      an optional HashMap<Oop, LivenessPathList> from the
      LivenessAnalysis class if liveness has already been computed */
  public ObjectListPanel(java.util.List els,
                         LivenessAnalysis.ProgressThunk thunk,
                         HashMap livenessResult) {
    super();

    elements = els;
    this.thunk = thunk;
    liveness = livenessResult;
    revPtrs = VM.getVM().getRevPtrs();
    computeNumColumns();
    listeners = new ArrayList();

    setLayout(new BorderLayout());

    dataModel = new AbstractTableModel() {
        public int getColumnCount() { return numColumns;      }
        public int getRowCount()    { return elements.size(); }
        public String getColumnName(int col) {
          switch (col) {
          case 0:
            return "Address";
          case 1:
            return "Oop";
          case 2:
            if (hasArrays) {
              return "Length";
            } else {
              return "Class Description";
            }
          case 3:
            if (hasArrays) {
              return "Class Description";
            } else if (liveness != null ||  revPtrs != null) {
              return "Liveness";
            }
          case 4:
            if (hasArrays && (liveness != null || revPtrs != null)) {
              return "Liveness";
            }
          }
          throw new RuntimeException("Index " + col + " out of bounds");
        }

        public Object getValueAt(int row, int col) {
          Oop oop = (Oop) elements.get(row);

          ByteArrayOutputStream bos = new ByteArrayOutputStream();

          switch (col) {
	  case 0:
	    Oop.printOopAddressOn(oop, new PrintStream(bos));
	    break;
          case 1:
            oop.printValueOn(new PrintStream(bos));
            break;
          case 2:
            if (hasArrays) {
              if (oop instanceof Array) {
                new PrintStream(bos).print(((Array) oop).getLength());
              }
              break;
            } else {
              oop.getKlass().printValueOn(new PrintStream(bos));
              break;
            }
          case 3:
            if (hasArrays) {
              oop.getKlass().printValueOn(new PrintStream(bos));
              break;
            } else {
              if (liveness != null) {
                if (liveness.get(oop) != null) {
                  return "Alive";
                } else {
                  return "Dead";
                }
              } else if (revPtrs != null) {
                if (revPtrs.get(oop) != null) {
                  return "Alive";
                } else {
                  return "Dead";
                }
              }
            }
          case 4:
            if (hasArrays) {
              if (liveness != null) {
                if (liveness.get(oop) != null) {
                  return "Alive";
                } else {
                  return "Dead";
                }
              } else if (revPtrs != null) {
                if (revPtrs.get(oop) != null) {
                  return "Alive";
                } else {
                  return "Dead";
                }
              }
            }
          default:
            throw new RuntimeException("Index (" + col + ", " + row + ") out of bounds");
          }

          return bos.toString();
        }
      };

    table = new JTable(dataModel);
    table.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);

    JScrollPane scrollPane = new JScrollPane(table);
    add(scrollPane, BorderLayout.CENTER);

    JPanel panel = new JPanel();
    panel.setLayout(new BoxLayout(panel, BoxLayout.X_AXIS));
    panel.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));
    Box box = Box.createHorizontalBox();
    box.add(Box.createGlue());
    JButton button = new JButton("Inspect");
    button.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
          fireShowOopInspector();
        }
      });
    box.add(button);

    box.add(Box.createHorizontalStrut(20));

    // Liveness button
    button = new JButton();
    livenessButton = button;
    if (liveness == null) {
      button.setText("Compute Liveness");
      livenessButtonListener = new ActionListener() {
          public void actionPerformed(ActionEvent e) {
            fireComputeLiveness();
          }
        };
    } else {
      button.setText("Show Liveness");
      livenessButtonListener = new ActionListener() {
          public void actionPerformed(ActionEvent e) {
            fireShowLiveness();
          }
        };
    }
    button.addActionListener(livenessButtonListener);
    box.add(button);
    box.add(Box.createGlue());
    panel.add(box);
    add(panel, BorderLayout.SOUTH);
  }

  public interface Listener {
    public void showOopInspector(Oop oop);
    public void showLiveness(Oop oop, LivenessPathList liveness);
  }

  public void addPanelListener(Listener listener) {
    listeners.add(listener);
  }

  public void removePanelListener(Listener listener) {
    listeners.remove(listener);
  }

  //--------------------------------------------------------------------------------
  // Internals only below this point
  //

  private void fireShowOopInspector() {
    int i = table.getSelectedRow();
    if (i < 0) {
      return;
    }

    Oop oop = (Oop) elements.get(i);

    for (Iterator iter = listeners.iterator(); iter.hasNext(); ) {
      Listener listener = (Listener) iter.next();
      listener.showOopInspector(oop);
    }
  }

  private void fireComputeLiveness() {
    if (liveness != null) return;

    final WorkerThread worker = new WorkerThread();
    worker.invokeLater(new Runnable() {
        public void run() {
          livenessButton.setEnabled(false);
          LivenessAnalysis analysis = new LivenessAnalysis();
          if (thunk != null) {
            analysis.setProgressThunk(thunk);
          }
          Set set = new HashSet();
          set.addAll(elements);
          analysis.setTargetObjects(set);
          analysis.run();
          liveness = analysis.result();
          livenessButton.removeActionListener(livenessButtonListener);
          livenessButtonListener = null;
          livenessButton.addActionListener(new ActionListener() {
              public void actionPerformed(ActionEvent e) {
                fireShowLiveness();
              }
            });
          computeNumColumns();
          livenessButton.setEnabled(true);
          livenessButton.setText(showLivenessText);
          dataModel.fireTableStructureChanged();
          worker.shutdown();
        }
      });
  }

  private void fireShowLiveness() {
    if (liveness == null) return;

    int i = table.getSelectedRow();
    if (i < 0) {
      return;
    }

    Oop oop = (Oop) elements.get(i);
    LivenessPathList list = (LivenessPathList) liveness.get(oop);
    if (list == null) {
      return; // dead object
    }
    
    for (Iterator iter = listeners.iterator(); iter.hasNext(); ) {
      Listener listener = (Listener) iter.next();
      listener.showLiveness(oop, list);
    }
  }

  private void checkForArrays() {
    if (checkedForArrays) return;
    checkedForArrays = true;
    for (Iterator iter = elements.iterator(); iter.hasNext(); ) {
      if (iter.next() instanceof Array) {
        hasArrays = true;
        return;
      }
    }
  }

  private void computeNumColumns() {
    checkForArrays();
    numColumns = 3;
    if (hasArrays)        ++numColumns;
    if (liveness != null || revPtrs != null) ++numColumns;
  }
}
