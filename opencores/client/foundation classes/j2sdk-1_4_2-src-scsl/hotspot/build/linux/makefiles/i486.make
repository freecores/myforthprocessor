# 
# @(#)i486.make	1.8 03/01/23 10:49:18
# 
# Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

# The copied fdlibm routines in sharedRuntimeTrig.o must not be optimized
OPT_CFLAGS/sharedRuntimeTrig.o = $(OPT_CFLAGS/NOOPT)
# Must also specify if CPU is little endian
CFLAGS += -DLITTLE_ENDIAN

# Some files must be optimized even in the debugging build.
# this was a workaround for a compiler bug seen long ago.
ifeq ("${COMPILER_REV}", "5.0")
#
# interestingly enough these same two files that must be optimized
# with SC4.2 can't be optimized at -O4 with SC5.0 otherwise they
# work fine. Tracked problem with jni.cpp and files bugtraq. Have
# not tracked down problem with jvm.cpp to see if it is same problem.
#
#OPT_CFLAGS/jni.o = -O3
#OPT_CFLAGS/jvm.o = -O3
# Here is another load of files that miscompile in compat=5 and -O4
#OPT_CFLAGS/cfgnode.o = -O3
#OPT_CFLAGS/interpreterRuntime.o = -O3
#OPT_CFLAGS/parse2.o = -O3
#OPT_CFLAGS/chaitin.o = -O3
#OPT_CFLAGS/library_call.o = -O3
#OPT_CFLAGS/parse3.o = -O3
#OPT_CFLAGS/connode.o = -O3
#OPT_CFLAGS/loopnode.o = -O3
#OPT_CFLAGS/parseHelper.o = -O3
#OPT_CFLAGS/doCall.o = -O3
#OPT_CFLAGS/loopopts.o = -O3
#OPT_CFLAGS/subnode.o = -O3
#OPT_CFLAGS/generateOptoStub.o = -O3
#OPT_CFLAGS/memnode.o = -O3
#OPT_CFLAGS/type.o = -O3
#OPT_CFLAGS/instanceKlass.o = -O3
#OPT_CFLAGS/os.o = -O3

# _lwp_create_interpose must have a frame
OPT_CFLAGS/os_solaris_i486.o = -xO1
else
# SC4.2 Tools
#
DEBUG_CFLAGS/jni.o = -O0
# with -O4 this produces symbols in debug build that dbx can't
# stand must strip to allow debugging.
#
DEBUG_CFLAGS/jvm.o = -O0
endif

# "small" x86 Solaris systems have trouble with full optimization
#OPT_CFLAGS/dfa_i486.o = -O1

#
# Bug in ild causes it to fail randomly. Until we get a fix we can't
# use ild.
#
ILDFLAG/debug     = -xildoff

# temp hack until ia64 assembler works
EXTRA_OBJS = 

