/*
 * @(#)InternetProxyInfo.java	1.10 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.proxy;

import java.util.*;

/**
 * A simple container for all of the system information about
 * the HTTP proxy servers address.
 *
 * @version 1.10, 01/23/03
 */

public class InternetProxyInfo
{
    public static final int NONE = 0;
    public static final int MANUAL = 1;  /* Matches NS "network.proxy.type" pref */
    public static final int AUTO = 2;

    private int type = NONE;
    private String _httpHost;
    private int _httpPort = -1;
    private String[] _overrides=null;
    private String _autoConfigURL;
    
    /**
     * These two are entries we find in the browser preferences
     * however the proxyType was AUTO, therefore these are used
     * as a default to verify and prompt with the user
     */
    private String _invalidhttpHost;
    private int _invalidhttpPort;


    /**
     * One of MANUAL, AUTO, or NONE.  If type is AUTO then
     * only the autoconfig property is guaranteed to be
     * valid.  If type is NONE then a proxy server isn't
     * being used.
     */
    public int getType() {
        return type;
    }

    public void setType(int type) {
        this.type = type;
    }


    public String getHTTPHost() {
        return _httpHost;
    }
    public String getInvalidHTTPHost() {
        return _invalidhttpHost;
    }
    
    void setInvalidHTTPHost(String httpHost){
        this._invalidhttpHost = httpHost;
    }

    public void setHTTPHost(String httpHost) {
	this._httpHost = httpHost;
    }


    /**
     * Returns the HTTP port number or -1, if this property
     * hasn't been set.
     */
    public int getHTTPPort() {
        return _httpPort;
    }

    public void setHTTPPort(int httpPort) {
        this._httpPort = httpPort;
    }

    public int getInvalidHTTPPort() {
        return this._invalidhttpPort;
    }
    
    void setInvalidHTTPPort(int portnum) {
        this._invalidhttpPort = portnum;
    }

    public String getOverrides() {
	String noProxyProp = "";
	if (_overrides != null && _overrides.length > 0) {
	    for (int idx = 0 ; idx < _overrides.length ; idx++) {
		if ( idx != _overrides.length -1 ) {
		    noProxyProp = noProxyProp.concat(_overrides[idx]+"|");
		} else {
		    noProxyProp = noProxyProp.concat(_overrides[idx]);
		}
	    }	   
	}
	
        return noProxyProp;
    }
    
    public void setOverrides(String[] overrides) {
      this._overrides=(String[])overrides.clone(); 
    }

    public void setOverrides(List overrides) {
        if (overrides != null) {
            ArrayList overrideAL = new ArrayList(overrides);
            _overrides = new String[overrideAL.size()];
            this._overrides = (String[])overrideAL.toArray(_overrides);
        }
    }


    public String getAutoConfigURL() {
        return _autoConfigURL;
    }

    void setAutoConfigURL(String autoConfigURL) {
        _autoConfigURL = autoConfigURL;
    }


    /**
     * Return true if the HTTP proxy address was configured
     * manually and the port/server properties are valid.
     * If this property is true, then it's safe to set the
     * System HTTP proxy properties.
     */
    public boolean isValidManualHTTPConfiguration() {
        return (type == MANUAL) && (_httpHost != null) && (_httpPort != -1);
    }
   /**
    * If AUTO is set if we were able to determine the host.
    */
    public boolean isValidAutoHTTPConfiguration() {
        return (type == AUTO) && (_httpHost != null) && (_httpPort != -1);
    }


    public String toString() {
        String cls = getClass().getName() + "@" + Integer.toHexString(hashCode());
        StringBuffer sb = new StringBuffer(cls);
        if (type == MANUAL) {
            sb.append(" Manual Configuration");

            if (_httpHost != null) {
                sb.append(" HTTP: " + _httpHost + ":" + _httpPort);
            }
            if (_overrides != null) {
                sb.append(" overrides: ");
                for (int idx = 0 ; idx < _overrides.length ; idx++) {
                    sb.append(_overrides[idx]+",");
                }
            }
        } else if (type == AUTO) {
            sb.append(" AutoConfiguration URL " + _autoConfigURL);
            sb.append(" Detected settings ");
            sb.append(" HTTP: " + _httpHost + ":" + _httpPort);
            sb.append(" (perhaps?) HTTP: " + _invalidhttpHost + ":" + _invalidhttpPort);
        } else if (type == NONE) {
            sb.append(" No Proxy Server");
        } else {
            sb.append(" <Unrecognized Proxy Type>");
        }
        return sb.toString();
    }
}

