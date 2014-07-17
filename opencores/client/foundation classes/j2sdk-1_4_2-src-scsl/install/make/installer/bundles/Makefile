#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
#
#  @(#)Makefile	1.4 03/01/23
#
#
# Most frequently used targets:
#
#    all            -- builds
#    clobber        -- clobber the builds
#
BUILDDIR=../..

include $(BUILDDIR)/common/Defs.gmk

all:
	if [ -r $(PLATFORM) ]; then \
	   ( $(CD) $(PLATFORM) ; $(MAKE) all ); \
	fi

clean::
	if [ -r $(PLATFORM) ]; then \
	   ( $(CD) $(PLATFORM) ; $(MAKE) clean ); \
	fi

#
# Phonies to avoid accidents.
#
.PHONY: all clean 
