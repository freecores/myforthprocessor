#
# @(#)Makefile	1.43 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../..
PACKAGE = java.security
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Rules
#
default: all
include FILES_java.gmk

#
# Directories
#

PROPS_SRC =	$(TOPDIR)/src/share/lib/security/java.security
PROPS_BUILD = $(LIBDIR)/security/java.security

POLICY_SRC =	$(TOPDIR)/src/share/lib/security/java.policy
POLICY_BUILD = $(LIBDIR)/security/java.policy

CACERTS_SRC =	$(CACERTS_FILE)
CACERTS_BUILD = $(LIBDIR)/security/cacerts

FILES_class = $(FILES_java:%.java=$(CLASSBINDIR)/%.class)

#
# Rules
#
include $(BUILDDIR)/common/Rules.gmk

all install : properties policy cacerts

properties : classes
	$(MKDIR) -p $(LIBDIR)/security
	$(RM) -r $(PROPS_BUILD)
	$(CP) $(PROPS_SRC) $(PROPS_BUILD)

policy : classes
	$(MKDIR) -p $(LIBDIR)/security
	$(RM) -r $(POLICY_BUILD)
	$(CP) $(POLICY_SRC) $(POLICY_BUILD)

cacerts : classes
	$(MKDIR) -p $(LIBDIR)/security
	$(RM) -r $(CACERTS_BUILD)
	$(CP) $(CACERTS_SRC) $(CACERTS_BUILD)

clean :: .delete.classlist
	$(RM) -r $(CLASSBINDIR)/java/security
	$(RM) $(PROPS_BUILD) $(POLICY_BUILD) $(CACERTS_BUILD)

# Additional Rule for building sun.security.util
$(CLASSBINDIR)/%.class: $(SHARE_SRC)/sun/%.java
	@$(ECHO) $? >> $(TMPDIR)/.classes.list
