#
# @(#)Makefile	1.21 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.21 03/01/23
#
# Makefile for building all of java
#

BUILDDIR = ../../../../..

PRODUCT = internal

include $(BUILDDIR)/common/Defs.gmk

# order is important

ifdef STANDALONE_CORBA_WS
SUBDIRS = POA PCosNaming Activation TransactionalPOA TxPOA Interceptors tnameserv
else
SUBDIRS = POA PCosNaming Activation Interceptors tnameserv
endif

all optimized debug clean clobber ::
	@for i in $(SUBDIRS) ; do \
	    $(ECHO) ">>>Recursively making "$$i" "$@" @ `$(DATE)` ..."; \
	    $(CD) $$i; $(MAKE) $@ RELEASE=$(RELEASE) FULL_VERSION=$(FULL_VERSION) \
            || exit 1; $(CD) ..;  \
	    $(ECHO) "<<<Finished Recursively making "$$i" "$@" @ `$(DATE)`." ; \
	done
