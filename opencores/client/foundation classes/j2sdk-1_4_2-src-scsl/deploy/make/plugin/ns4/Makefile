#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.7 03/01/23
#
# Makefile to build the Java Plug-in components
#

PLUGIN_TOP = ../../..

LIBRARY = javaplugin
SRCDIR = $(PLUGIN_TOP)/src/plugin/oji-plugin/src/motif/navig4
HEADERDIR = $(PLUGIN_TOP)/src/plugin/oji-plugin/include/solaris/navig4
SHAREDIR = $(PLUGIN_TOP)/src/plugin/oji-plugin/src/share

include $(PLUGIN_TOP)/make/plugin/common/config.gmk

LIB_LOCATION=$(OUTPUTDIR)/plugin/$(LIBARCH)/ns4

dummy:=$(shell $(MKDIR) -p $(LIB_LOCATION))

ifeq ($(PLATFORM), linux)
LDLIBS = -lXt -lX11 -ldl -lnsl -lc
else
LDLIBS = -lXt -lX11 -ldl -lnsl -lCrun -lc
endif

CPPINCPATH = $(SRCDIR) $(SRCDIR)/../common $(SRCDIR)/../badapter $(SHAREDIR) $(subst $(LIBRARY),,$(TMPDIR)) $(subst $(LIBRARY)/,,$(CLASSHDRDIR)) $(HEADERDIR)/nspr $(HEADERDIR)/xpcom $(HEADERDIR)/caps $(HEADERDIR)/js $(HEADERDIR)/raptor $(HEADERDIR)/oji $(HEADERDIR)/plugin
EXTRA_CPPFLAGS = -DRAPTOR_API -DNEW_STREAMING_API
vpath %.c $(SRCDIR):$(SRCDIR)/../common:$(SRCDIR)/../badapter
vpath %.cpp $(SRCDIR):$(SRCDIR)/../common:$(SRCDIR)/../badapter

include ./FileList.gmk 

include $(J2SE_TOPDIR)/make/common/Library.gmk
include $(PLUGIN_TOP)/make/plugin/common/rules.gmk

clobber::
	$(RM) -rf $(LIB_LOCATION)

