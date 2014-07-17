#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.12 03/06/18
#
# Makefile for "unpack" program which decompresses a packed JAR file
#

BUILDDIR= ../../..
PRODUCT = sun.tools.unpack

include $(BUILDDIR)/common/Defs.gmk

TARGET=unpack$(EXE_SUFFIX)
TARGET_G=unpack_g$(EXE_SUFFIX)
CXXFLAGS   = -DPRODUCT -DFULL=1
CXXFLAGS_G = -DFULL=1 



ifeq ($(PLATFORM), windows) #PLATFORM-WINDOWS
# optimized flags
CXXFLAGS   += -nologo -O1 -MD -c
CXXFLAGS_G += -nologo -c
LNKFLAGS   += -nologo /incremental:no /opt:ref
# The following settings are ia32 arch specific:
ifeq ($(ARCH_DATA_MODEL), 32)
  CXXFLAGS += -G5
  LNKFLAGS += /pdbtype:sept
endif

OBJOUTPUT = -Fo
LNKOUTPUT = -out:
WIN32LIBS = user32.lib
else #PLATFORM-UNIX
CXXFLAGS_G += -g -c

BUILD_DATE = $(shell $(DATE) '+\"%y\",\"%m\",\"%d\",\"%H\",\"%M\",\"%S\"')

ifeq ($(PLATFORM), solaris)
CXXFLAGS   += -xO4 -xspace -nolib -lCrun -lc -c
LNKFLAGS +=
else #PLATFORM-LINUX 
CXXFLAGS   += -O -c
LNKFLAGS += $(LIBCXX)
endif #PLATFORM_LINUX
OBJOUTPUT = -o #Caution:the space after the -o is significant!.
LNKOUTPUT = $(OBJOUTPUT)
LINK = $(CXX)
endif #PLATFORM


DATEDEFS = -DBUILD_DATE=$(BUILD_DATE)

all: $(OBJDIR) $(OBJDIR)/$(TARGET) $(OBJDIR)/$(TARGET_G)

$(OBJDIR):
	$(MKDIR) -p $(OBJDIR)

CTARGDIR=

FILES_c= \
	$(CTARGDIR)bytes.cpp \
	$(CTARGDIR)bands.cpp \
	$(CTARGDIR)coding.cpp \
	$(CTARGDIR)unpack.cpp \
	$(CTARGDIR)utils.cpp \
	$(CTARGDIR)zip.cpp

FILES_o   = $(patsubst %.cpp, %.$(OBJECT_SUFFIX), $(addprefix $(OBJDIR)/, $(notdir $(FILES_c))))

FILES_g_o = $(patsubst %.cpp, %_g.$(OBJECT_SUFFIX), $(addprefix $(OBJDIR)/, $(notdir $(FILES_c))))


$(OBJDIR)/%.$(OBJECT_SUFFIX): %.cpp
	$(CXX) $(CXXFLAGS) $(DATEDEFS) $(OBJOUTPUT)$@ $<

$(OBJDIR)/%_g.$(OBJECT_SUFFIX): %.cpp
	$(CXX) $(CXXFLAGS_G) $(DATEDEFS) $(OBJOUTPUT)$@ $<

$(OBJDIR)/$(TARGET): $(FILES_o)
	$(LINK) $(LNKFLAGS) $(LNKOUTPUT)$(OBJDIR)/$(@F) $(FILES_o) $(WIN32LIBS) 

$(OBJDIR)/$(TARGET_G): $(FILES_g_o)
	$(LINK) $(LNKFLAGS) $(LNKOUTPUT)$(OBJDIR)/$(@F) $(FILES_g_o) $(WIN32LIBS) 

clean clobber:
	$(RM) $(OBJDIR)/*.$(OBJECT_SUFFIX) $(OBJDIR)/$(TARGET) $(OBJDIR)/*.pdb
	$(RM) -rf $(OBJDIR) $(TMPDIR)

.PHONY: all clean clobber


