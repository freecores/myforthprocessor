#
# @(#)Makefile	1.6 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.6 03/01/23
#
# Makefile for locale data.
#

BUILDDIR = ../..

PACKAGE = sun.text
PRODUCT = sun

CLASSDESTDIR = $(TMPDIR)/classes
include $(BUILDDIR)/common/Defs.gmk

#
# Files
#
include FILES_java.gmk

#
# Rules
#
include $(BUILDDIR)/common/Classes.gmk

#
# Extra rules to build locale data.
#
LOCALEDATA_JAR = $(EXTDIR)/localedata.jar

SPECIALFILES = $(CLASSDESTDIR)/sun/text/resources/thai_dict

$(CLASSDESTDIR)/sun/text/resources/% : $(SHARE_SRC)/classes/sun/text/resources/%
	$(install-file)

$(LOCALEDATA_JAR): $(EXTDIR) $(FILES_class) $(SPECIALFILES)
	$(JAR) -cf $@ -C $(CLASSDESTDIR) sun

OPTIMIZED += $(LOCALEDATA_JAR)
DEBUG     += $(LOCALEDATA_JAR)

clean::
	$(RM) -r $(CLASSDESTDIR)
	$(RM) $(LOCALEDATA_JAR)

