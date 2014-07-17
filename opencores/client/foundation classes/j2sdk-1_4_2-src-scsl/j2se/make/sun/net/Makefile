#
# @(#)Makefile	1.53 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../..
PACKAGE = sun.net
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

SUBDIRS = others spi

#
# Files
#
include FILES_java.gmk

all optimized debug clean clobber :: 
	@for i in $(SUBDIRS) ; do \
	    $(ECHO) ">>>Recursively making "$$i" "$@" @ `$(DATE)` ..."; \
	    $(CD) $$i; $(MAKE) $@ RELEASE=$(RELEASE) $(LOCAL_MAKE_FLAGS) || exit 1; $(CD) ..; \
	    $(ECHO) "<<<Finished Recursively making "$$i" "$@" @ `$(DATE)`." ; \
	done

