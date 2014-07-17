/*
 * @(#)JRESearcher.java	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.ui.prefs;

import javax.swing.*;
import javax.swing.event.*;
import java.awt.*;
import java.beans.*;
import java.awt.event.*;
import java.io.*;
import java.lang.reflect.*;
import java.util.*;
import com.sun.javaws.*;
import com.sun.javaws.debug.*;
import com.sun.javaws.ui.general.*;

/**
 * Searchs for JRE's from a given location. The user is prompted for the
 * directory to search from, any file with java or javaw.exe is considered
 * a valid JRE and is executed to determine the version.
 *
 * @version 1.19 11/29/01
 */
public class JRESearcher extends PluggablePanel {
    private static String defaultClassPath;

    private static final int CANCEL_INDEX = 0;
    private static final int PREVIOUS_INDEX = 1;
    private static final int NEXT_INDEX = 2;
    private JButton _cancelButton;

    private JDialog dialog;
    /** Directory to search in. */
    private File directory;
    /** Resources used to get strings from. */
    private ResourceBundle resources;
    /** Maintains active Subcontroller. */
    private AbstractController controller;

    /** JREs that are to be returned. */
    private ConfigProperties.JREInformation[] jres;


    /**
     * Searches for JREs by prompting the user for a place to search from
     * and then searching for valid java programs. A null return value
     * indicates none were found.
     */
    public ConfigProperties.JREInformation[] search(Component c,
                                                    ResourceBundle resources,
                                                    String description) {
	this.resources = resources;
	setJREs(null);

	// Configure the text values.
	String[] titles = new String[3];
	titles[CANCEL_INDEX] = 
		Resources.getString("jresearcher.cancelButton");
	titles[PREVIOUS_INDEX] = 
	    Resources.getString("jresearcher.previousButton");
	titles[NEXT_INDEX] = 
	    Resources.getString("jresearcher.nextButton");
	setButtons(titles);
	setTitle(Resources.getString("jresearcher.title"));

	buttons[PREVIOUS_INDEX].setMnemonic(
	    Resources.getVKCode("jresearcher.previousButtonMnemonic"));
	buttons[NEXT_INDEX].setMnemonic(
	    Resources.getVKCode("jresearcher.nextButtonMnemonic"));

	_cancelButton = buttons[CANCEL_INDEX];



	// Set up the controller
	if (controller == null) {
	    createController();
	}

	SimpleController sc = new SimpleController();
	JTextArea ta =  new JTextArea(description);
	ta.setOpaque(false);
	ta.setEditable(false);
	ta.setLineWrap(true);
	ta.setWrapStyleWord(true);
	ta.setFont(UIManager.getFont("Label.font"));
	sc.setComponent(ta);
	controller.setSubcontroller(0, sc);

	directory = null;
	setState(0);
	show(c);
	return jres;
    }

    private void createController() {
	controller = new AbstractController() {
	    protected Subcontroller createSubcontroller(int index) {
		// PENDING: this should be more dynamic
		switch (index) {
		case 1:
		    return new PathController();
		default:
		    return new SearchController();
		}
	    }
	};
    }


    private void show(Component c) {
	// Create the dialog.
        Frame frame = c instanceof Frame ? (Frame)c
              : (Frame)SwingUtilities.getAncestorOfClass(Frame.class, c);
        dialog = GeneralUtilities.createDialog(frame, 
		 Resources.getString("jresearcher.maintitle"), true);
	dialog.getContentPane().setLayout(new BorderLayout());
	dialog.getContentPane().add(this, BorderLayout.CENTER);
        dialog.pack();
	// Constrain the size.
	Dimension size = dialog.getSize();
	size.width = Math.max(size.width, 500);
	size.height = Math.max(size.height, 400);
	Dimension ss = Toolkit.getDefaultToolkit().getScreenSize();
	dialog.setBounds((ss.width - size.width) / 2,
			 (ss.height - size.height) / 2, size.width,
			 size.height);

        dialog.addKeyListener(new KeyAdapter() {
            public void keyPressed(KeyEvent e) {
                int cancel = 
		    Resources.getVKCode("jresearcher.cancelButtonMnemonic");
                if (e.getKeyCode() == cancel) {
                    _cancelButton.doClick();
                }
            }
        });

	dialog.show();
	dialog.dispose();
	setState(-1);
    }

    /**
     * Cancels the current session.
     */
    protected void cancel() {
	setJREs(null);
	dialog.setVisible(false);
    }

    /**
     * Shows the next screen.
     */
    protected void next() {
	int state = getState();
	if (++state == 3) {
	    // We're done.
            dialog.setVisible(false);
	}
	else {
	    setState(state);
	}
    }

    /**
     * Shows the previous screen.
     */
    protected void previous() {
	int state = getState();
	setState(--state);
    }

    /**
     * This is inherited from our superclass, and is overriden to
     * invoke one of <code>next</code>, <code>cancel</code> or
     * <code>previous</code> based on <code>index</code>.
     */
    protected void buttonPressed(int index) {
	switch(index) {
	case CANCEL_INDEX:
	    cancel();
	    break;
	case PREVIOUS_INDEX:
	    previous();
	    break;
	default:
	    next();
	    break;
	}
    }

    /**
     * Sets the currently selected directory to search from.
     */
    private void setDirectory(File file) {
	this.directory = file;
	setButtonEnabled(NEXT_INDEX, (directory != null));
    }

    /**
     * Returns the current directory to search from.
     */
    private File getDirectory() {
	return directory;
    }

    /**
     * Sets the JREs that are to be returned.
     */
    private void setJREs(ConfigProperties.JREInformation[] jres) {
	this.jres = jres;
    }

    /**
     * Sets the current state, updating the display accordingly.
     */
    private void setState(int newState) {
	setButtonEnabled(CANCEL_INDEX, true);
	setButtonEnabled(PREVIOUS_INDEX, false);
	setButtonEnabled(NEXT_INDEX, true);
	controller.setActiveSubcontrollerIndex(newState);
	if (newState == 1) {
	    setJREs(null);
	}
	if (newState != -1) {
	    setComponent(controller.getActiveSubcontroller().getComponent());
	}
	revalidate();
	repaint();
    }

    /**
     * Returns the current state, which indicates what is currently being
     * displayed.
     */
    private int getState() {
	return controller.getActiveSubcontrollerIndex();
    }


    /**
     * An implementation of Controller that calls createComponent to
     * create the component. After the component is created it is then
     * cached.
     */
    private class SimpleController implements Subcontroller {
	private Component component;

	public void setComponent(Component c) {
	    this.component = c;
	}

	public Component getComponent() {
	    if (component == null) {
		setComponent(createComponent());
	    }
	    return component;
	}

	protected Component createComponent() {
	    return null;
	}

	public void start() {}
	public void stop() {}
	public void apply() {}
	public void revert() {}
    }


    /**
     * Controller to allow the user to select a directory. This is done
     * by way of a JFileChooser.
     */
    private class PathController extends SimpleController {
	private JFileChooser fc;

	public void start() {
	    setButtonEnabled(NEXT_INDEX, (getDirectory() != null));
	    setButtonEnabled(PREVIOUS_INDEX, true);
	}

	protected Component createComponent() {
	    fc = new JFileChooser();
	    fc.setFileSelectionMode(JFileChooser.FILES_AND_DIRECTORIES);
	    // Add an actionlistener to change the state when the user clicks
	    // ok/cancel.
	    // This is only necessary in < 1.3
	    fc.addActionListener(new ActionListener() {
		public void actionPerformed(ActionEvent ae) {
		    if (JFileChooser.APPROVE_SELECTION.equals
			(ae.getActionCommand())) {
			if (ae.getSource() instanceof JFileChooser) {
			    JFileChooser fc = (JFileChooser) ae.getSource();
			    File file = fc.getSelectedFile();
			    if (file != null && file.isDirectory()) {
				fc.setCurrentDirectory(file);
			    }
			}
			updateDirectory();
		    }
		    else if (JFileChooser.CANCEL_SELECTION.equals
			     (ae.getActionCommand())) {
			cancel();
		    }
		}
	    });
	    // Adds a change listener to update the directory as the selection
	    // changes.
	    fc.addPropertyChangeListener(new PropertyChangeListener() {
		public void propertyChange(PropertyChangeEvent e) {
		    if (JFileChooser.SELECTED_FILE_CHANGED_PROPERTY.
			equals(e.getPropertyName()) ||
			JFileChooser.DIRECTORY_CHANGED_PROPERTY.
			equals(e.getPropertyName())) {
			updateDirectory();
		    }
		}
	    });
	    // Use reflection to invoke setControlButtonsAreShown as only
	    // available in 1.3.
	    try {
		Class[] showParams = new Class[] { boolean.class };
		Method showMethod = JFileChooser.class.getMethod
		    ("setControlButtonsAreShown", showParams);
		if (showMethod != null) {
		    Object[] args = new Object[] { Boolean.FALSE };
		    showMethod.invoke(fc, args);
		}
	    }
	    catch (NoSuchMethodException nsme) {}
	    catch (InvocationTargetException ite) {}
	    catch (IllegalAccessException iae) {}
	    fc.setDialogType(JFileChooser.OPEN_DIALOG);
	    updateDirectory();
	    return fc;
	}

	private void updateDirectory() {
	    File file = fc.getSelectedFile();
	    if (file == null) {
		file = fc.getCurrentDirectory();
	    }
	    setDirectory(file);
	}
    }


    /**
     * Controller to do the searching. When start()ed, a Thread is spawned
     * to search all the files. If a file's name is java or javaw.exe it is
     * executed to determine the version. If succesfully executed the
     * path is added to a list.
     */
    private class SearchController extends SimpleController
	          implements ActionListener {
	/** Model for the list, will contain the paths. */
	private DefaultListModel model;
	/** List showing any found JREs. */
	private JList list;
	/** Shows the file currently being checked. */
	private JLabel searchLabel;
	/** Prefix before file name in searchLabel. */
	private String searchPrefix;
	/** Displays a descriptive title of what is happening. */
	private JLabel titleLabel;

	/** Handles the searching. */
	private Searcher searcher;
	/** Updates the state (searchLabel, next button) based on the
	 * searcher's state. */
	private javax.swing.Timer timer;

	/** Are we currently active. */
	private boolean active;

	public void start() {
	    active = true;
	    setButtonEnabled(NEXT_INDEX, false);
	    setButtonEnabled(PREVIOUS_INDEX, true);
	    if (model != null) {
		model.removeAllElements();
	    }
	    // Create the timer top uddate the display
	    timer = new javax.swing.Timer(100, this);
	    timer.setRepeats(true);
	    timer.start();
	    // Start searching
	    searcher = new Searcher();
	    searcher.start(getDirectory());
	}

	public void stop() {
	    active = false;
	    stopSearching();
	}

	private void stopSearching() {
	    if (searcher != null) {
		// Stop the timer and stop searching.
		searcher.stop();
		searcher = null;
		timer.stop();
		timer = null;
	    }
	}

	/**
	 * Invoked as a result of the Timer. Updates the display based on
	 * the state of the Searcher to show what is is currently being
	 * searched. If the searcher is no longer searching, this calls
	 * stop.
	 */
	public void actionPerformed(ActionEvent ae) {
	    if (!searcher.isFinished()) {
		File f = searcher.getCurrentFile();
		if (f != null) {
		    searchLabel.setText(searchPrefix + f.getPath());
		}
		else {
		    searchLabel.setText(searchPrefix);
		}
	    }
	    else {
		if (model.getSize() > 0) {
		    titleLabel.setText(
			Resources.getString("jresearcher.foundJREsTitle"));
		}
		else {
		    titleLabel.setText(
			Resources.getString("jresearcher.noJREsTitle"));
		}
		setButtonEnabled(NEXT_INDEX, true);
		searchLabel.setText(" ");
		stopSearching();
	    }
	}

	protected Component createComponent() {
	    searchPrefix = 
		Resources.getString("jresearcher.searchingPrefix");
	    model = new DefaultListModel();
	    list = new JList(model);
            list.setCellRenderer(new DefaultListCellRenderer() {
                public Component getListCellRendererComponent(
                          JList list, Object value, int index,
                          boolean isSelected, boolean cellHasFocus) {
                    if (value instanceof ConfigProperties.JREInformation) {
                        value = ((ConfigProperties.JREInformation)value).
                                                   getInstalledPath();
                    }
                    return super.getListCellRendererComponent(
                        list, value, index, isSelected, cellHasFocus);
                }
            });
	    list.addListSelectionListener(new ListSelectionListener() {
		public void valueChanged(ListSelectionEvent lse) {
		    updateJREs();
		}
	    });
	    searchLabel = new JLabel(" ");
	    titleLabel = new JLabel(
		Resources.getString("jresearcher.searchingTitle"));
	    JPanel component = new JPanel(new GridBagLayout());

	    GridBagConstraints cons = new GridBagConstraints();
	    cons.gridx = cons.gridy = 0;
	    cons.weightx = 1;
	    cons.weighty = 0;
	    cons.fill = GridBagConstraints.HORIZONTAL;
	    cons.insets = new Insets(5, 2, 0, 2);
	    cons.gridy = 0;
	    component.add(titleLabel, cons);

	    cons.gridy = 2;
	    component.add(searchLabel, cons);

	    cons.gridy = 1;
	    cons.weighty = 1;
	    cons.fill = GridBagConstraints.BOTH;
	    cons.insets.bottom = 0;
	    component.add(new JScrollPane(list), cons);
	    return component;
	}

	private void updateJREs() {
	    if (!active) {
		return;
	    }
	    if (model != null) {
		int size = model.getSize();
		if (size > 0) {
		    setButtonEnabled(NEXT_INDEX, true);
		    int[] selected = list.getSelectedIndices();
		    if (selected != null && selected.length > 0) {
			ConfigProperties.JREInformation jres[] = new
                              ConfigProperties.JREInformation[selected.length];
			for (int counter = 0; counter < selected.length;
			     counter++) {
			    jres[counter] = (ConfigProperties.JREInformation)
                                       model.getElementAt(selected[counter]);
			}
			setJREs(jres);
		    }
		    else {
			ConfigProperties.JREInformation jres[] = new
                                         ConfigProperties.JREInformation[size];
			model.copyInto(jres);
			setJREs(jres);
		    }
		}
		else {
		    setJREs(null);
		    setButtonEnabled(NEXT_INDEX, false);
		}
	    }
	    else {
		setJREs(null);
	    }
	}

	/**
	 * Indicates the Searcher has found a valid JRE.
	 */
	// Can come in on any thread.
	private void add(final Searcher searcher,
                         final ConfigProperties.JREInformation jre) {
	    SwingUtilities.invokeLater(new Runnable() {
		public void run() {
		    if (SearchController.this.searcher == searcher) {
			model.addElement(jre);
			updateJREs();
		    }
		}
	    });
	}


	/**
	 * Handles the actual searching. This will spawn a thread to
	 * descend the file system looking for any files named
	 * java or javaw.exe. If the name is java/javaw.exe it is executed
	 * to determine the version. If the process returns 0 then
	 * <code>add</code> is invoked add the path with the specified
	 * version.
	 * <p>
	 * Searching can be stopped by invoking <code>stop</code>.
	 * The current file that is being searched can be determined
	 * by <code>getCurrentFile</code>.
	 * <p>
	 * If the child process doesn't finish executing in around 5 seconds,
	 * it is killed.
	 */
	private class Searcher implements Runnable {
	    /** File to start searching from. */
	    private File file;
	    /** When true, indicates the search should stop. */
	    private boolean stop;
	    /** Current file being checked. */
	    private File currentFile;
	    /** True indicates we're done. */
	    private boolean finished;

	    /**
	     * Starts the traversal, spawning the thread.
	     */
	    void start(File f) {
		file = f;
		new Thread(this).start();
		updateJREs();
	    }

	    /**
	     * Returns the current file being checked.
	     */
	    public File getCurrentFile() {
		return currentFile;
	    }

	    /**
	     * Stops the processing.
	     */
	    public void stop() {
		stop = true;
	    }

	    /**
	     * Runnable method to start the searching.
	     */
	    public void run() {
		check(file);
		finished = true;
	    }

	    /**
	     * Returns true if searching has finished.
	     */
	    public boolean isFinished() {
		return finished;
	    }

	    /**
	     * If <code>f</code> is a file, and is named java or javaw.exe,
	     * <code>getVersion</code> is invoked to obtain the version,
	     * followed by <code>add</code>. If <code>f</code> is a directory,
	     * this is recursively called for all the children.
	     */
	    private void check(File f) {
		currentFile = f;
		String fileName = f.getName();
		if (f.isFile() && (fileName.equals("java") ||
				   fileName.equals("javaw.exe"))) {
		    ConfigProperties.JREInformation version = getVersion(f);
		    if (version != null) {
			add(Searcher.this, version);
		    }
		}
		else if (!f.isFile()) {
		    String[] kids = f.list();
		    if (kids != null) {
			for (int counter = 0, maxCounter = kids.length;
			     counter < maxCounter && !stop; counter++) {
			    check(new File(f, kids[counter]));
			}
		    }
		}
	    }

	    /**
	     * Returns the version for the JVM at <code>javaPath</code>.
	     * This will return null if there is problem with executing
	     * the path. This gives the child process at most 15 seconds to
	     * execute before assuming it is bogus.
	     */
	    private ConfigProperties.JREInformation getVersion(File javaPath) {
		if (Globals.TraceJRESearch) {
		    Debug.println("checking " + javaPath);
		}
		// Make sure its valid
		if (!isValidJavaPath(javaPath)) {
		    if (Globals.TraceJRESearch) {
			Debug.println("\tisn't valid java path");
		    }
		    return null;
		}
		// add in jre osname and osarch
		ConfigProperties.JREInformation jre = JRELocator.getVersion(javaPath);
		jre.setOsInfo(Globals.osName, Globals.osArch);
		return jre;
	    }

	    /**
	     * Returns true if <code>path</code> is a valid path to check
	     * for a JRE.
	     */
	    private boolean isValidJavaPath(File path) {
		String parent = path.getParent();
		if (parent.endsWith(File.separator + "native_threads") ||
		    parent.endsWith(File.separator + "green_threads")) {
		    return false;
		}
		// if path is of the form x/jre/bin and x/bin/java or 
		// x/bin/javaw.exe exists return false.
		String jreBin = File.separator + "jre" + File.separator +"bin";
		if (parent.endsWith(jreBin) && 
		    (parent.length() > jreBin.length())) {
		    String binPath = parent.substring(0, parent.length() -
						    jreBin.length() + 1) +
			             "bin" + File.separator;
		    File file = new File(binPath + "java");
		    if (file.exists() && file.isFile()) {
			return false;
		    }
		    file = new File(binPath + "javaw.exe");
		    if (file.exists() && file.isFile()) {
			return false;
		    }
		}
		return true;
	    }
	}
    }
}
