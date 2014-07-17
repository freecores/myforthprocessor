#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.4 03/01/23
#
# Makefile to build the Java Plug-in components
#

PLUGIN_TOP = ../../..

LIBRARY = jcomponent
SRCDIR = $(PLUGIN_TOP)/src/plugin/src/share/jcomponent

include $(PLUGIN_TOP)/make/plugin/common/config.gmk

ifeq ($(PLATFORM), linux)
else
LDFLAGS += -z defs
LDLIBS = -lsocket -lnsl -ldl -lCrun -lc
endif

CPPINCPATH = $(SRCDIR) $(SRCDIR)/public-include $(SRCDIR)/../common $(subst $(LIBRARY),,$(TMPDIR)) $(subst $(LIBRARY)/,,$(CLASSHDRDIR))

vpath %.cpp $(SRCDIR):$(SRCDIR)/../common

include ./FileList.gmk 

include $(J2SE_TOPDIR)/make/common/Library.gmk
include $(PLUGIN_TOP)/make/plugin/common/rules.gmk

