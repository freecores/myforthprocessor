#
# @(#)Makefile	1.10 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.10 03/01/23
#

BUILDDIR = ../../../..
PACKAGE = com.sun.security.auth
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files
#
include FILES_java.gmk

#
# Resources
#
LOCALE_SET_DEFINITION = j2re
RESOURCE_BUNDLES_JAVA = $(TARGDIR)/sun/security/util/AuthResources.java
include $(BUILDDIR)/common/Resources.gmk

#
# Rules
#
include $(BUILDDIR)/common/Classes.gmk
