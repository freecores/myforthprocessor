#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.1 02/07/23
#
# Makefile to build the Java Plug-in components
#

PLUGIN_TOP = ../../..

LIBRARY = javaplugin_oji
SRCDIR = $(PLUGIN_TOP)/src/plugin/oji-plugin/src/motif/navig5
SHARED_DIR=$(PLUGIN_TOP)/src/plugin/oji-plugin/src/share
SHARED_DIR2 = $(PLUGIN_TOP)/src/plugin/src/share

ALT_COMPILER_PATH = $(GCC32_COMPILER_PATH)bin
include $(PLUGIN_TOP)/make/plugin/common/config.gmk

LIB_LOCATION=$(OUTPUTDIR)/plugin/$(LIBARCH)/ns610-gcc32
dummy:=$(shell $(MKDIR) -p $(LIB_LOCATION))

ifdef ALT_GCC32_PLUGIN_LIB_PATH
all::
	$(CP) $(ALT_GCC32_PLUGIN_LIB_PATH)/lib$(LIBRARY).so \
		$(LIB_LOCATION)/
else
LDLIBS = -lXt -lX11 -ldl -lc
CPPINCPATH = $(SRCDIR) $(SRCDIR)/../common $(subst $(LIBRARY),,$(TMPDIR)) $(subst $(LIBRARY)/,,$(CLASSHDRDIR)) $(SHARED_DIR) $(SHARED_DIR2)/jcomponent
EXTRA_CPPFLAGS = -I$(MOZILLA_HEADERS_PATH)/mozilla_headers_ns610 -I$(MOZILLA_HEADERS_PATH)/mozilla_headers_ns610/nspr -DRAPTOR_API -DNEW_STREAMING_API

OBJDIRNAME = obj$(SUFFIX)
OBJDIR = $(OUTPUTDIR)/tmp/plugin/$(LIBRARY)_gcc32/$(OBJDIRNAME)
DBG_OBJDIR = $(OUTPUTDIR)/tmp/plugin/$(LIBRARY)_gcc32/obj$(DBG_SUFFIX)
OPT_OBJDIR = $(OUTPUTDIR)/tmp/plugin/$(LIBRARY)_gcc32/obj$(OPT_SUFFIX)

vpath %.c $(SRCDIR):$(SRCDIR)/../common:
vpath %.cpp $(SRCDIR):$(SRCDIR)/../common:$(SHARED_DIR2)/jcomponent

include ../ns6/FileList.gmk 
include $(J2SE_TOPDIR)/make/common/Library.gmk
include $(PLUGIN_TOP)/make/plugin/common/rules.gmk

endif

clobber::
	$(RM) -rf $(LIB_LOCATION)
