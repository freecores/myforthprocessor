# 
# @(#)sparcWorks.make	1.3 03/01/23 10:49:39
# 
# Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

# Compiler-specific flags for sparcworks.

# tell make which C++ compiler to use
CPP = CC

COMPILER_REV := $(shell CC -V 2>&1 | awk -e "{print \$$7; }" )
SOLARIS_REV := $(shell uname -r 2>&1 | sed s/5\.//)
#
# Some interfaces (_lwp_create) changed with LP64 and Solaris 7
#
ifneq ("${SOLARIS_REV}", "5")
ifneq ("${SOLARIS_REV}", "6")
CFLAGS += -DSOLARIS_7_OR_LATER
endif
endif

COMPAT4=0

# On the SPARC, there are some unrseolved issues with SC5.0
# The object layout appears to be different with COMPAT4=0 (the first
# field may be at offset 4 from the beginning of objects which breaks
# some alignment assertions. Until this is resolved, we use COMPAT4=1
# on the SPARC

ifeq ("${COMPILER_REV}", "5.0")
ifeq ("${Platform_arch}", "sparc")
COMPAT4=1
endif
endif

ifeq ("${COMPILER_REV}", "4.2")
COMPAT4=1
EXTRA_OPT_CFLAGS =
endif

# setting COMPAT4=1 means either 4.2 compilers are being used
# or a request to run SC5.0 compiler in compat=4 mode.
# By default if SC5.0 compilers detected will run in -compat=5 mode
# to run SC5.0 compilers in -compat=4 mode run gmake with COMPAT4=1
# to override this default.

ifeq ("${COMPILER_REV}", "5.0")
ifeq ("${Platform_arch}", "sparc")
#
# still some sparc issues with and/or/not/...
#
CFLAGS += -features=no%altspell
#
# Until proven safe disable putting restore in delay slot of a call.
#
# %%%% I think it is OK now.  We fixed __lwp_create.  -- jrose & borisw
#EXTRA_OPT_CFLAGS = -Qiselect-T0
endif

ifeq ("${COMPAT4}", "0")
# BobV says we need this, I haven't seen any evidence why
CFLAGS += -D__STD_CSIGNAL
CFLAGS += -library=iostream
LFLAGS += -library=iostream  -library=Crun
LIBS += -library=iostream -library=Crun -lCrun
else
# -compat=4 mode
CFLAGS += -compat=4
LFLAGS += -compat=4
LIBS += -lC -compat=4
endif
else
# 4.2 compilers
# nothing to do
# without this gamma will run, but in the jdk it fails.
LIBS += -lC
endif

# PIC is safer for SPARC, and is considerably slower
# a file foo.o which wants to compile -pic can set "PICFLAG/foo.o = -PIC"
PICFLAG/DEFAULT = -PIC
PICFLAG/BETTER  = -pic
PICFLAG/BYFILE  = $(PICFLAG/$@)$(PICFLAG/DEFAULT$(PICFLAG/$@))

# need position-indep-code for shared libraries
# (ild appears to get errors on PIC code, so we'll try non-PIC for debug)
ifeq ($(PICFLAGS),DEFAULT)
PICFLAG/LIBJVM  = $(PICFLAG/DEFAULT)
else
PICFLAG/LIBJVM  = $(PICFLAG/BYFILE)
endif
PICFLAG/AOUT    =
CFLAGS += $(PICFLAG/$(LINK_INTO))

# less dynamic linking (no PLTs, please)
#LIB_FLAGS += $(LINK_MODE)
# %%%%% despite -znodefs, -Bsymbolic gets link errors -- Rose

LINK_MODE = $(LINK_MODE/$(VERSION))
LINK_MODE/debug     =
LINK_MODE/optimized = -Bsymbolic -znodefs

# no more exceptions
CFLAGS += -noex

ifeq ($(Platform_arch),sparc)
# Have thread local errnos
CFLAGS += -D_REENTRANT
endif

# Flags for Debugging
DEBUG_CFLAGS = -g

# Flags for Optimization
OPT_CFLAGS=-xO4 -xtarget=native $(EXTRA_OPT_CFLAGS)
ifeq ("${Platform_arch}", "i486")
OPT_CFLAGS=-xtarget=pentium $(EXTRA_OPT_CFLAGS)
ifeq ("${COMPILER_REV}", "5.0")
# SC5.0 tools on x86 are flakey at -xO4
OPT_CFLAGS+=-xO3
else
OPT_CFLAGS+=-xO4
endif
endif

# Use this to work around compiler bugs:
OPT_CFLAGS/SLOWER=-xO3 -xtarget=native 

# The following options run into misaligned ldd problem (raj)
#OPT_CFLAGS = -fast -O4 -xarch=v8 -xchip=ultra


# Output source browser information
#CFLAGS += -sbfast

# use ild when debugging (but when optimizing we want reproducible results)
ILDFLAG = $(ILDFLAG/$(VERSION))
ILDFLAG/debug     = -xildon
ILDFLAG/optimized =
AOUT_FLAGS += $(ILDFLAG)

# put the *.o files in the a.out, not the shared library
LINK_INTO = $(LINK_INTO/$(VERSION))
LINK_INTO/debug     = AOUT
ifeq ($(Platform_arch),sparc)
LINK_INTO/optimized = LIBJVM
else
ifeq ($(Platform_arch),i486)
LINK_INTO/optimized = LIBJVM
else
LINK_INTO/optimized = AOUT
endif
endif

# apply this setting to link into the shared library even in the debug version:
#LINK_INTO = LIBJVM
