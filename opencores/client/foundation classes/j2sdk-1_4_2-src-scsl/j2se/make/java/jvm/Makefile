#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.158 03/01/23
#

BUILDDIR = ../..
include $(BUILDDIR)/common/Defs.gmk

#
# Make the directories of include files shipped with the JDK.
#

FILES_h = $(INCLUDEDIR)/jni.h 			\
	  $(PLATFORM_INCLUDE)/jni_md.h	\
	  $(INCLUDEDIR)/jvmdi.h \
	  $(INCLUDEDIR)/jvmpi.h

$(INCLUDEDIR)/%.h: $(SHARE_SRC)/javavm/export/%.h
	$(install-file)

$(PLATFORM_INCLUDE)/%.h: $(PLATFORM_SRC)/javavm/export/%.h
	$(install-file)

all:: $(FILES_h)

clobber:: 
	$(RM) -r $(INCLUDEDIR)

