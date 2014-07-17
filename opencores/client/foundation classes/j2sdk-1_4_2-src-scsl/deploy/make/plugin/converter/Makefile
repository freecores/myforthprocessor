#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.19 03/01/23
#
# Makefile to build the Java Plug-in components
#

PLUGIN_TOP = ../../..
EXT_TOPDIR = 		../../..
BUILDDIR = 		$(J2SE_TOPDIR)/make

ifeq ($(VARIANT), DBG)
DEBUG_CLASSFILES = true
endif

include $(PLUGIN_TOP)/make/plugin/common/config.gmk

ifeq ($(PLATFORM), windows)
PLATFORM_DIR = win32
else
PLATFORM_DIR = solaris
endif

PROGRAM_NAME = HtmlConverter

CLASSDESTDIR = 		$(TMPDIR)/classes$(SUFFIX)
OBJDIR = 		$(TMPDIR)/obj$(SUFFIX)

CONVERTER_SRC =		$(EXT_TOPDIR)/src/plugin/src
LAUNCHER_SRC =		$(EXT_TOPDIR)/src/plugin/src/win32/converter
LAUNCHER_EXE =		$(OBJDIR)/$(PROGRAM_NAME)$(SUFFIX)$(EXE_SUFFIX)

CONVERTER_JAR = 	$(LIBDIR)/htmlconverter$(SUFFIX).jar
CONVERTER_PLATFORM_SRC= $(CONVERTER_SRC)/$(PLATFORM_DIR)/converter


ifeq ($(VARIANT), DBG)
CFLAGS = /c /nologo /Z7
else
CFLAGS = /c /nologo /MD /W3 /GX /O2     
endif

LFLAGS = /PDB:NONE
OTHER_LDLIBS = user32.lib

#
# Rules
#

JAVAC = $(BINDIR)/javac
JAR   = $(BINDIR)/jar
include converter.jmk
include converter.gmk
include $(BUILDDIR)/common/Classes.gmk


#
# Extra rules
#
OPTIMIZED += 	.library.install 
DEBUG     += 	.library.install

ifeq ($(PLATFORM), windows)
OPTIMIZED +=	.launcher.install
DEBUG     +=	.launcher.install
else
OPTIMIZED +=	.scripts.install
DEBUG     +=	.scripts.install
endif

clobber::
	$(RM) -r $(CLASSDESTDIR)
	$(RM) -r $(OBJDIR)
	$(RM) $(CONVERTER_JAR)
	$(RM) $(LAUNCHER_EXE)
	$(RM) $(BINDIR)/$(PROGRAM_NAME)$(SUFFIX)$(EXE_SUFFIX)

#
# Rules to install extension library.
#

.library.install: $(CONVERTER_JAR)

$(CONVERTER_JAR): $(CLASSDESTDIR) $(FILES_class)
	@$(RM) $(@)
	$(JAR) -cfm $(@) $(CONVERTER_SRC)/share/converter/$(MANIFEST)  \
	               -C $(CLASSDESTDIR) HTMLConverter.class \
	               -C $(CLASSDESTDIR) sun
	for i in $(FILES_other); do \
	    $(JAR) -uf  $(@) -C $(CONVERTER_SRC)/share/converter $${i}; \
	done

#
# Rules to build and install the win32 launcher.
#

.launcher.install: $(LAUNCHER_EXE)

$(OBJDIR)/$(PROGRAM_NAME)$(SUFFIX).res: $(LAUNCHER_SRC)/$(PROGRAM_NAME).rc
	$(RC) $(CC_OBJECT_OUTPUT_FLAG)$(@) $(LAUNCHER_SRC)/$(PROGRAM_NAME).rc

$(OBJDIR)/$(PROGRAM_NAME)$(SUFFIX).$(OBJECT_SUFFIX): $(LAUNCHER_SRC)/$(PROGRAM_NAME).cpp
	$(CC) $(CFLAGS) $(LAUNCHER_SRC)/$(PROGRAM_NAME).cpp $(CC_OBJECT_OUTPUT_FLAG)$(@)

$(LAUNCHER_EXE): $(OBJDIR)/$(PROGRAM_NAME)$(SUFFIX).$(OBJECT_SUFFIX) \
		 $(OBJDIR)/$(PROGRAM_NAME)$(SUFFIX).res
	$(LINK) $(LFLAGS) $< $(OTHER_LDLIBS) /OUT:$(LAUNCHER_EXE:$(OBJDIR)/%=$(BINDIR)/%)
	@$(CHMOD) 755 $(LAUNCHER_EXE:$(OBJDIR)/%=$(BINDIR)/%)

#
# Rules to install scripts into the SDK bin directory.
#

.scripts.install: $(SCRIPTS)

$(SCRIPTS): $(SCRIPTS:$(BINDIR)/%=$(CONVERTER_PLATFORM_SRC)/%)
	$(install-file)
	@$(CHMOD) 755 $(@)

#
# Rules to compile
#
$(CLASSDESTDIR)/%.class: $(CONVERTER_SRC)/share/converter/%.java
	@$(ECHO) $? >> $(TMPDIR)/.classes.list


#
# Rules to create a source bundle.
#
source-bundle::
