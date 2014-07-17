#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.236 02/09/19
#
# Makefile for building java-web-start 
#

#  
# Note: The deploy workspace is 32-bit only 
#
# Most frequently used targets:
#
#    all            -- build javaws and plugin
#    javaws-all     -- build only the javaws
#    plugin-all     -- build only the plugin
#    clobber        -- clobber javaws and plugin
#    images         -- build the jaws and plugin images 

BUILDDIR=.
DEPLOY_TOPDIR=..
include $(BUILDDIR)/common/Defs.gmk

all:: 
	@$(ECHO) $(PLATFORM) $(ARCH) $(RELEASE) deploy build started: $(shell $(DATE) '+%y-%m-%d %H:%M')

#
# Core.
#
JAVAWS_SUBDIR = javaws
PLUGIN_SUBDIR = plugin
SUBDIRS = $(JAVAWS_SUBDIR) $(PLUGIN_SUBDIR)

all optimized debug::
	@for i in $(SUBDIRS) ; do \
	    $(ECHO) ">>>Recursively making "$$i" "$@" @ `$(DATE)` ..."; \
	    $(CD) $$i; $(MAKE) $(MAKEOVERRIDES) ABS_BUILDDIR=$(ABS_BUILDDIR) sanity-$@ $@ || exit 1; $(CD) ..; \
	    $(ECHO) "<<<Finished Recursively making "$$i" "$@" @ `$(DATE)`." ; \
	done

javaws-all: sanity-javaws
	$(CD) $(BUILDDIR)/javaws ; $(MAKE) installer-int dev-all doc aubundle

plugin-all: sanity-plugin
   ifeq ($(BUILD_PLUGIN), true)
	$(CD) $(BUILDDIR)/plugin ; $(MAKE) all
   endif

#
# Build the images for both plugin and webstart
# Note: webstart temp image must already be built.
#
images:
	$(CD) $(BUILDDIR)/plugin ; $(MAKE) deploy-images BUILD_PLUGIN=$(BUILD_PLUGIN)

#
# Sanity checks.
#
include $(BUILDDIR)/common/Sanity.gmk

dummy := $(shell $(MKDIR) -p $(OUTPUTDIR) $(TMPDIR))

clean: clobber

# cleanup everything. If the workspace is not being built by the control
# workspace, and if it is a Teamware workspace, then see if there are
# any files which are not under SCCS control.
clobber:
	@$(ECHO) ">>>Recursively making "$(JAVAWS_SUBDIR)" "clobber" @ `$(DATE)` ..."
	@$(CD) $(JAVAWS_SUBDIR); $(MAKE) clobber || exit 1; $(CD) ..
	@$(ECHO) "<<<Finished Recursively making "$(JAVAWS_SUBDIR)" clobber @ `$(DATE)`."
	@$(ECHO) ">>>Recursively making "$(PLUGIN_SUBDIR)" "clean" @ `$(DATE)` ..."
	@$(CD) $(PLUGIN_SUBDIR); $(MAKE) clean RELEASE="$(RELEASE)" \
		FULL_VERSION="$(FULL_VERSION)" || exit 1; $(CD) ..
	@$(ECHO) "<<<Finished Recursively making "$(PLUGIN_SUBDIR)" clean @ `$(DATE)`."
	$(CD) $(BUILDDIR)/plugin ; $(MAKE) deploy-images-clobber

# this should be the last rule in this file:
all::
	@$(ECHO) $(ARCH) "Deploy Build finished:  " $(FULL_VERSION)

#
# Phonies to avoid accidents.
#
.PHONY: all optimized javaws-all plugin-all images debug clobber clean
