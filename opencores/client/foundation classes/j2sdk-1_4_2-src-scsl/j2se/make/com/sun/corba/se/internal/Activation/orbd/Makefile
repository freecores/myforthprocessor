#
# @(#)Makefile	1.9 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../../../../../../..
PACKAGE = com.sun.corba.se.internal.Activation
PRODUCT = Activation
PROGRAM = orbd

include $(BUILDDIR)/common/Defs.gmk

#
# Files
#
include ../FILES_java.gmk

#
# Rules.
#
# default values passed using the -J property
JAVA_ARGS = "{ \"-J-Dcom.sun.CORBA.activation.DbDir=./orb.db\", \"-J-Dcom.sun.CORBA.activation.Port=1049\", \"-J-Dcom.sun.CORBA.POA.ORBServerId=1\",\"com.sun.corba.se.internal.Activation.ORBD\" }"

ifdef STANDALONE_CORBA_WS
FILES_c = $(SHARE_SRC)/native/bin/idlj.c $(SHARE_SRC)/native/bin/utility.c
OTHER_CPPFLAGS = -DMAIN_CLASS=\"com.sun.corba.se.internal.Activation.ORBD\"
endif

include $(BUILDDIR)/common/Program.gmk
