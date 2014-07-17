#
# @(#)Makefile	1.295 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../..
PACKAGE = sun.awt
LIBRARY = awt
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files
#
ifeq ($(PLATFORM), windows)
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv WINDOWS

#
# Files
#
include FILES_c_windows.gmk
include FILES_java_windows.gmk

OTHER_CXXFLAGS += -W3 -DD3D_OVERLOADS

# Treat compiler warnings as errors, unless building in INSANE mode.
ifndef INSANE
  OTHER_CXXFLAGS += -WX
endif
OTHER_CFLAGS +=-GX -DMLIB_NO_LIBSUNMATH
OTHER_CXXFLAGS += -GX

ifeq ($(ARCH_DATA_MODEL), 64)
  OTHER_CFLAGS += -DMLIB_OS64BIT
endif

include FILES_export_windows.gmk

# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ WINDOWS
endif # PLATFORM

ifneq ($(PLATFORM), windows)
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv SOLARIS/LINUX

#
# Files
#
include FILES_c_unix.gmk
include FILES_java_unix.gmk
include FILES_export_unix.gmk

ifeq  ($(PLATFORM), solaris)
FILES_c = $(FILES_2D_c)
FILES_c += awt_LoadLibrary.c
OTHER_LDLIBS = -lmlib_image$(SUFFIX) $(JVMLIB) $(LIBM) -ldl
endif

ifeq ($(PLATFORM), linux)
FILES_c = $(FILES_2D_c) $(FILES_MOTIF_c) $(FILES_NO_MOTIF_c)
FILES_c += $(TARGDIR)awt_Choice21.c
FILES_c += $(TARGDIR)awt_motif21.c
FILES_c += $(TARGDIR)Xinerama.c
OTHER_CFLAGS += -DMLIB_NO_LIBSUNMATH
OTHER_CFLAGS += -DMOTIF_VERSION=2
OTHER_LDLIBS = -lmlib_image$(SUFFIX) $(LIBCXX) $(JVMLIB) $(LIBXM) \
               -lXt -lXext $(LIBXTST) $(LIBXMU) -lX11 -lm -ldl
endif

FILES_c += initIDs.c

# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ SOLARIS/LINUX
endif # PLATFORM 

#
# Resources
#

LOCALE_SET_DEFINITION = j2re

RESOURCE_BUNDLES_COMPILED_PROPERTIES = \
        sun/awt/print/resources/pdialog.properties \
        sun/awt/print/resources/printcontrol.properties
RESOURCE_SRC_DIR = $(SHARE_SRC)/classes

ifeq ($(PLATFORM), windows)
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv WINDOWS

RESOURCE_BUNDLES_COMPILED_PROPERTIES += sun/awt/windows/awtLocalization.properties
RESOURCE_SRC_DIR_2 = $(PLATFORM_SRC)/classes

# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ WINDOWS
endif # PLATFORM

include $(BUILDDIR)/common/Resources.gmk

_RESOURCES_PRINT = \
	orientLandscape.gif		\
	orientPortrait.gif

RESDIR_PRINT    = $(CLASSBINDIR)/sun/awt/print/resources
RESOURCES_PRINT = $(_RESOURCES_PRINT:%=$(RESDIR_PRINT)/%)

.printresources: $(RESOURCES_PRINT)

$(RESDIR_PRINT)/%.gif: \
	    $(SHARE_SRC)/classes/sun/awt/print/resources/%.gif
	$(install-file)

.printresources.clean :
	$(RM) $(RESOURCES_PRINT)


ifeq ($(PLATFORM), linux)
  FILES_m = mapfile-vers-linux
  # libawt.so on Linux is statically linked with Motif and contains all the
  # Xm symbols. Mapfile is disabled so the symbols will remain public.
  LDNOMAP=true
endif

include $(BUILDDIR)/common/Mapfile-vers.gmk
include $(BUILDDIR)/common/Library.gmk

OPTIMIZED += fontproperties .printresources
DEBUG     += fontproperties .printresources

ifeq ($(PLATFORM), windows)
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv WINDOWS
#
# Additional dependencies.
#
# If make.depend is not working correctly, then it can be omitted
# if you always build from a clean workspace. 

include make.depend

# required additional rules:

# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ WINDOWS
endif # PLATFORM

ifneq ($(PLATFORM), windows)
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv SOLARIS/LINUX

ifeq ($(PLATFORM), solaris)
# Solaris production release specific
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv SOLARIS

ifeq ($(VARIANT),OPT)
  ifndef GLOBAL_KPIC
    CFLAGS += -Kpic
  else
    GLOBAL_KPIC = -Kpic
  endif
endif

ifeq ($(ARCH_DATA_MODEL), 64)
  CFLAGS += -K PIC
endif

# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ SOLARIS
endif # PLATFORM

OPTIMIZED += dgalibs robot_child
DEBUG     += dgalibs robot_child

#
# Rules to build the Robot child process executable
#
# NOTE: Do not change the executable name without changing
# the code in awt_Robot.c/MRobotPeer that references it
#
ROBOT_CHILD_EXE = $(OBJDIR)/awt_robot$(SUFFIX)
ROBOT_DIR = $(PLATFORM_SRC)/native/$(PKGDIR)/robot_child

ROBOT_CHILD_FILES_c = \
	$(TARGDIR)robot_common.c \
	$(TARGDIR)robot_proc.c \
	$(TARGDIR)multiVis.c \
	$(TARGDIR)list.c
ROBOT_CHILD_FILES_o   = $(patsubst %.c,$(OBJDIR)/%.o,$(ROBOT_CHILD_FILES_c))

robot_child : $(ROBOT_CHILD_EXE)
	$(CP) $(ROBOT_CHILD_EXE) $(LIBDIR)/$(LIBARCH)

$(ROBOT_CHILD_EXE) : $(ROBOT_CHILD_FILES_o)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS_COMMON) -L$(OPENWIN_LIB) \
	   $(AWT_RUNPATH) $(OTHER_LDFLAGS) $(TARGET_ARCH) -lXt -lXext \
	   -lXtst -lXi -lX11 $(ROBOT_CHILD_FILES_o) -o $(ROBOT_CHILD_EXE)
	$(POST_PROCESS)

# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ SOLARIS/LINUX
endif # PLATFORM

#
# Rules to build the DebugHelper class
#
DEBUG_HELPER_JAVA = $(GENSRCDIR)/sun/awt/DebugHelper.java
DEBUG_HELPER_JAVA_M4 = $(SHARE_SRC)/classes/sun/awt/DebugHelper.java.m4
DEBUG_HELPER_CLASS = sun/awt/DebugHelper.class

ifdef SUN_AWT_DEBUG_CLASSES
  M4DEFS=-D SUN_AWT_DEBUG_CLASSES=$(SUN_AWT_DEBUG_CLASSES)
  DEBUG_BUILD_MSG = "** DebugHelper being built for DEBUG mode **"
else # SUN_AWT_DEBUG_CLASSES
  DEBUG_BUILD_MSG = "** DebugHelper being built for RELEASE mode **"
endif # SUN_AWT_DEBUG_CLASSES

# Preprocess thru m4 to produce legal .java file,
# force the creation of the .java source, and add the .java file
# to the list of the java compile files.
$(CLASSBINDIR)/$(DEBUG_HELPER_CLASS) : $(CLASSDESTDIR) $(DEBUG_HELPER_JAVA)
	$(ECHO) $(DEBUG_HELPER_JAVA) >> $(TMPDIR)/.classes.list

$(DEBUG_HELPER_JAVA) : $(DEBUG_HELPER_JAVA_M4) $(GENSRCDIR)
	$(MKDIR) -p $(@D)
	@$(ECHO) $(DEBUG_BUILD_MSG)
	$(M4) $(M4DEFS) $(DEBUG_HELPER_JAVA_M4) > $(DEBUG_HELPER_JAVA)

debughelper : $(DEBUG_HELPER_JAVA) 

debughelper.clean :
	$(RM) $(DEBUG_HELPER_JAVA)
	$(RM) $(DEBUG_HELPER_CLASS)

clobber clean:: debughelper.clean

clobber clean:: java2d.clean motif.clean fontproperties.clean \
        .printresources.clean dgalib.clean 

java2d.clean:
	$(RM) -r $(CLASSBINDIR)/sun/java2d .classes.*

#
# Add to the ambient vpath to pick up files in subdirectories
#
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/alphacomposite
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/image
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/image/gif
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/image/cvutils
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/shell
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/medialib
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/debug
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/../java2d
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/../java2d/loops
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/../java2d/pipe
vpath %.cpp $(SHARE_SRC)/native/$(PKGDIR)/image
vpath %.c   $(PLATFORM_SRC)/native/$(PKGDIR)/robot_child

ifeq ($(PLATFORM), windows)
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv WINDOWS
vpath %.cpp   $(PLATFORM_SRC)/native/sun/windows
vpath %.cpp   $(PLATFORM_SRC)/native/$(PKGDIR)
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ WINDOWS
endif # PLATFORM

ifeq ($(PLATFORM), linux)
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv LINUX
vpath %.c   $(PLATFORM_SRC)/native/$(PKGDIR)/robot_child
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ LINUX
endif # PLATFORM

ifeq ($(PLATFORM), windows)
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv WINDOWS
OTHER_LDLIBS = kernel32.lib user32.lib gdi32.lib winspool.lib \
               imm32.lib ole32.lib uuid.lib $(JVMLIB) \
               $(TMPDIR)/awt.res

clean:: awt.clean

awt.clean: 
	$(RM) $(TMPDIR)/awt.ico 
	$(RM) $(TMPDIR)/awt.res 
	$(RM) $(TMPDIR)/hand.cur 
	$(RM) $(TMPDIR)/check.bmp
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ WINDOWS
endif # PLATFORM

ifneq ($(PLATFORM), windows)
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv SOLARIS/LINUX

ifeq ($(STATIC_MOTIF),true)
  LIBXM = $(MOTIF_LIB)/libXm.a -lXp
else # STATIC_MOTIF
  LIBXM = -lXm
endif # STATIC_MOTIF

LIBXTST = -lXtst

ifeq ($(VARIANT),DBG)
# Use -lXmu for EditRes support
LIBXMU	= -lXmu
else
LIBXMU	=
endif

#
# Extra C flags.
#
INIT += motif

motif: $(OBJDIR)/.library.dirs.$(ARCH) $(OBJDIR)/.library.motif.$(ARCH)

$(OBJDIR)/.library.motif.$(ARCH): $(MOTIF_LIB)/libXm.so.3
	$(CP) $(MOTIF_LIB)/libXm.so.3 $(LIBDIR)/$(LIBARCH)/$(TSOBJDIR)
	$(CD) $(LIBDIR)/$(LIBARCH)/$(TSOBJDIR); $(RM) libXm.so; \
		$(LN) -s libXm.so.3 libXm.so
	@-$(TOUCH) $(OBJDIR)/.library.motif.$(ARCH)

clean:: motif.clean

motif.clean :
	$(RM) $(LIBDIR)/$(LIBARCH)/$(TSOBJDIR)/libXm.*

#
# Solaris X11 Direct Graphics Access library
#

_DGALIBS_sparc = \
	libxinerama.so \
	libjdgaSUNWcg6.so \
	libjdgaSUNWffb.so \
	libjdgaSUNWm64.so \
	libjdgaSUNWafb.so

_DGALIBS_sparcv9 = \
	libxinerama.so \
	libjdgaSUNWcg6.so \
	libjdgaSUNWffb.so \
	libjdgaSUNWm64.so \
	libjdgaSUNWafb.so

_DGALIBS_i586 = 	# no i586 library yet

DGALIBS = $(_DGALIBS_$(ARCH):%=$(LIBDIR)/$(LIBARCH)/%)

dgalibs: $(DGALIBS)

$(LIBDIR)/$(LIBARCH)/libxinerama.so: $(PLATFORM_SRC)/lib/$(ARCH)/libxinerama.so
	$(RM) $@
	$(CP) $< $@
	$(CHMOD) a+x $@

$(LIBDIR)/$(LIBARCH)/libjdgaSUNW%.so: $(PLATFORM_SRC)/lib/$(ARCH)/libjdgaSUNW%.so
	$(RM) $@
	$(CP) $< $@
	$(CHMOD) a+x $@

$(LIBDIR)/$(LIBARCH)/libjdgaSUNWafb.so: $(LIBDIR)/$(LIBARCH)/libjdgaSUNWffb.so
	$(RM) $@
	$(LN) -s libjdgaSUNWffb.so $(LIBDIR)/$(LIBARCH)/libjdgaSUNWafb.so

clean:: dgalib.clean

dgalib.clean:
	$(RM) $(LIBDIR)/$(LIBARCH)/libjdgaSUNW* $(LIBDIR)/$(LIBARCH)/libxinerama.so

# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ SOLARIS/LINUX
endif # PLATFORM

#
# Font properties
#

ifeq ($(PLATFORM), windows)
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv WINDOWS

FONTPROPS_SRC = $(PLATFORM_SRC)/classes/sun/awt/windows
_FONTPROPS = \
        font.properties \
        font.properties.hi \
        font.properties.iw \
        font.properties.ja \
        font.properties.ko \
        font.properties.ru \
        font.properties.th \
        font.properties.zh \
        font.properties.zh.98 \
        font.properties.zh_CN_GB18030 \
        font.properties.MS950_HKSCS \
        font.properties.zh_TW \
        font.properties.zh_TW.95 \
        font.properties.zh_TW_MS950_HKSCS \
	font.properties.CP1250 \
	font.properties.CP1251 \
	font.properties.CP1253 \
	font.properties.CP1254 \
	font.properties.CP1256 \
	font.properties.CP1257

FONTPROPS_SRC_SUFFIX =

# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ WINDOWS
endif # PLATFORM

ifeq ($(PLATFORM), linux)
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv LINUX

FONTPROPS_SRC	= $(PLATFORM_SRC)/classes/sun/awt/motif
_FONTPROPS	= \
	font.properties.Redhat6.1			\
	font.properties					\
	font.properties.ja.Redhat6.1			\
	font.properties.ja.Redhat6.2			\
	font.properties.ja.Redhat7.2			\
	font.properties.ja.Redhat7.3			\
	font.properties.ja				\
	font.properties.ja.Turbo			\
	font.properties.ja.Turbo6.0			\
	font.properties.zh.Turbo			\
	font.properties.ja.Redhat8.0			\
	font.properties.ko.Redhat8.0			\
	font.properties.zh_CN.Redhat8.0			\
	font.properties.zh_TW.Redhat8.0			\
	font.properties.Redhat8.0			\
	font.properties.SuSE8.0			

FONTPROPS_SRC_SUFFIX = .$(PLATFORM)

# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ LINUX
endif # PLATFORM

ifeq ($(PLATFORM), solaris)
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv SOLARIS

FONTPROPS_SRC	= $(PLATFORM_SRC)/classes/sun/awt/motif
_FONTPROPS	= \
	font.properties			\
	font.properties.Cp1251          \
	font.properties.ISO8859_2	\
	font.properties.ISO8859_4	\
	font.properties.ISO8859_5	\
	font.properties.ISO8859_7	\
	font.properties.ISO8859_9	\
	font.properties.ISO8859_13	\
	font.properties.ISO8859_13.5.7	\
	font.properties.ISO8859_15	\
	font.properties.KOI8_R          \
	font.properties.UTF8		\
	font.properties.UTF8.5.6	\
	font.properties.UTF8.5.7	\
	font.properties.ja		\
	font.properties.ja_JP_UTF8	\
	font.properties.ja_JP_UTF8.5.7	\
	font.properties.ko		\
	font.properties.ko.5.6		\
	font.properties.ko.5.7		\
	font.properties.ko_KR_UTF8	\
	font.properties.ko_KR_UTF8.5.6	\
	font.properties.ko_KR_UTF8.5.7	\
	font.properties.zh_EUC_CN	\
	font.properties.zh_GBK		\
	font.properties.zh_GB18030	\
	font.properties.zh_CN_UTF8	\
	font.properties.zh_HK_Big5_HKSCS\
	font.properties.zh_TW_EUC_TW	\
	font.properties.zh_TW_Big5	\
	font.properties.zh_TW_Big5.5.6	\
	font.properties.zh_TW_UTF8	\
	font.properties.ar		\
	font.properties.iw		\
	font.properties.th  		\
	font.properties.th_UTF8		\
	font.properties.hi_IN_UTF8.5.9	\
	font.properties.zh_CN_UTF8.5.9	\
	font.properties.zh_HK_UTF8.5.9	\
	font.properties.zh_TW_UTF8.5.9	

# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ SOLARIS
endif # PLATFORM

FONTPROPS	= $(_FONTPROPS:%=$(LIBDIR)/%)

fontproperties: $(FONTPROPS)

$(LIBDIR)/%: $(FONTPROPS_SRC)/%$(FONTPROPS_SRC_SUFFIX)
	$(install-file)

fontproperties.clean :
	$(RM) $(FONTPROPS)


#
# Generate font.properties.checksums
#
ifeq ($(PLATFORM), windows)

CHECKSUMS = $(CLASSBINDIR)/sun/awt/windows/font.properties.checksums
$(CHECKSUMS): $(CLASSBINDIR)/sun/awt/windows $(BUILDDIR)/tools/GenerateChecksums/GenerateChecksums.java $(FONTPROPS)
	-$(RM) $@
	$(RM) -r $(TMPDIR)/GenerateChecksums
	$(MKDIR) -p $(TMPDIR)/GenerateChecksums
	$(JAVAC) -d $(TMPDIR)/GenerateChecksums $(BUILDDIR)/tools/GenerateChecksums/GenerateChecksums.java
	$(JAVA) -classpath $(TMPDIR)/GenerateChecksums GenerateChecksums $(FONTPROPS) > $@

OPTIMIZED += $(CHECKSUMS)

# This caused problems with javac_g on windows-ia64
#DEBUG     += $(CHECKSUMS)

clean::
	$(RM) -rf $(CHECKSUMS)
	$(RM) -r $(TMPDIR)/GenerateChecksums

endif # PLATFORM

ifeq ($(PLATFORM), windows)
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv WINDOWS
OTHER_CFLAGS += -W3 -GX
# Treat compiler warnings as errors, unless building in INSANE mode.
ifndef INSANE
  OTHER_CFLAGS += -WX
endif

#
# Useful variables that name parts of the src tree.
#
WINAWT_native        = $(PLATFORM_SRC)/native/sun/windows
WINAWT_COMMON_native = $(PLATFORM_SRC)/native/sun/awt_common

# FIXME: awt shouldn't have to see the VM's private headers.
# FIXME: if you re-order this list, the build breaks, possibly because 
#        of filename collision. Such a collision, if present, is pure
#        evil. Need to investigate this.

OTHER_INCLUDES += -I$(CLASSHDRDIR)/../../java/jvm \
	-I$(OBJDIR) \
	-I$(SHARE_SRC)/native/common \
	-I$(WINAWT_native) \
	-I$(DXSDK_INCLUDE_PATH) \
	-I$(SHARE_SRC)/native/sun/awt/image/cvutils \
	-I$(SHARE_SRC)/native/sun/awt/image \
	-I$(SHARE_SRC)/native/sun/java2d/loops \
	-I$(SHARE_SRC)/native/sun/java2d \
	-I$(PLATFORM_SRC)/native/sun/java2d \
	-I$(SHARE_SRC)/native/sun/java2d/pipe \
	-I$(SHARE_SRC)/native/sun/dc/path \
	-I$(SHARE_SRC)/native/sun/dc/doe \
	-I$(SHARE_SRC)/native/sun/awt/alphacomposite \
	-I$(SHARE_SRC)/native/sun/awt/debug \
#	-I$(WINAWT_COMMON_native)

# this is only required for compiling )/native/sun/awt/medialib/*.c files
#
OTHER_INCLUDES += -I$(SHARE_SRC)/native/sun/awt/medialib

#
# .res file construction.
#
$(OBJDIR)/$(LIBRARY)$(SUFFIX).lcf: $(TMPDIR)/awt.res

$(TMPDIR)/awt.res: $(WINAWT_native)/awt.rc $(TMPDIR)/awt.ico $(TMPDIR)/hand.cur $(TMPDIR)/check.bmp
	$(RC) /FO$(TMPDIR)/awt.res $(WINAWT_native)/awt.rc

$(TMPDIR)/awt.ico: $(WINAWT_native)/awt.ico
	$(CP) $< $@

$(TMPDIR)/hand.cur: $(WINAWT_native)/hand.cur
	$(CP) $< $@

$(TMPDIR)/check.bmp: $(WINAWT_native)/check.bmp
	$(CP) $< $@
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ WINDOWS
endif # PLATFORM

ifneq ($(PLATFORM), windows)
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv SOLARIS/LINUX

#
# Other extra flags needed for compiling.
#

ifeq ($(PLATFORM), linux)
CPPFLAGS += -I$(MOTIF_DIR)/include \
            -I$(OPENWIN_HOME)/include \
            -I$(OPENWIN_HOME)/include/X11/extensions \
            -I$(PLATFORM_SRC)/native/$(PKGDIR)/font 
endif
CPPFLAGS += -I$(SHARE_SRC)/native/$(PKGDIR)/debug \
            -I$(SHARE_SRC)/native/$(PKGDIR)/image \
	    -I$(SHARE_SRC)/native/$(PKGDIR)/image/cvutils \
	    -I$(SHARE_SRC)/native/$(PKGDIR)/shell \
	    -I$(SHARE_SRC)/native/$(PKGDIR)/alphacomposite \
            -I$(SHARE_SRC)/native/$(PKGDIR)/medialib \
            -I$(PLATFORM_SRC)/native/$(PKGDIR)/medialib \
	    -I$(SHARE_SRC)/native/$(PKGDIR)/../java2d \
	    -I$(PLATFORM_SRC)/native/$(PKGDIR)/../java2d \
	    -I$(SHARE_SRC)/native/$(PKGDIR)/../java2d/loops \
	    -I$(SHARE_SRC)/native/$(PKGDIR)/../java2d/pipe \
	    -I$(SHARE_SRC)/native/$(PKGDIR)/../dc/doe \
	    -I$(SHARE_SRC)/native/$(PKGDIR)/../dc/path \
            -I$(PLATFORM_SRC)/native/$(PKGDIR)/../jdga \
	    $(EVENT_MODEL)

ifeq ($(PLATFORM), linux)
LDFLAGS += -L$(MOTIF_LIB) -L$(OPENWIN_LIB)
endif
LDFLAGS += -L$(LIBDIR)/$(LIBARCH)/$(TSOBJDIR) \
	   $(AWT_RUNPATH)
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ SOLARIS/LINUX
endif # PLATFORM

CLASSES.export += java.io.InputStream \
		  java.lang.Integer \
		  java.lang.ThreadGroup \
		  sun.io.CharToByteConverter \
		  sun.io.ByteToCharConverter

#
# Has been converted to the JNI: generate JNI-style header files
#
JAVAHFLAGS += -jni

ifneq ($(PLATFORM), windows)
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv SOLARIS/LINUX
#
# Remove some files that are created in the main classes directory
# during compilation of X11 character converters, but should live in
# charsets.jar.
#
UNDESIRED_FILES = \
	$(CLASSDESTDIR)/sun/io/ByteToCharDoubleByte.class \
	$(CLASSDESTDIR)/sun/io/ByteToCharEUC_CN.class \
	$(CLASSDESTDIR)/sun/io/ByteToCharEUC_KR.class \
	$(CLASSDESTDIR)/sun/io/ByteToCharEUC_TW.class \
	$(CLASSDESTDIR)/sun/io/CharToByteDoubleByte.class \
	$(CLASSDESTDIR)/sun/io/CharToByteEUC_CN.class \
	$(CLASSDESTDIR)/sun/io/CharToByteEUC_KR.class \
	$(CLASSDESTDIR)/sun/io/CharToByteEUC_TW.class \
	$(CLASSDESTDIR)/sun/io/CharToByteGBK.class \
	$(CLASSDESTDIR)/sun/io/CharToByteJIS0208.class \
	$(CLASSDESTDIR)/sun/io/CharToByteJIS0212.class

OPTIMIZED += undesiredfiles
DEBUG     += undesiredfiles

undesiredfiles:
	$(RM) $(UNDESIRED_FILES)

# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ SOLARIS/LINUX
endif # PLATFORM

.PHONY: robot_child debughelper debughelper.clean java2d.clean awt.clean motif motif.clean \
	dgalibs dgalib.clean fontproperties fontproperties.clean .printresources \
	.printresources.clean undesiredfiles

