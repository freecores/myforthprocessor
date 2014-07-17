/*
 * @(#)workerprotocol.h	1.13 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Message code for work requests from the Java VM process to the plug-in.
 */
#define JAVA_PLUGIN_SHOW_STATUS		0xF60001
#define JAVA_PLUGIN_SHOW_DOCUMENT	0xF60002
#define JAVA_PLUGIN_FIND_PROXY   	0xF60003
#define JAVA_PLUGIN_FIND_COOKIE         0xF60004
#define JAVA_PLUGIN_JAVASCRIPT_REQUEST  0xF60006
#define JAVA_PLUGIN_SET_COOKIE          0xF60009

/*
 * Message codes for replies
 */
#define JAVA_PLUGIN_OK			0xFB0001



