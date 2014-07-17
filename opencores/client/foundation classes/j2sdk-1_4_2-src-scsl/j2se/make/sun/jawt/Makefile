#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.29 03/01/23
#

BUILDDIR = ../..
PACKAGE = sun.awt
LIBRARY = jawt
PRODUCT = sun
MIXEDCPLUSPLUSLIBRARY = 1
include $(BUILDDIR)/common/Defs.gmk

#
# Files
#
ifeq ($(PLATFORM), windows)
FILES_cpp = $(TARGDIR)jawt.cpp
else # PLATFORM
FILES_c = $(TARGDIR)jawt.c
endif # PLATFORM

FILES_h = $(INCLUDEDIR)/jawt.h \
    $(PLATFORM_INCLUDE)/jawt_md.h

ifeq ($(PLATFORM), windows)
#
# Extra C flags.
#

OTHER_CXXFLAGS += -W3
# Treat compiler warnings as errors, unless building in INSANE mode.
ifndef INSANE
  OTHER_CXXFLAGS += -WX
endif
OTHER_CXXFLAGS += -GX

#
# Other extra flags needed for compiling.
#
CPPFLAGS += -I$(SHARE_SRC)/native/common \
	-I$(DXSDK_INCLUDE_PATH) \
	-I$(PLATFORM_SRC)/native/sun/windows \
	-I$(CLASSHDRDIR)/../../awt/CClassHeaders \
        -I$(SHARE_SRC)/native/sun/awt/debug \
        -I$(SHARE_SRC)/native/sun/java2d \
        -I$(SHARE_SRC)/native/sun/java2d/loops \
        -I$(SHARE_SRC)/native/sun/java2d/pipe \
	-I$(PLATFORM_SRC)/native/sun/java2d \
	-I$(SHARE_SRC)/native/$(PKGDIR)/image/cvutils \
	-I$(SHARE_SRC)/javavm/export \
	-I$(PLATFORM_SRC)/javavm/export
INSTALL_DOT_LIB = true

vpath %.cpp   $(PLATFORM_SRC)/native/sun/windows

#
# Libraries to link in.
#
OTHER_LDLIBS = $(OBJDIR)/../../awt/$(OBJDIRNAME)/awt$(SUFFIX).lib 

else # PLATFORM

#
# Extra C flags.
#
ifneq ($(CC_VERSION),gcc)
CFLAGS += -xCC
endif # CC_VERSION
#
# Other extra flags needed for compiling.
#
CPPFLAGS += -I$(OPENWIN_HOME)/include \
	    -I$(MOTIF_DIR)/include \
	    -I$(SHARE_SRC)/native/$(PKGDIR)/debug \
            -I$(SHARE_SRC)/native/$(PKGDIR)/image \
	    -I$(SHARE_SRC)/native/$(PKGDIR)/image/cvutils \
	    -I$(SHARE_SRC)/native/$(PKGDIR)/alphacomposite \
            -I$(SHARE_SRC)/native/$(PKGDIR)/medialib \
            -I$(PLATFORM_SRC)/native/$(PKGDIR)/medialib \
	    -I$(SHARE_SRC)/native/$(PKGDIR)/../java2d/loops \
	    -I$(SHARE_SRC)/native/$(PKGDIR)/../java2d/pipe \
	    -I$(SHARE_SRC)/native/$(PKGDIR)/../dc/doe \
	    -I$(SHARE_SRC)/native/$(PKGDIR)/../dc/path \
            -I$(PLATFORM_SRC)/native/$(PKGDIR)/../jdga \
	    $(EVENT_MODEL)
#
# Libraries to link in.
#
ifeq ($(PLATFORM), solaris)
OTHER_LDLIBS = -L$(LIBDIR)/$(LIBARCH)/motif12 \
               -L$(LIBDIR)/$(LIBARCH)/motif21 -lmawt$(SUFFIX)
endif # PLATFORM

ifeq ($(PLATFORM), linux)
OTHER_LDLIBS = -L$(LIBDIR)/$(LIBARCH) -lawt$(SUFFIX)
endif # PLATFORM

endif # PLATFORM


#
# Lets not remove any class files.
#
DONT_CLOBBER_CLASSES = true

#
# Rules
#
include $(BUILDDIR)/common/Library.gmk

ifeq ($(PLATFORM), windows)
include make.depend
endif

#
# Install header files.
#
$(INCLUDEDIR)/%.h: $(SHARE_SRC)/javavm/export/%.h
	$(install-file)
$(PLATFORM_INCLUDE)/%.h: $(PLATFORM_SRC)/javavm/export/%.h
	$(install-file)

OPTIMIZED += $(FILES_h)
DEBUG     += $(FILES_h)

#
# Extra clean rule.
#
clean::
	$(RM) $(FILES_h)
