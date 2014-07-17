/*
 * @(#)DownloadRequest.java	1.11 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.servlet;
import com.sun.javaws.util.GeneralUtil;
import java.io.File;
import javax.servlet.*;
import javax.servlet.http.*;

/** 
 * The DownloadRequest incapsulates all the data in a request
 */
public class DownloadRequest {
    // Arguments
    private static final String ARG_ARCH                = "arch";
    private static final String ARG_OS                  = "os";
    private static final String ARG_LOCALE              = "locale";
    private static final String ARG_VERSION_ID          = "version-id";
    private static final String ARG_CURRENT_VERSION_ID  = "current-version-id";
    private static final String ARG_PLATFORM_VERSION_ID = "platform-version-id";
    private static final String ARG_KNOWN_PLATFORMS     = "known-platforms";
    
    private String _path = null;
    private String _version = null;
    private String _currentVersionId = null;
    private String[] _os = null;
    private String[] _arch = null;
    private String[] _locale = null;
    private String[] _knownPlatforms = null;
    private boolean _isPlatformRequest = false;
    private ServletContext _context = null;
    
    private HttpServletRequest _httpRequest = null;
    
    // Contruct Request object based on HTTP request
    public DownloadRequest(HttpServletRequest request) {
	this((ServletContext)null, request);
    }

    public DownloadRequest(ServletContext context, HttpServletRequest request) {
	_context = context;
	_httpRequest = request;
	_path = request.getRequestURI(); 
	String context_path = request.getContextPath();	
	if (context_path != null) _path = _path.substring(context_path.length());       
	if (_path == null) _path = request.getServletPath(); // This works for *.<ext> invocations
	if (_path == null) _path = "/"; // No path given
	_path = _path.trim();
	if (_context != null && !_path.endsWith("/")) {
	    String realPath = _context.getRealPath(_path);
	    // fix for 4474021 - getRealPath might returns NULL
	    if (realPath != null) {
		File f = new File(realPath);
		if (f != null && f.exists() && f.isDirectory()) {
		    _path += "/";
		}
	    }
	}
        // Append default file for a directory
        if (_path.endsWith("/")) _path += "launch.jnlp";
	_version = getParameter(request, ARG_VERSION_ID);
	_currentVersionId = getParameter(request, ARG_CURRENT_VERSION_ID);
	_os = getParameterList(request, ARG_OS);
	_arch = getParameterList(request, ARG_ARCH);
	_locale = getParameterList(request, ARG_LOCALE);	    	    
	_knownPlatforms = getParameterList(request, ARG_KNOWN_PLATFORMS);
	String platformVersion = getParameter(request, ARG_PLATFORM_VERSION_ID);	    
	_isPlatformRequest =  (platformVersion != null);
	if (_isPlatformRequest) _version = platformVersion;	    
    }
    
    /** Returns a DownloadRequest for the currentVersionId, that can be used
     *  to lookup the existing cached version
     */
    private DownloadRequest(DownloadRequest dreq) {
	_context = dreq._context;
	_httpRequest = dreq._httpRequest;
	_path = dreq._path;	        
	_version = dreq._currentVersionId;
	_currentVersionId = null;
	_os = dreq._os;
	_arch = dreq._arch;
	_locale = dreq._locale;
	_knownPlatforms = dreq._knownPlatforms;	
	_isPlatformRequest =  dreq._isPlatformRequest;	
    }
    
    
    private String getParameter(HttpServletRequest req, String key) {
	String res = req.getParameter(key);
	return (res == null) ? null : res.trim();    
    }
    
    /* Split parameter at spaces. Convert '\ ' insto a space */
    private String[] getParameterList(HttpServletRequest req, String key) {
	String res = req.getParameter(key);
	return (res == null) ? null : GeneralUtil.getStringList(res.trim());
    }
    
    // Query
    public String getPath() { return _path; }
    public String getVersion() { return _version; }
    public String getCurrentVersionId() { return _currentVersionId; }
    public String[] getOS() { return _os; }
    public String[] getArch() { return _arch; }
    public String[] getLocale() { return _locale; }
    public String[] getKnownPlatforms() { return _knownPlatforms; }
    public boolean isPlatformRequest() { return _isPlatformRequest; }	
    public HttpServletRequest getHttpRequest() { return _httpRequest; }
    
    /** Returns a DownloadRequest for the currentVersionId, that can be used
     *  to lookup the existing cached version
     */
    DownloadRequest getFromDownloadRequest() {
	return new DownloadRequest(this);
    }
    
    // Debug
    public String toString() { 
	return "DownloadRequest[path=" + _path + 
	    showEntry(" version=", _version) +
	    showEntry(" currentVersionId=", _currentVersionId) +
	    showEntry(" os=", _os) + 
	    showEntry(" arch=", _arch) + 
	    showEntry(" locale=", _locale) +
	    showEntry(" knownPlatforms=", _knownPlatforms)
	    + " isPlatformRequest=" + _isPlatformRequest + "]";
    }		
    
    private String showEntry(String msg, String value) {
	if (value == null) return "";
	return msg + value;		
    }
    
    private String showEntry(String msg, String[] value) {
	if (value == null) return "";
	return msg + java.util.Arrays.asList(value).toString();	
    }
}


