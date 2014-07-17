/*
 * @(#)ProxyInfo.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.java.browser.net;

/**
 *
 * @author  Zhengyu Gu
 * @version 
 */
public interface ProxyInfo {
    public String   getHost();
    public int      getPort();
    public boolean  isSocks();
}

