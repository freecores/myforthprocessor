#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)hpi_win32.cpp	1.9 03/01/23 11:07:52 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_hpi_win32.cpp.incl"

typedef jint (JNICALL *init_t)(GetInterfaceFunc *, void *);

#ifdef ASSERT
#define HPI_DLL "hpi_g.dll"
#else
#define HPI_DLL "hpi.dll"
#endif

void hpi::initialize_get_interface(vm_calls_t *callbacks)
{
  // Build name of HPI.
  char lib_name[JVM_MAXPATHLEN];
  os::jvm_path(lib_name, sizeof lib_name);
  *(::strrchr(lib_name, '\\')) = '\0';  /* get rid of "\\jvm.dll" */
  *(::strrchr(lib_name, '\\')) = '\0';  /* get rid of "\\hotspot" */
  strcat(lib_name, "\\" HPI_DLL);

        // Load it.
  if (TraceHPI)
    tty->print("Loading HPI %s ", lib_name);
  HINSTANCE lib_handle = LoadLibrary(lib_name);
  if (lib_handle == NULL) {
    if (TraceHPI)
      tty->print_cr("LoadLibrary failed, code = %d", GetLastError());
    return;
  }
  
  // Find hpi initializer.
  init_t initer = (init_t)GetProcAddress(lib_handle, "DLL_Initialize");
  if (initer == NULL) {
    if (TraceHPI)
      tty->print("GetProcAddress failed, errcode = %d", GetLastError());
    return;
  }

  // Call initializer.
  jint init_result = (*initer)(&_get_interface, callbacks);
  if (init_result < 0) {
    if (TraceHPI)
      tty->print_cr("DLL_Initialize failed, returned %ld", init_result);
    return;
  }     

  if (TraceHPI)
    tty->print_cr("success");
  return;
}
