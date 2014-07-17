#
# @(#)Makefile	1.21 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.21 03/01/23
#
# Makefile for building javax
#

BUILDDIR = ..
include $(BUILDDIR)/common/Defs.gmk

# imageio uses xml, so build it last

SUBDIRS = accessibility naming print swing security sound xa sql xml imageio

all optimized debug clean clobber ::
	@for i in $(SUBDIRS) ; do \
	    $(ECHO) ">>>Recursively making "$$i" "$@" @ `$(DATE)` ..."; \
	    $(CD) $$i; $(MAKE) $@ RELEASE=$(RELEASE) FULL_VERSION=$(FULL_VERSION) \
            || exit 1; $(CD) ..;  \
	    $(ECHO) "<<<Finished Recursively making "$$i" "$@" @ `$(DATE)`." ; \
	done

