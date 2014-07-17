#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)proc_service.c	1.2 03/01/23 11:02:52 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include <stdio.h>
#include <thread_db.h>

#include "LinuxDebuggerLocal.h"

// Linux does not have the proc service library, though it does provide the
// thread_db library which can be used to manipulate threads without having
// to know the details of LinuxThreads. Here we define some basic proc 
// service functions using ptrace interface so SA can use the thread_db
// library to obtain thread list.

// copied from Solaris "proc_service.h"
typedef enum {
        PS_OK,          /* generic "call succeeded" */
        PS_ERR,         /* generic error */
        PS_BADPID,      /* bad process handle */
        PS_BADLID,      /* bad lwp identifier */
        PS_BADADDR,     /* bad address */
        PS_NOSYM,       /* p_lookup() could not find given symbol */
        PS_NOFREGS      /* FPU register set not available for given lwp */
} ps_err_e;

// ps_getpid() is only defined on Linux to return a thread's process ID
pid_t ps_getpid(struct ps_prochandle *ph) {
  return ph->pid;
}

// ps_pglobal_lookup() looks up the symbol sym_name in the symbol table 
// of the load object object_name in the target process identified by ph. 
// It returns the symbol's value as an address in the target process in 
// *sym_addr.

ps_err_e ps_pglobal_lookup(struct ps_prochandle *ph, const char *object_name,
                    const char *sym_name, psaddr_t *sym_addr) {
  *sym_addr = lookup_symbol(ph->pid, object_name, sym_name, NULL);
  return (*sym_addr ? PS_OK : PS_NOSYM);
}

// read "size" bytes of data from debuggee at address "addr"
ps_err_e ps_pdread(struct ps_prochandle *ph, psaddr_t  addr, 
                   void *buf, size_t size) {
  bool rslt = ptrace_read_data(ph->pid, (address)addr, buf, size);
  return (rslt ? PS_OK : PS_ERR);
}

// ------------------------------------------------------------------------
// Functions below this point are not yet implemented. They are here only
// to make the linker happy

// write "size" bytes of data to debuggee at address "addr"
ps_err_e ps_pdwrite(struct ps_prochandle *ph, psaddr_t addr,
                    const void *buf, size_t size) {
  fatal("ps_pdwrite not implemented");
  return PS_OK;
}

ps_err_e ps_lsetfpregs(struct ps_prochandle *ph, lwpid_t lid, const prfpregset_t *fpregs) { 
  fatal("ps_lsetfpregs not implemented");
  return PS_OK;
}

ps_err_e ps_lsetregs(struct ps_prochandle *ph, lwpid_t lid, const prgregset_t gregset) {
  fatal("ps_lsetregs not implemented");
  return PS_OK;
}
  
ps_err_e  ps_lgetfpregs(struct  ps_prochandle  *ph,  lwpid_t lid, prfpregset_t *fpregs) {
  fatal("ps_lgetfpregs not implemented");
  return PS_OK;
}

ps_err_e ps_lgetregs(struct ps_prochandle *ph, lwpid_t lid, prgregset_t gregset) {
  fatal("ps_lgetregs not implemented");
  return PS_OK;
}

