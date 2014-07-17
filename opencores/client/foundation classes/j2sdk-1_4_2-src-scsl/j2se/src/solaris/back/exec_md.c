/*
 * @(#)exec_md.c	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include "sys.h"
#include "util.h"
  

static char *skipWhitespace(char *p) {
    while ((*p != '\0') && isspace(*p)) {
        p++;
    }
    return p;
}

static char *skipNonWhitespace(char *p) {
    while ((*p != '\0') && !isspace(*p)) {
        p++;
    }
    return p;
}

int 
dbgsysExec(char *cmdLine)
{
    int i;
    int argc;
    int pid = -1; /* this is the error return value */
    char **argv = NULL;
    char *p;
    char *args;

    /* Skip leading whitespace */
    cmdLine = skipWhitespace(cmdLine);

    args = jdwpAlloc(strlen(cmdLine)+1);
    if (args == NULL) {
        return SYS_NOMEM;
    }
    strcpy(args, cmdLine);

    p = args;

    argc = 0;
    while (*p != '\0') {
        p = skipNonWhitespace(p);
        argc++;
        if (*p == '\0') {
            break;
        }
        p = skipWhitespace(p);
    }

    argv = jdwpAlloc((argc + 1) * sizeof(char *));
    if (argv == 0) {
        jdwpFree(args);
        return SYS_NOMEM;
    }

    for (i = 0, p = args; i < argc; i++) {
        argv[i] = p;
        p = skipNonWhitespace(p);
        *p++ = '\0';
        p = skipWhitespace(p);
    }
    argv[i] = NULL;  /* NULL terminate */

    if ((pid = fork1()) == 0) {
        /* Child process */
        int i, max_fd;

        /* close everything */
        max_fd = sysconf(_SC_OPEN_MAX);
        for (i = 3; i < max_fd; i++) {
            close(i);
        }

        execvp(argv[0], argv);

        exit(-1);
    }
    jdwpFree(args);
    jdwpFree(argv);
    if (pid < 0) {
        return SYS_ERR;
    } else {
        return SYS_OK;
    }
}

