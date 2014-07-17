#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
#
#  @(#)Makefile	1.8 03/01/23
#
#
# Most frequently used targets:
#
#    all            -- build optimized and debug versions of the core JDK and 
#                      ext (Defined in this makefile only)
#    clean          -- clobber the builds of the core JDK and ext (defined
#                      in this makefile only)
#
BUILDDIR=../../../..
PRODUCT = installer-windows

include $(BUILDDIR)/common/Defs.gmk

ifeq ($(ARCH_DATA_MODEL),32)
SUBDIRS = zipper copier touchfile jre sdk
SUBDIRS2 = jre sdk
else
SUBDIRS = sdk
SUBDIRS2 =
endif

all: sanity-bundles post-sanity-bundles
	@for i in $(SUBDIRS) ; do \
	    $(ECHO) ">>>Recursively making "$$i" "$@" @ `$(DATE)` ..."; \
	    $(CD) $$i; $(MAKE)  \
            || exit 1; $(CD) ..; \
	    $(ECHO) "<<<Finished Recursively making "$$i" "$@" @ `$(DATE)`." ; \
	done
#
# Generate regular bundles if patch is enabled
#
ifneq ($(BASE_IMAGE_FULLVERSION), $(NEW_IMAGE_FULLVERSION))
	@for i in $(SUBDIRS2) ; do \
	    $(ECHO) ">>>Recursively making "$$i" "$@" @ `$(DATE)` ..."; \
	    $(CD) $$i; $(MAKE) BASE_IMAGE_FULLVERSION="$(NEW_IMAGE_FULLVERSION)" NEW_IMAGE_FULLVERSION="$(NEW_IMAGE_FULLVERSION)" \
	    || exit 1; $(CD) ..; \
	    $(ECHO) "<<<Finished Recursively making "$$i" "$@" @ `$(DATE)`." ; \
	done
endif


clean::
	@for i in $(SUBDIRS) ; do \
	    $(ECHO) ">>>Recursively making "$$i" "clean" @ `$(DATE)` ..."; \
	    $(CD) $$i; $(MAKE) clean RELEASE="$(RELEASE)" FULL_VERSION="$(FULL_VERSION)" \
            || exit 1; $(CD) ..; \
	    $(ECHO) "<<<Finished Recursively making "$$i" all @ `$(DATE)`." ; \
	done
#
# Generate regular bundles if patch is enabled
#
ifneq ($(BASE_IMAGE_FULLVERSION), $(NEW_IMAGE_FULLVERSION))
	@for i in $(SUBDIRS2) ; do \
	    $(ECHO) ">>>Recursively making "$$i" "clean" @ `$(DATE)` ..."; \
	    $(CD) $$i; $(MAKE) clean RELEASE="$(RELEASE)" FULL_VERSION="$(FULL_VERSION)" BASE_IMAGE_FULLVERSION="$(NEW_IMAGE_FULLVERSION)" NEW_IMAGE_FULLVERSION="$(NEW_IMAGE_FULLVERSION)" \
	    || exit 1; $(CD) ..; \
	    $(ECHO) "<<<Finished Recursively making "$$i" all @ `$(DATE)`." ; \
	done
endif

include $(BUILDDIR)/common/Sanity.gmk

#
# Phonies to avoid accidents.
#
.PHONY: all clean
