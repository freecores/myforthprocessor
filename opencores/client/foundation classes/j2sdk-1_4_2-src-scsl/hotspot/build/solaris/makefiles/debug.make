# 
# @(#)debug.make	1.13 03/01/23 10:50:12
# 
# Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

# Sets make macros for making debug version of VM

# Compiler specific DEBUG_CFLAGS are passed in from gcc.make, sparcWorks.make
DEBUG_CFLAGS/DEFAULT= $(DEBUG_CFLAGS)
DEBUG_CFLAGS/BYFILE = $(DEBUG_CFLAGS/$@)$(DEBUG_CFLAGS/DEFAULT$(DEBUG_CFLAGS/$@))
CFLAGS += $(DEBUG_CFLAGS/BYFILE)

G_SUFFIX = _g
SYSDEFS += -DASSERT -DDEBUG
VERSION = debug
PICFLAGS = DEFAULT
