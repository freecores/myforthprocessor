#
# @(#)Makefile	1.11 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

include	../Makefile.com

DATAFILES += $(PACKAGE_TMP_DIR)/depend

# Packages to list in the depend file (see ../Makefile.targ).
# This swing package depends on a 1.1.x SUNWjvrt package, *not* a 1.3 (j2se) package,
# so we use a SUNWjvrt dependency without version info.
PKG_DEPENDENCIES = SUNWmfrun SUNWjvrt_nover

all:	$(FILES) $(PACKAGE_TMP_DIR)/action

install: all pkg

include	../Makefile.targ
