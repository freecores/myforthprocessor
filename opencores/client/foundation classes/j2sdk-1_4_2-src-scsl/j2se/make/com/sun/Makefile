#
# @(#)Makefile	1.35 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.35 03/01/23
#
# Makefile for building com/sun
#

BUILDDIR = ../..
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

ifdef J2RE_ONLY
TOOLS_SUBDIRS =
else
TOOLS_SUBDIRS = javah extcheck javadoc
endif

SUBDIRS = image security jndi corba java inputmethods java2d $(TOOLS_SUBDIRS) 

all optimized debug clean clobber ::
	@for i in $(SUBDIRS) ; do \
	    $(ECHO) ">>>Recursively making "$$i" "$@" @ `$(DATE)` ..."; \
	    $(CD) $$i; $(MAKE) $@ RELEASE=$(RELEASE) FULL_VERSION=$(FULL_VERSION) \
            || exit 1; $(CD) ..;  \
	    $(ECHO) "<<<Finished Recursively making "$$i" "$@" @ `$(DATE)`." ; \
	done

