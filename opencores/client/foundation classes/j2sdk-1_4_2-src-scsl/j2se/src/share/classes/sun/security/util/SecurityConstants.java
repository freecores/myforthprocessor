/*
 * @(#)SecurityConstants.java	1.1 03/01/09
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.util;

import java.io.FilePermission;
import java.awt.AWTPermission;
import java.util.PropertyPermission;
import java.lang.RuntimePermission;
import java.net.SocketPermission;
import java.net.NetPermission;
import java.security.SecurityPermission;
import java.security.AllPermission;
import javax.security.auth.AuthPermission;

/**
 * Permission constants and string constants used to create permissions
 * used throughout the JDK.
 */
public final class SecurityConstants {
    // Cannot create one of these
    private SecurityConstants () {
    }

    // Commonly used string constants for permission actions used by
    // SecurityManager. Declare here for shortcut when checking permissions
    // in FilePermssion, SocketPermission, and PropertyPermission.
    
    public static final String FILE_DELETE_ACTION = "delete";
    public static final String FILE_EXECUTE_ACTION = "execute";
    public static final String FILE_READ_ACTION = "read";
    public static final String FILE_WRITE_ACTION = "write";

    public static final String SOCKET_RESOLVE_ACTION = "resolve";
    public static final String SOCKET_CONNECT_ACTION = "connect";
    public static final String SOCKET_LISTEN_ACTION = "listen";
    public static final String SOCKET_ACCEPT_ACTION = "accept";
    public static final String SOCKET_CONNECT_ACCEPT_ACTION = "connect,accept";

    public static final String PROPERTY_RW_ACTION = "read,write";
    public static final String PROPERTY_READ_ACTION = "read";
    public static final String PROPERTY_WRITE_ACTION = "write";

    // Permission constants used in the various checkPermission() calls in JDK.

    // java.lang.Class, java.lang.SecurityManager, java.lang.System,
    // java.net.URLConnection, java.security.AllPermission, java.security.Policy,
    // sun.security.provider.PolicyFile
    public static final AllPermission ALL_PERMISSION = new AllPermission();

    // java.lang.SecurityManager
    public static final AWTPermission TOPLEVEL_WINDOW_PERMISSION = 
        new AWTPermission("showWindowWithoutWarningBanner");

    // java.lang.SecurityManager
    public static final AWTPermission ACCESS_CLIPBOARD_PERMISSION =
        new AWTPermission("accessClipboard");

    // java.lang.SecurityManager
    public static final AWTPermission CHECK_AWT_EVENTQUEUE_PERMISSION =
        new AWTPermission("accessEventQueue");

    // java.awt.Robot
    public static final AWTPermission READ_DISPLAY_PIXELS_PERMISSION =
        new AWTPermission("readDisplayPixels");

    // java.awt.Robot
    public static final AWTPermission CREATE_ROBOT_PERMISSION = 
        new AWTPermission("createRobot");

    // java.awt.Toolkit
    public static final AWTPermission ALL_AWT_EVENTS_PERMISSION =
        new AWTPermission("listenToAllAWTEvents");

    // java.net.URL
    public static final NetPermission SPECIFY_HANDLER_PERMISSION =
       new NetPermission("specifyStreamHandler");

    // java.lang.SecurityManager, sun.applet.AppletPanel, sun.misc.Launcher
    public static final RuntimePermission CREATE_CLASSLOADER_PERMISSION =
        new RuntimePermission("createClassLoader");

    // java.lang.SecurityManager
    public static final RuntimePermission CHECK_MEMBER_ACCESS_PERMISSION =
	new RuntimePermission("accessDeclaredMembers");

    // java.lang.SecurityManager, sun.applet.AppletSecurity
    public static final RuntimePermission MODIFY_THREAD_PERMISSION = 
        new RuntimePermission("modifyThread");

    // java.lang.SecurityManager, sun.applet.AppletSecurity
    public static final RuntimePermission MODIFY_THREADGROUP_PERMISSION =
        new RuntimePermission("modifyThreadGroup");

    // java.lang.Class
    public static final RuntimePermission GET_PD_PERMISSION =
        new RuntimePermission("getProtectionDomain");

    // java.lang.Class, java.lang.ClassLoader, java.lang.Thread
    public static final RuntimePermission GET_CLASSLOADER_PERMISSION =
        new RuntimePermission("getClassLoader");

    // java.lang.Thread
    public static final RuntimePermission STOP_THREAD_PERMISSION =
       new RuntimePermission("stopThread");

    // java.security.AccessControlContext
    public static final SecurityPermission CREATE_ACC_PERMISSION =
       new SecurityPermission("createAccessControlContext");

    // java.security.AccessControlContext
    public static final SecurityPermission GET_COMBINER_PERMISSION =
       new SecurityPermission("getDomainCombiner");

    // java.security.Policy, java.security.ProtectionDomain
    public static final SecurityPermission GET_POLICY_PERMISSION =
        new SecurityPermission ("getPolicy");

    // java.lang.SecurityManager
    public static final SocketPermission LOCAL_LISTEN_PERMISSION =
	new SocketPermission("localhost:1024-", SOCKET_LISTEN_ACTION);

    // javax.security.auth.Subject
    public static final AuthPermission DO_AS_PERMISSION =
        new AuthPermission("doAs");
    
    // javax.security.auth.Subject
    public static final AuthPermission DO_AS_PRIVILEGED_PERMISSION =
        new AuthPermission("doAsPrivileged");
}
