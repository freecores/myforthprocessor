/*
 * @(#)LoadFrame.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


import java.awt.Frame;

public class LoadFrame {

    public static void main(String[] args) {
	new Frame().show();
	// This starts a thread which never exits - so we suicide.
	try {
	    Thread.sleep(5000);
	} catch (Exception e) {
	}
	System.exit(0);
    }
}
