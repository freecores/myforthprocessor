#
# @(#)Makefile	1.15 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Re-compile newjavac classes possibly built by bootstrap compiler.
#

BUILDDIR = ../../../..
PACKAGE = com.sun.tools.javac
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files-to-compile.
#
include $(BUILDDIR)/sun/javac/javac/FILES_java.gmk

#
# Override destination directory.
#
ALT_CLASSBINDIR = $(TMPDIR)/newjavac/newclasses
ALT_CLASSDESTDIR := $(CLASSDESTDIR)
CLASSDESTDIR    = $(ALT_CLASSBINDIR)
CLASSBINDIR     = $(ALT_CLASSBINDIR)

JAVAC_CLS_FLAG     = -classpath $(CLASSBINDIR)/classes

### This is a horrible hack to make sure sourceless classes like Normalizer.class can be found.
JAVAC_CLS_FLAG     += -bootclasspath $(SHARE_SRC)/classes

ifeq ($(PLATFORM), linux)
JAVAC_CLS_FLAG     = -classpath newclasses -J-Xbootclasspath/a:$(BUILDDIR)/gensrc
endif


#
# Rules.
#
include $(BUILDDIR)/common/Classes.gmk

# Don't leave old classes lying around in 'newclasses' !

clean clobber::
	$(RM) -r $(TMPDIR)/newjavac/newclasses
