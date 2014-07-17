#
# @(#)Makefile	1.30 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR  = ../../..
PACKAGE   = javax.swing.plaf
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

MISC_FILES = $(MISC_SWING_FILES)
FILES_java = $(FILES_SWING_java)


#
# Resources
#
RESOURCE_BUNDLES_COMPILED_PROPERTIES = \
	com/sun/swing/internal/plaf/basic/resources/basic.properties \
	com/sun/swing/internal/plaf/metal/resources/metal.properties \
	com/sun/java/swing/plaf/motif/resources/motif.properties \
	com/sun/java/swing/plaf/windows/resources/windows.properties \
	com/sun/java/swing/plaf/gtk/resources/gtk.properties


LOCALE_SET_DEFINITION = j2re
RESOURCE_SRC_DIR  = $(SHARE_SRC)/classes
include $(BUILDDIR)/common/Resources.gmk


#
# Rules
#
all: classes other_files

clean:: classes.clean

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

$(CLASSBINDIR)/%.png: $(SHARE_SRC)/classes/%.png
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
