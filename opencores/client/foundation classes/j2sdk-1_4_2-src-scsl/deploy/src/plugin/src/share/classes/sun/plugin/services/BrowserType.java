/*
 * @(#)BrowserType.java	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.services;

import java.net.URL;

/** 
 * BrowserType is an interface that encapsulates the browser type.
 */
public final class BrowserType
{
    private final static int UNKNOWN = 0x0000;

    private final static int BROWSER_WIN32 = 0x0100;

    private final static int BROWSER_UNIX = 0x1000;

    private final static int INTERNET_EXPLORER = 0x0001;

    private final static int NETSCAPE4 = 0x0002;

    private final static int NETSCAPE6 = 0x0003;

    private final static int NETSCAPE45 = 0x0004;

    public final static int INTERNET_EXPLORER_WIN32 = BROWSER_WIN32 | INTERNET_EXPLORER;

    public final static int NETSCAPE4_WIN32 = BROWSER_WIN32 | NETSCAPE4;

    public final static int NETSCAPE45_WIN32 = BROWSER_WIN32 | NETSCAPE45;

    public final static int NETSCAPE6_WIN32 = BROWSER_WIN32 | NETSCAPE6;

    public final static int NETSCAPE4_UNIX = BROWSER_UNIX | NETSCAPE4;

    public final static int NETSCAPE45_UNIX = BROWSER_UNIX | NETSCAPE45;

    public final static int NETSCAPE6_UNIX = BROWSER_UNIX | NETSCAPE6;

    public final static int AXBRIDGE = 0x0005;
}



