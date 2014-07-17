#
# @(#)Makefile	1.25 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

# @(#)Makefile	1.25 03/01/23

BUILDDIR = ../../../../../..
PACKAGE = com.sun.corba.se.internal.Activation
PRODUCT = Activation

include $(BUILDDIR)/common/Defs.gmk

SUBDIRS = orbd servertool

all optimized debug clean clobber ::
	@for i in $(SUBDIRS) ; do \
		$(ECHO) ">>>Recursively making "$$i" "$@" @ `$(DATE)` ..."; \
		$(CD) $$i; $(MAKE) $@ RELEASE=$(RELEASE) FULL_VERSION=$(FULL_VERSION) \
		|| exit 1; $(CD) ..;  \
		$(ECHO) "<<<Finished Recursively making "$$i" "$@" @ `$(DATE)`." ; \
	done
