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
 * @(#)i18n.c	1.6 03/01/23
 */


#include <windows.h>
#include <tchar.h>
#include <memory.h>

#include "fileform.h"
#include "i18n.h"
#include "ui.h"

extern uninstall_header *m_uninstheader;

StringTable *doLine(LPTSTR);
StringTable **getStringsForLangID(WORD);
int getIndexForLangID(WORD);

StringTable **getStrings(WORD *langID) {
	
	StringTable **table;
	WORD langID2;
	
	/* Try passed language first, if not null */
	if (langID == NULL || !(table=getStringsForLangID(*langID))) {
	  langID2 = GetUserDefaultLangID();
	  if (!(table = getStringsForLangID(langID2))) {
	    langID2 = GetSystemDefaultLangID();
	    if (!(table = getStringsForLangID(langID2))) {
	      langID2 = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
	      table = getStringsForLangID(langID2);
	    }
	  }
	}

	return table;
}

StringTable **getStringsForLangID(WORD langID) {
	int index=-1, len=0, lstate=0;
	LPTSTR p, buf = (LPTSTR)GlobalAlloc(GMEM_FIXED, 32769);
	StringTable **table=NULL;
	StringTable *entry=NULL;
	int ofs;
	buf[0]=0;

	if ((index = getIndexForLangID(langID)) < 0) return NULL;
	ofs = m_uninstheader ? m_uninstheader->stringdata_offsets[index]:m_header->stringdata_offsets[index];
	if (ofs == -1) return NULL;

	table=(StringTable **)GlobalAlloc(GMEM_FIXED, (sizeof (StringTable *) +1) * MAX_STRINGS);
	len=GetCompressedDataFromDataBlockToMemory(m_hFile, m_offset + ofs, buf, 32768);
	if (len <=0) return NULL;
	if ((p = _tcstok(buf, _T("\n"))) == NULL) return NULL;
	if (entry = doLine(p)) {
		table[g_nstrings++] = entry;
	}
		
	while((p = _tcstok(NULL, _T("\n"))) != NULL) {
		if (entry = doLine(p)) {
			table[g_nstrings++] = entry;
		}
	}

	return table;
}

StringTable *doLine(LPTSTR p) {
	LPTSTR q=NULL, s=NULL;
	int len, valid=0;
	StringTable *ent;
	if (p==NULL) return NULL;

	ent = (StringTable *)GlobalAlloc(GMEM_FIXED, sizeof(StringTable));
	if (ent == NULL) return NULL;
	while (*p && *p==_T(' ')) p = CharNext(p);
	if (*p && *p!=_T('#')) {
		q = p;
		while (*p && *p != _T('=')) p = CharNext(p);
		len = p - q;
		ent->name = (LPTSTR)GlobalAlloc(GMEM_FIXED, (len +1) * sizeof (TCHAR));
		_tcsncpy(ent->name, q, len);
		ent->name[len]=0;
		p = CharNext(p);
		q = p;
		while (*p) p = CharNext(p);
		len = p - q;
		ent->value = (LPTSTR)GlobalAlloc(GMEM_FIXED, (len +1) * sizeof (TCHAR));
		_tcsncpy(ent->value, q, len);
		ent->value[len]=0;
		valid=1;
	}
	return (valid == 1 ? ent : NULL);
}


int getIndexForLangID(WORD langID) {
	int match = 0, i;
	int nlangs, lID;

	nlangs=m_uninstheader ? m_uninstheader->num_langs: m_header->num_langs;
	for (i=0; i<nlangs; i++) {
		lID=m_uninstheader ? m_uninstheader->stringdata_langIDs[i] : m_header->stringdata_langIDs[i];
		if (lID == langID) {
			match = 1;
			break;
		}
	}

	return (match == 1 ? i : -1);
}
