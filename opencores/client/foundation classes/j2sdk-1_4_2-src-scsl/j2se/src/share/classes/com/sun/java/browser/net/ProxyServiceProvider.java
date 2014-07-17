/*
 * @(#)ProxyServiceProvider.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.java.browser.net;

import java.net.URL;

/**
 *
 * @author  Zhengyu Gu
 * @version 1.0
 */
public interface ProxyServiceProvider {
    public ProxyInfo[] getProxyInfo(URL url);
}

