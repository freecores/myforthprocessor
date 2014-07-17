#
# @(#)Makefile	1.12 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.12 03/01/23
#
# Makefile for re-building any classes that might have been built with
# a bootstrap compiler.
#

BUILDDIR = ../../..
include $(BUILDDIR)/common/Defs.gmk

SUBDIRS  = library new-javac

all optimized debug clean clobber::
	@for i in $(SUBDIRS) ; do \
	    $(ECHO) ">>>Recursively making "$$i" "$@" @ `$(DATE)` ..."; \
	    $(CD) $$i; $(MAKE) $@ RELEASE=$(RELEASE) FULL_VERSION=$(FULL_VERSION)\
            || exit 1; $(CD) ..;  \
	    $(ECHO) "<<<Finished Recursively making "$$i" "$@" @ `$(DATE)`." ; \
	done
