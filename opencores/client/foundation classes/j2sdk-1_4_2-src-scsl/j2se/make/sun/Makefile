#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.89 03/03/19
#
# Makefile for building all of sun tools
#

BUILDDIR = ..
PRODUCT  = sun
include $(BUILDDIR)/common/Defs.gmk

ifdef J2RE_ONLY
TOOLS_SUBDIRS =
else
ifeq ($(PLATFORM), windows)
TOOLS_SUBDIRS = javac toolsutil javap native2ascii serialver
else # PLATFORM
TOOLS_SUBDIRS = toolsutil javap native2ascii serialver
endif # PLATFORM
endif

ifneq ($(PLATFORM), windows)
HAVE_DGA=true
endif

ifeq ($(HAVE_DGA),true)
DGA_SUBDIR = jdga
endif

#
# corba must always be built before rmi, due to rmi-iiop support
#

ifeq ($(PLATFORM), windows)
SUBDIRS = security jar javazic awt dcpr font jpeg cmm audio net \
	  applet misc corba rmi jdbc beans $(TOOLS_SUBDIRS) jawt text io

LOCAL_MAKE_FLAGS = FULL_VERSION="$(FULL_VERSION)"
else # PLATFORM
SUBDIRS = security jar javazic misc net audio image awt dcpr motif12 motif21 \
          headless $(DGA_SUBDIR) font jpeg cmm applet corba rmi beans \
          jdbc $(TOOLS_SUBDIRS) jawt text io
LOCAL_MAKE_FLAGS = 
endif # PLATFORM

all optimized debug clean clobber ::
	@for i in $(SUBDIRS) ; do \
	    $(ECHO) ">>>Recursively making "$$i" "$@" @ `$(DATE)` ..."; \
	    $(CD) $$i; $(MAKE) $@ RELEASE=$(RELEASE) $(LOCAL_MAKE_FLAGS) || exit 1; $(CD) ..; \
	    $(ECHO) "<<<Finished Recursively making "$$i" "$@" @ `$(DATE)`." ; \
	done

