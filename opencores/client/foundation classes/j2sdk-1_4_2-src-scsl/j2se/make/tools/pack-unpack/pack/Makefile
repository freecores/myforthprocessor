#
# @(#)Makefile	1.6 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
#
# Makefile for building the "pack" program for compressing rt.jar

BUILDDIR= ../../..

PRODUCT = sun.tools.pack 

include $(BUILDDIR)/common/Defs.gmk

.SUFFIXES: .java .class

FILES_java = \
	$(FILES_java_common) $(DebugSwitch_off)
FILES_java_DebugSwitch_on = \
	$(FILES_java_common) $(DebugSwitch_on)
FILES_java_common = \
	sun/tools/io/ByteCountOutputStream.java \
	sun/tools/io/RenamingOutputStream.java \
	sun/tools/pack/BandStructure.java \
	sun/tools/pack/ClassReader.java \
	sun/tools/pack/ClassWriter.java \
	sun/tools/pack/Code.java \
	sun/tools/pack/Coding.java \
	sun/tools/pack/CodingChooser.java \
	sun/tools/pack/ConstantPool.java \
	sun/tools/pack/Constants.java \
	sun/tools/pack/Instruction.java \
	sun/tools/pack/Pack.java \
	sun/tools/pack/Package.java \
	sun/tools/pack/PackageReader.java \
	sun/tools/pack/PackageWriter.java \
	sun/tools/pack/Unpack.java

SOURCEPATH = .
CLASSDESTDIR = $(TMPDIR)

DebugSwitch_off = sun/tools/pack/DebugSwitch.java
DebugSwitch_on  = $(CLASSDESTDIR)/sun/tools/pack/DebugSwitch.java

all: $(FILES_java)
	$(MKDIR) -p $(CLASSDESTDIR)/sun/tools
	$(JAVAC_CMD) $(FILES_java)

debug: $(FILES_java_DebugSwitch_on)
	$(MKDIR) -p $(CLASSDESTDIR)/sun/tools
	$(JAVAC_CMD) $(FILES_java_DebugSwitch_on)

$(DebugSwitch_on): $(DebugSwitch_off)
	@$(ECHO) '# Generating debugging version of DebugSwitch.java'
	$(SED) '/debug *=/{s/false/true/;s/production/MACHINE GENERATED debug/;}' \
	  < $(DebugSwitch_off) \
	  > $(DebugSwitch_on)

clean: 
	$(RM) -r $(CLASSDESTDIR)/sun/tools

.PHONY: all clean debug
