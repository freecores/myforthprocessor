#
# @(#)Makefile	1.14 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

# @(#)Makefile	1.14 03/01/23
#
# Makefile for building the generic and vis versions of medialib
#

BUILDDIR = ../..
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

SUBDIRS = generic

# solaris-sparc and solaris-sparcv9 both build 'vis'
ifeq ($(PLATFORM),solaris)
  ifeq ($(ARCH),sparc)
    SUBDIRS += vis
  endif
  ifeq ($(ARCH),sparcv9)
    SUBDIRS += vis
  endif
endif

# libsunmath available only on solaris-sparc platforms
ifneq ($(ARCH), sparc)
  ifneq ($(ARCH), sparcv9)
    OTHER_CFLAGS += -DMLIB_NO_LIBSUNMATH
  endif
endif


all optimized debug clean clobber::
	@for i in $(SUBDIRS) ; do \
	    $(ECHO) ">>>Recursively making "$$i" "$@" @ `$(DATE)` ..."; \
	    $(CD) $$i; $(MAKE) $@ RELEASE=$(RELEASE) || exit 1; $(CD) ..; \
	    $(ECHO) "<<<Finished Recursively making "$$i" "$@" @ `$(DATE)`." ; \
	done
