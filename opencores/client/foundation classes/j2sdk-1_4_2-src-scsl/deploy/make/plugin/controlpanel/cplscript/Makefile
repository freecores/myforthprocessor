#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.7 03/01/23
#
#
# Makefile to build control panel subdirectories
#

PLUGIN_TOP = ../../../..
PACKAGE = ControlPanel

include $(PLUGIN_TOP)/make/plugin/common/config.gmk

DESKTOPDIR = $(OUTPUTDIR)/plugin/desktop
CPANELSRCDIR = $(PLUGIN_TOP)/src/plugin/src/solaris/controlpanel
SCRIPTFILE = $(OUTPUTDIR)/bin/ControlPanel
HTMLFILE = $(OUTPUTDIR)/ControlPanel.html
PNGFILE = $(DESKTOPDIR)/sun_java.png
DESKTOPFILE = $(DESKTOPDIR)/sun_java.desktop

PVER="PLUGIN_VERSION=$(JDK_VERSION)"
PNDVER="PLUGIN_NODOTVERSION=$(JDK_MAJOR_VERSION)$(JDK_MINOR_VERSION)$(JDK_MICRO_VERSION)$(UVSUFFIX)"

all: $(OBJDIR) $(DESKTOPDIR) $(SCRIPTFILE) $(HTMLFILE) $(PNGFILE) $(DESKTOPFILE)

$(OBJDIR) $(DESKTOPDIR):
	$(MKDIR) -p $@

$(SCRIPTFILE): $(CPANELSRCDIR)/ControlPanel
	printf "$(PVER) \n$(PNDVER)\n" > $(OBJDIR)/vertmp
	$(SED) -e '/REPLACEME/r $(OBJDIR)/vertmp' -e '/REPLACEME/d' < $< > $@
	$(CHMOD) a+x $@

$(HTMLFILE): $(CPANELSRCDIR)/ControlPanel.html
	printf "<EMBED type=application/x-java-applet;jpi-version=%s\n" $(JDK_VERSION) > $(OBJDIR)/mimetmp
	$(SED) -e '/REPLACEME/r $(OBJDIR)/mimetmp' -e '/REPLACEME/d' < $< > $@

$(PNGFILE): $(CPANELSRCDIR)/sun_java.png
	$(CP) $< $@

$(DESKTOPFILE): $(CPANELSRCDIR)/sun_java.desktop
	$(CP) $< $@

clobber:: 
	$(RM) $(SCRIPTFILE) $(HTMLFILE) $(PNGFILE) $(DESKTOPFILE)
	$(RM) -rf $(OBJDIR)
	$(RM) -rf $(DESKTOPDIR)
#
# Phonies to avoid accidents.
#
.PHONY: all optimized debug clobber  
