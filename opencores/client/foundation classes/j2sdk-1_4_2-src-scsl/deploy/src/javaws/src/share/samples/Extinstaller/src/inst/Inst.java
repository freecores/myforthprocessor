/*
 * @(#)Inst.java	1.3 03/01/23
 * 
 * Copyright (c) 2003 Sun Microsystems, Inc. All Rights Reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * -Redistribution of source code must retain the above copyright notice, this
 *  list of conditions and the following disclaimer.
 * 
 * -Redistribution in binary form must reproduce the above copyright notice, 
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 * 
 * Neither the name of Sun Microsystems, Inc. or the names of contributors may 
 * be used to endorse or promote products derived from this software without 
 * specific prior written permission.
 * 
 * This software is provided "AS IS," without a warranty of any kind. ALL 
 * EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND WARRANTIES, INCLUDING
 * ANY IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * OR NON-INFRINGEMENT, ARE HEREBY EXCLUDED. SUN MIDROSYSTEMS, INC. ("SUN")
 * AND ITS LICENSORS SHALL NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY LICENSEE
 * AS A RESULT OF USING, MODIFYING OR DISTRIBUTING THIS SOFTWARE OR ITS
 * DERIVATIVES. IN NO EVENT WILL SUN OR ITS LICENSORS BE LIABLE FOR ANY LOST 
 * REVENUE, PROFIT OR DATA, OR FOR DIRECT, INDIRECT, SPECIAL, CONSEQUENTIAL, 
 * INCIDENTAL OR PUNITIVE DAMAGES, HOWEVER CAUSED AND REGARDLESS OF THE THEORY 
 * OF LIABILITY, ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, 
 * EVEN IF SUN HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * 
 * You acknowledge that this software is not designed, licensed or intended
 * for use in the design, construction, operation or maintenance of any
 * nuclear facility.
 */


import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.util.*;

public class Inst {

    private native String getString();
    private static boolean _ready = false;
    private static final String configDir = "inst_test";
    private static final String configFile = "config.txt";

    static {
	String home = System.getProperty("user.home");
	if (!home.endsWith(File.separator)) {
	    home += File.separatorChar;
	}
	File config =
	    new File(home + configDir + File.separatorChar + configFile);

	if (config.exists()) {
	    Properties p = new Properties();
	    InputStream is = null;
	    try {
		is = new FileInputStream(config);
		p.load(is);
	    } catch (IOException ioe) {
		System.err.println("ioe: " + ioe);
		ioe.printStackTrace();
		System.exit(1);
	    } finally {
		try {
		    if (is != null) is.close();
		} catch (IOException ioe2) {
		    System.err.println("ioe2: " + ioe2);
		    ioe2.printStackTrace();
		    System.exit(1);
		}
	    }
	    String libPath = p.getProperty("libPath");
	    try {
		System.load(libPath);
	    } catch (UnsatisfiedLinkError e) {
		System.err.println("Error: " + e);
		e.printStackTrace();
		System.exit(1);
	    }
	    _ready = true;
	}
    }

    Inst() {
	JFrame t = new JFrame("Java Web Start");
	Container contentPane = t.getContentPane();
	contentPane.setLayout(new FlowLayout());
	String s = "Not working";
	if (_ready) {
	    try {
		s = getString();
	    } catch (UnsatisfiedLinkError ule) {
		ule.printStackTrace();
	    }
	}
        JLabel l = new JLabel(s);
        JButton b = new JButton("Click to exit.");
        contentPane.add(l);
        contentPane.add(b);
        
	t.addWindowListener(new WindowAdapter() {
	    public void windowClosing(WindowEvent e) {
		System.exit(0);
	    }
	});

        b.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
		System.exit(0);
            }
        }
        );
        
        t.pack();
	t.setSize(200,100);
        t.setVisible(true);
    }

    public static void main (String args[]) {
	new Inst();
    }
}

