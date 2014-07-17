#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ptrace.h>

#include "LinuxDebuggerLocal.h"

long page_size;

FILE * input, *output, *error;

// simple commands for SA
#define ATTACH   'A'   /* attach to a process and all its threads */
#define DETACH   'D'   /* detach from a process and all its attached threads */
#define HELP     'H'   /* print help screen */
#define GET      'G'
#define PROCLIST 'P'   /* list processes */
#define THRLIST  'T'   /* list threads */
#define QUIT     'Q'   /* quit */

// and the command array
char command_array[] = {
  ATTACH, DETACH, GET, PROCLIST, THRLIST, QUIT, HELP
};

process_info debuggee;

void init() {
  page_size = sysconf(_SC_PAGE_SIZE);

  utils_init();

  debuggee.attached = false;
  debuggee.pid = 0;
  debuggee.num_thread = 0;
  debuggee.libs = NULL;

  input = stdin;
  output = stdout;
  error = stderr;
}

// attach to process "pid" and all its threads
bool attach(pid_t pid) {
  if (debuggee.attached) {
    return false;
  } else if (!ptrace_attach(pid)) {
    return false;
  } else {
    char link[BUF_SIZE];

    debuggee.pid      = pid;

    // read library info and symbol tables, must do this before attach_threads,
    // as the symbols in the pthread library will be used to figure out
    // the list of threads within the same process.
    read_lib_info(pid);

    attach_threads(pid);

    debuggee.attached = true;
    
    sprintf(link, "/proc/%d/exe", pid);
    realpath(link, debuggee.name);

    return true;
  } 
}   

// detach from debuggee and all its threads
bool detach(void) {
  if (!debuggee.attached) {
    return false;
  } else if (debuggee.num_thread > 0) {
    int i;
    for (i = 0; i < debuggee.num_thread; i++) {
      if (ptrace(PTRACE_DETACH, debuggee.threads[i].pid, NULL, NULL) < 0) {
         // failed to detach
         perror("ptrace(PTRACE_DETACH, ...)");
         fatal("can't detach");
      }
    }
  } else {
    if (ptrace(PTRACE_DETACH, debuggee.pid, NULL, NULL) < 0) {
      // failed to detach
      perror("ptrace(PTRACE_DETACH, ...)");
      fatal("can't detach");
    }
  }

  destroy_lib_info();

  debuggee.attached = false;
  debuggee.pid = 0;
  debuggee.num_thread = 0;
  debuggee.libs = NULL;

  return true;
}

void handle_echo_cmd(char * command) {
  puts("inside handle_echo_cmd");
}

void handle_attach_cmd(char * command) {
  int pid;
  sscanf(command + 1, "%d", &pid);

  attach(pid);
}

void handle_detach_cmd(char * command) {
  detach();
}

void handle_help_cmd(char * command) {
  puts("A pid       - attach to process 'pid'");
  puts("D           - detach from the attached process");
  puts("G symbol    - inspect variable 'symbol'");
  puts("G addr size - inspect memory from 'addr'");
  puts("H           - print this help screen");
  puts("P           - list active processes");
  puts("T           - list threads in the attached process");
  puts("Q           - quit");

  printf("Current Status: ");
  if (debuggee.attached) {
    lib_info *lib;
    printf("attached to %d (%s)\n", debuggee.pid, debuggee.name);
    printf("libraries:\n");
    lib = debuggee.libs;
    while (lib) {
      printf("%x %s\n", lib->base, lib->name);
      lib = lib->next;
    }
  } else {
    printf("unattached");
  }
  puts("");
}

void handle_get_cmd(char * command) {
  address addr;
  int size;
  char s1[256], s2[256];
  byte *data;
  int i;

  if (!debuggee.attached) return;

  i = sscanf(command + 1, "%s %s", s1, s2);
  if (i == 1) {
    addr = lookup_symbol(debuggee.pid, NULL, s1, &size);
  } else if (i == 2) {
    fatal("unimplemented");
  } else return;

  data = malloc(size);
  if (ptrace_read_data(debuggee.pid, addr, data, size)) {
    for (i = 0; i < size; i++) {
      printf("%02x ", data[i]);
    }
  }
  free(data);
}

int print_pid_path(pid_t pid, char *path, int level) {
  // int i;
  // for (i = 0; i < level; i++) fprintf(output, ".");
  fprintf(output, "%d %s\n", pid, path);
  fflush(output);
  return 0;
}

void handle_proclist_cmd(char * command) {
  enumerate_processes(print_pid_path, EP_LINUXTHREADS);
}

void handle_thrlist_cmd(char * command) {
  int i;

  if (!debuggee.attached) return;
  if (debuggee.num_thread == 0) return;

  for (i = 0; i < debuggee.num_thread; i++) {
    fprintf(output, "%d [LWP: %d]\n", 
            debuggee.threads[i].tid, debuggee.threads[i].pid);
  }

  fflush(output);
}

void handle_quit_cmd() {
  detach();
  exit(0);
}

int main(void) {
  char command[256];

  init();

  while (true) {
    fgets(command, 256, input);

    switch(command[0]) {
    case ATTACH:   handle_attach_cmd(command);   break;
    case DETACH:   handle_detach_cmd(command);   break;
    case HELP:     handle_help_cmd(command);     break;
    case GET:      handle_get_cmd(command);     break;
    case PROCLIST: handle_proclist_cmd(command); break;
    case THRLIST:  handle_thrlist_cmd(command);  break;
    case QUIT:     handle_quit_cmd(command);     break;
    default:
#ifdef DEBUG
      fprintf(error, "unknown command: %s\n", command);
#endif
    }
  }

  return 0;
}

