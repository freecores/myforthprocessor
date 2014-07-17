/*
 * @(#)Console.java	1.33 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.ui.console;

import java.awt.*;
import java.awt.event.*;
import java.awt.geom.*;
import java.io.*;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.text.*;
import com.sun.javaws.*;
import java.util.Date;
import com.sun.javaws.debug.*;
import com.sun.javaws.ui.general.*;

/**
 * A simple Console that can be written to by way of OuputStream methods.
 * We create the Console no matter what, but we will only expose it if
 *the user sets it on startup or dynamically through the UI.
 * @version 1.3 10/18/99
 *
 */
public class Console extends OutputStream {
    /** The document writting to. */
    private static Document _document;
    private static JFrame _frame;
    private static JTextArea _ta;
    
    private boolean _showConsole=false;
    private int _consoleSize = 0;
    private File _logFile;
    private PrintStream _consolePrintStream;
    private PrintStream _filePrintStream;
    private PrintStream _sockLogStream;
    
    public Console() {
        _frame = GeneralUtilities.createFrame(null);
	_ta = new JTextArea();
        _ta.setEditable(false);
        _ta.setLineWrap(true);
        _ta.setWrapStyleWord(true);
        _document = _ta.getDocument();
        _frame.setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
        _frame.addWindowListener(new WindowAdapter() {
		    public void windowClosing(WindowEvent we) {
			dispose();
		    }
		});
        JButton clearButton = new JButton(
	    Resources.getString("console.clearButton"));
	clearButton.setMnemonic(Resources.getVKCode("console.clearMnemonic"));
        clearButton.addActionListener(new ActionListener() {
		    public void actionPerformed(ActionEvent ae) {
			Document doc = _document;
			if (doc != null) {
			    try {
				doc.remove(0, _document.getLength());
			    } catch (BadLocationException ble) {}
			}
		    }
		});
        Container parent = _frame.getContentPane();
        parent.setLayout(new GridBagLayout());
        GridBagConstraints cons = new GridBagConstraints();
        cons.gridx = cons.gridy = 0;
        cons.weightx = cons.weighty = 1;
        cons.fill = GridBagConstraints.BOTH;
        cons.insets = new Insets(2, 2, 5, 2);
        parent.add(new JScrollPane(_ta), cons);
        
        cons.gridy = 1;
        cons.fill = GridBagConstraints.NONE;
        cons.anchor = GridBagConstraints.WEST;
        cons.weightx = cons.weighty = 0;
        cons.insets.top = 0;
        cons.insets.bottom = 2;
        parent.add(clearButton, cons);
        
        init();
        
    }
    /**
     * This is a initialization method just so that we can preserve the
     * order of things.
     */
    private void init() {
        this._consolePrintStream = new PrintStream((OutputStream)this);
        this._sockLogStream = Debug.getSocketStream();
        initshowConsole();
        initFileLogging();
    }
    /**
     * Redirect the stdout and stderr to a common printStream, also takes in
     * an optional message which can be printed to the Console after a redirect.
     */
    private void redirectOutputs(PrintStream ps, String message){
        
        if ( (Globals.RedirectOutput) && (ps != null) ) {
	    System.setOut(ps);
	    System.setErr(ps);
        }

	_consolePrintStream.println(Resources.getString("appname") +" " +Resources.getString("console.message", new Date().toString()));

	_consolePrintStream.println(Resources.getString("console.j2re",
							System.getProperty("java.version"),
							System.getProperty("java.vendor")));
        
        if (message != null) {
	    System.err.println(message);
        }
    }
    /**
     * Initialize file logging, if we have a valid filename we will use it.
     * If the filename is "TEMP" or "temp" then we will create a unique
     * temporary file.
     * Note: If a fileName is specified then the most recent application will
     * overwrite the logfile, if logfiles need to be preserved over VM Instances
     * then use keyword TEMP, the file will not be deleted.
     */
    
    private void initFileLogging() {
        String msg=null;
        ConfigProperties cp = ConfigProperties.getInstance();
        String filelog = cp.getLogFilename();
	int len = filelog.length();

        if (cp.isLogToFileEnabled()) {
	    try {
		File dir = null;
		if (len == 0 || filelog.compareToIgnoreCase("TEMP") == 0) {
		    dir = new File(cp.getLogFileDir());
		} else if (filelog.endsWith("TEMP") && len > 5) {
		    dir = new File(filelog.substring(0, len-5));
		} else {
		    dir = new File(filelog);
		    if (!dir.exists() || !dir.isDirectory()) {
			dir = null;	// use as filename
		    }
		}
		if (dir != null) {
		    dir.mkdirs();
		    _logFile=File.createTempFile("javaws",".log", dir);
		    _filePrintStream = new PrintStream(new FileOutputStream(_logFile));
		    msg = Resources.getString("console.logfilemsg",_logFile.toString());
		} else {
		    _logFile = new File(filelog);
		    if (_logFile.exists() && _logFile.canWrite()) {
			_filePrintStream = new PrintStream(new FileOutputStream(filelog, true));
			msg = Resources.getString("console.logfilemsg",_logFile.toString());
		    } else if (_logFile.createNewFile()) {
			_filePrintStream = new PrintStream(new FileOutputStream(_logFile));
			msg = Resources.getString("console.logfilemsg",_logFile.toString());
		    } else {
			msg = Resources.getString("console.logfilewarning",filelog);
		    }
		}
	    } catch (IOException ioe) {
		Debug.ignoredException(ioe);
		msg = Resources.getString("console.logfilewarning",_logFile.toString());
	    }
        }
        redirectOutputs(_consolePrintStream, msg);
    }
    
    /**
     * This so that we can override the property display the Console
     * by an User Action...
     */
    public void setshowConsole(boolean set) {
        this._showConsole = set;
        return;
    }
    
    private void initshowConsole() {
        ConfigProperties props = ConfigProperties.getInstance();
        _showConsole = props.getshowConsole();
        _consoleSize = props.getConsoleSize();
        return;
    }
    
    private boolean getshowConsole() {
        return _showConsole;
    }
    
    public void show(String title) {
        if (_frame != null && getshowConsole()) {
	    _frame.pack();
	    _frame.setBounds(10, 10, 410, 310);
	    _frame.setTitle(Resources.getString("console.frameTitle", title));
	    _frame.show();
        } else {
	    dispose();
        }
    }
    
    public void write(int b) throws IOException {
        if (_filePrintStream != null) {
	    _filePrintStream.write(b);
	    _filePrintStream.flush();
        }
        if (_sockLogStream != null) {
	    _sockLogStream.write(b);
	    _sockLogStream.flush();
        }
	// fix for 4660253
	insert(String.valueOf((char)b));
    }
    
    public void write(byte b[], int off, int len) throws IOException {
        if (_filePrintStream != null) {
	    _filePrintStream.write(b, off, len);
	    _filePrintStream.flush();
        }
        if (_sockLogStream != null) {
	    _sockLogStream.write(b, off, len);
	    _sockLogStream.flush();
	    
        }
	insert(new String(b, off, len));
    }
    
    private void insert(final String string) {
     
	// fix for 4472381: System.exit not working with shutdownhook and JWS console
	SwingUtilities.invokeLater(new Runnable() {
		public void run() {

		    if (_document != null && _ta != null) {
			try {
			    int length = _document.getLength();

    // If the User has n't specified config property to control the
    // size of Console Buffer, then restrict the size of Console to  
    // default value (10000). If config property (consoleBufferSize) 
    // is specified, then use that specific value.
    // Remove that many characters in the top so that console will 
    // hold only fixed number of chars and will not grow infinitely.
    // This is to prevent any memory buildup.
    // If the value of the config property specified is 0,
    // then we will not delete any stuff and allow it to grow.
   
			    if ((length > _consoleSize) && (_consoleSize > 0))
				{
				    _document.remove(0,length - _consoleSize);
				    length = _consoleSize;
				}
			    
			    _document.insertString(length, string, null);
			    if (_ta.getCaretPosition() == length) {
				_ta.setCaretPosition(length + string.length());
			    }
			} catch (BadLocationException ble) {}
		    }
		}
	    });
    }
    
    private static void dispose() {
        _document = null;
        _ta = null;
        _frame.dispose();
        _frame = null;
    }

    public static Document consoleErrorExit() {
	Document doc = _document;
	if ( _frame != null) {
	    _frame.hide();
	    Console.dispose();
	}
        return doc;
    }
	
}





