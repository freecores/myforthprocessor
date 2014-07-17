/*
 * @(#)Config.java	1.3 03/01/23
 * 
 * Copyright (c) 2003 Sun Microsystems, Inc. All Rights Reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * -Redistribution of source code must retain the above copyright notice, this
 *  list of conditions and the following disclaimer.
 * 
 * -Redistribution in binary form must reproduce the above copyright notice, 
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 * 
 * Neither the name of Sun Microsystems, Inc. or the names of contributors may 
 * be used to endorse or promote products derived from this software without 
 * specific prior written permission.
 * 
 * This software is provided "AS IS," without a warranty of any kind. ALL 
 * EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND WARRANTIES, INCLUDING
 * ANY IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * OR NON-INFRINGEMENT, ARE HEREBY EXCLUDED. SUN MIDROSYSTEMS, INC. ("SUN")
 * AND ITS LICENSORS SHALL NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY LICENSEE
 * AS A RESULT OF USING, MODIFYING OR DISTRIBUTING THIS SOFTWARE OR ITS
 * DERIVATIVES. IN NO EVENT WILL SUN OR ITS LICENSORS BE LIABLE FOR ANY LOST 
 * REVENUE, PROFIT OR DATA, OR FOR DIRECT, INDIRECT, SPECIAL, CONSEQUENTIAL, 
 * INCIDENTAL OR PUNITIVE DAMAGES, HOWEVER CAUSED AND REGARDLESS OF THE THEORY 
 * OF LIABILITY, ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, 
 * EVEN IF SUN HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * 
 * You acknowledge that this software is not designed, licensed or intended
 * for use in the design, construction, operation or maintenance of any
 * nuclear facility.
 */

import javax.jnlp.ServiceManager;
import javax.jnlp.BasicService;
import javax.jnlp.ExtensionInstallerService;
import javax.jnlp.DownloadService;
import javax.jnlp.UnavailableServiceException;


/** Contains configuration information for an
 *  installer.
 *
 *  Part of the configuration is read as properties
 *  set in the JNLP file - other information is
 *  currently hard-coded as strings - they should
 *  properly be moved to resources.
 *
 */
public class Config {
    
    /** JNLP services */
    static private ExtensionInstallerService _eiService;
    static private BasicService _basicService;
    static private DownloadService _downloadService;
    
    /** Initialize basic services class */
    static private synchronized void initServices() {
        if (_eiService == null) {
            try {
                _eiService = (ExtensionInstallerService)ServiceManager.lookup("javax.jnlp.ExtensionInstallerService");
                _basicService = (BasicService)ServiceManager.lookup("javax.jnlp.BasicService");
                _downloadService = (DownloadService)ServiceManager.lookup("javax.jnlp.DownloadService");
            } catch(UnavailableServiceException use) {
                Config.trace("Unable to locate service: " + use);
            }
        }
    }
    
    static public BasicService getBasicService() {
        if (_basicService == null) {
            initServices();
        }
        return _basicService;
    }
    
    static public ExtensionInstallerService getInstallService() {
        if (_eiService == null) {
            initServices();
        }
        return _eiService;
    }

    static public DownloadService getDownloadService() {
        if (_downloadService == null) {
            initServices();
        }
        return _downloadService;
    }
    
    /*
     * Configuration for JNLP file
     */
    
    // Win/Solaris
    static public String getInstallerResource() {
        return System.getProperty("installerEntry");
    }
    
    // Win/Solaris
    static public String getInstallerLocation() {
        return System.getProperty("installerLocation");
    }
    
    // Win-only
    static public String getNativeLibName() {
        return System.getProperty("lib");
    }
    
    // Win/Solaris
    static public String getExecString() {
        return System.getProperty("execString");
    }
    
    // Solaris-only
    static public String getWaitString(int i) {
        return System.getProperty("waitString." + i);
    }
    
    // Solaris-only
    static public String getResponseString(int i) {
        return System.getProperty("responseString." + i);
    }
    
    // Win/Solaris
    static public String getJavaPath() {
        return System.getProperty("javaPath");
    }
    
    // Solaris-only
    static public boolean isSolarisInstall() {
        String result = System.getProperty("isSolarisInstall");
        return (result != null && result.length() > 0);
    }
    
    // Win/Solaris
    static public boolean isVerbose() {
        String result = System.getProperty("verbose");
        return (result != null && result.length() > 0);
    }
    
    // Win-only
    static public String getJavaVersion() {
        return System.getProperty("javaVersion");
    }
    
    // Win-only
    static public String getMsvcrtVersionMS() {
        return System.getProperty("msvcrt.versionMS");
    }
    
    // Win-only
    static public String getMsvcrtVersionLS() {
        return System.getProperty("msvcrt.versionLS");
    }
    
    /*
     *  String resources
     */
    public static void setUnpackingHeader() {
        _eiService.setHeading("Unpacking installer component");
        _eiService.setStatus(null);
    }
    
    public static void setInstallHeader() {
        _eiService.setHeading("Installing. please wait...");
        _eiService.setStatus(null);
        _eiService.hideProgressBar();
    }
    
    public static void setFailedUnpackHeader() {
        _eiService.setHeading("Failed to unpack installer component");
        _eiService.setStatus(null);
        _eiService.hideProgressBar();
    }
    
    public static void setProgress(int totalRead, int totalSize)  {
        if (totalSize != 0) {
            float fraction = ((float)totalRead)/((float)totalSize);
            int   percent = (int)(fraction * 100);
            _eiService.setStatus("Completed: " + percent + "%");
            _eiService.updateProgress(percent);
            
        }
    }
    
    public static String getRebootMessage() {
        return "Before you can use this program you must restart Windows.";
    }
    
    public static String getRebootTitle() {
        return "Restart";
    }
    
    public static String getRebootNowString() {
        return "Restart";
    }
    
    public static String getRebootLaterString() {
        return "Restart later";
    }
        
    /** debugging method */
    static public void trace(String msg) {
        if (isVerbose()) {
            System.out.println(msg);
        }
    }
}


