#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.12 03/01/23
#
# Makefile for building java command line tools
#

BUILDDIR = ../..

PRODUCT = java

include $(BUILDDIR)/common/Defs.gmk

ifeq ($(PLATFORM), windows) 
SUBDIRS = java javaw
else 
SUBDIRS = java
endif 

all optimized debug clean clobber::
	@for i in $(SUBDIRS) ; do \
	    $(ECHO) ">>>Recursively making "$$i" "$@" @ `$(DATE)` ..."; \
	    $(CD) $$i; $(MAKE) $@ RELEASE=$(RELEASE) FULL_VERSION=$(FULL_VERSION)\
            || exit 1; $(CD) ..;  \
	    $(ECHO) "<<<Finished Recursively making "$$i" "$@" @ `$(DATE)`." ; \
	done
