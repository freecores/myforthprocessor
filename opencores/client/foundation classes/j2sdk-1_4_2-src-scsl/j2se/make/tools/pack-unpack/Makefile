#
# @(#)Makefile	1.9 03/01/23 
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
#
# Targets:
#	all 		Builds the Java bases pack and the native unpack
#			though there is a verify it is not built from the
#			all target.
#
BUILDDIR = ../..

include $(BUILDDIR)/common/Defs.gmk

SUBDIRS = pack unpack


all optimized debug clean ::
	@for i in $(SUBDIRS) ; do \
	    $(ECHO) ">>>Recursively making "$$i" "$@" @ `$(DATE)` ..."; \
	    $(CD) $$i; $(MAKE) $@ RELEASE=$(RELEASE) FULL_VERSION=$(FULL_VERSION) \
            || exit 1; $(CD) ..;  \
	    $(ECHO) "<<<Finished Recursively making "$$i" "$@" @ `$(DATE)`." ; \
	done

clobber: clean pack-jre-clean pack-jdk-clean 
	$(RM) -r $(PACK_DIR)
	@$(CD) verify ; $(MAKE) clean

pack-jre-clean:
	$(RM) -r $(PACKED_JRE_JARDIR)

pack-jdk-clean:
	$(RM) -r $(PACKED_JDK_JARDIR)

.PHONY:  all clobber pack-jre-clean pack-jdk-clean clobber 
