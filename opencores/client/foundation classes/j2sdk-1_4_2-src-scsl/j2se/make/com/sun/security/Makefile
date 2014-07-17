#
# @(#)Makefile	1.6 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

# 
# @(#)Makefile	1.6 03/01/23 
# 
# Makefile for com.sun.security packages.

BUILDDIR = ../../..
include $(BUILDDIR)/common/Defs.gmk

# Note:  com.sun.security.sasl.* is built by make/com/sun/jndi/ldapsec/Makefile
SUBDIRS = jgss auth auth/module

all optimized debug clean clobber::
	@for i in $(SUBDIRS) ; do \
	    $(ECHO) ">>>Recursively making "$$i" "$@" @ `$(DATE)` ..."; \
	    $(CD) $$i; $(MAKE) $@ RELEASE=$(RELEASE) || exit 1; $(CD) ..; \
	    $(ECHO) "<<<Finished Recursively making "$$i" "$@" @ `$(DATE)`." ; \
	done

clean clobber::
	$(RM) -r $(CLASSDESTDIR)/com/sun/security
