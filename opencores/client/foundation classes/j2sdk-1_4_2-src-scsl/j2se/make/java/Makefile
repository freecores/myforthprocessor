#
# @(#)Makefile	1.75 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.75 03/01/23
#
# Makefile for building all of java
#

BUILDDIR=..

PRODUCT=java

include $(BUILDDIR)/common/Defs.gmk

ifdef J2RE_ONLY
TOOLS_SUBDIRS = 
else
TOOLS_SUBDIRS = javac javah
endif

#
# The order of subdirs here is important -- we want to bootstrap as
# quickly as possible.
#

# Bootstrap
BOOTSUBDIRS=hpi version jvm redist verify fdlibm java main zip $(TOOLS_SUBDIRS)

# Others
SUBDIRS    =$(BOOTSUBDIRS) security hprof jcov math awt text applet net nio \
				sql rmi jar beans logging

ifeq ($(PLATFORM), solaris)
ifeq ($(ARCH_DATA_MODEL), 32)
SUBDIRS += jexec
endif
endif

all optimized debug clean clobber::
	@for i in $(SUBDIRS) ; do \
	    $(ECHO) ">>>Recursively making "$$i" "$@" @ `$(DATE)` ..."; \
	    $(CD) $$i; $(MAKE) $@ RELEASE=$(RELEASE) FULL_VERSION=$(FULL_VERSION)\
            || exit 1; $(CD) ..;  \
	    $(ECHO) "<<<Finished Recursively making "$$i" "$@" @ `$(DATE)`." ; \
	done
