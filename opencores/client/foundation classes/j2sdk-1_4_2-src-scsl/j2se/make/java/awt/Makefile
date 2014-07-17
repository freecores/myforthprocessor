#
# @(#)Makefile	1.77 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../..
PACKAGE = java.awt
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Rules
#
all: sources classes properties cursors

#
# Files
#
include FILES_java.gmk

#
# Resources
#
RESOURCE_BUNDLES_COMPILED_PROPERTIES = sun/awt/resources/awt.properties
LOCALE_SET_DEFINITION = j2re
RESOURCE_SRC_DIR = $(SHARE_SRC)/classes
include $(BUILDDIR)/common/Resources.gmk

#
# Rules
#
include $(BUILDDIR)/common/Classes.gmk

sources: debughelper
#
# Extra things we want to do.
#
OPTIMIZED += properties cursors
DEBUG += properties cursors
clean:: debughelper.clean properties.clean sun.geom.clean cursors.clean

debughelper:
	$(CD) $(BUILDDIR)/sun/awt; $(MAKE) debughelper

debughelper.clean:
	$(CD) $(BUILDDIR)/sun/awt; $(MAKE) debughelper.clean


_LIBPROPS = \
	flavormap.properties

LIBPROPS = $(_LIBPROPS:%=$(LIBDIR)/%)

properties: $(LIBDIR) $(LIBPROPS)

$(LIBDIR)/%.properties: $(PLATFORM_SRC)/lib/%.properties
	$(install-file)

properties.clean :
	$(RM) $(LIBPROPS)

.SUFFIXES : $(SUFFIXES) .gif .properties

CURSORSRC     = $(TOPDIR)/src/share/lib/images/cursors
CURSORDIR     = $(LIBDIR)/images/cursors

ifeq ($(PLATFORM), windows)
CURSORPROPSRC = $(TOPDIR)/src/windows/lib/images/cursors
CURSORS   = $(CURSORDIR)/win32_CopyDrop32x32.gif      \
            $(CURSORDIR)/win32_MoveDrop32x32.gif      \
            $(CURSORDIR)/win32_LinkDrop32x32.gif      \
            $(CURSORDIR)/win32_CopyNoDrop32x32.gif    \
            $(CURSORDIR)/win32_MoveNoDrop32x32.gif    \
            $(CURSORDIR)/win32_LinkNoDrop32x32.gif    \
            $(CURSORDIR)/invalid32x32.gif
else # PLATFORM
CURSORPROPSRC = $(TOPDIR)/src/solaris/lib/images/cursors
CURSORS   = $(CURSORDIR)/motif_CopyDrop32x32.gif	\
	    $(CURSORDIR)/motif_MoveDrop32x32.gif	\
	    $(CURSORDIR)/motif_LinkDrop32x32.gif	\
	    $(CURSORDIR)/motif_CopyNoDrop32x32.gif	\
	    $(CURSORDIR)/motif_MoveNoDrop32x32.gif	\
	    $(CURSORDIR)/motif_LinkNoDrop32x32.gif	\
	    $(CURSORDIR)/invalid32x32.gif
endif # PLATFORM

$(CURSORDIR) :
	$(MKDIR) -p $(CURSORDIR)

$(CURSORDIR)/cursors.properties : $(CURSORPROPSRC)/cursors.properties
	@$(RM) $@
	$(CP) $< $@

$(CURSORDIR)/%.gif : $(CURSORSRC)/%.gif
	$(CP) $< $@

cursors : $(CURSORDIR) $(CURSORDIR)/cursors.properties $(CURSORS)

sun.geom.clean :
	$(RM) -r $(CLASSBINDIR)/sun/awt/geom

cursors.clean :
	$(RM) -r $(CURSORDIR)

.PHONY: sources debughelper debughelper.clean properties properties.clean \
	cursors sun.geom.clean cursors.clean

