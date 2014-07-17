#
# @(#)Makefile	1.9 03/01/23
# 
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

# Makefile for building the "crunch" program for compressing JARs
BUILDDIR = ../..
JAVAWS_TOPDIR = ../../..
include ../../common/Defs.gmk

SRCDIR = $(JAVAWS_TOPDIR)/src/javaws/src/win32/nsis/crunch
CRUNCHCLASSDIR=$(OUTPUTDIR)/crunchClasses

PRODUCT = sun.tools.crunch

.SUFFIXES: .java .class

FILES_java = \
    $(SRCDIR)/sun/tools/crunch/Crunch.java \
    $(SRCDIR)/sun/tools/crunch/GlobalTables.java \
    $(SRCDIR)/sun/tools/crunch/OffsetInputStream.java \
    $(SRCDIR)/sun/tools/crunch/ClassFile.java \
    $(SRCDIR)/sun/tools/crunch/Constant.java \
    $(SRCDIR)/sun/tools/crunch/Field.java \
    $(SRCDIR)/sun/tools/crunch/Method.java \
    $(SRCDIR)/sun/tools/crunch/Attribute.java \
    $(SRCDIR)/sun/tools/crunch/CodeAttribute.java \
    $(SRCDIR)/sun/tools/crunch/SyntheticAttribute.java \
    $(SRCDIR)/sun/tools/crunch/ExceptionsAttribute.java \
    $(SRCDIR)/sun/tools/crunch/InnerClassesAttribute.java \
    $(SRCDIR)/sun/tools/crunch/ValueAttribute.java

#CLASSBINDIR = $(GENSRCDIR)
SOURCEPATH = .
FILES_java_abs=$(FILES_java:$(SRCDIR)/%=%)
FILES_class = $(FILES_java_abs:%.java=$(CRUNCHCLASSDIR)/%.class)
CLASSDESTDIR = $(CRUNCHCLASSDIR)
JAVAC_CLASSPATH = $(LIBDIR_12)/tools.jar

all optimized debug: $(CRUNCHCLASSDIR)/sun/tools/crunch $(FILES_class)

$(CRUNCHCLASSDIR)/sun/tools/crunch:
	$(MKDIR) -p $(CRUNCHCLASSDIR)/sun/tools/crunch

$(CLASSDESTDIR)/%.class: $(SRCDIR)/%.java
	$(JAVAC_CMD) $(FILES_java)

clean: 
	$(RM) -r $(CRUNCHCLASSDIR)

.PHONY: clean all optimized debug
