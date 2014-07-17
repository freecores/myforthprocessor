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
 * @(#)fileform.h	1.4 03/01/23
 */


#include "config.h"
#define MAX_ENTRY_OFFSETS 5
#define MAX_LANGS 20

typedef struct 
{
  int which;
  /* meanings of which:
  ** 0: Set output dir: 1,[new directory]
  ** 1: File to extract: 3,[overwriteflag, output filename, compressed filedata]
  **    overwriteflag: 0x1 = no overwrite. 0x0=force overwrite, 0x2=overwrite if possible
  ** 2: Execute program: 2,[complete command line,waitflag]
  ** 3: Register DLL: 1,[DLL file name]
  ** 4: Install Netscape Plug-in: 2, [DLL file name without path, compressed filedata]
  ** 5: Write Registry value: 5, [RootKey(int),KeyName,ItemName,ItemData,typelen]
  **                typelen=1 for str, 2 for dword, 3 for binary
  ** 6: Write INI String: 4, [Section, Name, Value, INI File]
  ** 7: Make Shortcut: 5, [link file, target file, parameters, icon file, icon index]
  ** 8: Delete File: 1, [filename]
  ** 9: FindWindow: 3, [whattodo,window class,string for whattodo]
  **    whattodo: 0: close window, and wait. 1: close window, and resume. 
  **    2: prompt user to close window, and display text in parm 3
  ** 10: MessageBox: 2,[MB_flags,text]
  ** 11: Delete Netscape Plug-in: 1, [dll file name without path]
  ** 12: DeleteRegValue/DeleteRegKey: 3, [root key(int), KeyName, ValueName]. ValueName is -1 if delete key
  ** 13: RMDir1 [path]
  */
  int offsets[MAX_ENTRY_OFFSETS];	// count and meaning of offsets depend on 'which'
} entry;

typedef struct 
{
  char name[64]; // '' for non-optional components
  int default_state; // bits 0-3 set for each of the different install_types, if any.
  int num_entries;
  int size_kb;
} section;


typedef struct
{
  int  _xDEADBEEF;
  char _nsisinstall[12];
  int  _x0BADF00D_or_DEADF00D;
  int length_of_header;
  int length_of_all_following_data;
} firstheader;

typedef struct
{
  TCHAR name[256];
  TCHAR text[256];
  TCHAR componenttext[256];
  TCHAR licensetext[256];
  int licensedata_offset; // data is null terminated. offset is from end of header.
  int num_langs;
  int stringdata_offsets[MAX_LANGS];
  WORD stringdata_langIDs[MAX_LANGS];

  TCHAR install_directory[MAX_PATH];
  int  install_reg_rootkey;
  TCHAR install_reg_key[200];
  TCHAR install_reg_value[200];

  TCHAR install_types[4][32]; // empty strings if not used. can describe as lite, normal, full, etc.

  int silent_install;

#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
  TCHAR uninstall_exe_name[256]; // names executable for uninstaller
  int uninstdata_offset; // -1 if no uninst data. [0] is offset to uninstaller data, [1] is offset to new icon.
#endif
  int num_sections;
} header;

typedef struct 
{
  TCHAR name[255];
  TCHAR uninstalltext[255];
  int stringdata_offsets[MAX_LANGS];
  WORD stringdata_langIDs[MAX_LANGS];
  int num_entries;
  int num_langs;
} uninstall_header;


// stored in file:
// firstheader
// header
// section[header.num_sections]
// entry[sum(section[].num_entries)]
// big_data_block


#define COMPRESS_MAX_BLOCKSIZE (512*1024)

int isheader(firstheader *h);
// returns -1 on read error
// returns -2 on header corrupt
// returns -3 on error seeking (should never occur)
// returns offset of data block in file on success
// on success, *r_head will be set to a pointer that should eventually be GlobalFree()'d.
int loadHeaders(HANDLE hFile, header **r_head, section **r_sec, entry **r_ent,uninstall_header **r_uhead, int *datablocklen);

int GetStringFromDataBlock(HANDLE hFile, int offset, char *str, int maxlen);
int GetIntFromDataBlock(HANDLE hFile, int offset);
int GetCompressedDataFromDataBlock(HANDLE hFile, int offset, HANDLE hFileOut);
int GetCompressedDataFromDataBlockToMemory(HANDLE hFile, int offset, char *out, int out_len);
int ExecuteEntry(HANDLE hFile, int offset, entry *thisentry);
