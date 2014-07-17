/*
 * Nullsoft "SuperPimp" Installation System - makensis.cpp - installer compiler code
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
 * @(#)makenssi.cpp	1.11 03/01/23
 */


#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "zlib/zlib.h"
#include "lineparse.h"

#ifdef NO_IDE
#include "exehead.h"
#include "bitmap1.h"
#include "bitmap2.h"
#include "icon.h"
#include "unicon.h"
#else
#include "../exehead/Release/exehead.h"
#include "../exehead/Release/bitmap1.h"
#include "../exehead/Release/bitmap2.h"
#include "../exehead/Release/icon.h"
#include "../exehead/Release/unicon.h"
#endif

#include "../exehead/fileform.h"
#include "../exehead/config.h"
#include "resource.h"

HINSTANCE g_hInstance;
void ReportResourceError(UINT);

#define GETRESOURCE(tstr, id) \
do { \
    if (!LoadString(g_hInstance, id, tstr, MAX_PATH)) { \
	    ReportResourceError(id); \
	} \
} while (0);

#define GETRESOURCE2(tstr, id, args) \
do { \
	TCHAR tmpmsg[MAX_PATH]; \
	GETRESOURCE(tmpmsg, id); \
	FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY, \
				  (LPCVOID)tmpmsg, NULL, NULL, tstr, MAX_PATH, (va_list *)args); \
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

TCHAR g_features[MAX_PATH];

TCHAR build_out_file[MAX_PATH];

header build_header;

#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
uninstall_header build_uninst;
#endif

section *build_sections;
int build_sections_used,build_sections_alloc;
#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
int g_uninstall_size=-1;
#endif

int genUninstallData(void);

entry *build_entries;
int build_entries_used,build_entries_alloc, num_langs;
BYTE *build_datablock;
int build_datablock_used, build_datablock_alloc;

entry *ubuild_entries;
int ubuild_entries_used,ubuild_entries_alloc;
BYTE *ubuild_datablock;
int ubuild_datablock_used, ubuild_datablock_alloc;

int build_overwrite=0, build_compress=1,build_crcchk=1;

#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
int uninstall_mode;
#endif

#define SWAP(x,y,i) { i _ii; _ii=x; x=y; y=_ii; }

#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
void set_uninstall_mode(int un)
{
  if (un != uninstall_mode)
  {
    uninstall_mode=un;
    SWAP(build_entries,ubuild_entries,entry*);
    SWAP(build_entries_used,ubuild_entries_used,int);
    SWAP(build_entries_alloc,ubuild_entries_alloc,int);
    SWAP(build_datablock,ubuild_datablock,BYTE*);
    SWAP(build_datablock_used,ubuild_datablock_used,int);
    SWAP(build_datablock_alloc,ubuild_datablock_alloc,int);    
  }
}
#endif



void realloc_datablock(int v)
{
  if (build_datablock_used+v >= build_datablock_alloc)
  {
    if (build_datablock_alloc)
    {
      build_datablock_alloc+=v;
      build_datablock=(BYTE *)realloc(build_datablock,build_datablock_alloc);
    }
    else
    {
      build_datablock_alloc=32768;
      build_datablock=(BYTE *)malloc(build_datablock_alloc);
    }
  }
}

int add_data_int(int value) // returns offset
{
  realloc_datablock(sizeof(value));
  memcpy(build_datablock+build_datablock_used,&value,sizeof(value));
  build_datablock_used+=sizeof(value);
  return (build_datablock_used-sizeof(value));
}

int add_data_short(short int value) // returns offset
{
  realloc_datablock(sizeof(value));
  memcpy(build_datablock+build_datablock_used,&value,sizeof(value));
  build_datablock_used+=sizeof(value);
  return (build_datablock_used-sizeof(value));
}
int add_data_byte(BYTE value) // returns offset
{
  realloc_datablock(sizeof(value));
  memcpy(build_datablock+build_datablock_used,&value,sizeof(value));
  build_datablock_used+=sizeof(value);
  return (build_datablock_used-sizeof(value));
}

int add_data_misc(BYTE *data, int length) // returns offset
{
  realloc_datablock(length);
  memcpy(build_datablock+build_datablock_used,data,length);
  build_datablock_used+=length;
  return (build_datablock_used-length);
}

int add_data_compress(BYTE *data, int length) // returns offset
{
  int st=build_datablock_used;

  if (build_compress)
  {
    char cv[4]={_T('|'),_T('/'),_T('-'),_T('\\')};
    int cvp=0;
    BYTE *thisdata=data;
    int thislength=length;
    BYTE buffer[COMPRESS_MAX_BLOCKSIZE+12+COMPRESS_MAX_BLOCKSIZE/1000];
    z_stream stream={0,};

    if (deflateInit(&stream, 9) != Z_OK) 
    {
	  PRINTMESSAGE(JAVAWS_ERROR_DEFLATEINIT);
      exit(0);
    }
    add_data_byte(1);

    while (thislength > 0)
    {
      int thl=COMPRESS_MAX_BLOCKSIZE;
      int buffer_used=sizeof(buffer);
      printf("%c (%02d%%)\b\b\b\b\b\b\b",cv[cvp++&3],MulDiv(thisdata-data,100,length));

      stream.next_in = (Bytef*)thisdata;
      stream.avail_in = (uInt)min(thislength,thl);
      stream.next_out = (BYTE *)buffer;
      stream.avail_out = buffer_used;
      deflate(&stream, Z_SYNC_FLUSH);
      buffer_used-=stream.avail_out;
      thislength -= thl;
      thisdata+=thl;
      if (buffer_used > sizeof(buffer) || buffer_used < 0)
      {
		TCHAR message[MAX_PATH];
		GETRESOURCE(message, JAVAWS_ERROR_COMPRESS)
        _tprintf(_T("%s\n"), message);
        exit(0);
      }
      add_data_int(buffer_used);
      add_data_misc(buffer,buffer_used);
    }
    add_data_int(0);
    deflateEnd(&stream);
  }

  if (build_compress == 0 || (build_datablock_used-st-5 >= length && build_compress != 2))
  {
    build_datablock_used=st;
    add_data_byte(0);
    add_data_int(length);
    add_data_misc(data,length);
  }

  return st;
}

/* SBFIX: we need to do something with Unicode here, we
   probably need to encode it.  */
int add_data_string(LPTSTR string) // returns offset
{
  int x;
#ifdef _UNICODE
#warning UNICODE SUPPORT PROBABLY NOT FULLY WORKING OR TESTED, PROCEED WITH CAUTION
#endif
  realloc_datablock(_tcslen(string)+1);
  x=build_datablock_used;
  while (*string)
  {
    build_datablock[build_datablock_used++]=*string++;
  }
  build_datablock[build_datablock_used++]=0;
  return x;
}

/* SB: secname is INTERNAL thus leaving it as char * */
void add_section(char *secname)
{
  if (!stricmp(secname,"uninstall"))
  {
	TCHAR message[MAX_PATH];
#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
	GETRESOURCE(message, JAVAWS_UNINST_SECTION);
    _tprintf(_T("%s\n"), message);
    set_uninstall_mode(1);
#else
	GETRESOURCE(message, JAVAWS_ERROR_UNINST);
    _tprintf(_T("%s\n"), message);
    exit(1);
#endif
    return;
  }
#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
  else if (uninstall_mode)
  {
	TCHAR message[MAX_PATH];
	GETRESOURCE(message, JAVAWS_DONE_UNINST);
    _tprintf(_T("%s\n"), message);
    set_uninstall_mode(0);
  }
#endif
  if (build_sections_used >= build_sections_alloc)
  {
    if (build_sections_alloc)
    {
      build_sections_alloc*=2;
      build_sections=(section *)realloc(build_sections,build_sections_alloc*sizeof(section));
    }
    else
    {
      build_sections_alloc=8;
      build_sections=(section *)malloc(build_sections_alloc*sizeof(section));
    }
  }
  build_sections[build_sections_used].default_state=0;
  strncpy(build_sections[build_sections_used].name,secname,63);
  build_sections[build_sections_used].name[63]=0;
  build_sections[build_sections_used].num_entries=0;
  build_sections[build_sections_used].size_kb=0;

  build_header.num_sections++;
  build_sections_used++;
}


void add_entry(entry *ent)
{
  if (!build_sections_used 
#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
    && !uninstall_mode
#endif
    ) 
  {
//    printf("Initial \'Section\' command not found, creating default section!\n");
    add_section("");
  }
  if (build_entries_used >= build_entries_alloc)
  {
    if (build_entries_alloc)
    {
      build_entries_alloc*=2;
      build_entries=(entry *)realloc(build_entries,build_entries_alloc*sizeof(entry));
    }
    else
    {
      build_entries_alloc=8;
      build_entries=(entry *)malloc(build_entries_alloc*sizeof(entry));
    }
  }
  build_entries[build_entries_used]=*ent;
#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
  if (!uninstall_mode)
  {
#endif
    build_sections[build_sections_used-1].num_entries++;
#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
  }
  else
  {
    build_uninst.num_entries++;
  }
#endif

  build_entries_used++;
}

int g_mainicon_offset;
int get_data_offset(BYTE *hdr, int hdr_len, BYTE *srch, int srchlen, int startoffs);
void replace_data(BYTE *hdr,int hdr_len, BYTE *srch,int srchlen, LPTSTR filename);

typedef struct 
{
  char *name;
  int num_parms;
} tokenType;

tokenType tokenlist[] =
{
{"Name",1},
{"Icon",1},
{"UninstallIcon",1},
{"EnabledBitmap",1},
{"DisabledBitmap",1},
{"DirText",1}, // 5
{"ComponentText",1},
{"LicenseText",1}, // 7
{"UninstallText",1},
{"LicenseData",1},
{"SilentInstall",1},
{"InstType",1},
{"OutFile",1},
{"Section",1},
{"SectionIn",1},
{"InstallDir",1}, //15
{"SetOutPath",1},
{"Exec",1},
{"RegDLL",1},
{"MessageBox",2},
{"DeleteRegValue",3},
{"DeleteRegKey",2},
{"WriteRegStr",4},
{"WriteRegBin",4},
{"WriteRegDword",4},
{"WriteINIStr",4},
{"CreateShortCut",5},
{"FindWindow",3},
{"InstNSPlug",1},
{"DeleteNSPlug",1},
{"Delete",1},
{"RMDir",1},
{"SetOverwrite",1},
{"SetCompress",1},
{"File",1},
{"UninstallExeName",1},
{"CRCCheck",1},
{"InstallDirRegKey",3},
{"UnRegDLL",1},
{"ExecWait",1},
{"StringTable",2},
{"MKDir",1}, // 41
{"If",1},
{"EndIf",0},
{"RebootBox",0},// 44
{"Browser",1},
{"AltDirBox",3},
{"RMDirRecursive",1},
};

int parseScript(FILE *fp)
{
  int linecnt=0;
  LineParser line;
  TCHAR message[MAX_PATH];
  
  /*  SB:  All token names, no need to internationalize */
  char tliststr[1024]="";
  int x;
  int pos=0;
  for (x = 0; x < sizeof(tokenlist)/sizeof(tokenlist[0]); x ++)
  {
    strcpy(tliststr+pos,tokenlist[x].name);
    pos+=strlen(tokenlist[x].name)+1;
    tliststr[pos]=0;
  }

  GETRESOURCE(message, JAVAWS_PROCESSING_SCRIPT);
  _tprintf(_T("%s\n"), message);
  _tprintf(_T("-----------------------------------------\n"));
  while (1)
  {
    linecnt++;

    {
      TCHAR str[4096];
      str[0]=_T('\0');
      _fgetts(str,4095,fp);
      if (feof(fp)&&!str[0]) break;
      line.parse(str);
    }
    if (line.getnumtokens() < 1) continue;

    int which_token=line.gettoken_enum(0,tliststr);
    if (which_token == -1)
    {
		INT32 args[] = { (INT32)(line.gettoken_str(0)), linecnt };
		PRINTMESSAGE2(JAVAWS_ERROR_INVALIDCOMMAND, args);
		return 1;
    }

    {
      int v=line.getnumtokens()-(tokenlist[which_token].num_parms+1);
      if (v!=0)
      {
		INT32 args[] = { (INT32)(tokenlist[which_token].name),
						  tokenlist[which_token].num_parms,
						  linecnt };
		PRINTMESSAGE2(JAVAWS_ERROR_TOKEN, args);
        return 1;
      }
    }
    
    entry ent={0,};
    switch (which_token)
    {
      case 0:
        strncpy(build_header.name,line.gettoken_str(1),127);
		{
			INT32 args[] = { (INT32)(build_header.name) };
			PRINTMESSAGE2(JAVAWS_PARSE_NAME, args);
		}
      break;
      case 1:
		  {
			  INT32 args[] = { (INT32)(line.gettoken_str(1)) };
			  PRINTMESSAGE2(JAVAWS_PARSE_ICON, args);
		  }
        replace_data((BYTE*)header_data,sizeof(header_data),(BYTE*)icon_data,sizeof(icon_data),line.gettoken_str(1));
      break;
      case 2:
#ifndef NSIS_CONFIG_UNINSTALL_SUPPORT
        return 1;
#else
		{
			INT32 args[] = { (INT32)(line.gettoken_str(1)) };
			PRINTMESSAGE2(JAVAWS_PARSE_UNINSTALLICON, args);
		}
        replace_data((BYTE*)unicon_data,sizeof(unicon_data),(BYTE*)unicon_data,sizeof(unicon_data),line.gettoken_str(1));
      break;
#endif
      case 3:
		{
			INT32 args[] = { (INT32)(line.gettoken_str(1)) };
			PRINTMESSAGE2(JAVAWS_PARSE_ENABLEDBITMAP, args);
		}
        replace_data((BYTE*)header_data,sizeof(header_data),(BYTE*)bitmap1_data,sizeof(bitmap1_data),line.gettoken_str(1));
      break;
      case 4:
		{
			INT32 args[] = { (INT32)(line.gettoken_str(1)) };
			PRINTMESSAGE2(JAVAWS_PARSE_DISABLEDBITMAP, args);
		}
        replace_data((BYTE*)header_data,sizeof(header_data),(BYTE*)bitmap2_data,sizeof(bitmap2_data),line.gettoken_str(1));
      break;
      case 5:
        _tcsncpy(build_header.text,line.gettoken_str(1),127);
		{
			INT32 args[] = { (INT32)(build_header.text) };
			PRINTMESSAGE2(JAVAWS_PARSE_DIRTEXT, args);
		}
      break;
      case 6:
        _tcsncpy(build_header.componenttext,line.gettoken_str(1),127);
		{
			INT32 args[] = { (INT32)(build_header.componenttext) };
			PRINTMESSAGE2(JAVAWS_PARSE_COMPONENTTEXT, args);
		}
      break;

/*  D'oh.  Just realized this stuff will never be seen by a user, we don't */
/*  have to internationalize any "installer builder" text.  Sigh.          */

      case 7:
        _tcsncpy(build_header.licensetext,line.gettoken_str(1),127);
        printf("LicenseText: \"%s\"\n",build_header.licensetext);
      break;
      case 8:
#ifndef NSIS_CONFIG_UNINSTALL_SUPPORT
        printf("UninstallText: installer not built with NSIS_CONFIG_UNINSTALL_SUPPORT\n");
        return 1;
#else
        strncpy(build_uninst.uninstalltext,line.gettoken_str(1),127);
        printf("UninstallText: \"%s\"\n",build_uninst.uninstalltext);
      break;
#endif
      case 9:
        //if (!build_header.silent_install)
        {
          BYTE data[32768];
          FILE *fp;
          int datalen;
          fp=_tfopen(line.gettoken_str(1),"rb");
          if (!fp)
          {
            printf("LicenseData: Error opening \"%s\" on line %d\n",line.gettoken_str(1),linecnt);
            return 1;
          }
          datalen=fread(data,1,32768,fp);
          if (!feof(fp))
          {
            printf("LicenseData: Warning: license data truncated to 32k.\n");
          }
          printf("LicenseData: compressing\r");
          build_header.licensedata_offset=add_data_compress(data,datalen);
          fclose(fp);
          printf("LicenseData: \"%s\"      \n",line.gettoken_str(1));
        }
        //else
        //  printf("LicenseData: Warning: SilentInstall enabled. Ignoring license data!\n");
      break;
      case 10:
        build_header.silent_install=line.gettoken_enum(1,"normal\0silent\0silentlog\0");
        if (build_header.silent_install<0)
        {
          printf("SilentInstall: Error invalid install type \"%s\" on line %d\n"
                 "               valid types are \"normal\", \"silent\", and \"silentlog\".\n",
                      line.gettoken_str(1),linecnt);
          return 1;
        }
        printf("SilentInstall: %s\n",line.gettoken_str(1));
      break;
      case 11:
        {
          int x;
          for (x = 0; x < 4; x ++)
            if (!build_header.install_types[x][0]) break;
          if (x==4)
          {
            printf("InstType: Error: no more than 4 install types allowed. 5th specified on line %d\n",linecnt);
            return 1;
          }
          else 
          {
            _tcsncpy(build_header.install_types[x],line.gettoken_str(1),31);
            build_header.install_types[x][31]=0;
            printf("InstallType: %d=\"%s\"\n",x+1,build_header.install_types[x]);
          }
        }
      break;
      case 12:
        _tcsncpy(build_out_file,line.gettoken_str(1),MAX_PATH-1);
        printf("OutFile: \"%s\"\n",build_out_file);
      break;
      case 13:
        //if (!build_header.silent_install)
        {
          add_section(line.gettoken_str(1));
          printf("Section: \"%s\"\n",line.gettoken_str(1));
        }
        //else 
        //  printf("Section: Warning: silent install, ignoring all section descs!\n");
      break;
      case 14:
        {
          build_sections[build_sections_used-1].default_state=0;
          printf("SectionIn \"%s\":",build_sections[build_sections_used-1].name);
          char *p=line.gettoken_str(1);
          while (*p)
          {
            int x=*p-'1';
            if (x >= 0 && x <= 3)
            {
              build_sections[build_sections_used-1].default_state|=(1<<x);
              printf(" [%s]",build_header.install_types[x]);
            }
            p++;
          }
          printf("\n");
        }
      break;
      case 15:
#ifdef NSIS_CONFIG_WINAMPHACKCHECK
        if (!stricmp(line.gettoken_str(1),"$PROGRAMFILES\\Winamp"))
        {
          printf("InstallDir: WARNING, detected $PROGRAMFILES\\Winamp, and\n"
                 "NSIS_CONFIG_WINAMPHACKCHECK was not enabled! Use InstallRegKey\n"
                 "instead!\n");
        }
#endif
        _tcsncpy(build_header.install_directory,line.gettoken_str(1),MAX_PATH-1);
        printf("InstallDir: \"%s\"\n",build_header.install_directory);
      break;
      case 16:
        {
          TCHAR out_path[MAX_PATH];
          LPTSTR p=line.gettoken_str(1);
          if (*p == _T('-')) out_path[0]=_T('\0');
          else
          {
			LPTSTR ptr = NULL;
            while (*p == '\\') p=CharNext(p);
            _tcsncpy(out_path,p,MAX_PATH-1);
			ptr = CharPrev(out_path, &(out_path[_tcslen(out_path)]));
			if (*ptr == _T('\\')) _tcscat(out_path, _T("\\"));
//            if (out_path[_tcslen(out_path)-1]!='\\') _tcscat(out_path, _T("\\"));
          }
          if (!*out_path) _tcscpy(out_path,_T("$INSTDIR"));
          printf("SetOutPath: \"%s\"\n",out_path);
          ent.which=0;
          ent.offsets[0]=add_data_string(out_path);
          add_entry(&ent);
        }
      break;
      case 39: // ExecWait
      case 17:
        ent.which=2;
        ent.offsets[1]=0;
        if (which_token == 39)
          ent.offsets[1]=1;
        else ent.offsets[1]=0;

        ent.offsets[0]=add_data_string(line.gettoken_str(1));
        add_entry(&ent);
        printf("%s: \"%s\"\n",ent.offsets[1]?"ExecWait":"Exec",line.gettoken_str(1));
      break;
      case 38: // 38 is UnRegDLL
      case 18:
#ifndef NSIS_SUPPORT_ACTIVEXREG
        printf("RegDLL/UnRegDLL: support for RegDLL not compiled in (NSIS_SUPPORT_ACTIVEXREG)\n");
        return 1;
#else
        {
          LPTSTR rstr="RegDLL";
          ent.which=3;
          if (which_token == 38)
          {
            TCHAR buf[1024];
            wsprintf(buf,"- %s",line.gettoken_str(1));
            ent.offsets[0]=add_data_string(buf);
            rstr="UnRegDLL";
          }
          else ent.offsets[0]=add_data_string(line.gettoken_str(1));
          printf("%s: \"%s\"\n",rstr,line.gettoken_str(1));
          add_entry(&ent);
        }
      break;
#endif
      case 19:
        {
		  #define MBD(x) {x,#x},
		  LPTSTR p = NULL;
          int r=0;
          struct 
          {
            int id;
            LPTSTR str;
          } list[]=
          {
            MBD(MB_ABORTRETRYIGNORE)
            MBD(MB_OK)
            MBD(MB_OKCANCEL)
            MBD(MB_RETRYCANCEL)
            MBD(MB_YESNO)
            MBD(MB_YESNOCANCEL)
            MBD(MB_ICONEXCLAMATION)
            MBD(MB_ICONINFORMATION)
            MBD(MB_ICONQUESTION)
            MBD(MB_ICONSTOP)
            MBD(MB_TOPMOST)
            MBD(MB_SETFOREGROUND)
            MBD(MB_RIGHT)
          };
          #undef MBD
          int x;
          p=line.gettoken_str(1);

          while (*p)
          {
            LPTSTR np=p;
            while (*np && *np != _T('|')) np=CharNext(np);
            if (*np) { *np=_T('\0'); np=CharNext(np); }
            for (x  =0 ; x < sizeof(list)/sizeof(list[0]); x ++)
              if (!_tcscmp(list[x].str,p)) break;
            if (x < sizeof(list)/sizeof(list[0]))
            {
              r|=list[x].id;
            }
            else
            {
              printf("MessageBox: Error: unknown flag \"%s\" on line %d\n",p,linecnt);
              return 1;
            }            
            p=np;
          }
          ent.which=10;
          ent.offsets[0]=r;
          ent.offsets[1]=add_data_string(line.gettoken_str(2));
          add_entry(&ent);
          printf("MessageBox: %d: \"%s\"\n",r,line.gettoken_str(2));
        } 
      break;
      case 20: // value
      case 21: // key
        {
          int k=line.gettoken_enum(1,"HKEY_CLASSES_ROOT\0HKEY_LOCAL_MACHINE\0HKEY_CURRENT_USER\0HKEY_USERS\0HKEY_CURRENT_CONFIG\0");
          HKEY tab[]={HKEY_CLASSES_ROOT,HKEY_LOCAL_MACHINE,HKEY_CURRENT_USER,HKEY_USERS,HKEY_CURRENT_CONFIG};
          if (k == -1)
          {
            printf("DeleteReg: Error: unknown key name \"%s\" on line %d\n",line.gettoken_str(1),linecnt);
            return 1;
          }
          ent.which=12;
          ent.offsets[0]=(int)tab[k];      
          ent.offsets[1]=add_data_string(line.gettoken_str(2));
          ent.offsets[2]=(which_token==21)?
            -1:add_data_string(line.gettoken_str(3));
          add_entry(&ent);
          if (which_token==21)
            printf("DeleteRegKey: %s\\%s\n",line.gettoken_str(1),line.gettoken_str(2));
          else
            printf("DeleteRegValue: %s\\%s\\%s\n",line.gettoken_str(1),line.gettoken_str(2),line.gettoken_str(3));
        }
      break;
      case 22: // str
      case 23: // bin
      case 24: // dword
        {
          int k=line.gettoken_enum(1,"HKEY_CLASSES_ROOT\0HKEY_LOCAL_MACHINE\0HKEY_CURRENT_USER\0HKEY_USERS\0HKEY_CURRENT_CONFIG\0");
          HKEY tab[]={HKEY_CLASSES_ROOT,HKEY_LOCAL_MACHINE,HKEY_CURRENT_USER,HKEY_USERS,HKEY_CURRENT_CONFIG};
          if (k == -1)
          {
            printf("WriteReg: Error: unknown key name \"%s\" on line %d\n",line.gettoken_str(1),linecnt);
            return 1;
          }
          ent.which=5;
          ent.offsets[0]=(int)tab[k];
          ent.offsets[1]=add_data_string(line.gettoken_str(2));
          ent.offsets[2]=add_data_string(line.gettoken_str(3));
          if (which_token == 22)
          {
            printf("WriteRegStr: %s\\%s\\%s=%s\n",
              line.gettoken_str(1),line.gettoken_str(2),line.gettoken_str(3),line.gettoken_str(4));
            ent.offsets[3]=add_data_string(line.gettoken_str(4));
            ent.offsets[4]=1;
          }
          if (which_token == 23) 
          {
            BYTE data[512];
            LPTSTR p=line.gettoken_str(4);
            int data_len=0;
            while (*p)
            {
              int c;
              int a,b;
              a=*p;
              if (a >= '0' && a <= '9') a-='0';
              else if (a >= 'a' && a <= 'a') a-='a'-10;
              else if (a >= 'A' && a <= 'F') a-='A'-10;
              else break;
              b=*(p=CharNext(p));
              if (b >= '0' && b <= '9') b-='0';
              else if (b >= 'a' && b <= 'a') b-='a'-10;
              else if (b >= 'A' && b <= 'F') b-='A'-10;
              else break;
              p=CharNext(p);
              c=(a<<4)|b;
              if (data_len >= 512) 
              {
                printf("WriteRegBin: Error: max 512 bytes exceeded on line %d\n",linecnt);
                return 1;
              }
              data[data_len++]=c;
            }
            if (*p)
            {
              printf("WriteRegBin: Error: unknown character (%d) on line %d\n",(int)*p,linecnt);
              return 1;
            }
            printf("WriteRegBin: %s\\%s\\%s=%s\n",
              line.gettoken_str(1),line.gettoken_str(2),line.gettoken_str(3),line.gettoken_str(4));
            ent.offsets[3]=add_data_compress(data,data_len);
            ent.offsets[4]=3;
          }
          if (which_token == 24) 
          {
            int s;
            ent.offsets[3]=line.gettoken_int(4,&s);
            if (!s)
            {
              printf("WriteRegDword: Error: couldn't interpret integer on line %d\n",linecnt);
              return 1;
            }
            ent.offsets[4]=2;
            printf("WriteRegDword: %s\\%s\\%s=%d\n",
              line.gettoken_str(1),line.gettoken_str(2),line.gettoken_str(3),line.gettoken_int(4));
          }
          add_entry(&ent);
        }
      break;
      case 25:
        ent.which=6;
        ent.offsets[0]=add_data_string(line.gettoken_str(2));
        ent.offsets[1]=add_data_string(line.gettoken_str(3));
        ent.offsets[2]=add_data_string(line.gettoken_str(4));
        ent.offsets[3]=add_data_string(line.gettoken_str(1));
        add_entry(&ent);
        printf("WriteINIStr: [%s] %s=%s in %s\n",
          line.gettoken_str(2),line.gettoken_str(3),line.gettoken_str(4),line.gettoken_str(1));
      break;
      case 26:
#ifndef NSIS_SUPPORT_CREATESHORTCUT
        printf("CreateShortCut: support for CreateShortCut not compiled in (NSIS_SUPPORT_CREATESHORTCUT)\n");
        return 1;
#else
        {
          ent.which=7;
          ent.offsets[0]=add_data_string(line.gettoken_str(1));
          ent.offsets[1]=add_data_string(line.gettoken_str(2));
          ent.offsets[2]=add_data_string(line.gettoken_str(3));
          ent.offsets[3]=add_data_string(line.gettoken_str(4));
          int s;
          ent.offsets[4]=line.gettoken_int(5,&s);
          if (!s)
          {
            printf("CreateShortCut: Error: cannot interpret icon index on line %d\n",linecnt);
            return 1;
          }
          printf("CreateShortCut: \"%s\"->\"%s\" %s  icon:%s,%d \n",
            line.gettoken_str(1),line.gettoken_str(2),line.gettoken_str(3),
            line.gettoken_str(4),ent.offsets[4]);
          add_entry(&ent);
        }
      break;
#endif
      case 27:
        ent.which=9;
        ent.offsets[0]=line.gettoken_enum(1, _T("close\0closeinstant\0prompt\0"));
        if (ent.offsets[0] < 0)
        {
          printf("FindWindow: Error: unknown mode \"%s\" on line %d\n"
                 "            known modes are \"close\", \"closeinstant\", and \"prompt\".\n",
                 line.gettoken_str(1),linecnt);
          return 1;
        }
        ent.offsets[1]=add_data_string(line.gettoken_str(2));
        ent.offsets[2]=add_data_string(line.gettoken_str(3));
        add_entry(&ent);
        printf("FindWindow: mode=%s, class=\"%s\", text=\"%s\"\n",
          line.gettoken_str(1),line.gettoken_str(2),line.gettoken_str(3));
      break;
      case 28:
#ifndef NSIS_SUPPORT_NETSCAPEPLUGINS
        printf("InstNSPlug: support for InstNSPlug not compiled in (NSIS_SUPPORT_NETSCAPEPLUGINS)\n");
        return 1;
#else
#warning NSIS_SUPPORT_NETSCAPEPLUGINS defined, no internationalization support is provided for this feature!!
        {
          char dir[MAX_PATH];
          char newfn[MAX_PATH], *s;
          HANDLE h;
          WIN32_FIND_DATA d;
          strcpy(dir,line.gettoken_str(1));
          s=dir+strlen(dir);
          while (s > dir && *s != '\\') s--;
          *s=0;

          h = FindFirstFile(line.gettoken_str(1),&d);
          if (h != INVALID_HANDLE_VALUE)
          {
            do {
              FILE *fp;
              if (d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
              ent.which=4;
              sprintf(newfn,"%s%s%s",dir,dir[0]?"\\":"",d.cFileName);
              fp=_tfopen(newfn,"rb");
              if (fp)
              {
                char *data;
                int len;
                fseek(fp,0,SEEK_END);
                len=ftell(fp);
                build_sections[build_sections_used-1].size_kb+=(len+1023)/1024;
                fseek(fp,0,SEEK_SET);
                data=(char*)malloc(len);
                fread(data,len,1,fp);
                fclose(fp);
                ent.offsets[0]=add_data_string(d.cFileName);
                printf("InstNSPlug: compressing \"%s\"...\r",d.cFileName);
                ent.offsets[1]=add_data_compress(data,len);
                add_entry(&ent);
                printf("InstNSPlug: \"%s\" (%d/%d bytes)    \n",
                  d.cFileName,
                  build_datablock_used-ent.offsets[2],
                  len);
                free(data);
              }
              else 
              {
                printf("InstNSPlug: failed opening file \"%s\" on line %d!\n",newfn,linecnt);
                return 1;
              }
            } while (FindNextFile(h,&d));
            FindClose(h);
          }
          else
          {
            printf("InstNSPlug: failed finding file \"%s\" on line %d!\n",line.gettoken_str(1),linecnt);
            return 1;
          }
        }
      break;
#endif
      case 29:
#ifndef NSIS_SUPPORT_NETSCAPEPLUGINS
        printf("InstNSPlug: support for InstNSPlug not compiled in (NSIS_SUPPORT_NETSCAPEPLUGINS)\n");
        return 1;
#else
        ent.which=11;
        ent.offsets[0]=add_data_string(line.gettoken_str(1));
        add_entry(&ent);
        printf("DeleteNSPlug: \"%s\"\n",line.gettoken_str(1));
      break;
#endif
      case 30:
        ent.which=8;
        ent.offsets[0]=add_data_string(line.gettoken_str(1));
        add_entry(&ent);
        printf("Delete: \"%s\"\n",line.gettoken_str(1));
      break;
      case 31:
        ent.which=13;
        ent.offsets[0]=add_data_string(line.gettoken_str(1));
        add_entry(&ent);
        printf("RMDir: \"%s\"\n",line.gettoken_str(1));
      break;
      case 32:
        build_overwrite=line.gettoken_enum(1, _T("on\0off\0try\0"));
        if (build_overwrite==-1)
        {
          printf("SetOverwrite: Error: unknown mode \"%s\" on line %d.\n"
                 "              known modes are \"on\", \"off\", and \"try\"\n",line.gettoken_str(1),linecnt);
          return 1;
        }
        printf("SetOverwrite: %s\n",line.gettoken_str(1));
      break;
      case 33:
        build_compress=line.gettoken_enum(1, _T("off\0auto\0force\0"));
        if (build_compress==-1)
        {
          printf("SetCompress: Error: unknown mode \"%s\" on line %d.\n"
                 "              known modes are \"off\", \"auto\", and \"force\"\n",line.gettoken_str(1),linecnt);
          return 1;
        }
        printf("SetCompress: %s\n",line.gettoken_str(1));
      break;
      case 34:
        {
          TCHAR dir[MAX_PATH];
          TCHAR newfn[MAX_PATH];
		  LPTSTR s;
          HANDLE h;
          WIN32_FIND_DATA d;
          _tcscpy(dir,line.gettoken_str(1));
          s=dir+_tcslen(dir);
          while (s > dir && *s != _T('\\')) s=CharPrev(dir, s);
          *s=_T('\0');

          h = FindFirstFile(line.gettoken_str(1),&d);
          if (h != INVALID_HANDLE_VALUE)
          {
            do {
              FILE *fp;
              if (d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
              ent.which=1;
              _stprintf(newfn, _T("%s%s%s"),dir,dir[0]?_T("\\"):_T(""),d.cFileName);
              fp=_tfopen(newfn,"rb");
              if (fp)
              {
                BYTE *data;
                int len;
                fseek(fp,0,SEEK_END);
                len=ftell(fp);
                build_sections[build_sections_used-1].size_kb+=(len+1023)/1024;
                fseek(fp,0,SEEK_SET);
                data=(BYTE*)malloc(len);
                fread(data,len,1,fp);
                fclose(fp);
                ent.offsets[0]=build_overwrite;
                ent.offsets[1]=add_data_string(d.cFileName);
                printf("File: compressing \"%s\"...",d.cFileName);
                ent.offsets[2]=add_data_compress(data,len);
                add_entry(&ent);
                printf("\rFile: added \"%s\" (%d/%d bytes)\n",
                  d.cFileName,
                  build_datablock_used-ent.offsets[2],
                  len);
                free(data);
              }
              else 
              {
                printf("File: Error: failed opening file \"%s\" on line %d!\n",newfn,linecnt);
                return 1;
              }
            } while (FindNextFile(h,&d));
            FindClose(h);
          }
          else
          {
            printf("File: Error: failed finding file \"%s\" on line %d!\n",line.gettoken_str(1),linecnt);
            return 1;
          }
        }
      break;
      case 35:        
#ifndef NSIS_CONFIG_UNINSTALL_SUPPORT
        printf("UninstallExeName: installer not built with NSIS_CONFIG_UNINSTALL_SUPPORT\n");
        return 1;
#else
        _tcsncpy(build_header.uninstall_exe_name,line.gettoken_str(1),31);
        build_header.uninstall_exe_name[31]=_T('0');
        printf("UninstallExeName: \"%s\"\n",build_header.uninstall_exe_name);       
      break;
#endif
      case 36:
        build_crcchk=line.gettoken_enum(1,_T("off\0on\0"));
        header_data[get_data_offset((BYTE *)header_data,sizeof(header_data),(BYTE*)"DONOCHK=",8,0)+8]=build_crcchk?0:1;
        if (build_crcchk==-1)
        {
          printf("CRCCheck: Error: unknown mode \"%s\" on line %d.\n"
                 "              known modes are \"on\" and \"off\"\n",line.gettoken_str(1),linecnt);
          return 1;
        }
        printf("CRCCheck: %s\n",line.gettoken_str(1));
      break;
      case 37: // InstallDirRegKey
        {
          int k=line.gettoken_enum(1,_T("HKEY_CLASSES_ROOT\0HKEY_LOCAL_MACHINE\0HKEY_CURRENT_USER\0HKEY_USERS\0HKEY_CURRENT_CONFIG\0"));
          if (k == -1)
          {
            printf("InstallDirRegKey: Error: unknown key name \"%s\" on line %d\n",line.gettoken_str(1),linecnt);
            return 1;
          }
          HKEY tab[]={HKEY_CLASSES_ROOT,HKEY_LOCAL_MACHINE,HKEY_CURRENT_USER,HKEY_USERS,HKEY_CURRENT_CONFIG};
          build_header.install_reg_rootkey=(int)tab[k];
          _tcsncpy(build_header.install_reg_key,line.gettoken_str(2),199);
          _tcsncpy(build_header.install_reg_value,line.gettoken_str(3),199);
          build_header.install_reg_key[199]=_T('\0');
          build_header.install_reg_value[199]=_T('\0');
          printf("InstallRegKey: \"%s\\%s\\%s\"\n",line.gettoken_str(1),build_header.install_reg_key,build_header.install_reg_value);
        }
      break;
      // 38 is UnRegDLL (see above)
      // 39 is ExecWait (see above)
	  case 40: // StringTable
		{
          BYTE data[32768];
          FILE *fp;
          int datalen;
		  int *po, *pn;
		  WORD *pl;
		  int langID=line.gettoken_hex(2);
          fp=_tfopen(line.gettoken_str(1),"rb");
          if (!fp)
          {
            printf("StringTable: Error opening \"%s\" on line %d\n",line.gettoken_str(1),linecnt);
            return 1;
          }
          datalen=fread(data,1,32768,fp);
          if (!feof(fp))
          {
            printf("StringTable: Warning: string data truncated to 32k.\n");
          }
          printf("StringTable: compressing\r");
		  if (uninstall_mode) {
			pn=&build_uninst.num_langs;
			po=&build_uninst.stringdata_offsets[*pn];
			pl=&build_uninst.stringdata_langIDs[(*pn)++];
			*po=add_data_compress(data,datalen);
			*pl=(WORD)langID;
		  } else {
			pn=&build_header.num_langs;
			po=&build_header.stringdata_offsets[*pn];
			pl=&build_header.stringdata_langIDs[(*pn)++];
			*po=add_data_compress(data,datalen);
			*pl=(WORD)langID;
		  }
          fclose(fp);
          printf("StringTable: \"%s\" \"%d\" (0x%x) \n",line.gettoken_str(1), (WORD)langID, (WORD)langID);
		}
	  break;
	  case 41: // MKDir
        ent.which=41;
        ent.offsets[0]=add_data_string(line.gettoken_str(1));
        add_entry(&ent);
        printf("MKDir: \"%s\"\n",line.gettoken_str(1));
      break;
	  case 42: //If
		ent.which=14;
		ent.offsets[0]=add_data_string(line.gettoken_str(1));
        add_entry(&ent);
        printf("If: \"%s\"\n",line.gettoken_str(1));
	  break;
	  case 43: //EndIf
		ent.which=15;
        add_entry(&ent);
        printf("EndIf\n");
	  break;
 	  case 44: //RebootBox
		ent.which=16;
        add_entry(&ent);
        printf("RebootBox\n");
	  break;
 	  case 45: //Browser
		ent.which=17;
		ent.offsets[0]=add_data_string(line.gettoken_str(1));
        add_entry(&ent);
        printf("Browser: \"%s\"\n",line.gettoken_str(1));
	  break;
 	  case 46: //AltDirBox
		ent.which=18;
		ent.offsets[0]=add_data_string(line.gettoken_str(1));
		ent.offsets[1]=add_data_string(line.gettoken_str(2));
		ent.offsets[2]=add_data_string(line.gettoken_str(3));
        add_entry(&ent);
        printf("AltDirBox: \"%s\"  \"%s\"\n",line.gettoken_str(1), line.gettoken_str(2));
	  break;
    case 47: //RmDirRecursive
        ent.which=19;
        ent.offsets[0]=add_data_string(line.gettoken_str(1));
        add_entry(&ent);
        printf("RMDirRecursive: \"%s\"\n",line.gettoken_str(1));
      break;
   }
  }
  return 0;
}

int writeOutput(FILE *fp)
{
  int crc=adler32(0L, Z_NULL, 0);

  firstheader fh;
  _tcscpy(fh._nsisinstall,"nsisinstall");
  fh._x0BADF00D_or_DEADF00D=0x0BADF00D;
  fh._xDEADBEEF=0xDEADBEEF;
  fh.length_of_header=sizeof(header)+sizeof(section)*build_sections_used+sizeof(entry)*build_entries_used;
  fh.length_of_all_following_data=fh.length_of_header+build_datablock_used;

  printf("\n/-----------------------------------------\\\n");
  fwrite(header_data,sizeof(header_data),1,fp);
  crc=adler32(crc,header_data,sizeof(header_data));
  int hdrsize=ftell(fp);
  printf("| Header Size          %10d bytes   |\n",hdrsize);

  fwrite(&fh,sizeof(fh),1,fp);
  crc=adler32(crc,(BYTE*)&fh,sizeof(fh));

  fwrite(&build_header,sizeof(build_header),1,fp);
  crc=adler32(crc,(BYTE*)&build_header,sizeof(build_header));

  fwrite(build_sections,sizeof(section),build_sections_used,fp);
  crc=adler32(crc,(BYTE*)build_sections,sizeof(section)*build_sections_used);
  fwrite(build_entries,sizeof(entry),build_entries_used,fp);
  crc=adler32(crc,(BYTE*)build_entries,sizeof(entry)*build_entries_used);
  printf("| Install Info Size    %10d bytes   |\n",ftell(fp)-hdrsize);
  hdrsize=ftell(fp);

  fwrite(build_datablock,build_datablock_used,1,fp);

  crc=adler32(crc,(BYTE *)build_datablock,build_datablock_used);
  printf("| Data Size            %10d bytes   |\n",ftell(fp)-hdrsize);
#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
  if (g_uninstall_size>=0) printf("| Uninstall Data Size  %10d bytes   |\n",g_uninstall_size);
#endif
  if (build_crcchk) fwrite(&crc,sizeof(int),1,fp);
  printf("|-----------------------------------------|\n");
  printf("| Total Installer Size %10d bytes   |\n",ftell(fp));
  printf("\\-----------------------------------------/\n");
  return 0;
}

int main(int argc, char **argv)
{
  FILE *fp;

  _tcscat(g_features, _T("")
#ifdef NSIS_CONFIG_LOG
         _T("\n    NSIS_CONFIG_LOG")
#endif
#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
         _T("\n    NSIS_CONFIG_UNINSTALL_SUPPORT")
#endif
#ifdef NSIS_CONFIG_WINAMPHACKCHECK
         _T("\n    NSIS_CONFIG_WINAMPHACKCHECK")
#endif
#ifdef NSIS_SUPPORT_NETSCAPEPLUGINS
         _T("\n    NSIS_SUPPORT_NETSCAPEPLUGINS")
#endif
#ifdef NSIS_SUPPORT_ACTIVEXREG
         _T("\n    NSIS_SUPPORT_ACTIVEXREG")
#endif
#ifdef NSIS_SUPPORT_CREATESHORTCUT
         _T("\n    NSIS_SUPPORT_CREATESHORTCUT")
#endif
		 );

  g_hInstance=GetModuleHandle(NULL);

  printf("MakeNSIS v1.0j - Copyright 1999-2000 Nullsoft, Inc.\n"
         "\n"
         "NSIS Copyright (C) 2000 Nullsoft, Inc.\n"
         "\n");
  if (argc != 2)
  {
    printf(
         "  Compiled-in features (header size of %d bytes):%s\n"
         ,sizeof(header_data),g_features[0]?g_features:" (none)"
         );
    printf("\nUsage: makensis script.nsi\n");
    return 1;
  }
  
  memset((void *)(build_header.stringdata_offsets), -1, MAX_LANGS * sizeof(int));
  
  g_mainicon_offset=get_data_offset((BYTE *)header_data,sizeof(header_data),(BYTE*)icon_data,sizeof(icon_data),32);
  build_header.num_langs=0;
  build_uninst.num_langs=0;
#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
  build_header.uninstdata_offset=-1;
  _tcscpy(build_header.uninstall_exe_name,"nsuninst.exe");
#endif
  fp=_tfopen(argv[1],"rt");
  if (!fp)
  {
    TCHAR s[MAX_PATH];
    _stprintf(s,_T("%s.nsi"),argv[1]);
    fp=_tfopen(s,"rt");
    if (!fp)
    {
      perror("Can't open script");
      return 1;
    }
  }
  if (parseScript(fp))
  {
    printf("Error in script -- aborting creation process\n");
    fclose(fp);
    return 1;
  }
  fclose(fp);

  fp = _tfopen(build_out_file,"wb");
  if (!fp) 
  {
    perror("Can't open output file");
    return 1;
  }

#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
  set_uninstall_mode(0);
  if (ubuild_entries_used) 
  {
    build_header.uninstdata_offset=genUninstallData();
  }
#endif

  if (writeOutput(fp))
  { 
    printf("Error writing output - aborting creation process\n");
    fclose(fp);
    return 1;
  }
//  printf("stats:\n"
//         "  sections: %d\n"
//         "  entries: %d\n",build_sections_used,build_entries_used);
  fclose(fp);
  printf("\nsucceeded\n"); 
  return 0; 
}


int get_data_offset(BYTE *hdr, int hdr_len, BYTE *srch, int srchlen, int startoffs)
{
  int offs=startoffs;
  hdr_len-=srchlen;
  while (hdr_len>=0)
  {
    if (!memcmp(hdr+startoffs,srch+startoffs,srchlen-startoffs)) break;
    offs++;
    hdr++;
    hdr_len--;
  }
  if (hdr_len<0)
  {
    printf("get_data_offset: error searching data! ABORT!\n");
    exit(1);
  }
  return offs;
}

void replace_data(BYTE *hdr,int hdr_len, BYTE *srch,int srchlen, LPTSTR filename)
{
  BYTE temp[32];
  FILE *fp=_tfopen(filename,"rb");
  if (!fp)
  {
    printf("replace_data: WARNING: error opening file \"%s\" -- failing!\n",filename);
    return;
  }
  fseek(fp,0,SEEK_END);
  if (ftell(fp) != srchlen)
  {
    printf("replace_data: WARNING: length of file (%d) != destination length (%d) -- failing!\n",ftell(fp),srchlen);
    fclose(fp);
    return;
  }
  hdr_len-=srchlen-32;
  while (hdr_len>=0)
  {
    if (!memcmp(hdr,srch+32,srchlen-32)) break;
    hdr++;
    hdr_len--;
  }
  if (hdr_len<0)
  {
    printf("replace_data: WARNING: error searching data -- failing!\n");
    fclose(fp);
    return;
  }
  fseek(fp,0,SEEK_SET);
  fread(temp,32,1,fp);
  if (memcmp(temp,srch,32))
  {
    printf("replace_data: WARNING: first 32 bytes DO NOT MATCH -- proceeding anyway!\n");
  }
  fread(srch+32,srchlen-32,1,fp);
  fclose(fp);
  memcpy(hdr,srch+32,srchlen-32);
}


#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
int genUninstallData(void)
{
  int crc=adler32(0L, Z_NULL, 0);

  int soffs=build_datablock_used;
  int oinksize;
  firstheader fh;

  _tcscpy(build_uninst.name,build_header.name);

  add_data_int(g_mainicon_offset);
//  printf("mainicon_offset: %d\n",g_mainicon_offset);
  add_data_misc((BYTE *)unicon_data+32,766-32);
  
  add_data_byte(0);
  add_data_int(
    sizeof(fh)+sizeof(build_uninst)+sizeof(entry)*ubuild_entries_used+ubuild_datablock_used+(build_crcchk?4:0)
    );

  oinksize=build_datablock_used;

  crc=adler32(crc,header_data,g_mainicon_offset);
  crc=adler32(crc,unicon_data+32,766-32);
  crc=adler32(crc,header_data+g_mainicon_offset+766-32,sizeof(header_data)-766+32-g_mainicon_offset);

  _tcscpy(fh._nsisinstall,_T("nsisinstall"));
  fh._x0BADF00D_or_DEADF00D=0x0DEADF00D;
  fh._xDEADBEEF=0xDEADBEEF;
  fh.length_of_header=sizeof(uninstall_header)+sizeof(entry)*ubuild_entries_used;
  fh.length_of_all_following_data=fh.length_of_header+ubuild_datablock_used;
  add_data_misc((BYTE*)&fh,sizeof(fh));
  crc=adler32(crc,(BYTE*)&fh,sizeof(fh));

  add_data_misc((BYTE*)&build_uninst,sizeof(build_uninst));
  crc=adler32(crc,(BYTE*)&build_uninst,sizeof(build_uninst));

  add_data_misc((BYTE*)ubuild_entries,sizeof(entry)*ubuild_entries_used);
  crc=adler32(crc,(BYTE*)ubuild_entries,sizeof(entry)*ubuild_entries_used);

  add_data_misc(ubuild_datablock,ubuild_datablock_used);
  crc=adler32(crc,(BYTE *)ubuild_datablock,ubuild_datablock_used);

  if (build_crcchk) add_data_int(crc);

  g_uninstall_size=build_datablock_used-oinksize;
  return soffs;
}
#endif

void ReportResourceError(UINT uID) {
	TCHAR error[MAX_PATH];
	TCHAR caption[MAX_PATH];
	if (!LoadString(g_hInstance, JAVAWS_ERROR_RESOURCE, error, MAX_PATH)) {
		_stprintf(error, _T("Error loading resource: %ld"), uID);
	}		
    if (!LoadString(g_hInstance, JAVAWS_ERROR_RESOURCE_CAPTION, caption,
		MAX_PATH)) {
		_stprintf(caption, _T("Installer Error"));
	}
	_tprintf(_T("%s: %s\n"), caption, error);
}
