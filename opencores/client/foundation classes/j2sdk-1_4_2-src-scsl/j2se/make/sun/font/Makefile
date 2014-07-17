#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.62 03/01/23
#
# Makefile for building all of font tools
#

BUILDDIR = ../..
PACKAGE = sun.awt.font
LIBRARY = fontmanager
PRODUCT = sun

#
# Hack fix for mixed C and C++ library.  You can't link this lib with
# CC because not all the names mangle correctly (FIX ME!)  We can't
# link this with the regular cc because that adds -YP to the link
# line, and then it can't find the C++ lib (-lCrun).  So we use the C
# linker without the -YP.
#
CPLUSPLUSLIBRARY = true
MIXEDCPLUSPLUSLIBRARY = true

include $(BUILDDIR)/common/Defs.gmk

#
# Files
#

ifeq ($(PLATFORM), windows)
  include FILES_c_windows.gmk
  include FILES_java_windows.gmk

  # Files built here do not compile with warning level 3
  COMPILER_WARNING_LEVEL=0

  LDLIBS += user32.lib gdi32.lib $(OBJDIR)/../../../sun.awt/awt/$(OBJDIRNAME)/awt$(SUFFIX).lib

  ifeq ($(BCHECKER), 1)
    CC = C:/BChecker/nmcl.exe
  endif

  OTHER_CFLAGS += -DCC_NOEX
  OTHER_CXXFLAGS += -DCC_NOEX

  FILES_export = \
        sun/awt/font/NativeFontWrapper.java \
	sun/awt/font/GlyphList.java \
        java/awt/Image.java \
        java/awt/Color.java \
        java/awt/Graphics.java \
        java/awt/image/ColorModel.java \
        java/awt/image/ImageObserver.java \
        sun/awt/windows/WImage.java \
        java/awt/image/ImageObserver.java \
        sun/java2d/loops/GraphicsPrimitive.java \
        java/awt/Font.java \
	java/text/Bidi.java

else # PLATFORM

  include FILES_c_unix.gmk
  include FILES_java_unix.gmk

  FILES_export = \
	sun/awt/font/X11TextRenderer.java \
	sun/awt/font/NativeFontWrapper.java \
	sun/awt/font/GlyphList.java \
	java/awt/Font.java \
	sun/awt/X11GraphicsEnvironment.java \
	sun/java2d/loops/GraphicsPrimitive.java \
	java/text/Bidi.java

endif # PLATFORM

# The new "rm -rf" way of clobbering clobbers everything under $PACKAGE
# The files in sun.awt.font are in fact built by sun.awt
# if they are clobbered here, you might expect them to be rebuilt here
# but they aren't.
DONT_CLOBBER_CLASSES = true

ifeq ($(PLATFORM), solaris)
  FILES_reorder += reorder-$(ARCH)
endif
include $(BUILDDIR)/common/Mapfile-vers.gmk
include $(BUILDDIR)/common/Library.gmk

ifeq ($(PLATFORM), solaris)

# Sun C++ 5.0 (& later) compiler places template cache in output directory
postaction:
	$(RM) -r $(LIB_LOCATION)/SunWS_cache

OPTIMIZED += postaction
DEBUG     += postaction

clobber clean ::
	$(RM) -r $(OPT_OBJDIR)/SunWS_cache
	$(RM) -r $(DBG_OBJDIR)/SunWS_cache

endif # PLATFORM

#
# Add to the ambient vpath to pick up files in subdirectories
#
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/fontmanager/fontobjects
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/t2k
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/t2k/ttHints
vpath %.c   $(PLATFORM_SRC)/native/$(PKGDIR)
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/bidi
vpath %.cpp   $(SHARE_SRC)/native/$(PKGDIR)/layout
vpath %.cpp   $(SHARE_SRC)/native/$(PKGDIR)/opentype
vpath %.cpp   $(SHARE_SRC)/native/$(PKGDIR)/layoutengine
vpath %.cpp   $(SHARE_SRC)/native/$(PKGDIR)/fontmanager/fontobjects
vpath %.cpp   $(SHARE_SRC)/native/$(PKGDIR)/fontmanager/textcache
vpath %.cpp   $(SHARE_SRC)/native/$(PKGDIR)
vpath %.cpp   $(PLATFORM_SRC)/native/$(PKGDIR)

#
# Fontmanager is headless (independent of X/Motif)
#
CFLAGS       += -DHEADLESS
CXXFLAGS     += -DHEADLESS

ifeq ($(PLATFORM), windows)
vpath %.cpp   $(PLATFORM_SRC)/native/sun/windows
else # PLATFORM
#
# Libraries to link, and other C flags.
#

ifeq ($(CC_VERSION),gcc)
  CFLAGS       += -fPIC
  CXXFLAGS     += -fPIC
else # CC_VERSION
  CFLAGS 	+= -xCC
  CXXFLAGS      += -noex
# Make sure this library is always -KPIC
  ifndef GLOBAL_KPIC
    CFLAGS	 += -KPIC
    CXXFLAGS     += -KPIC
    LIBCXX        = -lCrun
  endif # GLOBAL_KPIC
endif # CC_VERSION

CXXFLAGS     += -DCC_NOEX
ifeq ($(PLATFORM), linux)
  CXXFLAGS += -DNEED_BOGUS_DELETE_OPERATORS
else
  # Note that on Solaris, fontmanager is built against the headless library.
  LDFLAGS      += -L$(LIBDIR)/$(LIBARCH)/headless
endif

# setup the list of libraries to link in...
ifeq ($(PLATFORM), linux)
  OTHER_LDLIBS  += -lawt$(SUFFIX) $(LIBM) $(LIBCXX)
else
  OTHER_LDLIBS  += -lawt$(SUFFIX) -lmawt$(SUFFIX) $(LIBM) $(LIBCXX)
endif

endif # PLATFORM

# set up compile flags..

CPPFLAGS += -I$(SHARE_SRC)/native/$(PKGDIR) \
            -I$(SHARE_SRC)/native/$(PKGDIR)/fontmanager/include \
            -I$(SHARE_SRC)/native/$(PKGDIR)/t2k \
            -I$(SHARE_SRC)/native/$(PKGDIR)/layout \
            -I$(SHARE_SRC)/native/$(PKGDIR)/opentype \
            -I$(SHARE_SRC)/native/$(PKGDIR)/layoutengine \
            -I$(SHARE_SRC)/native/$(PKGDIR)/bidi \
	    -I$(SHARE_SRC)/native/sun/awt/image/cvutils \
	    -I$(PLATFORM_SRC)/native/sun/awt \
	    -I$(SHARE_SRC)/native/sun/awt/debug \
	    -I$(SHARE_SRC)/native/sun/java2d/loops \
	    -I$(SHARE_SRC)/native/sun/java2d/pipe \
            -I$(PLATFORM_SRC)/native/sun/java2d \
	    -I$(SHARE_SRC)/native/sun/java2d \
	    -I$(TMPDIR)/../../sun.awt/awt/CClassHeaders

ifeq ($(PLATFORM), windows)
CPPFLAGS += -I$(PLATFORM_SRC)/native/sun/windows 
endif
#
# Has been converted to the JNI: generate JNI-style header files
#
JAVAHFLAGS += -jni
