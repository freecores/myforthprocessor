#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)libInfo.hpp	1.3 03/01/23 11:07:11 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#ifndef _LIBINFO_
#define _LIBINFO_

#include <vector>
#include <string>
#include <windows.h>

struct LibInfo {
  std::string name;
  void*  base;

  LibInfo(const std::string& name, void* base) {
    this->name = name;
    this->base = base;
  }
};

void libInfo(DWORD pid, std::vector<LibInfo>& info);

#endif  // #defined _LIBINFO_
