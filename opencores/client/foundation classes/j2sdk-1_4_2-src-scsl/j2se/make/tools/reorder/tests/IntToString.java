/*
 * @(#)IntToString.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */



public class IntToString {

    public static void main(String[] args) {
	String n = "10000";
	System.err.println("Hello");

	if (args.length == 0) {
	    try {
		Thread.currentThread().sleep(Integer.parseInt(n));
	    } catch (Exception e) {
		e.printStackTrace();
	    }
	    System.exit(0);
	}
    }
}
