#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)procList.hpp	1.5 03/01/23 11:07:24 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#ifndef _PROCLIST_
#define _PROCLIST_

#include <windows.h>
#include <vector>

class ProcEntry {
public:
  /** name may not be NULL */
  ProcEntry(ULONG pid, USHORT nameLength, wchar_t* name);
  ProcEntry(ULONG pid, USHORT nameLength, char* name);
  ~ProcEntry();
  ProcEntry(const ProcEntry& arg);
  ProcEntry& operator=(const ProcEntry& arg);
  
  ULONG getPid();
  /** Returns number of WCHAR characters in getName() */
  USHORT getNameLength();
  WCHAR* getName();

private:
  ULONG pid;
  USHORT nameLength;
  WCHAR* name;
  void copyFrom(const ProcEntry& arg);
};

typedef std::vector<ProcEntry> ProcEntryList;
void procList(ProcEntryList& processes);

#endif  // #defined _PROCLIST_
