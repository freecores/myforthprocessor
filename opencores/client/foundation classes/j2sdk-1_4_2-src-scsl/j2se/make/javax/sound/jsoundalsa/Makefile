#
# @(#)Makefile	1.4 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../../..
PACKAGE = javax.sound
LIBRARY = jsoundalsa
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

# this Makefile compiles the native files for alsa support in java sound

# include defines for sound
include ../SoundDefs.gmk

#
# Files
#
FILES_java = $(TARGDIR)com/sun/media/sound/DirectAudioDeviceProvider.java \
	$(TARGDIR)com/sun/media/sound/DirectAudioDevice.java

FILES_c = \
	$(CTARGDIR)Utilities.c \
	$(CTARGDIR)DirectAudioDeviceProvider.c \
	$(CTARGDIR)DirectAudioDevice.c \

#FILES_solaris =
#	$(CTARGDIR)PLATFORM_API_SolarisOS_Mixer.c

FILES_linux = \
	$(CTARGDIR)PLATFORM_API_LinuxOS_ALSA_Utils.c \
	$(CTARGDIR)PLATFORM_API_LinuxOS_ALSA_PCM.c

#FILES_windows =
#	$(CTARGDIR)PLATFORM_API_WinOS_DirectSound.c

FILES_export = \
	com/sun/media/sound/DirectAudioDeviceProvider.java \
	com/sun/media/sound/DirectAudioDevice.java

FILES_c += $(FILES_$(PLATFORM))

#ifeq ($(PLATFORM), windows)
#  LDLIBS += dsound.lib
#endif # PLATFORM win32

ifeq ($(PLATFORM), linux)
  LDFLAGS += -lasound
endif # PLATFORM linux

CPPFLAGS += -DUSE_DAUDIO=TRUE

#
# Include rules
#
include $(BUILDDIR)/common/Library.gmk



