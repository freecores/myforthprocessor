#
# @(#)Makefile	1.9 03/01/23
# 
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

# Makefile for "uncrunch" program the decompresses a crunched JAR file
BUILDDIR = ../..
JAVAWS_TOPDIR = ../../..
include ../../common/Defs.gmk

SRCDIR = $(JAVAWS_TOPDIR)/src/javaws/src/win32/nsis/uncrunch
INTDIR = $(PROTO_DIR)
OBJDIR = $(OUTPUTDIR)/obj/uncrunch

PRODUCT = sun.tools.uncrunch

all optimized debug: $(OBJDIR) $(INTDIR)/uncrunch.exe

$(OBJDIR):
	$(MKDIR) -p $(OBJDIR)

CTARGDIR=

FILES_c= \
	$(SRCDIR)/main.c \
	$(SRCDIR)/pool.c \
	$(SRCDIR)/prescan.c \
	$(SRCDIR)/convert.c

FILES_o   = $(patsubst %.c, %.obj, $(addprefix $(OBJDIR)/, $(notdir $(FILES_c))))

$(OBJDIR)/%.obj: $(SRCDIR)/%.c
	$(CC) $(CPPFLAGS) /D "WIN32" /D "_WINDOWS" -c -Fo$@ $<

$(INTDIR)/uncrunch.exe: $(FILES_o)
	$(LINK) $(LINK_FLAGS) /subsystem:windows -out:$(INTDIR)/$(@F) $(FILES_o) user32.lib

clean clobber:
	$(RM) -r $(OBJDIR) $(INTDIR)/uncrunch.exe

.PHONY: clean clobber all optimized debug
