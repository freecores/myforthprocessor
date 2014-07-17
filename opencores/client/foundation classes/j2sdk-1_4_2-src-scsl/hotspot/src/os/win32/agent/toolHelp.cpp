#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)toolHelp.cpp	1.3 03/01/23 11:07:32 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "toolHelp.hpp"
#include <assert.h>

namespace ToolHelp {

static HMODULE kernelDLL = NULL;

HMODULE loadDLL() {
  if (kernelDLL == NULL) {
    kernelDLL = LoadLibrary("KERNEL32.DLL");
  }
  
  assert(kernelDLL != NULL);
  return kernelDLL;
}

void unloadDLL() {
  if (kernelDLL != NULL) {
    FreeLibrary(kernelDLL);
    kernelDLL = NULL;
  }
}

} // namespace ToolHelp
