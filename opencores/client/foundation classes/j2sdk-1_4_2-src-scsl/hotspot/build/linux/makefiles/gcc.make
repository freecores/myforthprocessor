# 
# @(#)gcc.make	1.24 03/01/23 10:49:10
# 
# Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

# put the *.o files in the a.out, not the shared library
LINK_INTO = $(LINK_INTO/$(VERSION))
LINK_INTO/debug     = AOUT
#LINK_INTO/optimized = AOUT
LINK_INTO/optimized = LIBJVM

# apply this setting to link into the shared library even in the debug version:
LINK_INTO = LIBJVM

# If MAKE_OPTO is null, then we will compile with the -DOPTO.
# That switch, despite its name, has the effect of __disabling__ OPTO.
$(MAKE_OPTO)CFLAGS += -DOPTO

ifdef CC_INTERP
CFLAGS += -DCC_INTERP 
endif
ifdef NEED_ASM
CFLAGS += -save-temps
endif

CFLAGS += -fPIC
CFLAGS += -Winline
CFLAGS += -fno-rtti
CFLAGS += -fno-exceptions
CFLAGS += -D__STDC__=1 -D_LARGEFILE64_SOURCE
CFLAGS += -D__GNU__ -D_REENTRANT
CFLAGS += -fcheck-new

# Set GCC_SAVE_TEMPS=-save-temps to keep temporary files (.ii, .s).
CFLAGS += $(GCC_SAVE_TEMPS)

# The flags to use for an Optimized g++ build
OPT_CFLAGS += -O3 
# Hotspot uses very unstrict aliasing turn this optimization off
OPT_CFLAGS += -fno-strict-aliasing

# The gcc compiler segv's on ia64 when compiling cInterpreter.cpp 
# if we use expensive-optimizations 
ifeq ($(ARCH), ia64)
OPT_CFLAGS += -fno-expensive-optimizations
endif

OPT_CFLAGS/NOOPT=-O0

# Set the environment variable HOTSPARC_HOTSPARC_GENERIC to "true"
# to inhibit the effect of the previous line on CFLAGS.

ifdef USE_EGCS
CPP = egcs
CC  = egcs
else
# unlike g++, gcc statically links libstdc++ (add 83K in filesize), so people 
# without gcc/2.91 (or whatever version we use to build the binaries) can 
# run java without installing the compatibility libstdc++
CPP = gcc
CC  = gcc
endif

# statically link libstdc++.so
LIBS += -Wl,-Bstatic -lstdc++ -Wl,-Bdynamic
ifeq ($(ARCH), ia64)
LIBS += -Wl,-relax
endif

AOUT_FLAGS += -export-dynamic 

# -dumpversion in gcc-2.91 shows "egcs-2.91.66". In later version, it only
# prints the numbers (e.g. "2.95", "3.2.1")
CC_VER_MAJOR := $(shell $(CC) -dumpversion | sed 's/egcs-//' | cut -d'.' -f1)
CC_VER_MINOR := $(shell $(CC) -dumpversion | sed 's/egcs-//' | cut -d'.' -f2)

# statically link libgcc, libgcc does not exist before gcc-3.x.
ifeq ("${CC_VER_MAJOR}", "3")
LIB_FLAGS += -static-libgcc
endif

# Use the stabs format for debugging information (this is the default
# on gcc-2.91). It's good enough, has all the information about line
# numbers and local variables, and libjvm_g.so is only about 16M.
# Change this back to "-g" if you want the most expressive format.
# (warning: that could easily inflate libjvm_g.so to 150M!)
# Note: The Itanium gcc compiler crashes when using -gstabs.
ifeq ($(ARCH), ia64)
DEBUG_CFLAGS += -g
else
DEBUG_CFLAGS += -gstabs
endif
