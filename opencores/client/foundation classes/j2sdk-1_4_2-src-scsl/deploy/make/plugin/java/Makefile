#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.16 03/01/23
#
# Makefile to build the Java Plug-in components
#

PLUGIN_TOP = ../../..

include $(PLUGIN_TOP)/make/plugin/common/config.gmk

include ./FileList.gmk 

VPATH.java = $(PLUGIN_PLATFORM_SRC)/classes$(CLASSPATH_SEPARATOR)$(JAVASRCDIR)
JAVAC_CP	= "$(CLASSBINDIR)$(CLASSPATH_SEPARATOR)$(CLASSDIR)"
JAVAC		= $(BINDIR)/javac
JAR		= $(BINDIR)/jar

ifeq ($(VARIANT),DBG)
  JAVAC += -g -deprecation
else
  JAVAC += -deprecation
endif

CLASSDESTDIR = $(CLASSDIR)
$(OBJDIR)/.class.headers.$(ARCH): CLASSDESTDIR := "$(CLASSBINDIR)$(CLASSPATH_SEPARATOR)$(CLASSDIR)"

all: optimized debug

include $(J2SE_TOPDIR)/make/common/Rules.gmk

DEBUG = $(PLUGIN_JARFILE_g)
OPTIMIZED = $(PLUGIN_JARFILE)

$(CLASSDESTDIR)/%.class: $(PLUGIN_PLATFORM_SRC)/classes/%.java
	@$(RM) $@
	@$(ECHO) $? >> $(TMPDIR)/.classes.list

$(CLASSDESTDIR)/%.class: $(JAVASRCDIR)/%.java
	@$(RM) $@
	@$(ECHO) $? >> $(TMPDIR)/.classes.list

ifeq ($(VARIANT), DBG)
$(PLUGIN_JARFILE): classes
else
$(PLUGIN_JARFILE): classheaders
endif
	@if [ -s $(TMPDIR)/.classes.list ] ; \
        then \
            $(ECHO) Rebuilding: $(PLUGIN_JARFILE) ;\
            $(CP) -f $(JAVASRCDIR)/sun/plugin/util/*.gif $(CLASSDIR)/sun/plugin/util; \
            $(CP) -f $(JAVASRCDIR)/sun/plugin/util/*.jpg $(CLASSDIR)/sun/plugin/util; \
            $(CP) -f $(JAVASRCDIR)/sun/plugin/panel/*.gif $(CLASSDIR)/sun/plugin/panel; \
            $(MKDIR) -p $(dir $(PLUGIN_JARFILE)); \
            cd $(CLASSDIR); $(JAR) cf0 $(PLUGIN_JARFILE) *; \
        fi

clobber:: 
	$(RM) -r $(CLASSDIR) $(CLASSDIR_g)
	$(RM) $(PLUGIN_JARFILE) $(PLUGIN_JARFILE_g)
