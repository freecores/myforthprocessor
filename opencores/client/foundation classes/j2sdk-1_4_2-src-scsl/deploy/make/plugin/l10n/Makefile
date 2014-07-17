#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.3 03/01/23
#
#
# Makefile to build control panel subdirectories
#

PLUGIN_TOP = ../../..
PACKAGE = l10n
RESOURCESRCDIR= $(PLUGIN_TOP)/src/plugin/oji-plugin/src/motif/resources

include $(PLUGIN_TOP)/make/plugin/common/config.gmk

include ./FileList.gmk

all: $(L10N_MESSAGES)

$(OUTPUTDIR)/lib/locale/%/LC_MESSAGES/sunw_java_plugin.mo : $(RESOURCESRCDIR)/%/sunw_java_plugin.po
	@$(MKDIR) -p $(@D)
	msgfmt -o $(@) $<


clobber:: 
	$(RM) -f $(L10N_MESSAGES)
#
# Phonies to avoid accidents.
#
.PHONY: all optimized debug clobber  
