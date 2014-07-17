#
# @(#)Makefile	1.14 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building java.rmi.*
#

BUILDDIR = ../..

PACKAGE = java.jexec
PRODUCT = sun
PROGRAM = jexec

include $(BUILDDIR)/common/Defs.gmk


include         $(BUILDDIR)/common/Rules.gmk

JEXE_SRC =      $(PLATFORM_SRC)/bin

FILES_c =       jexec.c
FILES_o =       $(patsubst %.c,$(OBJDIR)/%.o,$(FILES_c))
vpath %.c       $(JEXE_SRC)

default: all

all: optimized debug


DEBUG     = $(LIB_LOCATION)/$(PROGRAM)$(SUFFIX)
OPTIMIZED = $(LIB_LOCATION)/$(PROGRAM)


$(OPTIMIZED) $(DEBUG) :: $(OBJDIR) $(FILES_o)
	$(LINK_PRE_CMD) $(CC) $(LDFLAGS) $(FILES_o) $(CC_OBJECT_OUTPUT_FLAG)$@

clobber:: clean

clean:: 
	$(RM) -r $(OBJROOTS) $(OPTIMIZED) $(DEBUG)

.PHONY: clean clobber all
