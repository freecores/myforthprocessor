#
# @(#)Makefile	1.3 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.3 03/01/23
#
# Makefile for indic input method.
#

BUILDDIR = ../../../..

PACKAGE = com.sun.inputmethods.internal.indicim
PRODUCT = sun

CLASSDESTDIR = $(TMPDIR)/classes
SERVICESDIR=META-INF/services
IMJAR = $(IMJARDIR)/indicim.jar
IMJARDIR=$(LIBDIR)/im
include $(BUILDDIR)/common/Defs.gmk

#
# Files
#
include FILES_java.gmk

FILES_copy = \
	$(CLASSDESTDIR)/com/sun/inputmethods/internal/indicim/resources/DisplayNames.properties

PROVIDER_CONF_FILE = \
	$(CLASSDESTDIR)/$(SERVICESDIR)/java.awt.im.spi.InputMethodDescriptor

#
# Rules
#
include $(BUILDDIR)/common/Classes.gmk

#
# Extra rules to build im.
#

$(CLASSDESTDIR)/com/sun/inputmethods/internal/indicim/resources/% : $(SHARE_SRC)/classes/com/sun/inputmethods/internal/indicim/resources/%
	$(install-file)

$(CLASSDESTDIR)/$(SERVICESDIR)/java.awt.im.spi.InputMethodDescriptor : $(SHARE_SRC)/classes/com/sun/inputmethods/internal/indicim/java.awt.im.spi.InputMethodDescriptor
	$(install-file)

$(IMJARDIR): $(LIBDIR)
	$(MKDIR) -p $(IMJARDIR)
	
$(IMJAR): $(IMJARDIR) $(FILES_class) $(FILES_copy) $(PROVIDER_CONF_FILE)
	$(JAR) -cf $@ -C $(CLASSDESTDIR) com -C $(CLASSDESTDIR) $(SERVICESDIR)

OPTIMIZED += $(IMJAR)
DEBUG     += $(IMJAR)

clean::
	$(RM) -r $(CLASSDESTDIR)
	$(RM) $(IMJAR)
