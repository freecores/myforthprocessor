#
# @(#)Makefile	1.31 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../..
PACKAGE = sun.awt
LIBRARY = cmm
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files
#
ifeq ($(PLATFORM), windows)
include FILES_c_windows.gmk
else # PLATFORM
include FILES_c_unix.gmk
endif # PLATFORM

FILES_java = \
	sun/awt/color/CMM.java

FILES_export = \
	sun/awt/color/CMM.java

ifeq ($(PLATFORM), windows)
  # Files built here do not compile with warning level 3
  COMPILER_WARNING_LEVEL=2
endif

#
# Rules
#
include $(BUILDDIR)/common/Mapfile-vers.gmk
include $(BUILDDIR)/common/Library.gmk

#
# Extra rules
#
OPTIMIZED += iccprofiles
DEBUG     += iccprofiles
clean clobber:: iccprofiles.clean

#
# Add to ambient vpath to get files in a subdirectory
#
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/cmm

ifeq ($(PLATFORM), windows)

ifeq ($(VARIANT), DBG)
CFLAGS += -D_DEBUG
endif # VARIANT
OTHER_LDLIBS = user32.lib version.lib

#
# .res file construction.
#
$(OBJDIR)/$(LIBRARY)$(SUFFIX).lcf: $(OBJDIR)/cmm$(SUFFIX).res

$(OBJDIR)/cmm$(SUFFIX).res: $(OBJDIR)/cmm.rc $(OBJDIR)/cmm.h
	$(RC) /FO$(OBJDIR)/cmm$(SUFFIX).res $(OBJDIR)/cmm.rc

$(OBJDIR)/cmm.rc: $(SHARE_SRC)/native/sun/awt/cmm/cmm.rc
	$(CP) $< $@

$(OBJDIR)/cmm.h: $(SHARE_SRC)/native/sun/awt/cmm/cmm.h
	$(CP) $< $@

clean:: res.clean

res.clean:
	$(RM) $(OPT_OBJDIR)/cmm.rc    $(DBG_OBJDIR)/cmm.rc
	$(RM) $(OPT_OBJDIR)/cmm.res   $(DBG_OBJDIR)/cmm$(DBG_SUFFIX).res
	$(RM) $(OPT_OBJDIR)/cmm.h     $(DBG_OBJDIR)/cmm.h

else # PLATFORM
OTHER_LDLIBS = $(LIBM)

ifeq ($(CC_VERSION),gcc)
CFLAGS += -fpic
else # CC_VERSION
ifndef GLOBAL_KPIC
  CFLAGS += -K pic
else
  # Use -Kpic instead of -KPIC on this library
  GLOBAL_KPIC = -Kpic
endif
endif # CC_VERSION

CPPFLAGS += -I$(CLASSHDRDIR) \

endif # PLATFORM

CFLAGS += -DJAVACMM -DFUT_CALC_EX -DNO_FUT_GCONST

#
# Has been converted to the JNI: generate JNI-style header files
#
JAVAHFLAGS += -jni

#
# ICC Profiles used by the CMM.
#
ICCPROFILE_SRC_DIR  = $(SHARE_SRC)/lib/cmm
ICCPROFILE_DEST_DIR = $(LIBDIR)/cmm

iccprofiles: $(ICCPROFILE_DEST_DIR)/sRGB.pf $(ICCPROFILE_DEST_DIR)/GRAY.pf \
	     $(ICCPROFILE_DEST_DIR)/CIEXYZ.pf $(ICCPROFILE_DEST_DIR)/PYCC.pf \
	     $(ICCPROFILE_DEST_DIR)/LINEAR_RGB.pf

$(ICCPROFILE_DEST_DIR)/sRGB.pf: $(ICCPROFILE_SRC_DIR)/sRGB.pf
	$(RM) $(ICCPROFILE_DEST_DIR)/sRGB.pf
	-$(MKDIR) -p $(ICCPROFILE_DEST_DIR)
	$(CP) $(ICCPROFILE_SRC_DIR)/sRGB.pf $(ICCPROFILE_DEST_DIR)
	$(CHMOD) 444 $(ICCPROFILE_DEST_DIR)/sRGB.pf

$(ICCPROFILE_DEST_DIR)/GRAY.pf: $(ICCPROFILE_SRC_DIR)/GRAY.pf
	$(RM) $(ICCPROFILE_DEST_DIR)/GRAY.pf
	-$(MKDIR) -p $(ICCPROFILE_DEST_DIR)
	$(CP) $(ICCPROFILE_SRC_DIR)/GRAY.pf $(ICCPROFILE_DEST_DIR)
	$(CHMOD) 444 $(ICCPROFILE_DEST_DIR)/GRAY.pf

$(ICCPROFILE_DEST_DIR)/CIEXYZ.pf: $(ICCPROFILE_SRC_DIR)/CIEXYZ.pf
	$(RM) $(ICCPROFILE_DEST_DIR)/CIEXYZ.pf
	-$(MKDIR) -p $(ICCPROFILE_DEST_DIR)
	$(CP) $(ICCPROFILE_SRC_DIR)/CIEXYZ.pf $(ICCPROFILE_DEST_DIR)
	$(CHMOD) 444 $(ICCPROFILE_DEST_DIR)/CIEXYZ.pf

$(ICCPROFILE_DEST_DIR)/PYCC.pf: $(ICCPROFILE_SRC_DIR)/PYCC.pf
	$(RM) $(ICCPROFILE_DEST_DIR)/PYCC.pf
	-$(MKDIR) -p $(ICCPROFILE_DEST_DIR)
	$(CP) $(ICCPROFILE_SRC_DIR)/PYCC.pf $(ICCPROFILE_DEST_DIR)
	$(CHMOD) 444 $(ICCPROFILE_DEST_DIR)/PYCC.pf

$(ICCPROFILE_DEST_DIR)/LINEAR_RGB.pf: $(ICCPROFILE_SRC_DIR)/LINEAR_RGB.pf
	$(RM) $(ICCPROFILE_DEST_DIR)/LINEAR_RGB.pf
	-$(MKDIR) -p $(ICCPROFILE_DEST_DIR)
	$(CP) $(ICCPROFILE_SRC_DIR)/LINEAR_RGB.pf $(ICCPROFILE_DEST_DIR)
	$(CHMOD) 444 $(ICCPROFILE_DEST_DIR)/LINEAR_RGB.pf

iccprofiles.clean:
	$(RM) -r $(ICCPROFILE_DEST_DIR)

.PHONY: res.clean iccprofiles iccprofiles.clean

