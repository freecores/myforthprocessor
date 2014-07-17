#
# @(#)Makefile	1.41 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building sun.rmi.rmic.* and the "rmic" command
#

BUILDDIR = ../../..
PACKAGE = sun.rmi.rmic
PRODUCT = sun
PROGRAM = rmic
include $(BUILDDIR)/common/Defs.gmk

all: classes

#
# Files to compile.
#

include FILES.gmk

ifdef PARTNERS_CORBA
FILES_java = \
	$(RMIC_java) \
	$(IIOP_java) \
	$(PARTNERS_EDITION_java)
else
FILES_java = \
	$(RMIC_java) \
	$(IIOP_java)
endif

#
# Rules.
#
JAVA_ARGS = "{ \"-J-ms8m\", \"sun.rmi.rmic.Main\" }"

ifdef STANDALONE_CORBA_WS
FILES_c = $(SHARE_SRC)/native/bin/$(PROGRAM).c $(SHARE_SRC)/native/bin/utility.c
endif

include $(BUILDDIR)/common/Program.gmk

#
# Resources.
#
LOCALE_SET_DEFINITION = j2sdk
RESOURCE_BUNDLES_PROPERTIES = rmic.properties
RESOURCE_SRC_DIR  = $(SHARE_SRC)/classes/sun/rmi/rmic/resources
RESOURCE_DEST_DIR = $(CLASSBINDIR)/sun/rmi/rmic/resources
include $(BUILDDIR)/common/Resources.gmk
