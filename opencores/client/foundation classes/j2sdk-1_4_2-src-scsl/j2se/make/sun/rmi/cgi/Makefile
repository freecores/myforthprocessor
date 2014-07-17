#
# @(#)Makefile	1.13 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

# @(#)Makefile	1.13 03/01/23
#
# Makefile for building rmiregistry program
#

BUILDDIR = ../../..
PACKAGE = sun.rmi
PRODUCT = sun
PROGRAM = java-rmi
include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile.
#
FILES_c = $(TARGDIR)java-rmi.c

#
# Rules.
#
include $(BUILDDIR)/common/Program.gmk

#
# Rename .exe to .cgi.
#

$(OPTIMIZED) $(DEBUG)::
	$(MV) -f  $(BINDIR)/$(PROGRAM)$(SUFFIX).exe $(BINDIR)/$(PROGRAM)$(SUFFIX).cgi

clean:: 
	-$(RM) $(BINDIR)/$(PROGRAM).cgi
	-$(RM) $(BINDIR)/$(PROGRAM)_g.cgi

