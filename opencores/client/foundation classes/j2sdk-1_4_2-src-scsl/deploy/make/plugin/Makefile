#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.26 03/05/15
#
# Makefile to build the Java Plug-in components
#
# Build each subdirectory
BUILDDIR = ..
PLUGIN_TOP = ../..
DEPLOY_TOPDIR = ../..
include ./common/config.gmk

SUBDIRS = java
ifeq ($(PLATFORM), windows)
  SUBDIRS += com jpishare regutils controlpanel oji-plugin jinstall eula
else
  SUBDIRS += controlpanel l10n jvmexec jvmnative ns4 ns6
  ADAPTERDIRS = jcomponent ns4-adapter
endif

# To build OJI plugin for GCC 3.2 
ifeq ($(PLATFORM), linux)
  SUBDIRS += ns6-gcc32
endif

SUBDIRS += converter

all optimized debug :: $(VERSION_DEST) $(SUBDIRS)

adapter-all:: $(ADAPTERDIRS)

$(ADAPTERDIRS)::
	$(MAKE) -C $@ all

clean:
	@for i in $(SUBDIRS) ; do \
	    $(ECHO) ">>>Recursively making "$$i" "clean" @ `$(DATE)` ..."; \
	    $(CD) $$i; $(MAKE) clobber RELEASE="$(RELEASE)" FULL_VERSION="$(FULL_VERSION)" \
            || exit 1; $(CD) ..; \
	    $(ECHO) "<<<Finished Recursively making "$$i" all @ `$(DATE)`." ; \
	done
	-$(RM) -r $(PLUGINDIR)

$(SUBDIRS)::
	$(MAKE) -C $@ $(MAKECMDGOALS)

ifeq ($(PLATFORM), windows)
# versioning for windows
CONVERT_MAJOR = $(SED) -e "s/_PLUGIN_MAJOR_VER/$(JDK_MAJOR_VERSION)/g"
CONVERT_MINOR = $(SED) -e "s/_PLUGIN_MINOR_VER/$(JDK_MINOR_VERSION)/g"
CONVERT_MICRO = $(SED) -e "s/_PLUGIN_MICRO_VER/$(JDK_MICRO_VERSION)/g"
CONVERT_FULL  = $(SED) -e "s/_PLUGIN_FULL_VER/$(FULL_VERSION)/g"
CONVERT_CLSID = $(SED) -e "s/_CLSID_CHAR/$(CONVERT_CLSID_CHAR)/g"

ifndef JDK_UPDATE_VERSION
  CONVERT_UPDAT = $(SED) -e "s/_PLUGIN_UPDAT_VER/00/g"
  CONVERT_UNDERSCORE_UPDAT = $(SED) -e "s/_PLUGIN_UNDERSCORE_UPDAT_VER//g"
  CONVERT_CAB_VER = $(SED) -e "s/_CAB_VER/0/g"
else
  CONVERT_UPDAT = $(SED) -e "s/_PLUGIN_UPDAT_VER/$(CAB_CHAR1)$(CAB_CHAR2)/g"
  CONVERT_UNDERSCORE_UPDAT = $(SED) -e "s/_PLUGIN_UNDERSCORE_UPDAT_VER/"_$(JDK_UPDATE_VERSION)"/g"
  CONVERT_CAB_VER = $(SED) -e "s/_CAB_VER/$(PLUGIN_CAB_VERSION)/g"
endif

SUBSTITUTE_VERSIONS = $(CONVERT_MAJOR) | $(CONVERT_MINOR) | $(CONVERT_MICRO) | $(CONVERT_UPDAT) | $(CONVERT_UNDERSCORE_UPDAT) | $(CONVERT_FULL) | $(CONVERT_CAB_VER) | $(CONVERT_CLSID) 

$(VERSION_DEST):$(VERSION_SRC)
	$(CAT) $< | $(SUBSTITUTE_VERSIONS) > $@
endif

include ../common/Release.gmk

#
# Phonies to avoid accidents.
#
.PHONY: all optimized debug clean $(SUBDIRS)
