/*
 * @(#)SocketDispatcher.c	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "jni_util.h"
#include "jvm.h"
#include "jlong.h"
#include "sun_nio_ch_FileDispatcher.h"

/* this is a fake c file to make the build happy since there is no
   real SocketDispatcher.c file on Solaris but there is on windows. */

static jfieldID fd_fdID;	/* for jint 'fd' in java.io.FileDescriptor */




