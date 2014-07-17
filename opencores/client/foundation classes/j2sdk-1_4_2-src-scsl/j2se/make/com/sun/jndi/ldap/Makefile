#
# @(#)Makefile	1.7 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building JNDI LDAP service provider
#

BUILDDIR = ../../../..
PACKAGE = com.sun.jndi.ldap
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

#
# Resources
#
OTHER_PROPERTIES  = jndiprovider.properties
RESOURCE_SRC_DIR  = $(SHARE_SRC)/classes/$(PKGDIR)
RESOURCE_DEST_DIR = $(CLASSDESTDIR)/$(PKGDIR)
include $(BUILDDIR)/common/Resources.gmk

clean::
	$(RM) -r $(CLASSDESTDIR)/com/sun/jndi/url/ldap
