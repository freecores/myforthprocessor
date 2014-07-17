#
# @(#)Makefile	1.6 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../../..

PACKAGE = org.omg.PortableInterceptor
PRODUCT = org

include $(BUILDDIR)/common/Defs.gmk

#
# Files
#

include ../Messaging_java.gmk
include ../IOP_java.gmk
include ../CORBAX_java.gmk
include ../PortableInterceptor_java.gmk

FILES_java = $(PI_GENERATED_java) $(Messaging_java) $(IOP_java) $(CORBA_java)

#
# Include
#

include $(BUILDDIR)/common/Classes.gmk

