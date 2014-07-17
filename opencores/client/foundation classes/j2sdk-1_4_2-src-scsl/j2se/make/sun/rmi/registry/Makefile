#
# @(#)Makefile	1.26 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.26 03/01/23
#
# Makefile for building sun.rmi.registry.* and the "rmiregistry" command
#

BUILDDIR = ../../..
PACKAGE = sun.rmi.registry
PRODUCT = sun
PROGRAM = rmiregistry
include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile.
#
include FILES_java.gmk

#
# Rules.
#
JAVA_ARGS = "{ \"-J-ms8m\", \"sun.rmi.registry.RegistryImpl\" }"
include $(BUILDDIR)/common/Program.gmk

#
# Resources.
#
LOCALE_SET_DEFINITION = j2re
RESOURCE_BUNDLES_PROPERTIES = rmiregistry.properties
RESOURCE_SRC_DIR = $(SHARE_SRC)/classes/sun/rmi/registry/resources
RESOURCE_DEST_DIR = $(CLASSBINDIR)/sun/rmi/registry/resources
include $(BUILDDIR)/common/Resources.gmk
