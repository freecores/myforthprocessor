/*
 * @(#)ApplicationLaunchException.java	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.net.www;

/**
 * An exception thrown by the MimeLauncher when it is unable to launch
 * an external content viewer.
 *
 * @version     1.9, 01/23/03
 * @author      Sunita Mani
 */

public class ApplicationLaunchException extends Exception {
    public ApplicationLaunchException(String reason) {
	super(reason);
    }
}
