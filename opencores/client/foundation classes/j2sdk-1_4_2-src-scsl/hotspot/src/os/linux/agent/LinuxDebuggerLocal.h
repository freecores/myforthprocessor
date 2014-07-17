#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)LinuxDebuggerLocal.h	1.2 03/01/23 11:02:49 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#ifndef _LINUX_DEBUGGER_LOCAL_H

#include <limits.h>
#include <sys/types.h>
#include <pthread.h>

#define DEBUG

typedef int           bool;
#define true          (1)
#define false         (0)

typedef unsigned char byte;
typedef char *        address;

#define BUF_SIZE    (PATH_MAX + NAME_MAX)

#define MAX_NUM_THREADS 1024

struct symtab;

typedef struct lib_info_struct {
  char name[BUF_SIZE];
  address base;
  struct symtab *symtab;
  struct lib_info_struct *next;
} lib_info;

typedef struct {
  pthread_t tid;
  pid_t     pid;
} thread_info;

typedef struct {
  char name[BUF_SIZE];
  bool attached;
  pid_t pid;
  int num_thread;
  thread_info threads[MAX_NUM_THREADS];
  lib_info    *libs;
} process_info;

extern process_info debuggee;

struct ps_prochandle {
  pid_t pid;
};

//-------------------------

#ifndef DEBUG
#define perror(x)
#endif

#define guarantee(cond, msg) if (!(cond)) fatal(msg)
void fatal(const char *msg);

void utils_init(void);

address lookup_symbol(pid_t pid, const char *object_name, 
                      const char *sym_name, int *sym_size);

bool ptrace_read_data(pid_t pid, address addr, char *buf, size_t size);
bool ptrace_attach(pid_t pid);
bool ptrace_detach(pid_t pid);
bool ptrace_getregs(pid_t pid, void *user);

bool attach_threads(pid_t pid);

pid_t string_to_pid(const char *s);
char *pid_to_path(pid_t pid);
pid_t get_parent_pid(pid_t pid);

// flags for enumerate_processes
#define EP_ALL           0x01   /* enumerate all pids, even if not having    */
                                /* enough priviledge to attach               */
#define EP_LINUXTHREADS  0x10   /* skip pids that are created by LinuxThreads*/

typedef int enumerate_processes_callback(pid_t pid, char *path, int level);
bool enumerate_processes(enumerate_processes_callback *callback, int flags);

bool read_lib_info(pid_t pid);
void destroy_lib_info(void);

#endif //  _LINUX_DEBUGGER_LOCAL_H
