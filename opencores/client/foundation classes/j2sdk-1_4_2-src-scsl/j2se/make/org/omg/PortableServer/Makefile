#
# @(#)Makefile	1.25 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../../..

PACKAGE = org.omg.PortableServer
PRODUCT = org

include $(BUILDDIR)/common/Defs.gmk

#
# Files
#

include ../PortableServer_java.gmk

# order of building files is important

FILES_java = $(PortableServer_java) $(POA_GENERATED_java)

include $(BUILDDIR)/common/Classes.gmk
