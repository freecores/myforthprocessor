#
# @(#)Makefile	1.48 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR  = ../..
PACKAGE   = javax.swing
PRODUCT   = com
SWING_SRC = $(SHARE_SRC)/classes/javax/swing
include $(BUILDDIR)/common/Defs.gmk

ifeq ($(NOCORE),true)
ifeq ($(DEBUG_CLASSFILES),true)
JAVAC_DEBUG_FLAG = -g
endif
BINDIR=$(BOOTDIR)/bin
JAVAC_CMD  = $(LATEST_JDK12)/bin/javac $(JAVAC_DEBUG_FLAG) $(JAVAC_VMFLAGS_J) $(JAVAC_BOOTCLASSPATH_NAME) $(CLASSBINDIR)$(CLASSPATH_SEPARATOR)$(BOOTDIR)/jre/lib/rt.jar -sourcepath $(VPATH.java) -d $(CLASSDESTDIR)
endif

#
# Files
#
include FILES.gmk
include FILES_java.gmk

#
# Rules
#
all: classes other_files plaf html32dtd

clean:: classes.clean plaf.clean html32dtd.clean

html32dtd html32dtd.clean:
	@$(ECHO) ">>>Recursively making html32dtd "$@" @ `$(DATE)` ..."; \
	$(CD) html32dtd; $(MAKE) RELEASE=$(RELEASE) FULL_VERSION=$(FULL_VERSION) NOCORE=$(NOCORE) $@\
        || exit 1; $(CD) ..; \
	$(ECHO) "<<<Finished Recursively making html32dtd "$@" @ `$(DATE)`." ; \

plaf:
	@$(ECHO) ">>>Recursively making plaf all @ `$(DATE)` ..."; \
	$(CD) plaf; $(MAKE) RELEASE=$(RELEASE) FULL_VERSION=$(FULL_VERSION) NOCORE=$(NOCORE) all\
        || exit 1; $(CD) ..; \
	$(ECHO) "<<<Finished Recursively making plaf all @ `$(DATE)`." ; \

plaf.clean:
	@$(ECHO) ">>>Recursively making plaf "$@" @ `$(DATE)` ..."; \
	$(CD) plaf; $(MAKE) RELEASE=$(RELEASE) FULL_VERSION=$(FULL_VERSION) clobber\
        || exit 1; $(CD) ..; \
	$(ECHO) "<<<Finished Recursively making plaf "$@" @ `$(DATE)`." ; \

.PHONY: html32dtd html32dtd.clean plaf plaf.clean

#
# Include
#
include $(BUILDDIR)/common/Classes.gmk

other_files: $(MISC_FILES)

$(CLASSBINDIR)/%.wav: $(SHARE_SRC)/classes/%.wav
	@$(MKDIR) -p $(@D)
	@$(RM) $@
	$(CP) $< $@

$(CLASSBINDIR)/%.gif: $(SHARE_SRC)/classes/%.gif
	@$(MKDIR) -p $(@D)
	@$(RM) $@
	$(CP) $< $@

$(CLASSBINDIR)/%.css: $(SHARE_SRC)/classes/%.css
	@$(MKDIR) -p $(@D)
	@$(RM) $@
	$(CP) $< $@

$(CLASSBINDIR)/%.bdtd: $(SHARE_SRC)/classes/%.bdtd
	@$(MKDIR) -p $(@D)
	@$(RM) $@
	$(CP) $< $@

$(CLASSBINDIR)/%.txt: $(SHARE_SRC)/classes/%.txt
	@$(MKDIR) -p $(@D)
	@$(RM) $@
	$(CP) $< $@


