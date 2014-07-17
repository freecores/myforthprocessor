#
# @(#)Makefile	1.8 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.8 03/01/23
#
# Makefile for building the FontTest Demo
#

BUILDDIR = ../../..
DEMOS    = FontTest

# identify this as a demo/jfc/FontTest product - used to identify a
# unique name for OBJDIR
PRODUCT = demo/jfc/FontTest

include $(BUILDDIR)/common/Defs.gmk

#
# What do we compile?
#
DEMODST  = $(DEMODIR)/jfc/FontTest
DEMOSRC  = $(SHARE_SRC)/demo/jfc/FontTest
include FILES.gmk

#
# Activate some hooks so Classes.gmk looks elsewhere for sources, and
# dumps classes where we want.
#
SOURCEPATH              = $(DEMOSRC)

# use "simple expansion" (GNU Make 6.2) for setting BOOTCLASSPATH 
# because CLASSBINDIR is changed a few lines down:
BOOTCLASSPATH           := $(CLASSBINDIR)

DEMOJAR	                = $(DEMODST)/FontTest.jar
ALT_CLASSES_DISPOSITION = src docs $(DEMOJAR)
ALT_CLASSBINDIR         = $(TMPDIR)/class
CLASSDESTDIR            = $(ALT_CLASSBINDIR)
CLASSBINDIR             = $(ALT_CLASSBINDIR)

# Do not build Debug versions of the demo:
ifeq ($(VARIANT), DBG)
classes $(ALT_CLASSES_DISPOSITION):
	@$(TRUE)
# VARIANT=DBG processing stops here!
else # VARIANT

#
# Rules.
#
include $(BUILDDIR)/common/Classes.gmk

#
# Sources.
#
SRCSRC = $(DEMOSRC)
SRCDST = $(DEMODST)/src
FILES_src = $(FILES_java:%.java=$(SRCDST)/%.java)

src: $(FILES_src)

$(SRCDST)/%.java: $(SRCSRC)/%.java
	$(install-file)

#
# Docs.
#
docs: $(DEMODST)/README.txt $(DEMODST)/FontTest.html

$(DEMODST)/README.txt: $(DEMOSRC)/README.txt
	$(install-file)

$(DEMODST)/FontTest.html: $(DEMOSRC)/FontTest.html
	$(install-file)


#
# Jar file.
#
jar: $(DEMODST)/FontTest.jar

$(DEMODST)/FontTest.jar: $(FILES_class) $(FILES_java:%.java=$(DEMOSRC)/%.java)
		$(ECHO) Main-Class: FontTest > $(TMPDIR)/manifest
		$(JAR) -cvfm $@ $(TMPDIR)/manifest -C $(TMPDIR)/class .
		$(RM) $(TMPDIR)/manifest

endif # VARIANT

