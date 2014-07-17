/*
 * @(#)ControlPanelHelp.java	1.1 03/01/14
 *
 * Copyright 2002 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.resources;

import java.util.ListResourceBundle;
import java.awt.event.KeyEvent;

/**
 * US English verison of ControlPanel strings.
 *
 * @author Bruce Murphy
 */

public class ControlPanelHelp extends ListResourceBundle {

    private static String newline = "\n";

    public Object[][] getContents() {
        return contents;
    }

    static final Object[][] contents = {
    { "help.file", newline +
                   "Java Plug-in Control Panel Help" + newline + newline + newline +
                   "The following topics are covered in this help:" + newline +
                   "      Overview" + newline + 
                   "      Saving Options" + newline + 
                   "      Setting Control Panel Options" + newline + 
                   "      Basic Panel" + newline + 
                   "      Advanced Panel" + newline +
                   "      Browser Panel" + newline + 
                   "      Proxies Panel" + newline + 
                   "      Cache Panel" + newline + 
                   "      Certificates Panel" + newline + 
                   "      Update Panel" + newline + newline + 
                   "Overview" + newline + newline +
                   "The Java Plug-in Control Panel enables you to change the default settings used by the Java Plug-in at startup." + newline +
                   "All applets running in an active instance of Java Plug-in will use these settings." + newline +
                   "The Java Plug-in Developer Guide, mentioned in this document, can be found at (URL subject to change)." + newline +
                   "http://java.sun.com/j2se/1.4/docs/guide/plugin/developer_guide/contents.html" + newline + newline +
                   "Saving Options" + newline + newline +
                   "When you have completed your changes to the Control Panel options, click Apply to save the changes." + newline +
                   "Click Reset to cancel your changes and revert back to the last values that were set." + newline +
                   "Note that this is not the same as the set of default values that were originally set for Java Plug-in when it was installed." + newline + newline +
                   "Setting Control Panel Options" + newline + newline +
                   "There are six panels from which you can set various options within the Java Plug-in Control Panel." + newline +
                   "These panels are labeled: " + newline +
                   "      Basic" + newline + 
                   "      Advanced" + newline + 
                   "      Browser" + newline + 
                   "      Proxies" + newline + 
                   "      Cache" + newline + 
                   "      Certificates" + newline + 
                   "      Update" + newline + newline +
                   "Each is described separately below." + newline + newline + newline +
                   " Basic" + newline +
                   "Show Java Console" + newline + newline + 
                   "      Displays the Java Console while running applets. The console displays messages printed by System.out and System.err." + newline +
                   "      It is useful for debugging problems." + newline + newline +
                   "Hide console" + newline + newline + 
                   "      The Java Console is running but hidden. This is the default setting (checked)." + newline + newline +
                   "Do not start console" + newline + newline + 
                   "      The Java Console is not started." + newline + newline + 
                   "Show Exception Dialog Box" + newline + newline + 
                   "      Show an exception dialog box when exceptions occur. The default is to not show the exception dialog box (unchecked)." + newline + newline +
                   "Show Java in System Tray (Windows only)" + newline + newline + 
                   "      When this option is enabled the Java coffee-cup logo displays in the system tray when the Java Plug-in is started" + newline +
                   "      and is removed from the system tray when the Java Plug-in is shutdown." + newline +
                   "      The Java coffee-cup logo indicates to the user that a Java VM is running, and it provides information about the" + newline +
                   "      Java release and control over the Java Console." + newline +
                   "      This option is enabled by default (checked)." + newline + newline +
                   "      Java system tray functionality:" + newline + newline + 
                   "      When the mouse is pointed at the Java coffee-cup logo, the text \"Java\" is displayed." + newline + newline +

                   "      When the Java system tray icon is left-double-clicked, the Java Console window will be brought up." + newline + newline + 
                   "      When the Java system tray icon is right-clicked, a popup menu will be shown with the following menu items:" + newline + newline +
                   "            Open/Close Console" + newline + 
                   "            About Java" + newline + 
                   "            Disable" + newline + 
                   "            Exit" + newline + newline + 
                   "      Open/Close Console opens/closes the Java Console window. The menu item will display Open Console if the Java" + newline +
                   "      Console is hidden and Close Console if the Java Console is showing." + newline + newline +
                   "      About Java will bring up the About box for Java 2 Standard Edition." + newline + newline +
                   "      Disable disables and removes the Java icon from the system tray for this and future sessions. When the Java Plug-in is" + newline +
                   "      restarted, the Java icon will not appear in the system tray." + newline +
                   "      On how to show the Java icon in the system tray, after it has been disabled, see the below note." + newline + newline +
                   "      Exit removes the Java icon from the system tray for the current session only. When the Java Plug-in is restarted, the" + newline +
                   "      Java icon will again appear in the system tray." + newline + newline + newline + 
                   "                Notes" + newline + newline +
                   "                1. If \"Show Java in System Tray\" is checked, the Java icon will show up in the system tray even if \"Do not start" + newline +
                   "                console\" is selected." + newline + newline +
                   "                2. To enable the Java icon after it has been disabled, start the Java Plug-in Control Panel, check \"Show Java" + newline +
                   "                in System Tray\", and press \"Apply\"." + newline + newline +
                   "                3. If other Java VMs are already running and other Java icons have been added to the system tray, changing" + newline +
                   "                the setting in the Java Plug-in Control Panel will not affect these icons." + newline +
                   "                The setting will only affect the Java icon behavior when the Java VM is started afterwards." + newline + newline + newline +
                   " Advanced" + newline + newline + newline +
                   "Java Run Time Environment" + newline + newline +
                   "      Enables Java Plug-in to run with any Java 2 JRE or SDK, Standard Edition v 1.3 or 1.4 installed on your machine." + newline +
                   "      Java Plug-in 1.3/1.4 is delivered with a default JRE." + newline +
                   "      However, you can override the default JRE and use an older or newer version. The Control Panel automatically detects" + newline +
                   "      all versions of the Java 2 SDK or JRE installed on the machine. In the list box all the versions which you" + newline +
                   "      can use are displayed." + newline +
                   "      The first item in the list will always be the default JRE; the last item will always say Other. If you choose Other, you must" + newline +
                   "      specify the path to the Java 2 JRE or SDK, Standard Edition v 1.3/1.4." + newline + newline + 
                   "                Note" + newline + newline +
                   "                Only advanced users should change this option. Changing the default JRE is not recommended." + newline + newline + newline +
                   "Java Run Time Parameters" + newline + newline + 
                   "      Overrides the Java Plug-in default startup parameters by specifying custom options. The syntax is the same as used" + newline +
                   "      with parameters to the Java command line invocation. See the Java 2 Standard Edition (J2SE) documentation for" + newline +
                   "      a full list of command line options." + newline +
                   "      The URL below is subject to change:" + newline + newline +
                   "            http://java.sun.com/j2se/1.4/docs/tooldocs/<platform>/java.html" + newline + newline + 
                   "            where <platform> is one of the operating systems: solaris, linux, win32." + newline + newline + 
                   "      Below are some examples of Java runtime parameters." + newline + newline +
                   "      Enabling and disabling assertion support" + newline + newline +
                   "            To enable assertion support, the following system property must be specified in the Java Runtime Parameters:" + newline + newline +
                   "                  -D[ enableassertions | ea ][:<package name>\"...\" | : <class name> ]" + newline + newline +
                   "            To disable assertion in the Java Plug-in, specify the following in the Java Runtime Parameters:" + newline + newline + 
                   "                  -D[ disableassertions | da ][:<package name>\"...\" | : <class name> ]" + newline + newline + 
                   "            See Assertion Facility for more details on enabling/disabling assertions." + newline +
                   "            http://java.sun.com/j2se/1.4/docs/guide/lang/assert.html (URL subject to change)." + newline + newline +
                   "            Assertion is disabled in Java Plug-in code by default. Since the effect of assertion is determined during Java Plug-in" + newline +
                   "            startup, changing assertion settings in the Java Plug-in Control Panel will require a browser restart in order" + newline +
                   "            for the new settings to take effect." + newline + newline + 
                   "            Because Java code in Java Plug-in also has built-in assertion, it is possible to enable the assertion in" + newline +
                   "            Java Plug-in code through the following:" + newline + newline +
                   "                  -D[ enableassertions | ea ]:sun.plugin" + newline + newline + 
                   "      Tracing and logging support" + newline + newline +
                   "            Tracing is a facility to redirect any output in the Java Console to a trace file (.plugin<version>.trace)." + newline + newline +
                   "                  -Djavaplugin.trace=true" + newline +
                   "                  -Djavaplugin.trace.option=basic|net|security|ext|liveconnect" + newline + newline +
                   "            If you do not want to use the default trace file name:" + newline + newline +
                   "                  -Djavaplugin.trace.filename=<tracefilename>" + newline + newline + 
                   "            Similar to tracing, logging is a facility to redirect any output in the Java Console to a log file (.plugin<version>.log)" + newline +
                   "            using the Java Logging API." + newline +
                   "            Logging can be turned on by enabling the property javaplugin.logging." + newline + newline +
                   "                  -Djavaplugin.logging=true" + newline + newline +
                   "            If you do not want to use the default log file name, enter:" + newline + newline +
                   "                  -Djavaplugin.log.filename=<logfilename>" + newline + newline + 
                   "            Furthermore, if you do not want to overwrite the trace and log files each session, you can set the property:" + newline + newline + 
                   "                  -Djavaplugin.outputfiles.overwrite=false." + newline + newline + 
                   "            If the property is set to false, then trace and log files will be uniquely named for each session. If the default trace" + newline +
                   "            and log file names are used, then the files would be named as follows" + newline + newline +
                   "                  .plugin<username><date hash code>.trace" + newline +
                   "                  .plugin<username><date hash code>.log" + newline + newline +
                   "            Tracing and logging set through the Control Panel will take effect when the Plug-in is launched, but changes" + newline +
                   "            made through the Control Panel while a  Plug-in is running will have no effect until a restart." + newline + newline + 
                   "            For more information about tracing and logging, see Tracing and Logging in the Java Plug-in Developer Guide." + newline + newline +
                   "      Debugging applets in Java Plug-in" + newline + newline +
                   "            The following options are used when debugging applets in the Java Plug-in." + newline +
                   "            For more information on this topic see the Debugging Support in the Java Plug-in Developer Guide." + newline + newline +
                   "                  -Djava.compiler=NONE" + newline +
                   "                  -Xnoagent" + newline +
                   "                  -Xdebug" + newline + 
                   "                  -Xrunjdwp:transport=dt_shmem,address=<connect-address>,server=y,suspend=n" + newline + newline +
                   "            The <connect-address> can be any string (example: 2502) which is used by the Java Debugger (jdb) later" + newline +
                   "            to connect to the JVM." + newline + newline + 
                   "      Default connection timeout" + newline + newline +
                   "            When a connection is made by an applet to a server and the server doesn't respond properly, the applet may" + newline +
                   "            appear to hang and may also cause the browser to hang, because there is no network connection timeout" + newline +
                   "            (by default it's set to -1, which means there is no timeout set)." + newline + newline +
                   "            To avoid this problem, Java Plug-in has added a default network timeout value (2 minutes) for all HTTP connections.:" + newline +
                   "            You can override this setting in the Java Runtime Parameters:" + newline + newline +
                   "                  -Dsun.net.client.defaultConnectTimeout=value in milliseconds" + newline + newline +
                   "            Another networking property that you can set is sun.net.client.defaultReadTimeout." + newline + newline +
                   "                  -Dsun.net.client.defaultReadTimeout=value in milliseconds" + newline + newline +
                   "                  Note" + newline + newline +
                   "                  Java Plug-in does not set sun.net.client.defaultReadTimeout by default. If you want to set it, do so through" + newline +
                   "                  the Java Runtime Parameters as shown above." + newline + newline + newline +
                   "            Networking properties description:" + newline + newline +
                   "            sun.net.client.defaultConnectTimeout" + newline +
                   "            sun.net.client.defaultReadTimeout" + newline + newline +
                   "                  These properties specify, respectively, the default connect and read timeout values for the protocol handlers" + newline +
                   "                  used by java.net.URLConnection. The default values set by the protocol handlers is -1, which means" + newline +
                   "                  there is no timeout set." + newline + newline +
                   "                  sun.net.client.defaultConnectTimeout specifies the timeout (in milliseconds) to establish the connection to the host." + newline +
                   "                  For example, for http connections it is the timeout when establishing the connection to the http server." + newline +
                   "                  For ftp connections it is the timeout when establishing the connection to ftp servers." + newline + newline +
                   "                  sun.net.client.defaultReadTimeout specifies the timeout (in milliseconds) when reading from an input stream" + newline +
                   "                  when a connection is established to a resource." + newline + newline + 
                   "            For the official description of these networking properties," + newline +
                   "            see http://java.sun.com/j2se/1.4/docs/guide/net/properties.html." + newline + newline + newline +
                   " Browser" + newline + newline + newline +
                   "This panel relates only to Microsoft Windows installations; it does not appear in other installations. Check any browser" + newline +
"for which you want Java Plug-in to be the default Java runtime, instead of the internal JVM of that browser." + newline +
"This is to enable APPLET tag support in Internet Explorer and Netscape 6 via Java Plug-in." + newline + newline + newline +
                   " Proxies" + newline + newline +newline +
                   "Use the Proxies panel to use the browser default settings or to override the proxy address and port for the different protocols." + newline + newline + 
                   "Use browser settings" + newline + newline + 
                   "      Check this to use the browser default proxy settings. This is the default setting (checked)." + newline + newline +
                   "Proxy information table" + newline + newline + 
                   "      You can override the default settings by unchecking the \"Use Browser Settings\" check box, then completing the proxy" + newline +
                   "      information table beneath the check box. You can enter the proxy address and port for each of the supported" + newline +
                   "      protocols: HTTP, Secure (HTTPS), FTP, Gopher, and Socks." + newline + newline + 
                   "No proxy host" + newline + newline + 
                   "      This is a host or list of hosts for which no proxy/proxies are to be used. No proxy host is usually used for an internal" + newline +
                   "      host in an intranet environment." + newline + newline + 
                   "Automatic proxy configuration URL" + newline + 
                   "      This is the URL for the JavaScript file (.js or .pac extension) that contains the FindProxyForURL function." + newline +
                   "      FindProxyForURL has the logic to determine the proxy server to use for a connection request." + newline + newline + 
                   "For additional details about proxy configuration, see the chapter called Proxy Configuration in the Java Plug-in" + newline +
                   "Developer Guide." + newline + newline + newline +
                   " Cache" + newline + newline + newline +
                   "           Note" + newline + newline +
                   "           The cache referred to here is the sticky cache; i.e., the disk cache created and controlled by Java Plug-in which" + newline +
                   "           the browser cannot overwrite. See Applet Caching in the Java Plug-in Developer Guide for more information." + newline + newline + newline +
                   "Enable Caching" + newline + newline + 
                   "      Check this to enable caching. This the default setting (checked). With applet caching enabled, performance is" + newline +
                   "      improved because once an applet is cached it no longer needs to be downloaded when referenced again." + newline + newline +
                   "      The Java Plug-in caches files of the following types downloaded via HTTP/HTTPS:" + newline + newline +
                   "            .jar (jar file)" + newline +
                   "            .zip (zip file)" + newline +
                   "            .class (java class file)" + newline +
                   "            .au (audio file)" + newline +
                   "            .wav (audio file)" + newline +
                   "            .jpg (image file)" + newline +
                   "            .gif (image file)" + newline + newline +
                   "View files in Cache" + newline + newline + 
                   "      Press this to view the cached files. Another dialog (Java Plug-in Cache Viewer) will pop up and display the cached files." + newline +
                   "      The Cache Viewer displays the following information about the files in cache: Name, Type, Size, Expire Date," + newline +
                   "      Last Modified, Version, and URL. In the Cache Viewer you can also selectively delete files in the cache." + newline +
                   "      This is an alternative to the Clear Cache option described below, which deletes all files in the cache." + newline + newline +
                   "Clear Cache" + newline + newline + 
                   "      Press this to clear all files in the cache. You will be prompted (Erase all files in ... _cache?) before the files are removed." + newline + newline + 
                   "Location" + newline + newline + 
                   "      You can use this to specify the location of the cache. The default location of the cache is <user home>/.jpi_cache, where" + newline +
                   "      <user home> is the value of the system property user.home. Its value depends on the OS." + newline + newline +
                   "Size" + newline + newline + 
                   "      You can check Unlimited to make the cache unlimited in size; or you can set the Maximum size of the cache." + newline +
                   "      If the cache size exceeds the specified limit, the oldest files cached will be removed until the cache size" + newline +
                   "      is within the limit." + newline + newline + 
                   "Compression" + newline + newline + 
                   "      You can set the compression of the JAR cache files between None and High. While you will save memory by" + newline +
                   "      specifying higher compression, performance will be degraded; best performance will be achieved" + newline +
                   "      with no compression." + newline + newline + newline + 
                   " Certificates" + newline + newline + newline +
                   "Four types of certificates may be selected:" + newline + newline +
                   "      Signed applet" + newline + 
                   "      Secure site" + newline + 
                   "      Signer CA" + newline +
                   "      Secure site CA" + newline + newline + 
                   "Signed applet" + newline + newline +
                   "      These are certificates for signed applets that are trusted by the user. The certificates that appear in the signed applet" + newline +
                   "      list are read from the certificate file jpicerts<version> located in the <user home>/.java directory." + newline + newline +
                   "Secure site" + newline + newline +
                   "      These are certificates for secure sites. The certificates that appear in the Secure site list are read from the certificate file" + newline +
                   "      jpihttpscerts<version> located in the <user home>/.java directory." + newline + newline + 
                   "Signer CA" + newline + newline +
                   "      These are certificates of Certificate Authorities (CAs) for signed applets; they are the ones who issue the certificates" + newline +
                   "       to the signers of signed applets. The certificates that appear in the Signer CA list are read from the certificate" + newline +
                   "       file cacerts, located in the <jre>/lib/security directory." + newline + newline +
                   "Secure site CA" + newline + newline +
                   "      These are certificates of Certificate Authorities (CAs) for secure sites; they are the ones who issue the certificates" + newline +
                   "      for secure sites. The certificates that appear in the Secure site CA list are read from the certificate file jssecacerts, located" + newline +
                   "      in the <jre>/lib/security directory." + newline + newline +
                   "For Signed applet and Secure site certificates, there are four options: Import, Export, Remove, and Details." + newline +
                   "The user can import, export, remove and view the details of a certificate." + newline + newline + 
                   "For Signer CA and Secure site CA, there is only one option: Details. The user can only view the details of a certificate." + newline + newline + newline +
                   " Update" + newline + newline + newline +
                   "In this panel, there is a \"Get Java Update\" button that allows users to obtain the latest update for the Java Runtime" + newline +
"Environment from the Java Update web site. This panel relates only to the Microsoft Windows platform; it does not appear in" + newline +
"other platforms (i.e. Solaris/Linux)."}
};
} 

 

