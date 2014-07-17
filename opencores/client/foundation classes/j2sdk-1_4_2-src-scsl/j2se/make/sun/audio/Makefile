#
# @(#)Makefile	1.47 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building sun.audio
#

BUILDDIR = ../..
PACKAGE = sun.audio
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile
#
include FILES_java.gmk

#
# Rules
#
include $(BUILDDIR)/common/Classes.gmk

clean::
	$(RM) -r $(CLASSDESTDIR)/sun/audio
