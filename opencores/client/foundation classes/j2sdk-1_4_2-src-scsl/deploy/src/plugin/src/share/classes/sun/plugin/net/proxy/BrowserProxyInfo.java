/*
 * @(#)BrowserProxyInfo.java	1.16 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.plugin.net.proxy;

import java.util.ArrayList;
import java.util.List;
import sun.plugin.resources.ResourceHandler;


/**
 * A simple container for all of the system information about
 * the HTTP proxy servers address.
 */

public class BrowserProxyInfo
{
    private int type = ProxyType.NONE;
    private String _httpHost;
    private int _httpPort = -1;
    private String _httpsHost;
    private int _httpsPort = -1;
    private String _ftpHost;
    private int _ftpPort = -1;
    private String _gopherHost;
    private int _gopherPort = -1;
    private String _socksHost;
    private int _socksPort = -1;
    private String[] _overrides=null;
    private String _autoConfigURL;
    
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


    public String getHttpHost() {
        return _httpHost;
    }

    public void setHttpHost(String httpHost) {
	this._httpHost = httpHost;
    }


    /**
     * Returns the HTTP port number or -1, if this property
     * hasn't been set.
     */
    public int getHttpPort() {
        return _httpPort;
    }

    public void setHttpPort(int httpPort) {
        this._httpPort = httpPort;
    }

    public String getHttpsHost() {
        return _httpsHost;
    }

    public void setHttpsHost(String httpsHost) {
	this._httpsHost = httpsHost;
    }

    public int getHttpsPort() {
        return _httpsPort;
    }

    public void setHttpsPort(int httpsPort) {
        this._httpsPort = httpsPort;
    }

    public String getFtpHost() {
        return _ftpHost;
    }

    public void setFtpHost(String ftpHost) {
	this._ftpHost = ftpHost;
    }

    public int getFtpPort() {
        return _ftpPort;
    }

    public void setFtpPort(int ftpPort) {
        this._ftpPort = ftpPort;
    }

    public String getGopherHost() {
        return _gopherHost;
    }

    public void setGopherHost(String gopherHost) {
	this._gopherHost = gopherHost;
    }

    public int getGopherPort() {
        return _gopherPort;
    }

    public void setGopherPort(int gopherPort) {
        this._gopherPort = gopherPort;
    }

    public String getSocksHost() {
        return _socksHost;
    }

    public void setSocksHost(String socksHost) {
	this._socksHost = socksHost;
    }

    public int getSocksPort() {
        return _socksPort;
    }

    public void setSocksPort(int socksPort) {
        this._socksPort = socksPort;
    }

    public String[] getOverrides() {
        return _overrides;
    }
    
    public void setOverrides(String[] overrides) {
        this._overrides=overrides; 
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

    public String toString() 
    {
        StringBuffer sb = new StringBuffer();

	// Print out proxy setting -- this is very important for debugging
        sb.append(ResourceHandler.getMessage("proxy_configuration"));

        switch (type)  
	{
	    case ProxyType.BROWSER:
		sb.append(ResourceHandler.getMessage("browser_config"));
		break;
	    case ProxyType.AUTO:
		sb.append(ResourceHandler.getMessage("auto_config"));
		sb.append("\n");
		sb.append("     URL: " + _autoConfigURL);
		break;
	    case ProxyType.MANUAL:
		sb.append(ResourceHandler.getMessage("manual_config"));
		sb.append("\n");
		sb.append("     " + ResourceHandler.getMessage("proxy_is"));

		if (_httpHost != null)
		{
		    sb.append("http=" + _httpHost);
		    if (_httpPort != -1)
			sb.append(":" + _httpPort);
		}
		if (_httpsHost != null)
		{
		    sb.append(",https=" + _httpsHost);
		    if (_httpsPort != -1)
			sb.append(":" + _httpsPort);
		}
		if (_ftpHost != null)
		{
		    sb.append(",ftp=" + _ftpHost);
		    if (_ftpPort != -1)
			sb.append(":" + _ftpPort);
		}
		if (_gopherHost != null)
		{
		    sb.append(",gopher=" + _gopherHost);
		    if (_gopherPort != -1)
			sb.append(":" + _gopherPort);
		}
		if (_socksHost != null)
		{
		    sb.append(",socks=" + _socksHost);
		    if (_socksPort != -1)
			sb.append(":" + _socksPort);
		}

		sb.append("\n");
		sb.append("     " + ResourceHandler.getMessage("proxy_override_is"));

		if (_overrides != null) 
		{
		    boolean first = true;
		    for (int idx = 0 ; idx < _overrides.length ; idx++) 
		    {
			if (idx != 0)
			    sb.append(",");    

			sb.append(_overrides[idx]);
		    }
		}
	        break;
	    case ProxyType.NONE:
		 sb.append(ResourceHandler.getMessage("no_proxy"));	     
		 break;
	    default:
		sb.append("<Unrecognized Proxy Type>");		  
        }

        return sb.toString();
    }
}

