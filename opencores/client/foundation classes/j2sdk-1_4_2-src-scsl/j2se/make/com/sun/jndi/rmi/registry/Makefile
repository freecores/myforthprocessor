#
# @(#)Makefile	1.8 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building JNDI RMI registry service provider
#

BUILDDIR = ../../../../..
PACKAGE = com.sun.jndi.rmi.registry
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
	$(RM) -r $(CLASSDESTDIR)/com/sun/jndi/url/rmi


#
# The remainder of this makefile contains defs and rules
# for generating RMI stubs.
# With inspiration from build/solaris/sun/rmi/rmi/GNUmakefile.
#

#
# Full class names of implementations requiring stubs
#
REMOTE_impls = \
	$(PACKAGE).ReferenceWrapper

DEBUG     += stubs
OPTIMIZED += stubs

REMOTE_files = $(subst .,/,$(REMOTE_impls))
FILES_stubs  = $(REMOTE_files:%=$(CLASSDESTDIR)/%_Stub.class)
FILES_skels  = $(REMOTE_files:%=$(CLASSDESTDIR)/%_Skel.class)

#
# Compile stubs and skeletons for remote implementations
#
$(CLASSDESTDIR)/%_Skel.class $(CLASSDESTDIR)/%_Stub.class:	\
		$(CLASSDESTDIR)/%.class
	$(RMIC) -classpath "$(CLASSDESTDIR)"	\
		-d $(CLASSDESTDIR) 				\
		$(subst /,.,$(<:$(CLASSDESTDIR)/%.class=%))

stubs: $(FILES_stubs) $(FILES_skels)
