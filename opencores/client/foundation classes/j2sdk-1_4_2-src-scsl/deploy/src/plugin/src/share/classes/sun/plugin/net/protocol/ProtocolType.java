/*
 * @(#)ProtocolType.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.net.protocol;

public interface ProtocolType {
    /*
     * Protocol type
     */
    public static final int HTTP = 0x0001;
    public static final int HTTPS = 0x0002;
    public static final int FTP = 0x0004;
    public static final int GOPHER = 0x0008;
    public static final int SOCKS = 0x0010;
    public static final int JAR = 0x0020;
    public static final int JAVASCRIPT = 0x0040;
    public static final int RMI = 0x0080;
}



