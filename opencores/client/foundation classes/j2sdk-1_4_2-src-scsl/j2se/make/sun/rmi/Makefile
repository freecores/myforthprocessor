#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.36 03/01/23
#
# Build rmi tools, and sun.rmi.* implementation classes

BUILDDIR = ../..
include $(BUILDDIR)/common/Defs.gmk

ifdef J2RE_ONLY
TOOLS_SUBDIRS = 
else
TOOLS_SUBDIRS = rmic
endif

#
# Order is important -- you can't build stubs without building rmic first.
#
ifdef STANDALONE_CORBA_WS
SUBDIRS = rmic
else
SUBDIRS = $(TOOLS_SUBDIRS) rmi registry rmid

ifeq ($(PLATFORM), windows)
SUBDIRS += cgi
endif # PLATFORM
endif # STANDALONE_CORBA_WS

all optimized debug clean clobber ::
	@for i in $(SUBDIRS) ; do \
	    $(ECHO) ">>>Recursively making "$$i" "$@" @ `$(DATE)` ..."; \
	    $(CD) $$i; $(MAKE) $@ RELEASE=$(RELEASE) || exit 1; $(CD) ..; \
	    $(ECHO) "<<<Finished Recursively making "$$i" "$@" @ `$(DATE)`." ; \
	done

clean::
	$(RM) -r $(CLASSBINDIR)/sun/rmi $(CLASSBINDIR)/com/sun/rmi
