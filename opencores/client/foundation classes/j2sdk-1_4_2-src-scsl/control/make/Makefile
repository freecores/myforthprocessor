#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.260 03/04/18
#

BUILDDIR=.
include $(BUILDDIR)/common/Platform.gmk
include $(BUILDDIR)/common/Defs.gmk

all::
	@$(ECHO) $(PLATFORM) $(ARCH) $(RELEASE) build started: $(shell $(DATE) '+%y-%m-%d %H:%M')

include $(BUILDDIR)/sanity-rules.gmk
include $(BUILDDIR)/control-rules.gmk
include $(BUILDDIR)/hotspot-rules.gmk
include $(BUILDDIR)/motif-rules.gmk
include $(BUILDDIR)/j2se-rules.gmk
include $(BUILDDIR)/generics-rules.gmk
include $(BUILDDIR)/install-rules.gmk
include $(BUILDDIR)/deploy-rules.gmk


all:: sanity $(HOTSPOT) $(MOTIF) $(GENERICS) $(J2SE) $(DEPLOY)

ifndef DEV_ONLY
  all :: $(INSTALL)
endif

ifndef DEV_ONLY
  ifndef SKIP_COMPARE_IMAGES
    all :: compare-images
  endif
endif

clobber: hotspot-clobber motif-clobber generics-clobber j2se-clobber install-clobber deploy-clobber clean-check
	$(RM) -r $(OUTPUTDIR)/*
	-($(RMDIR) -p $(OUTPUTDIR) > $(DEV_NULL) 2>&1; $(TRUE))

bringover: control-bringover hotspot-bringover motif-bringover j2se-bringover generics-bringover install-bringover deploy-bringover

source: j2se-source generics-source deploy-source

all:: 
	@$(ECHO) Control workspace build finished at `$(DATE)`


################################################################
#  PHONY
################################################################

.PHONY: all what clobber insane bringover \
	j2se-source deploy-source generics-source
