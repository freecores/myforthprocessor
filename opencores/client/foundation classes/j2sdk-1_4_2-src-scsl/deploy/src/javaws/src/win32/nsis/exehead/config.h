/*
 * Nullsoft "SuperPimp" Installation System 
 * version 1.0j - November 12th 2000
 *
 * Copyright (C) 1999-2000 Nullsoft, Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * Justin Frankel
 * justin@nullsoft.com
 *
 * This source distribution includes portions of zlib. see zlib/zlib.h for
 * its license and so forth. Note that this license is also borrowed from zlib.
 */

/*
 * @(#)config.h	1.4 02/02/20
 */


#ifndef NSIS_CONFIG_H
#define NSIS_CONFIG_H

// NSIS_CONFIG_LOG enables the logging facility.
// turning this off (commenting it out) saves about
// 2k.
// #define NSIS_CONFIG_LOG

// NSIS_CONFIG_UNINSTALL_SUPPORT enables the uninstaller
// support. Comment it out if your installers don't need
// uninstallers (saves a few k)
#define NSIS_CONFIG_UNINSTALL_SUPPORT

// NSIS_SUPPORT_CREATESHORTCUT enables the shortcut creation
// code. Commenting it out will save somewhere around 0.5k or so.
#define NSIS_SUPPORT_CREATESHORTCUT

// NSIS_SUPPORT_NETSCAPEPLUGINS enables netscape plug-in install
// and uninstall. Comment it out if you don't need it.
// #define NSIS_SUPPORT_NETSCAPEPLUGINS

// NSIS_SUPPORT_ACTIVEXREG enables activeX plug-in registration
// and deregistration. Comment it out if you don't need it.
#define NSIS_SUPPORT_ACTIVEXREG

// NSIS_CONFIG_WINAMPHACKCHECK enables a hack that will automatically
// check for Winamp's installation path when using InstallDir
// of $PROGRAMFILES\Winamp .. Not really needed by most people,
// (though handy for Winamp plugins), you can also use InstRegKey
// to get this functionality. I leave it on by default, cause it 
// really won't make a difference in most instances.
// #define NSIS_CONFIG_WINAMPHACKCHECK

#endif // NSIS_CONFIG_H
