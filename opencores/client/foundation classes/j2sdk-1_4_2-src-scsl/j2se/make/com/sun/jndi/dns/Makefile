#
# @(#)Makefile	1.4 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.4 03/01/23
#
# Makefile for building JNDI DNS Naming service provider
#

BUILDDIR = ../../../..
PACKAGE = com.sun.jndi.dns
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile
#
include FILES_java.gmk

#
# Rules
#
include $(BUILDDIR)/common/Classes.gmk

clean::
	$(RM) -r $(CLASSDESTDIR)/com/sun/jndi/url/dns
