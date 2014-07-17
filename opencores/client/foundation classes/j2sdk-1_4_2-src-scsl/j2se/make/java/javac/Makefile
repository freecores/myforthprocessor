#
# @(#)Makefile	1.14 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.14 03/01/23
#
# An alias for sun/javac/GNUmakefile
#
BUILDDIR=../..

PRODUCT=java

include $(BUILDDIR)/common/Defs.gmk

all optimized debug clean clobber classes ::
	@$(ECHO) ">>>Recursively making ../../sun/javac "$@" @ `$(DATE)` ...";
	$(CD) ../../sun/javac ; $(MAKE) $@ RELEASE=$(RELEASE) FULL_VERSION=$(FULL_VERSION)
	@$(ECHO) "<<<Finished Recursively making ../../sun/javac "$@" @ `$(DATE)` ...";
