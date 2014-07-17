#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)utils.c	1.2 03/01/23 11:02:54 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/ptrace.h>
#include <dirent.h>
#include <libelf/libelf.h>
#include <thread_db.h>
#include <assert.h>
#include <search.h>

#include "LinuxDebuggerLocal.h"

void utils_init(void) {
  static bool utils_init_done = false;

  if (utils_init_done) return;

  // initialize the elf library
  if (elf_version(EV_CURRENT) == EV_NONE) {
    fatal("libelf out of date");
  }

  // initialize the thread_db library
  if (td_init() != TD_OK) {
    fatal("td_init failed");
  }

  utils_init_done = true;
}

static inline intptr_t align(intptr_t ptr, intptr_t size) {
  size = size - 1;
  return ((ptr + size) & ~size);
}

// FIXME: we probably should try to detach the target process first.
void fatal(const char *msg) {
  puts(msg);
  exit(-1);
}

// ---------------------------------------------
// ptrace functions
// ---------------------------------------------

// read "size" bytes of data from "addr" within the target process.
// unlike the standard ptrace() function, ptrace_read_data() can handle
// unaligned address - alignment check, if required, should be done 
// before calling ptrace_read_data() 
bool ptrace_read_data(pid_t pid, address addr, char *buf, size_t size) {
  long rslt;
  size_t i, words;
  address end_addr = addr + size;
  address aligned_addr = (address)align((intptr_t)addr, sizeof(long));

  if (aligned_addr != addr) {
    char *ptr = (char *)&rslt;
    errno = 0;
    rslt = ptrace(PTRACE_PEEKDATA, pid, aligned_addr, 0);
    if (errno) {
      perror("ptrace(PTRACE_PEEKDATA, ..)");
      return false;
    }
    for (; aligned_addr != addr; aligned_addr++, ptr++);
    for (; (intptr_t)aligned_addr % sizeof(long); aligned_addr++) 
       *(buf++) = *(ptr++);
  }

  words = (end_addr - aligned_addr) / sizeof(long);

  assert((intptr_t)aligned_addr % sizeof(long) == 0);
  for (i = 0; i < words; i++) {
    errno = 0;
    rslt = ptrace(PTRACE_PEEKDATA, pid, aligned_addr, 0);
    if (errno) { 
      perror("ptrace(PTRACE_PEEKDATA, ..)");
      return false;
    }
    *(long *)buf = rslt;
    buf += sizeof(long);
    aligned_addr += sizeof(long);
  }

  if (aligned_addr != end_addr) {
    char *ptr = (char *)&rslt;
    errno = 0;
    rslt = ptrace(PTRACE_PEEKDATA, pid, aligned_addr, 0);
    if (errno) {
      perror("ptrace(PTRACE_PEEKDATA, ..)");
      return false;
    }
    for (; aligned_addr != end_addr; aligned_addr++) 
       *(buf++) = *(ptr++);
  }
  return true;
}

// attach to a process/thread specified by "pid"
// to attach to all threads within the same overall process, use 
// attach_threads() after having attached to any one thread.
bool ptrace_attach(pid_t pid) {
  if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) < 0) {
    perror("ptrace(PTRACE_ATTACH, ..)");
    return false;
  } else {
//  FIXME: we should read the status and make sure it's stopped. But this
//         sometimes never return when running within SA. However, it 
//         works OK in single threaded test debugger.
//    waitpid(pid, NULL, 0);
    return true;
  } 
} 

bool ptrace_detach(pid_t pid) {
  if (ptrace(PTRACE_DETACH, pid, NULL, NULL) < 0) {
    perror("ptrace(PTRACE_DETACH, ..)");
    return false;
  } else {
    return true;
  }
}

// "user" should be a pointer to a user_regs_struct
bool ptrace_getregs(pid_t pid, void *user) {
  if (ptrace(PTRACE_GETREGS, pid, NULL, user) < 0) {
    perror("ptrace(PTRACE_GETREGS, ..)");
    return false;
  } else {
    return true;
  }
}

// ----------------------------------------------------
// functions for symbol lookups
// ----------------------------------------------------

struct elf_section {
  Elf32_Shdr *c_shdr;
  Elf_Data   *c_data;
};

struct elf_symbol {
  char *name;
  unsigned offset;
  int size;
};

struct symtab {
  char *strs;
  struct elf_symbol *symbols;
  struct hsearch_data *hash_table;
};

// read symbol table from elf_file
struct symtab *build_symtab(const char *elf_file) {
  int fd;
  Elf *elf;
  Elf32_Ehdr *ehdr;
  char *names;
  struct symtab *symtab = NULL;

  if ((fd = open(elf_file, O_RDONLY)) < 0) {
    perror("open");
    return NULL;
  }

  elf = elf_begin(fd, ELF_C_READ, NULL);
  if (elf == NULL || elf_kind(elf) != ELF_K_ELF) {
    // not an elf
    close(fd);
    return NULL;
  }

  // read ELF header
  if ((ehdr = elf32_getehdr(elf)) != NULL) {
    Elf_Scn *scn;
    struct elf_section *scn_cache, *scn_cache_ptr;
    int cnt;

    // read section headers into scn_cache
    scn_cache = (struct elf_section *)
                malloc(ehdr->e_shnum * sizeof(struct elf_section));
    scn_cache_ptr = scn_cache;
    scn_cache_ptr++;

    for (scn = NULL; scn = elf_nextscn(elf, scn); scn_cache_ptr++) {
      scn_cache_ptr->c_shdr = elf32_getshdr(scn);
      scn_cache_ptr->c_data = elf_getdata(scn, NULL);
    }

    for (cnt = 1; cnt < ehdr->e_shnum; cnt++) {
      Elf32_Shdr *shdr = scn_cache[cnt].c_shdr;

      if (shdr->sh_type == SHT_SYMTAB) {
        Elf32_Sym  *syms;
        int j, n, rslt;
        size_t size;

        // FIXME: there could be multiple data buffers associated with the
        // same ELF section. Here we can handle only one buffer. See man page
        // for elf_getdata on Solaris.

        guarantee(symtab == NULL, "multiple symtab");
        symtab = (struct symtab *)calloc(1, sizeof(struct symtab));

        // the symbol table
        syms = (Elf32_Sym *)scn_cache[cnt].c_data->d_buf;

        // number of symbols
        n = shdr->sh_size / shdr->sh_entsize;

        // create hash table, we use hcreate_r, hsearch_r and hdestroy_r to
        // manipulate the hash table.
        symtab->hash_table = calloc(1, sizeof(struct hsearch_data));
        rslt = hcreate_r(n, symtab->hash_table);
        guarantee(rslt, "unexpected failure: hcreate_r");

        // shdr->sh_link points to the section that contains the actual strings
        // for symbol names. the st_name field in Elf32_Sym is just the
        // string table index. we make a copy of the string table so the
        // strings will not be destroyed by elf_end.
        size = scn_cache[shdr->sh_link].c_data->d_size;
        symtab->strs = (char *)malloc(size);
        memcpy(symtab->strs, scn_cache[shdr->sh_link].c_data->d_buf, size);

        // allocate memory for storing symbol offset and size;
        symtab->symbols = (struct elf_symbol *)malloc(n * sizeof(struct elf_symbol));

        // copy symbols info our symtab and enter them info the hash table
        for (j = 0; j < n; j++, syms++) {
          ENTRY item, *ret;
          char *sym_name = symtab->strs + syms->st_name;

          symtab->symbols[j].name   = sym_name;
          symtab->symbols[j].offset = syms->st_value;
          symtab->symbols[j].size   = syms->st_size;

          // skip empty strings
          if (*sym_name == '\0') continue;

          item.key = sym_name;
          item.data = (void *)&(symtab->symbols[j]);

          hsearch_r(item, ENTER, &ret, symtab->hash_table);
        }
      }
    }

    free(scn_cache);
  }

  elf_end(elf);
  close(fd);

  return symtab;
}

void destroy_symtab(struct symtab *symtab) {
  if (!symtab) return;
  if (symtab->strs) free(symtab->strs);
  if (symtab->symbols) free(symtab->symbols);
  if (symtab->hash_table) {
     hdestroy_r(symtab->hash_table);
     free(symtab->hash_table);
  }
  free(symtab);
}

// FIXME: what should we do with object_name?? The library names are obtained
// by parsing /proc/<pid>/maps, which may not be the same as object_name.
// What we need is a utility to map object_name to real file name, something
// dlopen() does by looking at LD_LIBRARY_PATH and /etc/ld.so.cache. For
// now, we just ignore object_name and do a global search for the symbol.
address lookup_symbol(pid_t pid, const char *object_name, 
                      const char *sym_name, int *sym_size) {
  lib_info *lib;

  lib = debuggee.libs;
  for (lib = debuggee.libs; lib; lib = lib->next) {
    ENTRY item;
    ENTRY *ret;

    // library does not have symbol table
    if (!lib->symtab || !lib->symtab->hash_table) continue;

    item.key = strdup(sym_name);
    hsearch_r(item, FIND, &ret, lib->symtab->hash_table);

    if (ret) {
      struct elf_symbol * sym = (struct elf_symbol *)(ret->data);
      address rslt = lib->base + sym->offset;
      if (sym_size) *sym_size = sym->size;
      free(item.key);
      return rslt;
    }

    free(item.key);
  }

  return NULL;
} 

// ----------------------------------------------------
// functions to help parsing /proc
// ----------------------------------------------------

pid_t string_to_pid(const char * s) {
  pid_t rslt = 0;

  // empty string is not pid
  if (!s || *s == '\0') return 0;
  while (*s != '\0') {
    if (!isdigit(*s)) return 0;
    rslt = rslt * 10 + (*s - '0');
    s++;
  }
  return rslt;
}
  
char * pid_to_path(pid_t pid) {
  char link[BUF_SIZE];
  static char path[BUF_SIZE]; 
  char * rslt;

  sprintf(link, "/proc/%d/exe", pid);
  rslt = realpath(link, path);

  return rslt;
}

pid_t get_parent_pid(pid_t pid) {
  char link[BUF_SIZE];
  char buf[2048];
  char * s;
  int statlen, i;
  FILE * fp;
  char state;
  pid_t ppid;

  sprintf(link, "/proc/%d/stat", pid);
  fp = fopen(link, "r");
  statlen = fread(buf, 1, 2047, fp);
  buf[statlen] = '\0';
  fclose(fp);

  s = strrchr(buf, ')');

  if (s) {
    do s++; while (isspace(*s));

    i = sscanf(s, "%c %d", &state, &ppid);

    if (i == 2) return ppid;
    else return 0;
  } else {
    return 0;
  }
}

// --------------------------------------------------------------
// functions for building a process tree (similar to "pstree")
// --------------------------------------------------------------

struct proc_pid {
  struct proc_pid *next;
  struct proc_pid *parent, *first_child, *brother;
  pid_t pid, ppid;
  int num_child;
};

struct proc_pid * pid_list = NULL;
struct proc_pid * pid_tree = NULL;

static struct proc_pid * new_proc_pid_struct(pid_t pid, pid_t ppid) {
  struct proc_pid * rslt;

  rslt = (struct proc_pid *)malloc(sizeof(struct proc_pid));
  guarantee(rslt, "out of memory");

  rslt->pid = pid;
  rslt->ppid = ppid;
  rslt->parent = NULL;
  rslt->next = NULL;
  rslt->brother = NULL;
  rslt->first_child = NULL;
  rslt->num_child = 0;

  return rslt;
}

static void pid_list_add(pid_t pid, pid_t ppid) {
  struct proc_pid *p;

  p = new_proc_pid_struct(pid, ppid);

  p->next = pid_list;
  pid_list = p;
}

static struct proc_pid * pid_list_find(pid_t pid) {
  struct proc_pid *p;

  p = pid_list;
  while (p) {
    if (p->pid == pid) return p;
    p = p->next;
  }

  return NULL;
}

static void pid_list_create_tree(void) {
  struct proc_pid *p, *parent;

  p = pid_list;
  pid_tree = NULL;
  while (p) {
    p->parent = p->first_child = p->brother = NULL;
    p->num_child = 0;
    p = p -> next;
  }

  p = pid_list;
  while (p) {
    parent = pid_list_find(p->ppid);
    if (parent) {
      p->parent = parent;
      p->brother = parent->first_child;
      parent->first_child = p;
      parent->num_child++;
    }
    p = p->next;
  }

  p = pid_list;
  while (p) {
    if (p->parent == NULL) {
      p->brother = pid_tree;
      pid_tree = p;
    }
    p = p->next;
  }
}

static void pid_list_destroy(void) {
  struct proc_pid * p;
  while (pid_list) {
    p = pid_list->next;
    free(pid_list);
    pid_list = p;
  }
  pid_tree = NULL;
}

// ----------------------------------------------------
// functions to list all active processes
// ----------------------------------------------------

// It's hard to figure out if this is the LinuxThreads main thread or
// just a regular process forked a couple of children without actually
// attaching to the process and looking at its internal __pthread_handles
// array. We use a simple heuristic here: check if current process only has
// one child (i.e. LinuxThreads manager thread) and the child process has
// more than 4 children - should work well for Java, as VM itself will
// create about 10 threads.
static bool is_pthread_initial_thread(struct proc_pid * p) {
  if (p->num_child != 1) return false;
  if (p->first_child->num_child < 5) return false;
  return true;
}

void pid_tree_do(struct proc_pid * start, 
                 enumerate_processes_callback *callback, int level, int flags) {
  int i;

  if (start == NULL) return;

  for(; start; start = start->brother) {
    char *path;

    path = pid_to_path(start->pid);

    if (flags & EP_ALL || path) {
      callback(start->pid, path, level);
    }

    if (start->num_child == 0 ||
       (flags & EP_LINUXTHREADS && is_pthread_initial_thread(start))) {
       continue;
    }

    pid_tree_do(start->first_child, callback, level + (path ? 1 : 0), flags);
  }
}

// enumerate all processes 
bool enumerate_processes(enumerate_processes_callback *callback, int flags) {
  struct dirent *subdir;
  DIR *dir;
  char *path;
  pid_t pid, ppid;

  // enumerate through /proc
  dir = opendir("/proc");
  if (dir != NULL) {
    // first, get a list of all pids
    while ((subdir = readdir(dir)) != NULL) {
      if (subdir->d_type == DT_DIR) {
        pid = string_to_pid(subdir->d_name);
        if (pid != 0) {
          ppid = get_parent_pid(pid);
          pid_list_add(pid, ppid);
        }
      }
    }

    // create pid tree
    pid_list_create_tree();

    pid_tree_do(pid_tree, callback, 0, flags);

    pid_list_destroy();

    closedir(dir);

    return true;
  } else {
    perror("opendir");
  }
}

// --------------------------------------------------------
// obtain thread list and attach to each of them
// --------------------------------------------------------

void add_thread(thread_t tid, pid_t pid) {
  debuggee.threads[debuggee.num_thread].tid = tid;
  debuggee.threads[debuggee.num_thread].pid = pid;
  debuggee.num_thread++;
}

int new_thread_callback(const td_thrhandle_t *th_p, void *data) {
  td_thrinfo_t ti;
  td_err_e err;

  err = td_thr_get_info(th_p, &ti);
  if (err != TD_OK) {
#ifdef DEBUG
    puts("cannot get thread info");
#endif
    return err;
  }

#ifdef DEBUG
  printf("new thread: %d (LWP %d)\n", ti.ti_tid, ti.ti_lid);
#endif

  if (ti.ti_lid != debuggee.pid) {
    if (!ptrace_attach(ti.ti_lid)) {
      printf("can't attach to thread %d (LWP %d)\n", ti.ti_tid, ti.ti_lid);
      return TD_ERR;
    }
  }

  add_thread(ti.ti_tid, ti.ti_lid);

  return TD_OK;
}

// pid is already attached to. use this function to attach to the rest of
// threads that belong to the same overall process.
bool attach_threads(pid_t pid) {
  td_err_e err;
  td_thragent_t *thread_agent;
  struct ps_prochandle proc_handle;

  guarantee(debuggee.pid == pid, "wrong pid value");

  proc_handle.pid = pid;
  err = td_ta_new(&proc_handle, &thread_agent);
  
  // assume "pid" is already attached to. we need to figure out all other LWPs 
  // that belong to the process and then attach to each of them.
  err = td_ta_thr_iter(thread_agent, new_thread_callback, NULL,
                       TD_THR_ANY_STATE, TD_THR_LOWEST_PRIORITY,
                       TD_SIGNO_MASK, TD_THR_ANY_USER_FLAGS);

  return (err = TD_OK ? true : false);
}

// -------------------------------------------------------
// functions for obtaining library information
// -------------------------------------------------------

/*
 * splits a string _str_ into substrings with delimiter _delim_ by replacing old * delimiters with _new_delim_ (ideally, '\0'). the address of each substring
 * is stored in array _ptrs_ as the return value. the maximum capacity of _ptrs_ * array is specified by parameter _n_.
 * RETURN VALUE: total number of substrings (always <= _n_)
 * NOTE: string _str_ is modified if _delim_!=_new_delim_
 */
static int split_n_str(char * str, int n, char ** ptrs, char delim, char new_delim)
{
   int i;
   for(i = 0; i < n; i++) ptrs[i] = NULL;
   if (str == NULL || n < 1 ) return 0;
   
   i = 0;

   // skipping leading blanks
   while(*str&&*str==delim) str++;
   
   while(*str&&i<n){
     ptrs[i++] = str;
     while(*str&&*str!=delim) str++;
     while(*str&&*str==delim) *(str++) = new_delim;
   }
   
   return i;
}

/*
 * fgets without storing '\n' at the end of the string
 */
static char * fgets_no_cr(char * buf, int n, FILE *fp)
{
   char * rslt = fgets(buf, n, fp);
   if (rslt && buf && *buf){
       char *p = strchr(buf, '\0');
       if (*--p=='\n') *p='\0';
   }
   return rslt;
}

// return base address 
address find_lib(const char *lib_name) {
  lib_info *p;

  p = debuggee.libs;
  while (p) {
    if (strcmp(p->name, lib_name) == 0) {
      return p->base;
    }
    p = p->next;
  }
  return NULL;
}

bool read_lib_info(pid_t pid) {
  char fname[32];
  char buf[256];
  FILE *fp = NULL;
  char *execfile = "";

  guarantee(debuggee.pid = pid, "wrong pid value");

  sprintf(fname, "/proc/%d/maps", pid);
  fp = fopen(fname, "r");
  if (fp == NULL) {
    fatal("can't open maps file");
  }

  while(fgets_no_cr(buf, 256, fp)){
    char * word[6];
    int nwords = split_n_str(buf, 6, word, ' ', '\0');
#   define EXE_LOC "08048000-"
    if (strncmp(buf, EXE_LOC, strlen(EXE_LOC)) == 0){
       execfile = strdup(word[5]);
    } else if (nwords > 5 && strcmp(word[5], execfile)!=0
            && find_lib(word[5]) == NULL) {
       lib_info *lib = malloc(sizeof(lib_info));

       lib->symtab = build_symtab(word[5]);
       if (lib->symtab) {
          sscanf(word[0], "%x", &lib->base);
          strcpy(lib->name, word[5]);

          lib->next = debuggee.libs;
          debuggee.libs = lib;
       } else {
          // not an elf file
          // FIXME: we could also reach here if library does not have
          // symbol table.
          free(lib);
       }
    }
  }
  fclose(fp);
}

void destroy_lib_info(void) {
  while (debuggee.libs) {
    lib_info *next = debuggee.libs->next;
    if (debuggee.libs->symtab) destroy_symtab(debuggee.libs->symtab);
    free(debuggee.libs);
    debuggee.libs = next;
  }
}

