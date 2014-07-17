#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.4 03/01/23
#

#
# @(#)Makefile	1.11 01/10/09
#
# Makefile for building klist utility.
#

BUILDDIR = ../../..
PACKAGE = sun.tools.security
PRODUCT = sun
PROGRAM = klist
include $(BUILDDIR)/common/Defs.gmk

#
# Class to launch tools with.
#
JAVA_ARGS = "{ \"-J-ms8m\", \"sun.security.krb5.internal.tools.Klist\" }"

#
# Rules.
#
include $(BUILDDIR)/common/Program.gmk
