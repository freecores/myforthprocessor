#
# @(#)Makefile	1.30 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.30 03/01/23
#
# Install a wrapper for the "rmid" command. The associated classfiles
# are built at a higher level.
#

BUILDDIR = ../../..
PACKAGE = sun.rmi.activation
PRODUCT = sun
PROGRAM = rmid
include $(BUILDDIR)/common/Defs.gmk

#
# Rules to install the wrapper script
#
JAVA_ARGS = "{ \"-J-ms8m\", \"sun.rmi.server.Activation\" }"
include $(BUILDDIR)/common/Program.gmk

#
# Resources.
#
LOCALE_SET_DEFINITION = j2re
RESOURCE_BUNDLES_PROPERTIES = rmid.properties
RESOURCE_SRC_DIR  = $(SHARE_SRC)/classes/sun/rmi/server/resources
RESOURCE_DEST_DIR = $(CLASSBINDIR)/sun/rmi/rmid/resources
include $(BUILDDIR)/common/Resources.gmk

#
# Extra dependencies.
#
classes: stubs

#
# Rules to build activation-system stubs
#
REMOTE_impls = \
	sun.rmi.server.Activation$$ActivatorImpl \
	sun.rmi.server.Activation$$ActivationSystemImpl \
	sun.rmi.server.Activation$$ActivationMonitorImpl \
	java.rmi.activation.ActivationGroup

REMOTE_files = $(subst .,/,$(REMOTE_impls))
FILES_stubs = $(REMOTE_files:%=$(CLASSBINDIR)/%_Stub.class)

# Full package names of implementations requiring stubs

#
# Compile stubs for remote implementations
# (use -v1.2 stubs for activation system)
#

$(CLASSBINDIR)/%_Stub.class: $(CLASSBINDIR)/%.class
	$(RMIC) -v1.2 -classpath $(CLASSBINDIR)	\
	    	-d $(CLASSBINDIR) '$(subst /,.,$(<:$(CLASSBINDIR)/%.class=%))'

stubs: $(FILES_stubs)

.PHONY: stubs
