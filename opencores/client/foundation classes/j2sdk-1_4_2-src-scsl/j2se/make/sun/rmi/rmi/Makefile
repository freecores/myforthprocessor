#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.27 03/01/23
#
# Makefile for building sun.rmi.*
#

BUILDDIR = ../../..
PACKAGE = sun.rmi
PRODUCT = sun
LIBRARY = rmi
include $(BUILDDIR)/common/Defs.gmk

#
# Java files to compile.
#
include FILES_java.gmk

#
# Native files to compile.
#
FILES_c = \
	$(CTARGDIR)sun/rmi/server/MarshalInputStream.c

#
# Add ambient vpath to pick up files not part of sun.rmi package
#
vpath %.c $(SHARE_SRC)/native/sun/rmi/server

#
# Exported files that require generated .h 
#
FILES_export = \
    sun/rmi/server/MarshalInputStream.java

#
# Link to JVM for JVM_LatestUserDefinedLoader
#
OTHER_LDLIBS = $(JVMLIB)

#
# Rules
#
include $(BUILDDIR)/common/Library.gmk

#
# Full package names of implementations requiring stubs
#
REMOTE_impls = \
	sun.rmi.registry.RegistryImpl \
	sun.rmi.transport.DGCImpl

ifeq ($(PLATFORM), windows)
DEBUG     += stubs
OPTIMIZED += stubs
else # PLATFORM
ifneq ($(ARCH_DATA_MODEL), 32)
DEBUG     += stubs
OPTIMIZED += stubs
else # ARCH_DATA_MODEL
DEBUG     += stubs bin
OPTIMIZED += stubs bin
endif
endif

clean:: bin.clean


#
# Compile stubs and skeletons for remote implementations
# (use -v1.1 for backward interoperability)
#
#  gnumake 3.78.1 on windows attempts to build the target even
#  though it exists. Not sure why, but a check for file existence
#  has been added...
#
$(CLASSBINDIR)/%_Skel.class $(CLASSBINDIR)/%_Stub.class: $(CLASSBINDIR)/%.class
	if [ ! -s $@ ] ; \
	then $(RMIC) -v1.1 -classpath \
	"$(CLASSBINDIR)" \
	-d $(CLASSBINDIR) '$(subst /,.,$(<:$(CLASSBINDIR)/%.class=%))' ; \
	fi
REMOTE_files = $(subst .,/,$(REMOTE_impls))
FILES_stubs  = $(REMOTE_files:%=$(CLASSBINDIR)/%_Stub.class)
FILES_skels  = $(REMOTE_files:%=$(CLASSBINDIR)/%_Skel.class)

stubs: $(FILES_stubs) $(FILES_skels)

bin: $(TMPDIR)/.bin.dirs $(PLATFORM_SRC)/bin/java-rmi.cgi.sh
	$(RM) -r $(BINDIR)/java-rmi.cgi
	$(CP) $(PLATFORM_SRC)/bin/java-rmi.cgi.sh $(BINDIR)/java-rmi.cgi
	$(CHMOD) +x $(BINDIR)/java-rmi.cgi

bin.clean:
	$(RM) -r $(TMPDIR)/.bin.* $(BINDIR)/java-rmi.cgi

$(TMPDIR)/.bin.dirs: $(TMPDIR)
	$(MKDIR) -p $(BINDIR)
	$(TOUCH) $(TMPDIR)/.bin.dirs
