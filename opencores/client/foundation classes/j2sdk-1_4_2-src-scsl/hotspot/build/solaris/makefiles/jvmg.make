# 
# @(#)jvmg.make	1.11 03/01/23 10:50:26
# 
# Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

# Sets make macros for making debug version of VM

# Compiler specific DEBUG_CFLAGS are passed in from gcc.make, sparcWorks.make
DEBUG_CFLAGS/DEFAULT= $(DEBUG_CFLAGS)
DEBUG_CFLAGS/BYFILE = $(DEBUG_CFLAGS/$@)$(DEBUG_CFLAGS/DEFAULT$(DEBUG_CFLAGS/$@))
CFLAGS += $(DEBUG_CFLAGS/BYFILE)

# Set the environment variable HOTSPARC_GENERIC to "true"
# to inhibit the effect of the previous line on CFLAGS.

# Linker mapfiles
MAPFILE = $(GAMMADIR)/build/solaris/makefiles/mapfile-vers \
          $(GAMMADIR)/build/solaris/makefiles/mapfile-vers-debug \
          $(GAMMADIR)/build/solaris/makefiles/mapfile-vers-nonproduct

G_SUFFIX = _g
VERSION = optimized
SYSDEFS += -DASSERT -DDEBUG
PICFLAGS = DEFAULT
