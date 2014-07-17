#
# @(#)Makefile	1.24 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.24 03/01/23
#
# Makefile for building all the demos
#

BUILDDIR = ..

PRODUCT = java

include $(BUILDDIR)/common/Defs.gmk

# sound removed until 4401488 is fixed. - neal.gafter@eng 2000-01-03
# SUBDIRS = applets jfc sound
SUBDIRS = applets jfc

# Solaris has one more demo...

# jni removed until 4401489 is fixed. - neal.gafter@eng 2000-01-03
ifeq ($(PLATFORM), solaris)
 SUBDIRS += jni
endif

all optimized debug clean clobber ::
	@for i in $(SUBDIRS) ; do \
	    $(ECHO) ">>>Recursively making "$$i" "$@" @ `$(DATE)` ..."; \
	    $(CD) $$i; $(MAKE) $@ RELEASE=$(RELEASE) FULL_VERSION=$(FULL_VERSION) \
            || exit 1; $(CD) ..;  \
	    $(ECHO) "<<<Finished Recursively making "$$i" "$@" @ `$(DATE)` ..."; \
	done

clobber::
	$(RM) -r $(DEMODIR)
