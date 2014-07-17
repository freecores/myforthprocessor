/*
 * @(#)HSDB.java	1.32 03/01/23 11:13:54
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot;

import java.io.*;
import java.awt.*;
import java.awt.event.*;
import java.math.*;
import javax.swing.*;
import javax.swing.tree.*;
import java.util.*;

import sun.jvm.hotspot.code.*;
import sun.jvm.hotspot.compiler.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.interpreter.*;
import sun.jvm.hotspot.memory.*;
import sun.jvm.hotspot.oops.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.ui.*;
import sun.jvm.hotspot.ui.tree.*;
import sun.jvm.hotspot.ui.classbrowser.*;
import sun.jvm.hotspot.utilities.*;

/** The top-level HotSpot Debugger. FIXME: make this an embeddable
    component! (Among other things, figure out what to do with the
    menu bar...) */

public class HSDB implements JavaThreadsPanel.Listener, ObjectHistogramPanel.Listener, ObjectListPanel.Listener {
  public static void main(String[] args) {
    new HSDB(args).run();
  }

  //--------------------------------------------------------------------------------
  // Internals only below this point
  //
  private HotSpotAgent agent;
  private JDesktopPane desktop;
  private boolean      attached;
  /** List <JMenuItem> */
  private java.util.List attachMenuItems;
  /** List <JMenuItem> */
  private java.util.List detachMenuItems;
  private JMenu toolsMenu;
  private JMenuItem showConsoleMenuItem;
  private JMenuItem computeRevPtrsMenuItem;
  private JInternalFrame attachWaitDialog;
  private JInternalFrame threadsFrame;
  private JInternalFrame consoleFrame;
  private WorkerThread workerThread;
  // These had to be made data members because they are referenced in inner classes.
  private String pidText;
  private int pid;
  private String execPath;
  private String coreFilename;

  private void doUsage() {
    System.out.println("Usage:  java HSDB [[pid] | [path-to-java-executable [path-to-corefile]] | help ]");
    System.out.println("           pid:                     attach to the process whose id is 'pid'");
    System.out.println("           path-to-java-executable: Debug a core file produced by this program");
    System.out.println("           path-to-corefile:        Debug this corefile.  The default is 'core'");
    System.out.println("        If no arguments are specified, you can select what to do from the GUI.\n");
    HotSpotAgent.showUsage();
  }

  private HSDB(String[] args) {
    switch (args.length) {
    case (0):
      break;

    case (1):
      if (args[0].equals("help") || args[0].equals("-help")) {
        doUsage();
        System.exit(0);
      }
      // If all numbers, it is a PID to attach to
      // Else, it is a pathname to a .../bin/java for a core file.
      try {
        int unused = Integer.parseInt(args[0]);
        // If we get here, we have a PID and not a core file name
        pidText = args[0];
      } catch (NumberFormatException e) {
        execPath = args[0];
        coreFilename = "core";
      }
      break;
        
    case (2):
      execPath = args[0];
      coreFilename = args[1];
      break;

    default:
      System.out.println("HSDB Error: Too many options specified");
      doUsage();
      System.exit(1);
    }
  }

  private void run() {
    // At this point, if pidText != null we are supposed to attach to it.
    // Else, if execPath != null, it is the path of a jdk/bin/java
    // and coreFilename is the pathname of a core file we are
    // supposed to attach to.

    agent = new HotSpotAgent();
    workerThread = new WorkerThread();
    attachMenuItems = new java.util.ArrayList();
    detachMenuItems = new java.util.ArrayList();

    JFrame frame = new JFrame("HSDB - HotSpot Debugger");
    frame.setSize(800, 600);
    frame.setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);

    JMenuBar menuBar = new JMenuBar();

    //
    // File menu
    //

    JMenu menu = new JMenu("File");
    JMenuItem item;
    item = createMenuItem("Attach to HotSpot process...",
                          new ActionListener() {
                              public void actionPerformed(ActionEvent e) {
                                showAttachDialog();
                              }
                            });
    menu.add(item);
    attachMenuItems.add(item);

    item = createMenuItem("Open HotSpot core file...",
                          new ActionListener() {
                              public void actionPerformed(ActionEvent e) {
                                showOpenCoreFileDialog();
                              }
                            });
    menu.add(item);
    attachMenuItems.add(item);

    item = createMenuItem("Connect to debug server...",
                          new ActionListener() {
                              public void actionPerformed(ActionEvent e) {
                                showConnectDialog();
                              }
                            });
    menu.add(item);
    attachMenuItems.add(item);

    item = createMenuItem("Detach",
                          new ActionListener() {
                              public void actionPerformed(ActionEvent e) {
                                detach();
                              }
                            });
    menu.add(item);
    detachMenuItems.add(item);
    
    // Disable detach menu items at first
    setMenuItemsEnabled(detachMenuItems, false);

    menu.addSeparator();

    menu.add(createMenuItem("Exit",
                            new ActionListener() {
                                public void actionPerformed(ActionEvent e) {
                                  System.exit(0);
                                }
                              }));

    menuBar.add(menu);

    //
    // Tools menu
    //

    toolsMenu = new JMenu("Tools");
    item = createMenuItem("Find Pointer",
                          new ActionListener() {
                              public void actionPerformed(ActionEvent e) {
                                showFindPanel();
                              }
                            });
    toolsMenu.add(item);

    item = createMenuItem("Find In Heap",
                          new ActionListener() {
                              public void actionPerformed(ActionEvent e) {
                                showFindInHeapPanel();
                              }
                            });
    toolsMenu.add(item);

    item = createMenuItem("Heap Parameters",
                          new ActionListener() {
                              public void actionPerformed(ActionEvent e) {
                                showHeapParametersPanel();
                              }
                            });
    toolsMenu.add(item);

    item = createMenuItem("Deadlock Detection",
                          new ActionListener() {
                              public void actionPerformed(ActionEvent e) {
                                showDeadlockDetectionPanel();
                              }
                            });
    toolsMenu.add(item);

    item = createMenuItem("Monitor Cache Dump",
                          new ActionListener() {
                              public void actionPerformed(ActionEvent e) {
                                showMonitorCacheDumpPanel();
                              }
                            });
    toolsMenu.add(item);

    item = createMenuItem("Object Histogram",
                          new ActionListener() {
                              public void actionPerformed(ActionEvent e) {
                                showObjectHistogram();
                              }
                            });
    toolsMenu.add(item);

    item = createMenuItem("Oop Inspector",
                          new ActionListener() {
                              public void actionPerformed(ActionEvent e) {
                                showOopInspector(null);
                              }
                            });
    toolsMenu.add(item);

    item = createMenuItem("Compute Reverse Ptrs",
                          new ActionListener() {
                              public void actionPerformed(ActionEvent e) {
                                fireComputeReversePtrs();
                              }
                            });
    computeRevPtrsMenuItem = item;
    toolsMenu.add(item);

    item = createMenuItem("Class Browser",
                          new ActionListener() {
                             public void actionPerformed(ActionEvent e) {
                                showClassBrowser();
                             }
                          });
    toolsMenu.add(item);

    item = createMenuItem("Code Viewer",
                          new ActionListener() {
                             public void actionPerformed(ActionEvent e) {
                                showCodeViewer();
                             }
                          });
    toolsMenu.add(item);

    toolsMenu.setEnabled(false);
    menuBar.add(toolsMenu);

    //
    // Windows menu
    //

    JMenu windowsMenu = new JMenu("Windows");
    showConsoleMenuItem = createMenuItem("Debugger Console",
                                         new ActionListener() {
                                             public void actionPerformed(ActionEvent e) {
                                               showDebuggerConsole();
                                             }
                                           });
    windowsMenu.add(showConsoleMenuItem);
    showConsoleMenuItem.setEnabled(false);

    menuBar.add(windowsMenu);
    

    frame.setJMenuBar(menuBar);

    desktop = new JDesktopPane();
    frame.getContentPane().add(desktop);
    GraphicsUtilities.reshapeToAspectRatio(frame, 4.0f/3.0f, 0.75f, Toolkit.getDefaultToolkit().getScreenSize());
    GraphicsUtilities.centerInContainer(frame, Toolkit.getDefaultToolkit().getScreenSize());
    frame.show();

    Runtime.getRuntime().addShutdownHook(new java.lang.Thread() {
        public void run() {
          detachDebugger();
        }
      });

    if (pidText != null) {
      attach(pidText);
    } else if (execPath != null) {
      attach(execPath, coreFilename);
    }
  }

  // FIXME: merge showAttachDialog, showOpenCoreFileDialog, showConnectDialog
  private void showAttachDialog() {
    // FIXME: create filtered text field which only accepts numbers
    setMenuItemsEnabled(attachMenuItems, false);
    final JInternalFrame attachDialog = new JInternalFrame("Attach to HotSpot process");
    attachDialog.getContentPane().setLayout(new BorderLayout());

    JPanel panel = new JPanel();
    panel.setLayout(new BoxLayout(panel, BoxLayout.X_AXIS));
    panel.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));
    attachDialog.setBackground(panel.getBackground());

    panel.add(new JLabel("Enter process ID:"));
    final JTextField pidTextField = new JTextField(10);
    ActionListener attacher = new ActionListener() {
        public void actionPerformed(ActionEvent e) {
          attachDialog.setVisible(false);
          desktop.remove(attachDialog);
          workerThread.invokeLater(new Runnable() {
              public void run() {
                attach(pidTextField.getText());
              }
            });
        }
      };

    pidTextField.addActionListener(attacher);
    panel.add(pidTextField);
    attachDialog.getContentPane().add(panel, BorderLayout.NORTH);

    Box vbox = Box.createVerticalBox();
    panel = new JPanel();
    panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));
    panel.setBorder(BorderFactory.createEmptyBorder(8, 8, 8, 8));
    JTextArea ta = new JTextArea(
                                 "Enter the process ID of a currently-running HotSpot process. On " +
                                 "Solaris and most Unix operating systems, this can be determined by " +
                                 "typing \"ps -u <your username> | grep java\"; the process ID is the " +
                                 "first number which appears on the resulting line. On Windows, the " +
                                 "process ID is present in the Task Manager, which can be brought up " +
                                 "while logged on to the desktop by pressing Ctrl-Alt-Delete.");
    ta.setLineWrap(true);
    ta.setWrapStyleWord(true);
    ta.setEditable(false);
    ta.setBackground(panel.getBackground());
    panel.add(ta);
    vbox.add(panel);

    Box hbox = Box.createHorizontalBox();
    hbox.add(Box.createGlue());
    JButton button = new JButton("OK");
    button.addActionListener(attacher);
    hbox.add(button);
    hbox.add(Box.createHorizontalStrut(20));
    button = new JButton("Cancel");
    button.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
          attachDialog.setVisible(false);
          desktop.remove(attachDialog);
          setMenuItemsEnabled(attachMenuItems, true);
        }
      });
    hbox.add(button);
    hbox.add(Box.createGlue());
    panel = new JPanel();
    panel.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));
    panel.add(hbox);
    vbox.add(panel);

    attachDialog.getContentPane().add(vbox, BorderLayout.SOUTH);

    desktop.add(attachDialog);
    attachDialog.setSize(400, 300);
    GraphicsUtilities.centerInContainer(attachDialog);
    attachDialog.show();
    pidTextField.requestFocus();
  }

  // FIXME: merge showAttachDialog, showOpenCoreFileDialog, showConnectDialog
  private void showOpenCoreFileDialog() {
    setMenuItemsEnabled(attachMenuItems, false);
    final JInternalFrame dialog = new JInternalFrame("Open Core File");
    dialog.getContentPane().setLayout(new BorderLayout());

    JPanel panel = new JPanel();
    panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));
    panel.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));
    dialog.setBackground(panel.getBackground());

    Box hbox = Box.createHorizontalBox();
    Box vbox = Box.createVerticalBox();
    vbox.add(new JLabel("Path to core file:"));
    vbox.add(new JLabel("Path to Java executable:"));
    hbox.add(vbox);

    vbox = Box.createVerticalBox();
    final JTextField corePathField = new JTextField(40);
    final JTextField execPathField = new JTextField(40);
    vbox.add(corePathField);
    vbox.add(execPathField);
    hbox.add(vbox);

    final JButton browseCorePath = new JButton("Browse ..");
    final JButton browseExecPath = new JButton("Browse ..");
    browseCorePath.addActionListener(new ActionListener() {
                                        public void actionPerformed(ActionEvent e) {
                                           JFileChooser fileChooser = new JFileChooser(new File("."));
                                           int retVal = fileChooser.showOpenDialog(dialog);
                                           if (retVal == JFileChooser.APPROVE_OPTION) {
                                              corePathField.setText(fileChooser.getSelectedFile().getPath());
                                           }
                                        }
                                     });
    browseExecPath.addActionListener(new ActionListener() {
                                        public void actionPerformed(ActionEvent e) {
                                           JFileChooser fileChooser = new JFileChooser(new File("."));
                                           int retVal = fileChooser.showOpenDialog(dialog);
                                           if (retVal == JFileChooser.APPROVE_OPTION) {
                                              execPathField.setText(fileChooser.getSelectedFile().getPath());
                                           }
                                        }
                                     });
    vbox = Box.createVerticalBox();
    vbox.add(browseCorePath);
    vbox.add(browseExecPath);
    hbox.add(vbox);

    panel.add(hbox);
    dialog.getContentPane().add(panel, BorderLayout.NORTH);

    ActionListener attacher = new ActionListener() {
        public void actionPerformed(ActionEvent e) {
          dialog.setVisible(false);
          desktop.remove(dialog);
          workerThread.invokeLater(new Runnable() {
              public void run() {
                attach(execPathField.getText(), corePathField.getText());
              }
            });
        }
      };
    corePathField.addActionListener(attacher);
    execPathField.addActionListener(attacher);
    
    vbox = Box.createVerticalBox();
    panel = new JPanel();
    panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));
    panel.setBorder(BorderFactory.createEmptyBorder(8, 8, 8, 8));
    JTextArea ta = new JTextArea(
                                 "Enter the full path names to the core file from a HotSpot process " +
                                 "and the Java executable from which it came. The latter is typically " +
                                 "located in the JDK/JRE directory under the directory " +
                                 "jre/bin/<arch>/native_threads.");
    ta.setLineWrap(true);
    ta.setWrapStyleWord(true);
    ta.setEditable(false);
    ta.setBackground(panel.getBackground());
    panel.add(ta);
    vbox.add(panel);

    hbox = Box.createHorizontalBox();
    hbox.add(Box.createGlue());
    JButton button = new JButton("OK");
    button.addActionListener(attacher);
    hbox.add(button);
    hbox.add(Box.createHorizontalStrut(20));
    button = new JButton("Cancel");
    button.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
          dialog.setVisible(false);
          desktop.remove(dialog);
          setMenuItemsEnabled(attachMenuItems, true);
        }
      });
    hbox.add(button);
    hbox.add(Box.createGlue());
    panel = new JPanel();
    panel.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));
    panel.add(hbox);
    vbox.add(panel);

    dialog.getContentPane().add(vbox, BorderLayout.SOUTH);

    desktop.add(dialog);
    dialog.setSize(500, 300);
    GraphicsUtilities.centerInContainer(dialog);
    dialog.show();
    corePathField.requestFocus();
  }

  // FIXME: merge showAttachDialog, showOpenCoreFileDialog, showConnectDialog
  private void showConnectDialog() {
    // FIXME: create filtered text field which only accepts numbers
    setMenuItemsEnabled(attachMenuItems, false);
    final JInternalFrame dialog = new JInternalFrame("Connect to HotSpot Debug Server");
    dialog.getContentPane().setLayout(new BorderLayout());

    JPanel panel = new JPanel();
    panel.setLayout(new BoxLayout(panel, BoxLayout.X_AXIS));
    panel.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));
    dialog.setBackground(panel.getBackground());

    panel.add(new JLabel("Enter machine name:"));
    final JTextField pidTextField = new JTextField(40);
    ActionListener attacher = new ActionListener() {
        public void actionPerformed(ActionEvent e) {
          dialog.setVisible(false);
          desktop.remove(dialog);
          workerThread.invokeLater(new Runnable() {
              public void run() {
                connect(pidTextField.getText());
              }
            });
        }
      };

    pidTextField.addActionListener(attacher);
    panel.add(pidTextField);
    dialog.getContentPane().add(panel, BorderLayout.NORTH);

    Box vbox = Box.createVerticalBox();
    panel = new JPanel();
    panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));
    panel.setBorder(BorderFactory.createEmptyBorder(8, 8, 8, 8));
    JTextArea ta = new JTextArea(
                                 "Enter the name of a machine on which the HotSpot \"Debug Server\" is " +
                                 "running and is attached to a process or core file.");
    ta.setLineWrap(true);
    ta.setWrapStyleWord(true);
    ta.setEditable(false);
    ta.setBackground(panel.getBackground());
    panel.add(ta);
    vbox.add(panel);

    Box hbox = Box.createHorizontalBox();
    hbox.add(Box.createGlue());
    JButton button = new JButton("OK");
    button.addActionListener(attacher);
    hbox.add(button);
    hbox.add(Box.createHorizontalStrut(20));
    button = new JButton("Cancel");
    button.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
          dialog.setVisible(false);
          desktop.remove(dialog);
          setMenuItemsEnabled(attachMenuItems, true);
        }
      });
    hbox.add(button);
    hbox.add(Box.createGlue());
    panel = new JPanel();
    panel.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));
    panel.add(hbox);
    vbox.add(panel);

    dialog.getContentPane().add(vbox, BorderLayout.SOUTH);

    desktop.add(dialog);
    dialog.setSize(400, 300);
    GraphicsUtilities.centerInContainer(dialog);
    dialog.show();
    pidTextField.requestFocus();
  }

  public void showThreadOopInspector(JavaThread thread) {
    showOopInspector(thread.getThreadObj());
  }

  public void showOopInspector(Oop oop) {
    JInternalFrame inspectorFrame = new JInternalFrame("Oop Inspector");
    inspectorFrame.getContentPane().setLayout(new BorderLayout());
    inspectorFrame.setResizable(true);
    inspectorFrame.setClosable(true);
    inspectorFrame.setIconifiable(true);
    OopTreeNodeAdapter adapter = null;
    if (oop != null) {
      adapter = new OopTreeNodeAdapter(oop, null);
    }
    inspectorFrame.getContentPane().add(new Inspector(adapter), BorderLayout.CENTER);
    desktop.add(inspectorFrame);
    GraphicsUtilities.reshapeToAspectRatio(inspectorFrame, 1.0f, 0.65f, inspectorFrame.getParent().getSize());
    inspectorFrame.show();
  }

  public void showLiveness(Oop oop, LivenessPathList liveness) {
    ByteArrayOutputStream bos = new ByteArrayOutputStream();
    PrintStream tty = new PrintStream(bos);
    for (int i = 0; i < liveness.size(); i++) {
      LivenessPath path = liveness.get(i);
      tty.println("Path " + (i + 1) + " of " + liveness.size() + ":");
      for (int j = 0; j < path.size(); j++) {
        LivenessPathElement el = path.get(j);
        tty.print("  - ");
        if (el.getObj() != null) {
          el.getObj().printValueOn(tty);
        }
        if (el.getField() != null) {
          if (el.getObj() != null) {
            tty.print(", field ");
          }
          tty.print(el.getField().getName());
        }
        tty.println();
      }
    }
    JTextArea ta = new JTextArea(bos.toString());
    ta.setLineWrap(true);
    ta.setWrapStyleWord(true);
    ta.setEditable(false);
    
    JPanel panel = new JPanel();
    panel.setLayout(new BorderLayout());

    JScrollPane scroller = new JScrollPane();
    scroller.getViewport().add(ta);

    panel.add(scroller, BorderLayout.CENTER);
    
    bos = new ByteArrayOutputStream();
    tty = new PrintStream(bos);
    tty.print("Liveness result for ");
    oop.printValueOn(tty);

    JInternalFrame frame = new JInternalFrame(bos.toString());
    frame.setResizable(true);
    frame.setClosable(true);
    frame.setIconifiable(true);
    frame.getContentPane().setLayout(new BorderLayout());
    frame.getContentPane().add(panel, BorderLayout.CENTER);
    frame.pack();
    desktop.add(frame);
    GraphicsUtilities.reshapeToAspectRatio(frame, 0.5f / 0.2f, 0.5f, frame.getParent().getSize());
    frame.show();
  }

  private void fireComputeReversePtrs() {
    workerThread.invokeLater(new Runnable() {
        public void run() {
          VM.registerVMInitializedObserver(new Observer() {
              public void update(Observable o, Object data) {
                VM.getVM().setRevPtrs(null);
                computeRevPtrsMenuItem.setEnabled(true);
              }
            });
          VM.getVM().setRevPtrs(new ReversePtrs());
          ReversePtrsAnalysis analysis = new ReversePtrsAnalysis();
          analysis.setProgressThunk(new ReversePtrsProgress());
          analysis.run();
          computeRevPtrsMenuItem.setEnabled(false);
        }
      });
  }

  // Simple struct containing signal information
  class SignalInfo {
    public int sigNum;
    public String sigName;
  }

  // Need to have mutable vframe as well as visible memory panel
  abstract class StackWalker implements Runnable {
    protected JavaVFrame vf;
    protected AnnotatedMemoryPanel annoPanel;

    StackWalker(JavaVFrame vf, AnnotatedMemoryPanel annoPanel) {
      this.vf = vf;
      this.annoPanel = annoPanel;
    }
  }

  public void showThreadStackMemory(final JavaThread thread) {
    // dumpStack(thread);
    JavaVFrame vframe = getLastJavaVFrame(thread);
    if (vframe == null) {
      JOptionPane.showInternalMessageDialog(desktop,
                                            "Thread \"" + thread.getThreadName() +
                                            "\" has no Java frames on its stack",
                                            "Show Stack Memory",
                                            JOptionPane.INFORMATION_MESSAGE);
      return;
    }

    JInternalFrame stackFrame = new JInternalFrame("Stack Memory for " + thread.getThreadName());
    stackFrame.getContentPane().setLayout(new BorderLayout());
    stackFrame.setResizable(true);
    stackFrame.setClosable(true);
    stackFrame.setIconifiable(true);
    final long addressSize = agent.getTypeDataBase().getAddressSize();
    boolean is64Bit = (addressSize == 8);
    // This is somewhat of a  hack to guess a thread's stack limits since the
    // JavaThread doesn't support this functionality. However it is nice in that
    // it locks us into the active region of the thread's stack and not its
    // theoretical limits.
    //
    sun.jvm.hotspot.runtime.Frame tmpFrame = thread.getCurrentFrameGuess();
    Address sp = tmpFrame.getSP();
    Address starting = sp;
    Address maxSP = starting;
    Address minSP = starting;
    RegisterMap tmpMap = thread.newRegisterMap(false);
    while ((tmpFrame != null) && (!tmpFrame.isFirstFrame())) {
	tmpFrame = tmpFrame.sender(tmpMap);
	if (tmpFrame != null) {
	  sp = tmpFrame.getSP();
	  if (sp != null) {
	    maxSP = AddressOps.max(maxSP, sp);
	    minSP = AddressOps.min(minSP, sp);
	  }
	}

    }
    // It is useful to be able to see say +/- 8K on the current stack range
    AnnotatedMemoryPanel annoMemPanel = new AnnotatedMemoryPanel(agent.getDebugger(), is64Bit, starting, 
								 minSP.addOffsetTo(-8192),
								 maxSP.addOffsetTo( 8192));

    stackFrame.getContentPane().add(annoMemPanel, BorderLayout.CENTER);
    desktop.add(stackFrame);
    GraphicsUtilities.reshapeToAspectRatio(stackFrame, 4.0f / 3.0f, 0.85f, stackFrame.getParent().getSize());
    stackFrame.show();

    // Stackmap computation for interpreted frames is expensive; do
    // all stackwalking work in another thread for better GUI
    // responsiveness
    workerThread.invokeLater(new StackWalker(vframe, annoMemPanel) {
        public void run() {
          Address startAddr = null;
    
          // As this is a debugger, we want to provide potential crash
          // information to the user, i.e., by marking signal handler frames
          // on the stack. Since this system is currently targeted at
          // annotating the Java frames (interpreted or compiled) on the
          // stack and not, for example, "external" frames (note the current
          // absence of a PC-to-symbol lookup mechanism at the Debugger
          // level), we want to mark any Java frames which were interrupted
          // by a signal. We do this by making two passes over the stack,
          // one which finds signal handler frames and puts the parent
          // frames in a table and one which finds Java frames and if they
          // are in the table indicates that they were interrupted by a signal.

          Map interruptedFrameMap = new HashMap();
          {
            sun.jvm.hotspot.runtime.Frame tmpFrame = thread.getCurrentFrameGuess();
            RegisterMap tmpMap = thread.newRegisterMap(false);
            while ((tmpFrame != null) && (!tmpFrame.isFirstFrame())) {
              if (tmpFrame.isSignalHandlerFrameDbg()) {
                // Add some information to the map that we can extract later
                sun.jvm.hotspot.runtime.Frame interruptedFrame = tmpFrame.sender(tmpMap);
                SignalInfo info = new SignalInfo();
                info.sigNum  = tmpFrame.getSignalNumberDbg();
                info.sigName = tmpFrame.getSignalNameDbg();
                interruptedFrameMap.put(interruptedFrame, info);
              }
              tmpFrame = tmpFrame.sender(tmpMap);
            }
          }

          sun.jvm.hotspot.runtime.Frame lastFrame = null;
          while (vf != null) {
            Method method = vf.getMethod();
            int bci = vf.getBCI();
            String lineNumberAnno = "";
            if (method.hasLineNumberTable()) {
              if ((bci == DebugInformationRecorder.SYNCHRONIZATION_ENTRY_BCI) ||
                  (bci >= 0 && bci < method.getCodeSize())) {
                lineNumberAnno = ", line " + method.getLineNumberFromBCI(bci);
              } else {
                lineNumberAnno = " (INVALID BCI)";
              }
            }
            String anno = (method.getMethodHolder().getName().asString() + "." +
                           method.getName().asString() + method.getSignature().asString() +
                           "\n@bci " + bci + lineNumberAnno);
            if (vf.isCompiledFrame()) {
              anno = anno + " (Compiled frame";
            } else if (vf.isInterpretedFrame()) {
              anno = anno + " (Interpreted frame";
            }
            if (vf.mayBeImpreciseDbg()) {
              anno = anno + "; information may be imprecise";
            }
            anno = anno + ")";
            if (vf.isInterpretedFrame()) {
              // Find the codelet
              InterpreterCodelet codelet = VM.getVM().getInterpreter().getCodeletContaining(vf.getFrame().getPC());
              String description = null;
              if (codelet != null) {
                description = codelet.getDescription();
              }
              if (description == null) {
                anno = anno + "\n(Unknown interpreter codelet)";
              } else {
                anno = anno + "\nExecuting in codelet \"" + description + "\"\nat PC = " + vf.getFrame().getPC();
              }
            } else if (vf.isCompiledFrame()) {
              anno = anno + "\nExecuting at PC = " + vf.getFrame().getPC();
            }

            // FIXME: some compiled frames with empty oop map sets have been
            // found (for example, Vector's inner Enumeration class, method
            // "hasMoreElements"). Not sure yet why these cases are showing
            // up -- should be possible (though unlikely) for safepoint code
            // to patch the return instruction of these methods and then
            // later attempt to get an oop map for that instruction. For
            // now, we warn if we find such a method.
            boolean shouldSkipOopMaps = false;
            if (vf.isCompiledFrame()) {
              CodeBlob cb = VM.getVM().getCodeCache().findBlob(vf.getFrame().getPC());
              OopMapSet maps = cb.getOopMaps();
              if ((maps == null) || (maps.getSize() == 0)) {
                shouldSkipOopMaps = true;
              }
            }

            // NOTE: redundant test here (see below) to simplify the overall
            // control flow
            if ((lastFrame == null) || (!vf.getFrame().equals(lastFrame))) {
              // Add signal information to annotation if necessary
              SignalInfo sigInfo = (SignalInfo) interruptedFrameMap.get(vf.getFrame());
              if (sigInfo != null) {
                // This frame took a signal and we need to report it.
                anno = (anno + "\n*** INTERRUPTED BY SIGNAL " + Integer.toString(sigInfo.sigNum) +
                        " (" + sigInfo.sigName + ")");
              }
            }

            if (shouldSkipOopMaps) {
              anno = anno + "\nNOTE: null or empty OopMapSet found for this CodeBlob";
            }

            // FIXME: THIS IS DEPENDENT ON THE STACK DIRECTION!!!
            // Could make Annotation determine which is the low or high endpoint.
            // That would solve the problem pretty easily, I think.
            // NOTE: here, stack grows toward smaller addresses, so FP > SP
            if (vf.getFrame().getFP() != null) {
              annoPanel.addAnnotation(new Annotation(vf.getFrame().getSP(),
                                                     vf.getFrame().getFP(),
                                                     anno));
            } else {
              // For C2, which has null frame pointers on x86
              sun.jvm.hotspot.runtime.Frame fr = vf.getFrame();
              CodeBlob cb = VM.getVM().getCodeCache().findBlob(fr.getPC());
              Address sp = fr.getSP();
              if (Assert.ASSERTS_ENABLED) {
                Assert.that(cb.getFrameSize() > 0, "CodeBlob must have non-zero frame size");
              }
              annoPanel.addAnnotation(new Annotation(sp,
                                                     sp.addOffsetTo(cb.getFrameSize()),
                                                     anno));
            }
            // Add in annotation of plausible oops to show the point
            // FIXME: this is dependent on stack direction too
            //      Address sp = vf.getFrame().getSP();
            //      Address fp = vf.getFrame().getFP();
            //      while (sp.lessThan(fp)) {
            //        if (RobustOopDeterminator.oopLooksValid(sp.getOopHandleAt(0))) {
            //          annoPanel.addAnnotation(new Annotation(sp, sp.addOffsetTo(addressSize),
            //                                                 "plausible oop"));
            //        }
            //        sp = sp.addOffsetTo(addressSize);
            //      }

            if ((lastFrame == null) || (!vf.getFrame().equals(lastFrame))) {
              lastFrame = vf.getFrame();
              // Annotate with oop map information
              RegisterMap rm = (RegisterMap) vf.getRegisterMap().clone();
              if (!shouldSkipOopMaps) {
                lastFrame.oopsDo(new AddressVisitor() {
                    public void visitAddress(Address addr) {
                      if (Assert.ASSERTS_ENABLED) {
                        Assert.that(addr.andWithMask(VM.getVM().getAddressSize() - 1) == null,
                                    "Address " + addr + "should have been aligned");
                      }
                      // Check contents
                      OopHandle handle = addr.getOopHandleAt(0);
                      String anno = "null oop";
                      if (handle != null) {
                        // Find location
                        GenCollectedHeap heap = (GenCollectedHeap) VM.getVM().getUniverse().heap();
                        boolean bad = true;
                        anno = "BAD OOP";
                        for (int i = 0; i < heap.nGens(); i++) {
                          if (heap.getGen(i).isIn(handle)) {
                            if (i == 0) {
                              anno = "NewGen ";
                            } else if (i == 1) {
                              anno = "OldGen ";
                            } else {
                              anno = "Gen " + i + " ";
                            }
                            bad = false;
                            break;
                          }
                        }

                        if (bad) {
                          // Try perm gen
                          if (heap.permGen().isIn(handle)) {
                            anno = "PermGen ";
                            bad = false;
                          }
                        }

                        if (!bad) {
                          try {
                            Oop oop = VM.getVM().getObjectHeap().newOop(handle);
                            if (oop instanceof Instance) {
                              // Java-level objects always have workable names
                              anno = anno + oop.getKlass().getName().asString();
                            } else {
                              ByteArrayOutputStream bos = new ByteArrayOutputStream();
                              oop.printValueOn(new PrintStream(bos));
                              anno = anno + bos.toString();
                            }
                          }
                          catch (AddressException e) {
                            anno += "CORRUPT OOP";
                          }
                          catch (NullPointerException e) {
                            anno += "CORRUPT OOP (null pointer)";
                          }
                        }
                      }
                
                      annoPanel.addAnnotation(new Annotation(addr, addr.addOffsetTo(addressSize), anno));
                    }
                  }, rm);
              }
            }

	    if (startAddr == null) {
	      startAddr = vf.getFrame().getSP();
	    }
            vf = vf.javaSender();
          }
	  // This used to paint as we walked the frames. This caused the display to be refreshed
	  // enough to be annoying on remote displays. It also would cause the annotations to
	  // be displayed in varying order which caused some annotations to overwrite others
	  // depending on the races between painting and adding annotations. This latter problem
	  // still exists to some degree but moving this code here definitely seems to reduce it
	  annoPanel.makeVisible(startAddr);
	  annoPanel.repaint();
        }
      });
  }

  /** NOTE we are in a different thread here than either the main
      thread or the Swing/AWT event handler thread, so we must be very
      careful when creating or removing widgets */
  private void attach(String pidText) {
      try {
      this.pidText = pidText;
      pid = Integer.parseInt(pidText);
    }
    catch (NumberFormatException e) {
      SwingUtilities.invokeLater(new Runnable() {
          public void run() {
            setMenuItemsEnabled(attachMenuItems, true);
            JOptionPane.showInternalMessageDialog(desktop,
                                                  "Unable to parse process ID \"" + HSDB.this.pidText + "\".\nPlease enter a number.",
                                                  "Parse error",
                                                  JOptionPane.WARNING_MESSAGE);
          }
        });
      return;
    }

    // Try to attach to this process
    Runnable remover = new Runnable() {
          public void run() {
            attachWaitDialog.setVisible(false);
            desktop.remove(attachWaitDialog);
            attachWaitDialog = null;
          }
      };

    try {
      SwingUtilities.invokeLater(new Runnable() {
          public void run() {
            JOptionPane pane = new JOptionPane("Attaching to process " + pid + ", please wait...", JOptionPane.INFORMATION_MESSAGE);
            pane.setOptions(new Object[] {});
            attachWaitDialog = pane.createInternalFrame(desktop, "Attaching to Process");
            attachWaitDialog.show();
          }
        });

      // FIXME: display exec'd debugger's output messages during this
      // lengthy call
      agent.attach(pid);
      if (agent.getDebugger().hasConsole()) {
        showConsoleMenuItem.setEnabled(true);
      }
      attached = true;
      SwingUtilities.invokeLater(remover);
    }
    catch (DebuggerException e) {
      SwingUtilities.invokeLater(remover);
      final String errMsg = formatMessage(e.getMessage(), 80);
      SwingUtilities.invokeLater(new Runnable() {
          public void run() {
            setMenuItemsEnabled(attachMenuItems, true);
            JOptionPane.showInternalMessageDialog(desktop,
                                                  "Unable to connect to process ID " + pid + ":\n\n" + errMsg,
                                                  "Unable to Connect",
                                                  JOptionPane.WARNING_MESSAGE);
          }
        });
      agent.detach();
      return;
    }

    // OK, the VM should be available. Create the Threads dialog.
    showThreadsDialog();
  }

  /** NOTE we are in a different thread here than either the main
      thread or the Swing/AWT event handler thread, so we must be very
      careful when creating or removing widgets */
  private void attach(final String executablePath, final String corePath) {
    // Try to open this core file
    Runnable remover = new Runnable() {
          public void run() {
            attachWaitDialog.setVisible(false);
            desktop.remove(attachWaitDialog);
            attachWaitDialog = null;
          }
      };

    try {
      SwingUtilities.invokeLater(new Runnable() {
          public void run() {
            JOptionPane pane = new JOptionPane("Opening core file, please wait...", JOptionPane.INFORMATION_MESSAGE);
            pane.setOptions(new Object[] {});
            attachWaitDialog = pane.createInternalFrame(desktop, "Opening Core File");
            attachWaitDialog.show();
          }
        });

      // FIXME: display exec'd debugger's output messages during this
      // lengthy call
      agent.attach(executablePath, corePath);
      if (agent.getDebugger().hasConsole()) {
        showConsoleMenuItem.setEnabled(true);
      }
      attached = true;
      SwingUtilities.invokeLater(remover);
    }
    catch (DebuggerException e) {
      SwingUtilities.invokeLater(remover);
      final String errMsg = formatMessage(e.getMessage(), 80);
      SwingUtilities.invokeLater(new Runnable() {
          public void run() {
            setMenuItemsEnabled(attachMenuItems, true);
            JOptionPane.showInternalMessageDialog(desktop,
                                                  "Unable to open core file\n" + corePath + ":\n\n" + errMsg,
                                                  "Unable to Open Core File",
                                                  JOptionPane.WARNING_MESSAGE);
          }
        });
      agent.detach();
      return;
    }

    // OK, the VM should be available. Create the Threads dialog.
    showThreadsDialog();
  }

  /** NOTE we are in a different thread here than either the main
      thread or the Swing/AWT event handler thread, so we must be very
      careful when creating or removing widgets */
  private void connect(final String remoteMachineName) {
    // Try to open this core file
    Runnable remover = new Runnable() {
          public void run() {
            attachWaitDialog.setVisible(false);
            desktop.remove(attachWaitDialog);
            attachWaitDialog = null;
          }
      };

    try {
      SwingUtilities.invokeLater(new Runnable() {
          public void run() {
            JOptionPane pane = new JOptionPane("Connecting to debug server, please wait...", JOptionPane.INFORMATION_MESSAGE);
            pane.setOptions(new Object[] {});
            attachWaitDialog = pane.createInternalFrame(desktop, "Connecting to Debug Server");
            attachWaitDialog.show();
          }
        });

      agent.attach(remoteMachineName);
      if (agent.getDebugger().hasConsole()) {
        showConsoleMenuItem.setEnabled(true);
      }
      attached = true;
      SwingUtilities.invokeLater(remover);
    }
    catch (DebuggerException e) {
      SwingUtilities.invokeLater(remover);
      final String errMsg = formatMessage(e.getMessage(), 80);
      SwingUtilities.invokeLater(new Runnable() {
          public void run() {
            setMenuItemsEnabled(attachMenuItems, true);
            JOptionPane.showInternalMessageDialog(desktop,
                                                  "Unable to connect to machine \"" + remoteMachineName + "\":\n\n" + errMsg,
                                                  "Unable to Connect",
                                                  JOptionPane.WARNING_MESSAGE);
          }
        });
      agent.detach();
      return;
    }

    // OK, the VM should be available. Create the Threads dialog.
    showThreadsDialog();
  }

  private void detachDebugger() {
    if (!attached) {
      return;
    }
    agent.detach();
    attached = false;
  }

  private void detach() {
    detachDebugger();
    attachWaitDialog = null;
    threadsFrame = null;
    consoleFrame = null;
    setMenuItemsEnabled(attachMenuItems, true);
    setMenuItemsEnabled(detachMenuItems, false);
    toolsMenu.setEnabled(false);
    showConsoleMenuItem.setEnabled(false);
    // FIXME: is this sufficient, or will I have to do anything else
    // to the components to kill them off? What about WorkerThreads?
    desktop.removeAll();
    desktop.invalidate();
    desktop.validate();
    desktop.repaint();
  }

  /** NOTE that this is called from another thread than the main or
      Swing thread and we have to be careful about synchronization */
  private void showThreadsDialog() {
    SwingUtilities.invokeLater(new Runnable() {
        public void run() {
          threadsFrame = new JInternalFrame("Java Threads");
          threadsFrame.setResizable(true);
          threadsFrame.setIconifiable(true);
          JavaThreadsPanel threadsPanel = new JavaThreadsPanel();
          threadsPanel.addPanelListener(HSDB.this);
          threadsFrame.getContentPane().add(threadsPanel);
          threadsFrame.setSize(500, 300);
          threadsFrame.pack();
          desktop.add(threadsFrame);
          GraphicsUtilities.moveToInContainer(threadsFrame, 0.75f, 0.25f, 0, 20);
          threadsFrame.show();
          setMenuItemsEnabled(attachMenuItems, false);
          setMenuItemsEnabled(detachMenuItems, true);
          toolsMenu.setEnabled(true);
        }
      });
  }

  private void showObjectHistogram() {
    sun.jvm.hotspot.oops.ObjectHistogram histo = new sun.jvm.hotspot.oops.ObjectHistogram();
    ObjectHistogramCleanupThunk cleanup =
      new ObjectHistogramCleanupThunk(histo);
    doHeapIteration("Object Histogram",
                    "Generating histogram...",
                    histo,
                    cleanup);
  }

  class ObjectHistogramCleanupThunk implements CleanupThunk {
    sun.jvm.hotspot.oops.ObjectHistogram histo;

    ObjectHistogramCleanupThunk(sun.jvm.hotspot.oops.ObjectHistogram histo) {
      this.histo = histo;
    }

    public void heapIterationComplete() {
      SwingUtilities.invokeLater(new Runnable() {
          public void run() {
            JInternalFrame histoFrame = new JInternalFrame("Object Histogram");
            histoFrame.setResizable(true);
            histoFrame.setClosable(true);
            histoFrame.setIconifiable(true);
            histoFrame.getContentPane().setLayout(new BorderLayout());
            ObjectHistogramPanel panel = new ObjectHistogramPanel(histo);
            panel.addPanelListener(HSDB.this);
            histoFrame.getContentPane().add(panel);
            desktop.add(histoFrame);
            GraphicsUtilities.reshapeToAspectRatio(histoFrame, 4.0f / 3.0f, 0.6f,
                                       histoFrame.getParent().getSize());
            GraphicsUtilities.centerInContainer(histoFrame);
            histoFrame.show();
          }
        });
    }
  }

  public void showObjectsOfType(Klass type) {
    FindObjectByType finder = new FindObjectByType(type);
    FindObjectByTypeCleanupThunk cleanup =
      new FindObjectByTypeCleanupThunk(finder);
    ByteArrayOutputStream bos = new ByteArrayOutputStream();
    type.printValueOn(new PrintStream(bos));
    String typeName = bos.toString();
    doHeapIteration("Show Objects Of Type",
                    "Finding instances of \"" + typeName + "\"",
                    finder,
                    cleanup);
  }

  class LivenessProgress implements LivenessAnalysis.ProgressThunk {
    private JInternalFrame frame;
    private ProgressBarPanel bar;

    public void visitedFractionUpdate(final double fractionOfHeapVisited) {
      if (frame == null) {
        SwingUtilities.invokeLater(new Runnable() {
            public void run() {
              frame = new JInternalFrame("Liveness Analysis");
              frame.setResizable(true);
              frame.setIconifiable(true);
              frame.getContentPane().setLayout(new BorderLayout());
              bar = new ProgressBarPanel("Percentage of heap visited");
              frame.getContentPane().add(bar, BorderLayout.CENTER);
              desktop.add(frame);
              frame.pack();
              GraphicsUtilities.centerInContainer(frame);
              frame.show();
            }
          });
      }

      SwingUtilities.invokeLater(new Runnable() {
          public void run() {
            bar.setValue(fractionOfHeapVisited);
          }
        });
    }

    public void done() {
      SwingUtilities.invokeLater(new Runnable() {
          public void run() {
            desktop.remove(frame);
            desktop.repaint();
          }
        });
    }
  }

  class ReversePtrsProgress implements ReversePtrsAnalysis.ProgressThunk {
    private JInternalFrame frame;
    private ProgressBarPanel bar;

    public void visitedFractionUpdate(final double fractionOfHeapVisited) {
      if (frame == null) {
        SwingUtilities.invokeLater(new Runnable() {
            public void run() {
              frame = new JInternalFrame("Reverse Ptrs Analysis");
              frame.setResizable(true);
              frame.setIconifiable(true);
              frame.getContentPane().setLayout(new BorderLayout());
              bar = new ProgressBarPanel("Percentage of heap visited");
              frame.getContentPane().add(bar, BorderLayout.CENTER);
              desktop.add(frame);
              frame.pack();
              GraphicsUtilities.centerInContainer(frame);
              frame.show();
            }
          });
      }

      SwingUtilities.invokeLater(new Runnable() {
          public void run() {
            bar.setValue(fractionOfHeapVisited);
          }
        });
    }

    public void done() {
      SwingUtilities.invokeLater(new Runnable() {
          public void run() {
            desktop.remove(frame);
            desktop.repaint();
          }
        });
    }
  }

  class FindObjectByTypeCleanupThunk implements CleanupThunk {
    FindObjectByType finder;    

    FindObjectByTypeCleanupThunk(FindObjectByType finder) {
      this.finder = finder;
    }

    public void heapIterationComplete() {
      SwingUtilities.invokeLater(new Runnable() {
          public void run() {
            JInternalFrame finderFrame = new JInternalFrame("Show Objects of Type");
            finderFrame.getContentPane().setLayout(new BorderLayout());
            finderFrame.setResizable(true);
            finderFrame.setClosable(true);
            finderFrame.setIconifiable(true);
            ObjectListPanel panel = new ObjectListPanel(finder.getResults(),
                                                        new LivenessProgress(),
                                                        null);
            panel.addPanelListener(HSDB.this);
            finderFrame.getContentPane().add(panel);
            desktop.add(finderFrame);
            GraphicsUtilities.reshapeToAspectRatio(finderFrame, 4.0f / 3.0f, 0.6f,
                                       finderFrame.getParent().getSize());
            GraphicsUtilities.centerInContainer(finderFrame);
            finderFrame.show();
          }
        });
    }
  }

  private void showDebuggerConsole() {
    if (consoleFrame == null) {
      consoleFrame = new JInternalFrame("Debugger Console");
      consoleFrame.setResizable(true);
      consoleFrame.setClosable(true);
      consoleFrame.setIconifiable(true);
      consoleFrame.getContentPane().setLayout(new BorderLayout());
      consoleFrame.getContentPane().add(new DebuggerConsolePanel(agent.getDebugger()), BorderLayout.CENTER);
      GraphicsUtilities.reshapeToAspectRatio(consoleFrame, 5.0f, 0.9f, desktop.getSize());
    }
    if (consoleFrame.getParent() == null) {
      desktop.add(consoleFrame);
    }
    consoleFrame.setVisible(true);
    consoleFrame.show();
    consoleFrame.getContentPane().getComponent(0).requestFocus();
  }

  private void showFindPanel() {
    showPanel("Find Pointer", new FindPanel());
  }

  private void showFindInHeapPanel() {
    showPanel("Find Address In Heap", new FindInHeapPanel());
  }

  private void showHeapParametersPanel() {
    showPanel("Heap Parameters", new HeapParametersPanel());
  }

  public void showThreadInfo(final JavaThread thread) {
    showPanel("Info for " + thread.getThreadName(), new ThreadInfoPanel(thread));
  }

  public void showJavaStackTrace(final JavaThread thread) {
    showPanel("Java stack trace for " + thread.getThreadName(), new JavaStackTracePanel(thread));
  }

  private void showDeadlockDetectionPanel() {
    showPanel("Deadlock Detection", new DeadlockDetectionPanel());
  }

  private void showMonitorCacheDumpPanel() {
    showPanel("Monitor Cache Dump", new MonitorCacheDumpPanel());
  }

  public void showClassBrowser() {
    final JInternalFrame progressFrame = new JInternalFrame("Class Browser");
    progressFrame.setResizable(true);
    progressFrame.setClosable(true);
    progressFrame.setIconifiable(true);
    progressFrame.getContentPane().setLayout(new BorderLayout());
    final ProgressBarPanel bar = new ProgressBarPanel("Generating class list ..");
    bar.setIndeterminate(true);
    progressFrame.getContentPane().add(bar, BorderLayout.CENTER);
    desktop.add(progressFrame);
    progressFrame.pack();
    GraphicsUtilities.centerInContainer(progressFrame);
    progressFrame.show();

    workerThread.invokeLater(new Runnable() {
                                public void run() {
                                   HTMLGenerator htmlGen = new HTMLGenerator();
                                   InstanceKlass[] klasses = SystemDictionaryHelper.getAllInstanceKlasses();
                                   final String htmlText = htmlGen.genHTMLForKlassNames(klasses);
                                   SwingUtilities.invokeLater(new Runnable() {
                                      public void run() {
                                         JInternalFrame cbFrame = new JInternalFrame("Class Browser");
                                         cbFrame.getContentPane().setLayout(new BorderLayout());
                                         cbFrame.setResizable(true);
                                         cbFrame.setClosable(true);
                                         cbFrame.setIconifiable(true);
                                         ClassBrowserPanel cbPanel = new ClassBrowserPanel();
                                         cbFrame.getContentPane().add(cbPanel, BorderLayout.CENTER);
                                         desktop.remove(progressFrame); 
                                         desktop.repaint();
                                         desktop.add(cbFrame);
                                         GraphicsUtilities.reshapeToAspectRatio(cbFrame, 1.25f, 0.85f, 
                                                                      cbFrame.getParent().getSize());
                                         cbFrame.show();
                                         cbPanel.setClassesText(htmlText);
                                      }
                                   });
                                }
                             });
  }

  public void showCodeViewer() {
    JInternalFrame codeViewerFrame = new JInternalFrame("Code Viewer");
    codeViewerFrame.getContentPane().setLayout(new BorderLayout());
    codeViewerFrame.setResizable(true);
    codeViewerFrame.setClosable(true);
    codeViewerFrame.setIconifiable(true);
    CodeViewerPanel cv = new CodeViewerPanel();
    codeViewerFrame.getContentPane().add(cv, BorderLayout.CENTER);
    desktop.add(codeViewerFrame);
    GraphicsUtilities.reshapeToAspectRatio(codeViewerFrame, 1.25f, 0.85f, codeViewerFrame.getParent().getSize());
    codeViewerFrame.show();
  }

  private void showPanel(String name, JPanel panel) {
    JInternalFrame frame = new JInternalFrame(name);
    frame.getContentPane().setLayout(new BorderLayout());
    frame.setResizable(true);
    frame.setClosable(true);
    frame.setIconifiable(true);
    frame.getContentPane().add(panel, BorderLayout.CENTER);
    desktop.add(frame);
    GraphicsUtilities.reshapeToAspectRatio(frame, 5.0f / 3.0f, 0.4f, frame.getParent().getSize());
    frame.show();
  }

  //--------------------------------------------------------------------------------
  // Framework for heap iteration with progress bar
  //

  interface CleanupThunk {
    public void heapIterationComplete();
  }

  class ProgressThunk implements ProgressiveHeapVisitor.Thunk {
    ProgressBarPanel bar;
    JInternalFrame frame;
    CleanupThunk cleanup;

    ProgressThunk(ProgressBarPanel bar, JInternalFrame frame,
                  CleanupThunk cleanup) {
      this.bar     = bar;
      this.frame   = frame;
      this.cleanup = cleanup;
    }
    
    public void heapIterationPercentageUpdate(double fractionVisited) {
      SwingUtilities.invokeLater(new ProgressBarUpdater(bar, fractionVisited));
    }

    public void heapIterationComplete() {
      SwingUtilities.invokeLater(new Runnable() {
          public void run() {
            desktop.remove(frame);
            desktop.repaint();
          }
        });
      cleanup.heapIterationComplete();
    }
  }

  class ProgressBarUpdater implements Runnable {
    ProgressBarPanel bar;
    double fractionVisited;

    public ProgressBarUpdater(ProgressBarPanel bar, double fractionVisited) {
      this.bar = bar;
      this.fractionVisited = fractionVisited;
    }

    public void run() {
      bar.setValue(fractionVisited);
    }
  }

  class VisitHeap implements Runnable {
    HeapVisitor visitor;

    VisitHeap(HeapVisitor visitor) {
      this.visitor = visitor;
    }

    public void run() {
      VM.getVM().getObjectHeap().iterate(visitor);
    }
  }

  private void doHeapIteration(String frameTitle,
                               String progressBarText,
                               HeapVisitor visitor,
                               CleanupThunk cleanup) {
    sun.jvm.hotspot.oops.ObjectHistogram histo = new sun.jvm.hotspot.oops.ObjectHistogram();

    JInternalFrame progressFrame = new JInternalFrame(frameTitle);
    progressFrame.setResizable(true);
    progressFrame.setIconifiable(true);
    progressFrame.getContentPane().setLayout(new BorderLayout());
    ProgressBarPanel bar = new ProgressBarPanel(progressBarText);
    progressFrame.getContentPane().add(bar, BorderLayout.CENTER);
    ProgressiveHeapVisitor.Thunk thunk =
      new ProgressThunk(bar, progressFrame, cleanup);
    HeapVisitor progVisitor = new ProgressiveHeapVisitor(visitor, thunk);
    desktop.add(progressFrame);
    progressFrame.pack();
    GraphicsUtilities.constrainToSize(progressFrame, progressFrame.getParent().getSize());
    GraphicsUtilities.centerInContainer(progressFrame);
    progressFrame.show();
    workerThread.invokeLater(new VisitHeap(progVisitor));
  }
                              
  //--------------------------------------------------------------------------------
  // Stack trace helper
  //

  private static JavaVFrame getLastJavaVFrame(JavaThread cur) {
    RegisterMap regMap = cur.newRegisterMap(true);
    sun.jvm.hotspot.runtime.Frame f = cur.getCurrentFrameGuess();
    if (f == null) return null;
    boolean imprecise = true;
    if (f.isInterpretedFrame() && !f.isInterpretedFrameValid()) {
      System.err.println("Correcting for invalid interpreter frame");
      f = f.sender(regMap);
      imprecise = false;
    }
    VFrame vf = VFrame.newVFrame(f, regMap, cur, true, imprecise);
    if (vf == null) {
      System.err.println(" (Unable to create vframe for topmost frame guess)");
      return null;
    }
    if (vf.isJavaFrame()) {
      return (JavaVFrame) vf;
    }
    return (JavaVFrame) vf.javaSender();
  }

  // Internal routine for debugging
  private static void dumpStack(JavaThread cur) {
    RegisterMap regMap = cur.newRegisterMap(true);
    sun.jvm.hotspot.runtime.Frame f = cur.getCurrentFrameGuess();
    PrintStream tty = System.err;
    while (f != null) {
      tty.print("Found ");
           if (f.isInterpretedFrame()) { tty.print("interpreted"); }
      else if (f.isCompiledFrame())    { tty.print("compiled"); }
      else if (f.isEntryFrame())       { tty.print("entry"); }
      else if (f.isNativeFrame())      { tty.print("native"); }
      else if (f.isDeoptimizedFrame()) { tty.print("deoptimized"); }
      else if (f.isOSRAdapterFrame())  { tty.print("OSR adapter"); }
      else if (f.isGlueFrame())        { tty.print("glue"); }
      else { tty.print("external"); }
      tty.print(" frame with PC = " + f.getPC() + ", SP = " + f.getSP() + ", FP = " + f.getFP());
      if (f.isSignalHandlerFrameDbg()) {
        tty.print(" (SIGNAL HANDLER)");
      }
      tty.println();
      
      if (!f.isFirstFrame()) {
        f = f.sender(regMap);
      } else {
        f = null;
      }
    }
  }

  //--------------------------------------------------------------------------------
  // Component utilities
  //

  private static JMenuItem createMenuItem(String name, ActionListener l) {
    JMenuItem item = new JMenuItem(name);
    item.addActionListener(l);
    return item;
  }

  /** Punctuates the given string with \n's where necessary to not
      exceed the given number of characters per line. Strips
      extraneous whitespace. */
  private String formatMessage(String message, int charsPerLine) {
    StringBuffer buf = new StringBuffer(message.length());
    StringTokenizer tokenizer = new StringTokenizer(message);
    int curLineLength = 0;
    while (tokenizer.hasMoreTokens()) {
      String tok = tokenizer.nextToken();
      if (curLineLength + tok.length() > charsPerLine) {
        buf.append('\n');
        curLineLength = 0;
      } else {
        if (curLineLength != 0) {
          buf.append(' ');
          ++curLineLength;
        }
      }
      buf.append(tok);
      curLineLength += tok.length();
    }
    return buf.toString();
  }

  private void setMenuItemsEnabled(java.util.List items, boolean enabled) {
    for (Iterator iter = items.iterator(); iter.hasNext(); ) {
      ((JMenuItem) iter.next()).setEnabled(enabled);
    }
  }
}
