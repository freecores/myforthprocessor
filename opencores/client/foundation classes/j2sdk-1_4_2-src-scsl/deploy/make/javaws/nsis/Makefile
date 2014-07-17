#
# @(#)Makefile	1.10 03/01/23
# 
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

# Nullsoft main makefile
BUILDDIR = ..
JAVAWS_TOPDIR = ../..
include ../common/Defs.gmk

override SUBDIRS = exehead makenssi crunch uncrunch

all optimized debug clean::
	@for i in $(SUBDIRS) ; do \
	    $(ECHO) ">>>Recursively making "$$i" "$@" @ `$(DATE)` ..."; \
	    $(CD) $$i; $(MAKE) $(MAKEOVERRIDES) $@ || exit 1; $(CD) ..; \
	    $(ECHO) "<<<Finished Recursively making "$$i" "$@" @ `$(DATE)`."; \
	done

.PHONY: all clean optimized debug
