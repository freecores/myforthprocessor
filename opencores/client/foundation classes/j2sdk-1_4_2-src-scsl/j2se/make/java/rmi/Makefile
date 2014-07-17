#
# @(#)Makefile	1.24 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.24 03/01/23
#
# Makefile for building java.rmi.*
#

BUILDDIR = ../..

PACKAGE = java.rmi
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

default: all

include FILES_java.gmk

include $(BUILDDIR)/common/Classes.gmk


all: classes

clean clobber :: classes.clean
