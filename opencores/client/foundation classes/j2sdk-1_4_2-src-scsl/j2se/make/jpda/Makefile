#
# @(#)Makefile	1.51 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.51 03/01/23
#
# Makefile for building all of JPDA, including JDWP, JDI, and
# example debuggers. See also included files.
#

BUILDDIR = ..

include $(BUILDDIR)/common/Defs.gmk

#
# Core.
#
SUBDIRS    = jdwpgen jdwp back front transport expr tty bdi gui
all release optimized debug clean clobber:: 
	@for i in $(SUBDIRS) ; do \
	    $(ECHO) ">>>Recursively making "$$i" "$@" @ `$(DATE)` ..."; \
	    $(CD) $$i; $(MAKE) $@ RELEASE=$(RELEASE) FULL_VERSION=$(FULL_VERSION) \
            || exit 1; $(CD) ..; \
	    $(ECHO) "<<<Finished Recursively making "$$i" "$@" @ `$(DATE)`." ; \
	done

#
# Release engineering targets.
#
#     include $(BUILDDIR)/common/Release.gmk

#
# Documentation targets "releasedocs" and "alldocs"
#
#     include $(BUILDDIR)/common/Docs.gmk

#
# Phony targets
.PHONY: all optimized debug clean clobber 
