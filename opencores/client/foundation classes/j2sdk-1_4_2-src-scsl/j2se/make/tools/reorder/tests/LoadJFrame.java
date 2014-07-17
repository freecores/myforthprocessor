/*
 * @(#)LoadJFrame.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


import javax.swing.JFrame;

public class LoadJFrame {

    public static void main(String[] args) {
	new JFrame().show();
	// This starts a thread which never exits - so we suicide.
	try {
	    Thread.sleep(10000);
	} catch (Exception e) {
	}
	System.exit(0);
    }
}
