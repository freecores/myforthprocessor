/*
 * @(#)FtpProtocolException.java	1.17 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.net.ftp;

import java.io.*;

/**
 * This exeception is thrown when unexpected results are returned during
 * an FTP session.
 *
 * @version	1.17, 01/23/03
 * @author	Jonathan Payne
 */
public class FtpProtocolException extends IOException {
    FtpProtocolException(String s) {
	super(s);
    }
}

