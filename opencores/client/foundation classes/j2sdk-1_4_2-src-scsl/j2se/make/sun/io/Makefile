#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.48 03/01/23
#
# Makefile for character converters.
#

BUILDDIR = ../..

PACKAGE = sun.io
PRODUCT = sun

CLASSDESTDIR = $(TMPDIR)/classes
include $(BUILDDIR)/common/Defs.gmk

#
# Files
#
include FILES_java.gmk

# For Cygwin, command line arguments that are paths must be converted to
# windows style paths. These paths cannot be used as targets, however, because 
# the ":" in them  will interfere with GNU Make rules, generating "multiple
# target pattern" errors.

# this define is for the rule:
CHARSETS_JAR = $(LIBDIR)/charsets.jar

# absolute paths
# force creation of TMPDIR
dummy := $(shell $(MKDIR) $(TMPDIR))
ifndef USING_CYGWIN
   ABS_TMPDIR   := $(shell $(CD) $(TMPDIR); $(PWD))
   ABS_BINDIR   := $(shell $(CD) $(BINDIR); $(PWD))
   ABS_BUILDDIR := $(shell $(CD) $(BUILDDIR); $(PWD))
   ABS_CHARSETS_JAR := $(shell $(CD) $(LIBDIR); $(PWD))/charsets.jar
   REORDER_DIR  = $(ABS_TMPDIR)/reorder/file_list
else

# cygpath.exe bug? `$(CYGPATH) -w -a $(BINDIR)` returns
# d:\j2se\build\windows-i586\tmp\build\windows-i586\bin   instead of
# d:\j2se\build\windows-i586\bin
# so we get the absolute path this way:
   ABS_BINDIR   := `$(CYGPATH) -w $(shell $(CD) $(BINDIR); $(PWD))`
   ABS_TMPDIR   := `$(CYGPATH) -w $(shell $(CD) $(TMPDIR); $(PWD))`
   ABS_BUILDDIR := `$(CYGPATH) -w $(shell $(CD) $(BUILDDIR); $(PWD))`

# windows versions of the following paths are needed because without this 
# processing they get converted by the cygwin shell to a path that the 
# jar command does not understand.
   ABS_CHARSETS_JAR := `$(CYGPATH) -w $(shell $(CD) $(LIBDIR); $(PWD))/charsets.jar`
   REORDER_DIR      := `$(CYGPATH) -w $(shell $(CD) $(TMPDIR); $(PWD))/reorder/file_list`

endif

#
# Rules
#
include $(BUILDDIR)/common/Classes.gmk

#
# Extra rules to build character converters.

SERVICE_DESCRIPTION = java.nio.charset.spi.CharsetProvider

$(CLASSDESTDIR)/META-INF/services/$(SERVICE_DESCRIPTION): \
  $(SHARE_SRC)/classes/sun/nio/cs/ext/META-INF/services/$(SERVICE_DESCRIPTION)
	$(MKDIR) -p $(CLASSDESTDIR)/META-INF/services
	$(install-file)

$(CHARSETS_JAR): $(FILES_class) $(TMPDIR)/reorder $(CLASSDESTDIR)/META-INF/services/$(SERVICE_DESCRIPTION)
	$(JAVAC)  -d $(TMPDIR)/reorder $(BUILDDIR)/tools/reorder/JarReorder.java
	@#
	$(RM) $(ABS_TMPDIR)/reorder/file_list
	$(CD) $(CLASSDESTDIR) ; \
		$(ABS_JAVA) \
			-classpath $(ABS_TMPDIR)/reorder JarReorder \
		$(ABS_BUILDDIR)/tools/reorder/reorder_charsets_jar-$(PLATFORM) \
                        - sun > $(ABS_TMPDIR)/reorder/file_list
		$(ECHO) META-INF/services/$(SERVICE_DESCRIPTION) >> $(ABS_TMPDIR)/reorder/file_list
	@#
	$(CD) $(CLASSDESTDIR) ; \
	    $(ABS_JAR) cf $(ABS_CHARSETS_JAR) @$(REORDER_DIR)

$(TMPDIR)/reorder:
	$(MKDIR) -p $(TMPDIR)/reorder

OPTIMIZED += $(CHARSETS_JAR)
DEBUG     += $(CHARSETS_JAR)

clean::
	$(RM) -r $(CLASSDESTDIR)
	$(RM) $(CHARSETS_JAR)
	$(RM) -r $(TMPDIR)/reorder
