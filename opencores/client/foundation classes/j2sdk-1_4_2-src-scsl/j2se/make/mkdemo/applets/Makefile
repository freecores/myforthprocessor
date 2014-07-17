#
# @(#)Makefile	1.14 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.14 03/01/23
#
# Makefile for building the demo applets
#

BUILDDIR = ../..
include $(BUILDDIR)/common/Defs.gmk

SUBDIRS = Animator ArcTest BarChart Blink CardTest Clock DitherTest DrawTest \
          Fractal GraphLayout GraphicsTest ImageMap JumpingBox MoleculeViewer \
          NervousText SimpleGraph SortDemo SpreadSheet TicTacToe WireFrame


all optimized debug clean clobber::
	@for i in $(SUBDIRS) ; do \
	    $(ECHO) ">>>Recursively making "$$i" "$@" @ `$(DATE)` ..."; \
	    $(CD) $$i; $(MAKE) $@ RELEASE=$(RELEASE) FULL_VERSION=$(FULL_VERSION)\
            || exit 1; $(CD) ..;  \
	    $(ECHO) "<<<Finished Recursively making "$$i" "$@" @ `$(DATE)`." ; \
	done

clobber clean::
	$(RM) -r $(DEMODIR)/applets
