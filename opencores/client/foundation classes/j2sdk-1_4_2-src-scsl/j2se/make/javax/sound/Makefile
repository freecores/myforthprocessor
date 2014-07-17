#
# @(#)Makefile	1.28 03/05/14
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../..
PACKAGE = javax.sound
LIBRARY = jsound
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

# include defines for sound
include SoundDefs.gmk

# support for different mixer provider files on different platforms */
MXSP = javax.sound.sampled.spi.MixerProvider
# Mixer Service Provider Additional path
MXSPP_ADD =

#
# Files
#
include FILES_c.gmk
include FILES_java.gmk
FILES_c += $(FILES_engine) $(FILES_$(PLATFORM))

#
# system dependent flags
#
ifeq ($(PLATFORM), windows)
  CPPFLAGS += 	-DUSE_DIRECTSOUND=0 \
		-DUSE_EXTERNAL_SYNTH=TRUE \
		-DUSE_PLATFORM_MIDI_OUT=TRUE \
		-DUSE_PLATFORM_MIDI_IN=TRUE \
		-DUSE_PORTS=TRUE
endif # PLATFORM win32

ifeq ($(PLATFORM), linux)
  ifeq ($(ARCH), i586)
    SUBDIRS += jsoundalsa
    EXTRA_SOUND_JNI_LIBS += jsoundalsa
    MXSPP_ADD = $(PLATFORM)-$(ARCH)/
  endif
endif # PLATFORM linux

ifeq ($(PLATFORM), solaris)
  CPPFLAGS += -DUSE_PORTS=TRUE
endif # PLATFORM solaris

# for dynamic inclusion of extra sound libs: these
# JNI libs will be loaded from Platform.java
CPPFLAGS += -DEXTRA_SOUND_JNI_LIBS="\"$(EXTRA_SOUND_JNI_LIBS)\""

#
# Include rules
#
include $(BUILDDIR)/common/Library.gmk

#
# Rules that copy files.
#
OPTIMIZED += copy-files recurse-sound-optimized
DEBUG += copy-files recurse-sound-debug

copy-files: $(FILES_copy)


$(SERVICEDIR)/$(MXSP): $(SHARE_SRC)/classes/com/sun/media/sound/services/$(MXSPP_ADD)$(MXSP)
	$(install-file)

$(SERVICEDIR)/%: $(SHARE_SRC)/classes/com/sun/media/sound/services/%
	$(install-file)

$(LIBDIR)/audio/%: $(SHARE_SRC)/lib/audio/%
	$(install-file)

recurse-sound-%:
	@for i in dummy $(SUBDIRS) ; do \
	  if [ ! ".$$i" = ".dummy" ] ; then \
	    $(ECHO) ">>>Recursively making "$$i" "$*" @ `$(DATE)` ..." ; \
	    $(CD) $$i ; \
	    $(MAKE) $* RELEASE=$(RELEASE) FULL_VERSION=$(FULL_VERSION) || exit 1 ; \
	    $(CD) .. ;  \
	    $(ECHO) "<<<Finished Recursively making "$$i" "%" @ `$(DATE)`." ; \
	  fi ; \
	done

#
# Since we build more than the javax.sound (as stated in the
# PACKAGE variable), we need to nuke the extra packages explicitly.
#
clean clobber::
	$(RM) -r $(CLASSBINDIR)/com/sun/media/sound $(LIBDIR)/audio
	$(RM) $(FILES_copy)
	@for i in dummy $(SUBDIRS) ; do \
	  if [ ! ".$$i" = ".dummy" ] ; then \
	    $(CD) $$i; $(MAKE) $@ RELEASE=$(RELEASE) FULL_VERSION=$(FULL_VERSION); \
	  fi ; \
	done

#
# Declare extra phony targets.
#
.PHONY: copy-files
