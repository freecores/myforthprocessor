#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.7 03/01/23
#
# Makefile to build the Java Plug-in components
#

PLUGIN_TOP = ../../..

LIBRARY = javaplugin_jni
SRCDIR = $(PLUGIN_TOP)/src/plugin/oji-plugin/src/motif/jvm_natives

include $(PLUGIN_TOP)/make/plugin/common/config.gmk

ifeq ($(PLATFORM), linux)
LDLIBS = -lXt -lX11
else
LDLIBS = /usr/lib/libm.so -ldl -lXt -lX11 -lsocket -lnsl -lintl -lc
endif

CPPINCPATH = $(SRCDIR) $(SRCDIR)/../common $(subst $(LIBRARY),,$(TMPDIR)) $(subst $(LIBRARY)/,,$(CLASSHDRDIR))

vpath %.c $(SRCDIR):$(SRCDIR)/../common

include ./FileList.gmk 

include $(J2SE_TOPDIR)/make/common/Library.gmk
include $(PLUGIN_TOP)/make/plugin/common/rules.gmk

