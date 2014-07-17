#
# @(#)Makefile	1.15 03/01/23
# 
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

# Makefile for Nullsoft installer compiler build
BUILDDIR = ../..
JAVAWS_TOPDIR = ../../..
include ../../common/Defs.gmk

SRCDIR = $(JAVAWS_TOPDIR)/src/javaws/src/win32/nsis/exehead
SRCDIRZIP = $(JAVAWS_TOPDIR)/src/javaws/src/win32/nsis/exehead/zlib
MAKDIR = $(SRCDIR)/../makenssi
OBJDIR := $(OBJDIR)/exehead
TMPDIR = $(OUTPUTDIR)/tmp/exehead$(I18N_PARAM)

FILES_cpp = \
shell.cpp 

FILES_C = \
ADLER32.C \
INFCODES.C \
INFFAST.C \
INFLATE.C \
ZUTIL.C \
INFTREES.C 

FILES_c = \
Infblock.c \
Infutil.c \
exec.c \
fileform.c \
i18n.c \
libc.c \
Main.c \
Ui.c \
util.c 

FILES_obj = $(FILES_cpp:%.cpp=$(OBJDIR)/%.obj)
FILES_obj += $(FILES_C:%.C=$(OBJDIR)/%.obj)
FILES_obj += $(FILES_c:%.c=$(OBJDIR)/%.obj)

BIN2H_C_FILES = \
	$(SRCDIR)/bin2h.c 

BIN2H_OBJS=$(BIN2H_C_FILES:$(SRCDIR)/%.c=$(OBJDIR)/%.obj)

RSC_PROJ=-l 0x409 -d "NDEBUG"
CPP_PROJ=/nologo /ML /W3 /GX /O1 /Oy /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fo"$(OBJDIR)\\" /Fd"$(OBJDIR)\\" /FD /c
CPP_PROJ_LIBC=/nologo /GB /Zp8 /ML /W3 /GX /O1 /Oy /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fo"$(OBJDIR)\\" /Fd"$(OBJDIR)\\" /FD /c
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libc.lib /nologo /subsystem:windows /pdb:none /machine:I386 /opt:nowin98 

BIN2H=$(TMPDIR)/bin2h.exe

HEADERS = \
	$(TMPDIR)/bitmap1.h \
	$(TMPDIR)/bitmap2.h \
	$(TMPDIR)/icon.h \
	$(TMPDIR)/unicon.h \
	$(TMPDIR)/exehead.h 

EXEHEAD=$(TMPDIR)/exehead.exe

ifeq ($(I18N),1)
RES_SRC=$(SRCDIR)/resource-int.rc
RES_FILE=$(OBJDIR)/resource-int.res
else
RES_SRC=$(SRCDIR)/resource-us.rc
RES_FILE=$(OBJDIR)/resource-us.res
endif

all optimized debug:: makeheader

makeheader: $(PROTO_DIR) $(TMPDIR) $(BIN2H) $(OUTDIR) $(EXEHEAD) headers

$(EXEHEAD): $(FILES_obj) $(RES_FILE)
	$(LINK32) $(LINK32_FLAGS) /out:"$@" $(FILES_obj) $(RES_FILE)

headers: FRC
	$(BIN2H) $(SRCDIR)/bitmap1.bmp $(TMPDIR)/bitmap1.h bitmap1_data
	$(BIN2H) $(SRCDIR)/bitmap2.bmp $(TMPDIR)/bitmap2.h bitmap2_data
	$(BIN2H) $(SRCDIR)/nullsoft.ico $(TMPDIR)/icon.h icon_data
	$(BIN2H) $(SRCDIR)/uninst.ico $(TMPDIR)/unicon.h unicon_data
	$(BIN2H) $(EXEHEAD) $(TMPDIR)/exehead.h header_data
	$(TOUCH) $(PROTO_DIR)/headers.jtg

$(BIN2H): $(TMPDIR) $(BIN2H_OBJS)
	$(LINK32) /subsystem:console /nologo /out:$@ $(BIN2H_OBJS)

$(RES_FILE): $(RES_SRC)
	$(MKDIR) -p $(OBJDIR)
	$(RSC) $(RSC_PROJ) /fo"$@" $?

$(OBJDIR)/libc.obj: $(SRCDIR)/libc.c
	$(MKDIR) -p $(@D)
	$(CC) $(CPP_PROJ_LIBC) $?

$(OBJDIR)/%.obj: $(SRCDIRZIP)/%.c
	$(MKDIR) -p $(@D)
	$(CC) $(CPP_PROJ) $?

$(OBJDIR)/%.obj: $(SRCDIRZIP)/%.C
	$(MKDIR) -p $(@D)
	$(CC) $(CPP_PROJ) $?

$(OBJDIR)/%.obj: $(SRCDIR)/%.c
	$(MKDIR) -p $(@D)
	$(CC) $(CPP_PROJ) $?

$(OBJDIR)/%.obj: $(SRCDIR)/%.cpp
	$(MKDIR) -p $(@D)
	$(CC) $(CPP_PROJ) $?

$(OBJDIR) $(TMPDIR) $(PROTO_DIR):
	$(MKDIR) -p $@

clean:
	$(RM) -r $(RES_FILE) $(FILES_obj) $(BIN2H_OBJS) $(OBJDIR) $(BIN2H) $(HEADERS) $(EXEHEAD) $(TMPDIR)

FRC:
.PHONY: all makeheader clean headers optimized debug
