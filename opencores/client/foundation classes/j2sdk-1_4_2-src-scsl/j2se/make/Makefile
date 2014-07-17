#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.240 03/01/23
#
# Makefile for building and packaging all of the JDK and the JRE. See
# also included files.
#

#
# Most frequently used targets:
#
#    all            -- build optimized and debug versions of the core JDK
#                      (defined in this makefile only)
#    docs           -- run javadoc to generate the JDK documentation (defined 
#                      in Docs.gmk)
#    images         -- build the J2SDK and J2RE image directories (defined in 
#                      Release.gmk)
#    clobber        -- clobber the builds of the core JDK (defined in this
#                      makefile only)

all:: 
	@$(ECHO) $(PLATFORM) $(ARCH) $(RELEASE) build started: $(shell $(DATE) '+%y-%m-%d %H:%M')

BUILDDIR=.
include $(BUILDDIR)/common/Defs.gmk

#
# Core.
#
SUBDIRS    = java javax org sun sunw com jpda mkdemo tools/pack-unpack
all optimized debug:: sanity-all post-sanity-all
	@for i in $(SUBDIRS) ; do \
	    $(ECHO) ">>>Recursively making "$$i" "$@" @ `$(DATE)` ..."; \
	    $(CD) $$i; $(MAKE) $@ RELEASE=$(RELEASE) FULL_VERSION=$(FULL_VERSION) \
            || exit 1; $(CD) ..; \
	    $(ECHO) "<<<Finished Recursively making "$$i" "$@" @ `$(DATE)`." ; \
	done

#
# Docs
#
DOCSUBDIRS = docs
docs: sanity-docs post-sanity-docs
	@for i in $(DOCSUBDIRS) ; do \
	    $(ECHO) ">>>Recursively making "$$i" "$@" @ `$(DATE)` ..."; \
	    $(CD) $$i; $(MAKE) all RELEASE=$(RELEASE) FULL_VERSION=$(FULL_VERSION) \
            || exit 1; $(CD) ..; \
	    $(ECHO) "<<<Finished Recursively making "$$i" "$@" @ `$(DATE)`." ; \
	done

#
# Release engineering targets.
#
include $(BUILDDIR)/common/Release.gmk

#
# Cscope targets.
#
include $(BUILDDIR)/common/Cscope.gmk

#
# Sanity checks.
#
include $(BUILDDIR)/common/Sanity.gmk

$(OUTPUTDIR) $(TMPDIR):
	$(MKDIR) -p $@

# cleanup everything. If the workspace is not being built by the control
# workspace, and if it is a Teamware workspace, then see if there are
# any files which are not under SCCS control.
clobber:: images-clobber
	$(RM) -r $(CLASSDESTDIR) $(BINDIR) $(LIBDIR) $(GENSRCDIR) 
	$(RM) -r $(INCLUDEDIR) $(DEMODIR) $(DOCSDIR) $(TMPDIR)
	$(RM) -r $(JDK_IMAGE_DIR) $(JDK_DEBUG_IMAGE_DIR) $(JRE_IMAGE_DIR)
	$(RM) $(WARNING_FILE) $(ERROR_FILE)
	-($(RMDIR) -p $(OUTPUTDIR) > $(DEV_NULL) 2>&1; $(TRUE))
ifndef EXTERNALSANITYCONTROL
	@if [ -d $(TOPDIR)/Codemgr_wsdata ]; then \
	  $(ECHO) '\nPerforming workspace scan for remnant files.\n' \
		'  Any files listed below are not under SCCS control in the workspace\n' \
		'  and you should review them and possibly remove them manually:' ; \
	  $(FIND) $(TOPDIR)/make $(TOPDIR)/src $(TOPDIR)/ext -type f | \
		$(SED) 's+SCCS/[ps]\.++' | $(SORT) | $(UNIQ) -c | $(NAWK) '$$1<2 {print $$2;}' ; \
	  $(ECHO) 'End of workspace scan.' ; \
	fi
endif

# this should be the last rule in this file:
all::
	@if [ -r $(WARNING_FILE) ]; then \
	  $(CAT) $(WARNING_FILE) ; \
	fi
	@$(ECHO) $(PLATFORM) $(ARCH) $(RELEASE) build finished: $(shell $(DATE) '+%y-%m-%d %H:%M')

#
# Phonies to avoid accidents.
#
.PHONY: all optimized debug clobber
