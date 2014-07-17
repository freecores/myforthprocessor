#
# @(#)Makefile	1.5 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

include	../Makefile.com

DATAFILES += $(PACKAGE_TMP_DIR)/depend

PKGFILES =  demo

# Packages to list in the depend file (see ../Makefile.targ).
PKG_DEPENDENCIES = SUNWj3rtx

all:	$(FILES) $(PACKAGE_TMP_DIR)/action

install: all generate-prototype make-package

#Search through the j2sdk-image and get all the demo directories
#and put them in the prototype file with 755 permissions.  Then
#find all the demo files and give them 755 permissions if they 
#are executable, 644 otherwise.
generate-prototype: $(PKG_ARCHIVE) FRC
		for i in `$(CD) $(JDK_IMAGE_DIR) ; $(FIND) $(PKGFILES) -type d | $(CAT)` ; do \
		      $(ECHO) d none $(DEMO_HOME)/$$i 755 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp; \
		done;
		for i in `$(CD) $(JDK_IMAGE_DIR) ; $(FIND) $(PKGFILES) -type f | $(CAT)` ; do \
		    p=`$(FIND) $(JDK_IMAGE_DIR)/$$i \( -perm -0100 -o -perm -0010 -o -perm -0001 \) -print `; \
		    if [ -n "$$p" ]; then \
		          $(ECHO) f none $(DEMO_HOME)/$$i=$(JDK_IMAGE_DIR)/$$i 755 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp; \
		    else \
		          $(ECHO) f none $(DEMO_HOME)/$$i=$(JDK_IMAGE_DIR)/$$i 644 root bin >> $(PACKAGE_TMP_DIR)/proto.$(ARCH).tmp; \
		    fi; \
		done;

include	../Makefile.targ
