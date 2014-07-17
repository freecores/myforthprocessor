#
# @(#)Makefile	1.39 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.39 03/01/23
#
# Makefile for building the jfc demos
#

BUILDDIR = ../..
PRODUCT = java
include $(BUILDDIR)/common/Defs.gmk

SWING_DEMOS = SwingSet2 FileChooserDemo Metalworks \
          Notepad Stylepad SampleTree SwingApplet TableExample

OTHER_DEMOS = Java2D Font2DTest

SUBDIRS = $(SWING_DEMOS) $(OTHER_DEMOS)

all optimized debug clean clobber ::
	@for i in $(SUBDIRS) ; do \
		$(ECHO) ">>>Recursively making "$$i" "$@" @ `$(DATE)` ..."; \
		$(CD) $$i; $(MAKE) $@ \
		|| exit 1; $(CD) ..;  \
		$(ECHO) "<<<Finished Recursively making "$$i" "$@" @ `$(DATE)`." ; \
	done

swing-demo ::
	@for i in $(SWING_DEMOS) ; do \
		$(ECHO) ">>>Recursively making "$$i" "$@" @ `$(DATE)` ..."; \
		$(CD) $$i; $(MAKE) $@ SWING_DEMODIR=demo/jfc \
		|| exit 1; $(CD) ..;  \
		$(ECHO) "<<<Finished Recursively making "$$i" "$@" @ `$(DATE)`." ; \
	done
