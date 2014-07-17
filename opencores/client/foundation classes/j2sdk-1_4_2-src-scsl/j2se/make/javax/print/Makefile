#
# @(#)Makefile	1.15 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building all of JPS (Java Print Service) API
#

BUILDDIR = ../..
PACKAGE = javax.print
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile
#
include FILES_java.gmk

#
# Resources
#

LOCALE_SET_DEFINITION = j2re
RESOURCE_BUNDLES_COMPILED_PROPERTIES = sun/print/resources/serviceui.properties
RESOURCE_SRC_DIR = $(SHARE_SRC)/classes
include $(BUILDDIR)/common/Resources.gmk

#
# Rules
#
include $(BUILDDIR)/common/Classes.gmk

#
# Rules that copy files.
#
OPTIMIZED += copy-files properties
DEBUG += copy-files properties

_PSFONTPROPS = \
	psfontj2d.properties            \
	psfont.properties.ja            \

PSFONTPROPS_SRC = $(SHARE_SRC)/classes/sun/print
PSFONTPROPS       = $(_PSFONTPROPS:%=$(LIBDIR)/%)

_RESOURCES_PRINT = \
	orientLandscape.gif		\
	orientPortrait.gif		\
	orientRevLandscape.gif		\
	orientRevPortrait.gif		\
	oneside.gif			\
	duplex.gif			\
	tumble.gif


RESDIR_PRINT    = $(CLASSBINDIR)/sun/print/resources
RESOURCES_PRINT = $(_RESOURCES_PRINT:%=$(RESDIR_PRINT)/%)

$(LIBDIR)/%: $(PSFONTPROPS_SRC)/% $(FONTPROPS)
	$(install-file)

$(RESDIR_PRINT)/%: $(SHARE_SRC)/classes/sun/print/resources/%
	$(install-file)

properties: $(LIBDIR) $(PSFONTPROPS) $(RESDIR_PRINT) $(RESOURCES_PRINT)

properties.clean:
	$(RM) $(PSFONTPROPS)
	$(RM) -r $(RESDIR_PRINT)

$(LIB_LOCATION) $(RESDIR_PRINT):
	$(MKDIR) -p $@

copy-files: $(FILES_copy)

$(SERVICEDIR)/%: $(PLATFORM_SRC)/classes/sun/print/services/%
	$(install-file)

#
# Clean up internal-use-only package.
#
clean::
	$(RM) -r $(CLASSDESTDIR)/javax/print $(CLASSDESTDIR)/sun/print
