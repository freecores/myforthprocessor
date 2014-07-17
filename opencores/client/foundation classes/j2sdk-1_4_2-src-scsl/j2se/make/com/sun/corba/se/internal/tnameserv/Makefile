#
# @(#)Makefile	1.10 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Build the tnameserv executable.
#

BUILDDIR = ../../../../../..
PACKAGE = com.sun.corba.se.internal.CosNaming
PRODUCT = sun
PROGRAM = tnameserv
include $(BUILDDIR)/common/Defs.gmk

#
# Files.
#
include FILES_java.gmk

#
# Rules.
#
JAVA_ARGS = "{ \"com.sun.corba.se.internal.CosNaming.TransientNameServer\" }"

ifdef STANDALONE_CORBA_WS
FILES_c = $(SHARE_SRC)/native/bin/idlj.c $(SHARE_SRC)/native/bin/utility.c
OTHER_CPPFLAGS = -DMAIN_CLASS=\"com.sun.corba.se.internal.CosNaming.TransientNameServer\"
endif

include $(BUILDDIR)/common/Program.gmk
