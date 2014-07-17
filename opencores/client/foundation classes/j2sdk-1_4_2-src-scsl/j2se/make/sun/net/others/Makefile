#
# @(#)Makefile	1.5 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../../..
PACKAGE = sun.net
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files
#
include $(BUILDDIR)/sun/net/FILES_java.gmk

#
# Rules
#
include $(BUILDDIR)/common/Classes.gmk

#
# Extra things we want to do.
#
OPTIMIZED += lib
DEBUG += lib
clean:: lib.clean

#
# Install/nuke properties.
#
PROPS = $(PLATFORM_SRC)/lib/content-types.properties

lib: $(TMPDIR)/.lib.dirs
	$(RM) -r $(LIBDIR)/content-types.properties
	$(CP) $(PROPS) $(LIBDIR)/content-types.properties

lib.clean:
	$(RM) $(OPT_OBJDIR)/.lib.* $(DBG_OBJDIR)/.lib.* 
	$(RM) -r $(LIBDIR)/content-types.properties

$(TMPDIR)/.lib.dirs:
	$(MKDIR) -p $(LIBDIR)
	$(TOUCH) $(TMPDIR)/.lib.dirs
