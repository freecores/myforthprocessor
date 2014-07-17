#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)isNT4.cpp	1.4 03/01/23 11:07:04 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "isNT4.hpp"
#include <windows.h>

bool
isNT4() {
  OSVERSIONINFO info;
  info.dwOSVersionInfoSize = sizeof(info);

  if (!GetVersionEx(&info)) {
    return false;
  }

  return ((info.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
          (info.dwMajorVersion == 4));
}
