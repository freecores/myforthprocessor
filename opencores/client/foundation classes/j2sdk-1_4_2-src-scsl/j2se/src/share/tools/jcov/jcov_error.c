/*
 * @(#)jcov_error.c	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdlib.h>
#include "jcov.h"

void jcov_internal_error(const char *message) {
    fprintf(stderr, "*** JCOV internal error: %s\n", message);
    CALL(ProfilerExit)((jint)1);
}

void jcov_error(const char *message) {
    fprintf(stderr, "*** JCOV error : %s\n", message);
}

void jcov_warn(const char *message) {
    fprintf(stderr, "*** JCOV WARNING : %s\n", message);
}

void jcov_info(const char *message) {
    fprintf(stdout, "### JCOV INFO : %s\n", message);
}

void jcov_error_stop(const char *message) {
    jcov_error(message);
    CALL(ProfilerExit)((jint)1);
}
