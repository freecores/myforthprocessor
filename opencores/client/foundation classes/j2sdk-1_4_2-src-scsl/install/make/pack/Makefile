#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
#
# @(#)Makefile	1.3 03/01/23
#
#
# Makefile for PACKing JRE and SDK jar files prior to building bundles
#

BUILDDIR=..
include $(BUILDDIR)/common/Defs.gmk
include $(BUILDDIR)/common/Pack.gmk


all:: 
	@$(ECHO) $(ARCH) "Installer Build started:  " $(FULL_VERSION)

all optimized debug:: pack-jre pack-sdk

pack-jre:
	$(MAKE) PACK_OUTPUTDIR=$(PACKED_JRE_JARDIR) \
		VAR_IMAGE_DIR=$(JRE_IMAGE_DIR) \
		JARS_TO_PACK="$(JRE_PACKED_JARS)" \
		pack-jars

pack-sdk:
	$(MAKE) PACK_OUTPUTDIR=$(PACKED_JDK_JARDIR) \
		VAR_IMAGE_DIR=$(JDK_IMAGE_DIR) \
		JARS_TO_PACK="$(JDK_PACKED_JARS)" \
		PACK_FLAGS+="-C -D" \
		pack-jars

pack-jars:
	$(MKDIR) -p $(PACK_OUTPUTDIR)
	for i in $(JARS_TO_PACK) ; do \
	    $(LS) -l $(VAR_IMAGE_DIR)/$$i | $(NAWK) '{printf("Packing %s %s bytes\n",$$9, $$5)}'  ; \
	    $(MKDIR) -p $(PACK_OUTPUTDIR)/`$(DIRNAME) $$i` ; \
            $(MKDIR) -p $(VAR_IMAGE_DIR)/`$(DIRNAME) $$i` ; \
	    if [ "x$(NO_PACK)" = "x" ]; then \
		$(PACK_CMD) $(PACK_FLAGS) $(PACK_OUTPUTDIR)/`$(DIRNAME) $$i`/`$(BASENAME) $$i .jar`$(PACK_SUFFIX) $(VAR_IMAGE_DIR)/$$i ; \
		$(RM) $(VAR_IMAGE_DIR)/$$i ; \
		$(UNPACK_CMD) $(UNPACK_FLAGS) $(PACK_OUTPUTDIR)/`$(DIRNAME) $$i`/`$(BASENAME) $$i .jar`$(PACK_SUFFIX) $(VAR_IMAGE_DIR)/$$i ; \
	    else \
		$(CP) $(VAR_IMAGE_DIR)/$$i $(PACK_OUTPUTDIR)/`$(DIRNAME) $$i`/`$(BASENAME) $$i .jar`$(PACK_SUFFIX) ; \
	    fi ; \
	done

clean: clobber

clobber:
	rm -rf $(PACKED_JRE_JARDIR)
	rm -rf $(PACKED_JDK_JARDIR)


# this should be the last rule in this file:
all::
	@$(ECHO) $(ARCH) "Installer Build finished:  " $(FULL_VERSION)
#
# Phonies to avoid accidents.
#
.PHONY: all pack-jre pack-sdk pack-jars
