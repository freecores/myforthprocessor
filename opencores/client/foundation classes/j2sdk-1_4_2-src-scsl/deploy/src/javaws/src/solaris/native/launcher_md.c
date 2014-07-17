/*
 * @(#)launcher_md.c	1.6 03/02/19
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "system.h"

/*  Nothing to do
 *
 */
void LauncherSetup_md() {
}
/* always 1 (pass) for EULA on solaris/linux */
int EULA_md(int argc, char** argv, int isPlayer) {
  return 1;
}

