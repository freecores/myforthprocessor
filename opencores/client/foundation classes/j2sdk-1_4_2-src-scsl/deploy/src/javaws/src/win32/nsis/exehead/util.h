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
 *
 *
 * Portions Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)util.h	1.6 03/01/23
 */


#include "config.h"

void recursive_create_directory(LPTSTR directory);
int evalStr(LPTSTR s);
// returns 0 if out==in
// returns 2 if invalid symbol
// returns 3 if error looking up symbol
int process_string(LPTSTR out, LPTSTR in, LPTSTR install_dir);
int IsCurrentUserAdministratorOrPowerUser(void);
BOOL IsRunningOnNT4(void);
BOOL IsRunningOn2000XP(void);

#ifdef NSIS_CONFIG_LOG
extern char log_text[2048];
void log_write(int close);
#define log_printf(x1) wsprintf(log_text,x1); log_write(0)
#define log_printf2(x1,x2) wsprintf(log_text,x1,x2); log_write(0)
#define log_printf3(x1,x2,x3) wsprintf(log_text,x1,x2,x3); log_write(0)
#define log_printf4(x1,x2,x3,x4) wsprintf(log_text,x1,x2,x3,x4); log_write(0)
#define log_printf5(x1,x2,x3,x4,x5) wsprintf(log_text,x1,x2,x3,x4,x5); log_write(0)
#define log_printf6(x1,x2,x3,x4,x5,x6) wsprintf(log_text,x1,x2,x3,x4,x5,x6); log_write(0)
#else
#define log_printf(x1)
#define log_printf2(x1,x2)
#define log_printf3(x1,x2,x3)
#define log_printf4(x1,x2,x3,x4)
#define log_printf5(x1,x2,x3,x4,x5)
#define log_printf6(x1,x2,x3,x4,x5,x6)
#endif

extern int log_dolog;
extern HANDLE g_hInstance;
extern char g_log_file[MAX_PATH];
void ReportResourceError(UINT);

#define GETRESOURCE(tstr, id) \
do { \
    if (!LoadString(g_hInstance, id, (LPSTR)tstr, MAX_PATH)) { \
	    ReportResourceError(id); \
	} \
} while (0);

#define GETRESOURCE2(tstr, id, args) \
do { \
	TCHAR tmpmsg[MAX_PATH]; \
	GETRESOURCE(tmpmsg, id); \
	FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY, \
				  (LPCVOID)tmpmsg, (unsigned long)NULL, (unsigned long)NULL, tstr, MAX_PATH, (va_list *)args); \
} while(0);

#define PRINTMESSAGE(id) \
do { \
	TCHAR msg[MAX_PATH]; \
	GETRESOURCE(msg, id); \
	_tprintf(_T("%s\n"), msg); \
} while(0);

#define PRINTMESSAGE2(id, args) \
do { \
	TCHAR msg[MAX_PATH]; \
	GETRESOURCE2(msg, id, args); \
	_tprintf(_T("%s\n"), msg); \
} while(0);
