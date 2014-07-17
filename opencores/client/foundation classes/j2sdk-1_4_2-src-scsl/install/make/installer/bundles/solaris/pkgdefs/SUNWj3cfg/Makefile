#  @(#)Makefile	1.1 03/02/19
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.

include	../Makefile.com


install: init $(PACKAGE_TMP_DIR)/pkginfo generate-prototype 

init:
	[ -d $(PACKAGE_TMP_DIR) ] || $(MKDIR) -p $(PACKAGE_TMP_DIR)
	$(CHMOD) -f +w $(PACKAGE_TMP_DIR)/*
	$(CP) prototype_com $(PROTOTYPE)
	$(CHMOD)  -f +w $(PACKAGE_TMP_DIR)/*


generate-prototype: $(PKG_ARCHIVE) FRC
	$(CP) ../common/copyright depend space postinstall $(PACKAGE_TMP_DIR)
	$(CD) $(PACKAGE_TMP_DIR); \
	$(PKGMK) -f $(PROTOTYPE) -d $(PKG_ARCHIVE) \
		 -o pkg_arch=$(ARCH) $(PACKAGE)


include	../Makefile.targ
