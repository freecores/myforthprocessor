#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.5 03/01/23
#
# Makefile to build the Java Plug-in components
#

PLUGIN_TOP = ../../../..

EXECUTABLE = jpicpl32
PROGRAM = jpicplexec
SRCDIR = $(PLUGIN_TOP)/src/plugin/src/win32/controlpanel/cplexec
PCHFILENAME=$(OBJDIR)/$(EXECUTABLE).pch

include $(PLUGIN_TOP)/make/plugin/common/config.gmk

CPPINCPATH = $(SRCDIR) $(subst $(EXECUTABLE),,$(TMPDIR)) $(subst $(EXECUTABLE)/,,$(CLASSHDRDIR)) $(PLUGIN_TOP)/src/plugin/src/win32/common

RSCINC = -i $(SRCDIR) -i $(subst $(EXECUTABLE),,$(TMPDIR))
EXTRA_CPPFLAGS := -DUSRDLL -Fp"$(PCHFILENAME)" -Yu"stdafx.h"
LDLIBS += user32.lib

$(OBJDIR)/StdAfx.obj: EXTRA_CPPFLAGS := -DUSRDLL -Fp"$(PCHFILENAME)" -Yc"stdafx.h"

vpath %.cpp $(SRCDIR):$(SRCDIR)/../../common
vpath %.rc $(SRCDIR)

include ./FileList.gmk 

include $(PLUGIN_TOP)/make/plugin/common/exec.gmk
include $(PLUGIN_TOP)/make/plugin/common/rules.gmk
