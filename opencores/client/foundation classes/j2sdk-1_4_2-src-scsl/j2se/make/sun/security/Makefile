#
# @(#)Makefile	1.44 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.44 03/01/23
#
# Build classes for security tools:
#
#		* keytool
#		* jarsigner
#		* policytool
#
# Also install wrappers for all of these.
#

BUILDDIR = ../..
include $(BUILDDIR)/common/Defs.gmk

ifdef J2RE_ONLY
TOOLS_SUBDIRS =
else 
TOOLS_SUBDIRS = jarsigner policytool
ifneq ($(PLATFORM), solaris)
    TOOLS_SUBDIRS += kinit klist ktab
endif  # solaris
endif

SUBDIRS = acl action util x509 pkcs provider tools keytool jgss validator $(TOOLS_SUBDIRS)

all optimized debug clean clobber ::
	@for i in $(SUBDIRS) ; do \
	    $(ECHO) ">>>Recursively making "$$i" "$@" @ `$(DATE)` ..."; \
	    $(CD) $$i; $(MAKE) $@ RELEASE=$(RELEASE) || exit 1; $(CD) ..; \
	    $(ECHO) "<<<Finished Recursively making "$$i" "$@" @ `$(DATE)`." ; \
	done

clean::
	$(RM) -r $(CLASSBINDIR)/sun/security
