/*
 * @(#)dirent_md.h	1.12 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Posix-compatible directory access routines
 */

#ifndef _WIN32_DIRENT_H_
#define _WIN32_DIRENT_H_

#include "jvm_md.h"	/* DIR actually defined in here */

DIR *opendir(const char *dirname);
struct dirent *readdir(DIR *dirp);
int closedir(DIR *dirp);
void rewinddir(DIR *dirp);

#endif
