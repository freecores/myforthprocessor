#
# @(#)Makefile	1.20 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# pkgdefs/Makefile
#

include		../Makefile.com

SYMLINK=	$(LN) -s

all:		$(TMP_AWK) 

install:	$(TMP_AWK) 

clean:
		$(RM) $(TMP_AWK)

clobber:	clean
		$(RM) -rf $(PKG_PROTO)


$(TMP_AWK):	bld_awk_pkginfo.ksh
		$(MKDIR) -p $(TMP_DIR)
		$(KSH) ./bld_awk_pkginfo.ksh -m $(ARCH) \
		    -p "$(RELEASE)/$(FULL_VERSION)" -o $@
