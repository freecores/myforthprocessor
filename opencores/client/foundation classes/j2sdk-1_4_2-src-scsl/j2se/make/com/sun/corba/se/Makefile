#
# @(#)Makefile	1.22 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.22 03/01/23
#
# Makefile for building all of java
#

BUILDDIR = ../../../..

PRODUCT = se

include $(BUILDDIR)/common/Defs.gmk

# IDL code generation is moved out to an extra step before the
# actual classes are compiled
# the files should be compiled in the following order, core to be built
# before connection

ifdef STANDALONE_CORBA_WS
SUBDIRS = sources ActivationIDL CosTransactions core connection extension interceptor internal
else
SUBDIRS = sources ActivationIDL core connection extension interceptor internal
endif

all optimized debug clean clobber ::
	@for i in $(SUBDIRS) ; do \
	    $(ECHO) ">>>Recursively making "$$i" "$@" @ `$(DATE)` ..."; \
	    $(CD) $$i; $(MAKE) $@ RELEASE=$(RELEASE) FULL_VERSION=$(FULL_VERSION) \
            || exit 1; $(CD) ..;  \
	    $(ECHO) "<<<Finished Recursively making "$$i" "$@" @ `$(DATE)`." ; \
	done
