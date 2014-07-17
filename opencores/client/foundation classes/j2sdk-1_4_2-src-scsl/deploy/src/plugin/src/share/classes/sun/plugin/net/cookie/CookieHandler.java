/*
 * @(#)CookieHandler.java	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.net.cookie;

import java.net.URL;
import sun.plugin.services.ServiceUnavailableException;


public interface CookieHandler 
{
    /* 
     * getCookieInfo takes a particular URL and returns its cookie info.
     */
    String getCookieInfo(String u) throws ServiceUnavailableException;

    /* 
     * setCookieInfo takes a particular URL and its cookie info.
     */
    void setCookieInfo(String url, String value) throws ServiceUnavailableException;
}



