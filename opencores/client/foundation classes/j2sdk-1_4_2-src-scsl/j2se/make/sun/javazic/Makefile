#
# @(#)Makefile	1.7 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

# Makefile for building javazic utility & TimeZone resource files


BUILDDIR = ../..
PACKAGE = sun.tools.javazic
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files
#
include FILES_java.gmk

#
# Rules
#
$(TMPDIR)/%.class: $(SHARE_SRC)/classes/%.java
	@$(ECHO) $? >>$(TMPDIR)/.classes.list

TZDATA = ./tzdata/
TZDATA_VER = `cat $(TZDATA)VERSION`
TZFILE = \
    africa antarctica asia australasia europe northamerica \
    pacificnew southamerica systemv backward \
    etcetera solar87 solar88 solar89 systemv
JDKTZDATA = ./tzdata_jdk/
JDKTZFILES = gmt jdk11_backward
TZFILES = \
    $(addprefix $(TZDATA),$(TZFILE)) \
    $(addprefix $(JDKTZDATA),$(JDKTZFILES))

FILES_class = $(FILES_java:%.java=$(TMPDIR)/%.class)
WORKDIR = $(TMPDIR)/zi
INSTALLDIR = $(LIBDIR)/zi
MAPFILE = ZoneInfoMappings

all: $(TMPDIR) .delete.classlist $(FILES_class) .compile.classlist $(INSTALLDIR)/$(MAPFILE)

$(TMPDIR):
	$(MKDIR) -p $@

.delete.classlist:
	$(RM) $(TMPDIR)/.classes.list

.compile.classlist:
	if [ -s $(TMPDIR)/.classes.list ] ; \
	then $(JAVAC_CMD) -d $(TMPDIR) \
	    $(shell if [ -s $(TMPDIR)/.classes.list ] ; then $(CAT) $(TMPDIR)/.classes.list; fi ) ; \
	fi

$(WORKDIR)/$(MAPFILE): $(FILES_class) $(TZFILES)
	$(RM) -r $(WORKDIR)
	$(JAVA) -classpath $(TMPDIR) sun.tools.javazic.Main -V "$(TZDATA_VER)" -d $(WORKDIR) $(TZFILES)

$(INSTALLDIR)/$(MAPFILE): $(WORKDIR)/$(MAPFILE)
	if [ ! -d $(INSTALLDIR) ] ; then $(MKDIR) $(INSTALLDIR); \
	else $(RM) -r $(INSTALLDIR)/*; fi
	$(CP) -r $(WORKDIR)/* $(INSTALLDIR)

clobber:
	$(RM) -r $(TMPDIR) $(INSTALLDIR)
