#
# @(#)Makefile	1.6 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building all of JNDI core
#

BUILDDIR = ../..
PACKAGE = javax.naming
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

#
# Clean up internal-use-only package.
#
clean::
	$(RM) -r $(CLASSDESTDIR)/com/sun/naming
