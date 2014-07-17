#
# @(#)Makefile.com	1.10 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

include		../Makefile.master

DATAFILES=	$(PACKAGE_TMP_DIR)/copyright

TMP_AWK=	$(TMP_DIR)/awk_pkginfo

FILES=		$(DATAFILES) $(PACKAGE_TMP_DIR)/pkginfo

PACKAGE:sh=	basename `pwd`

CLOBBERFILES=	$(FILES) $(PACKAGE_TMP_DIR)/action $(PACKAGE_TMP_DIR)/pkginfo

ARCH:sh= 	uname -p
