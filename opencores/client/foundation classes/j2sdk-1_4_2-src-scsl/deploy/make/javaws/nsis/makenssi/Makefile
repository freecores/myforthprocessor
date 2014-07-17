#
# %W% %E%
# 
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

# Makefile for Nullsoft installer compiler build
BUILDDIR = ../..
JAVAWS_TOPDIR = ../../..
include ../../common/Defs.gmk

SRCDIR = $(JAVAWS_TOPDIR)/src/javaws/src/win32/nsis/makenssi
SRCDIRZIP = $(JAVAWS_TOPDIR)/src/javaws/src/win32/nsis/makenssi/zlib
OBJDIR := $(OBJDIR)/makenssi
TMPDIREXEHEAD = $(OUTPUTDIR)/tmp/exehead$(I18N_PARAM)

MAKENSSI=$(PROTO_DIR)/makenssi.exe

FILES_C = \
ADLER32.C \
DECOMP.C \
INFBLOCK.C \
INFCODES.C \
INFFAST.C \
INFLATE.C \
INFTREES.C \
INFUTIL.C \
UNCOMPR.C \
ZUTIL.C 

FILES_c = \
trees.c \
deflate.c 

FILES_cpp = \
makenssi.cpp 

FILES_obj = $(FILES_C:%.C=$(OBJDIR)/%.obj)
FILES_obj += $(FILES_c:%.c=$(OBJDIR)/%.obj)
FILES_obj += $(FILES_cpp:%.cpp=$(OBJDIR)/%.obj)

RSC_PROJ=/l 0x409 /d "NDEBUG"
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "NO_IDE" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /I"$(TMPDIREXEHEAD)" /Fo"$(OBJDIR)\\" /Fd"$(OBJDIR)\\" /FD /c 
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /pdb:none /machine:I386


ifeq ($(I18N),1)
RES_SRC=$(SRCDIR)/Script1-int.rc
RES_FILE=$(OBJDIR)/resource-int.res
else
RES_SRC=$(SRCDIR)/Script1-us.rc
RES_FILE=$(OBJDIR)/resource-us.res
endif

all optimized debug:: compiler

compiler: $(PROTO_DIR) $(OBJDIR) $(MAKENSSI) FRC

$(MAKENSSI): $(FILES_obj) $(RES_FILE)
	$(LINK32) $(LINK32_FLAGS) /out:"$@" $(FILES_obj) $(RES_FILE)

$(RES_FILE): $(RES_SRC)
	$(RSC) $(RSC_PROJ) /fo"$@" $?

$(OBJDIR) $(PROTO_DIR):
	$(MKDIR) -p $@

$(OBJDIR)/%.obj: $(SRCDIRZIP)/%.c
	$(MKDIR) -p $(@D)
	$(CC) $(CPP_PROJ) $?

$(OBJDIR)/%.obj: $(SRCDIRZIP)/%.C
	$(MKDIR) -p $(@D)
	$(CC) $(CPP_PROJ) $?

$(OBJDIR)/%.obj: $(SRCDIR)/%.c
	$(MKDIR) -p $(@D)
	$(CC) $(CPP_PROJ) $?

$(OBJDIR)/makenssi.obj: $(SRCDIR)/makenssi.cpp FRC
	$(MKDIR) -p $(@D)
	$(CC) $(CPP_PROJ) $(SRCDIR)/makenssi.cpp

clean:
	$(RM) -r $(MAKENSSI) $(FILES_obj) $(RES_FILE) $(TMPDIREXEHEAD) \
	$(OBJDIR)

FRC:
.PHONY: all clean compiler optimized debug
