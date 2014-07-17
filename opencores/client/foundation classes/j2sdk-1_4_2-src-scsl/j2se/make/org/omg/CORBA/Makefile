#
# @(#)Makefile	1.28 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../../..

PACKAGE = org.omg.CORBA
PRODUCT = org

include $(BUILDDIR)/common/Defs.gmk

#
# Files
#
include FILES_java.gmk

ifdef STANDALONE_CORBA_WS
ORG_OMG_CLASSES11=false
else
ORG_OMG_CLASSES11=true
endif

ifeq ($(ORG_OMG_CLASSES11),true)
	CORBA_java = $(CORBA_1_2) $(CORBA_1_3) $(CORBA_NEW)
else
	CORBA_java = $(CORBA_NEW) $(CORBA_TSIDENTIFICATION)
endif

FILES_java = \
	$(SENDINGCONTEXT) \
	$(CORBA_2_3) \
	$(CORBA_java)

#
# Include
#
include $(BUILDDIR)/common/Classes.gmk

