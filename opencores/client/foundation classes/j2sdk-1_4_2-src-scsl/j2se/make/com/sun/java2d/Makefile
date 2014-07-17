#
# @(#)Makefile	1.2 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../../..
PACKAGE = com.sun.java2d.fontchecker
PRODUCT = fontchecker
# Destination directory looks like  :-
# ../../../../build/solaris-sparc/tmp/fontchecker/com.sun.java2d.fontchecker
# This is the location the installer needs to get the FontChecker.jar from
FCDEST = $(TMPDIR)
FCJAR = $(FCDEST)/FontChecker.jar
ALT_CLASSES_DISPOSITION = $(FCJAR)
ALT_CLASSBINDIR		= $(TMPDIR)/classes
CLASSDESTDIR		= $(ALT_CLASSBINDIR)
CLASSBINDIR		= $(ALT_CLASSBINDIR)

ifeq ($(VARIANT), DBG)
# Do nothing
classes:
else

include FILES_java.gmk
include $(BUILDDIR)/common/Defs.gmk

# Only need to build this on windows, but no harm to make sure
# it builds properly elsewhere.
#
#ifneq ($(PLATFORM), windows)
#classes:
#else

include $(BUILDDIR)/common/Classes.gmk

jar:	$(FCJAR)

$(FCJAR): $(FILES_class) 
	$(ECHO) Main-Class: com.sun.java2d.fontchecker.FontChecker > $(TMPDIR)/manifest
	$(JAR) -cvfm $@ $(TMPDIR)/manifest -C $(TMPDIR)/classes .
	$(RM) $(TMPDIR)/manifest

.PHONY: jar

#endif

clobber clean::
	$(ECHO) Removing $(TMPDIR)
	$(RM) -r $(TMPDIR)

endif
