#
# @(#)Makefile	1.21 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building all of java
#

BUILDDIR = ../../..

PACKAGE = com.sun.tools.corba.se.idl
PRODUCT = sun
PROGRAM = idlj

include $(BUILDDIR)/common/Defs.gmk

#
# Files
#
ifdef STANDALONE_CORBA_WS
FILES_c = $(SHARE_SRC)/native/bin/$(PROGRAM).c $(SHARE_SRC)/native/bin/utility.c
endif

include FILES_java.gmk

JAVA_ARGS = "{ \"com.sun.tools.corba.se.idl.toJavaPortable.Compile\" }"

#
# Rules.
#
include $(BUILDDIR)/common/Program.gmk

#
# Resources.
#

LOCALE_SET_DEFINITION = j2sdk
RESOURCE_BUNDLES_PROPERTIES = idl.prp toJavaPortable/toJavaPortable.prp
RESOURCE_SRC_DIR = $(SHARE_SRC)/classes/com/sun/tools/corba/se/idl
RESOURCE_DEST_DIR = $(CLASSBINDIR)/com/sun/tools/corba/se/idl
include $(BUILDDIR)/common/Resources.gmk

#
# IDL files
#

IDL_FILES = orb.idl ir.idl

IDL_SRC_DIR  = $(SHARE_SRC)/classes/com/sun/tools/corba/se/idl
IDL_DEST_DIR = $(LIBDIR)

IFILES = $(IDL_FILES:%=$(IDL_DEST_DIR)/%)

OPTIMIZED += $(IFILES)
DEBUG     += $(IFILES)

$(IDL_DEST_DIR)/%: $(IDL_SRC_DIR)/%
	$(install-file)

