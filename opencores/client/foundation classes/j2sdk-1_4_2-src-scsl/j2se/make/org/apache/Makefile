#
# @(#)Makefile	1.10 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.10 03/01/23
#
# Makefile for building org.apache code
#

BUILDDIR = ../..
include $(BUILDDIR)/common/Defs.gmk

SUBDIRS = crimson xalan xml xpath

all optimized debug clean clobber::
	@for i in $(SUBDIRS) ; do \
	    $(ECHO) ">>>Recursively making "$$i" "$@" @ `$(DATE)` ..."; \
	    $(CD) $$i; $(MAKE) $@ RELEASE=$(RELEASE) || exit 1; $(CD) ..; \
	    $(ECHO) "<<<Finished Recursively making "$$i" "$@" @ `$(DATE)`." ; \
	done
