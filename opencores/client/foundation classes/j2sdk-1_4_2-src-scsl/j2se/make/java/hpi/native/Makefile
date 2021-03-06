#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.13 03/01/23
#
# Makefile for native threads HPI.
#

BUILDDIR     = ../../..
LIBRARY      = hpi
PRODUCT      = java
THREADDIR    = native_threads
LIB_LOCATION = $(LIBDIR)/$(LIBARCH)/$(THREADDIR)
include $(BUILDDIR)/common/Defs.gmk

#
# Native threads specific C and .s files.
#
FILES_c = \
	$(TARGDIR)monitor_md.c \
	$(TARGDIR)threads_md.c \
	$(TARGDIR)condvar_md.c \
	$(TARGDIR)interrupt_md.c \
	$(TARGDIR)mutex_md.c \
	$(TARGDIR)sys_api_td.c \
	$(TARGDIR)threads_$(PLATFORM).c

#
# Other files/flags shared between the HPIs.
#
include $(BUILDDIR)/java/hpi/hpi_common.gmk

#
# Create the thread specific libdir.
#
INIT = $(LIBDIR)/$(LIBARCH)/$(THREADDIR)

#
# Rules for the .so file.
#
ifeq ($(PLATFORM), solaris)
  FILES_reorder += reorder-$(ARCH)
endif
include $(BUILDDIR)/common/Mapfile-vers.gmk
include $(BUILDDIR)/common/Library.gmk

#
# HPI flags for native threads.
#
OTHER_CPPFLAGS += -D_REENTRANT -DNATIVE

ifeq ($(USE_PTHREADS),true)
OTHER_CPPFLAGS += -DUSE_PTHREADS
ifeq ($(MOOT_PRIORITIES),true)
OTHER_CPPFLAGS += -DMOOT_PRIORITIES
endif
LIBPOSIX4	= -lposix4
OTHER_LDLIBS   += -lpthread $(LIBPOSIX4)
endif

HAVE_GETHRVTIME=true
ifeq ($(HAVE_GETHRVTIME),true)
OTHER_CPPFLAGS += -DHAVE_GETHRVTIME
endif

HAVE_FILIOH=true
ifeq ($(HAVE_FILIOH),true)
OTHER_CPPFLAGS += -DHAVE_FILIOH
endif

ifeq ($(NO_INTERRUPTIBLE_IO),true)
OTHER_CPPFLAGS += -DNO_INTERRUPTIBLE_IO
endif

$(LIBDIR)/$(LIBARCH)/$(THREADDIR):
	$(MKDIR) -p $@
