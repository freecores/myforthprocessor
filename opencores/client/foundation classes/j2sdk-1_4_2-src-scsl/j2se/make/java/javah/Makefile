#
# @(#)Makefile	1.44 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.44 03/01/23
#
# An alias for the real javah build.
#

BUILDDIR=../..

PRODUCT=java

include $(BUILDDIR)/common/Defs.gmk

REDIRECT = ../../com/sun/javah

all optimized debug clean clobber classes ::
	@$(ECHO) ">>>Recursively making $(REDIRECT) "$@" @ `$(DATE)` ...";
	$(CD) $(REDIRECT); $(MAKE) $@ RELEASE=$(RELEASE) FULL_VERSION=$(FULL_VERSION)
	@$(ECHO) "<<<Finished Recursively making $(REDIRECT) "$@" @ `$(DATE)` ...";

