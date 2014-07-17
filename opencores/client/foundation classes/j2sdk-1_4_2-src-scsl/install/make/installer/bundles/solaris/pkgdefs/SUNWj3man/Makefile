#
# @(#)Makefile	1.10 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

include	../Makefile.com

DATAFILES += $(PACKAGE_TMP_DIR)/depend

# Packages to list in the depend file (see ../Makefile.targ).
PKG_DEPENDENCIES = SUNWj3rt

EXTRA_FILES = javaman.ent

all:	$(FILES) $(PACKAGE_TMP_DIR)/action

install: all man-pkg

include	../Makefile.targ
