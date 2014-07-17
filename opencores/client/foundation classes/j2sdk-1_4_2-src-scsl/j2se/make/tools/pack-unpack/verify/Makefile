#
# @(#)Makefile	1.12 03/01/23 
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
#
# Makefile for building the "verify" program for compressing Pack/Unpack

BUILDDIR= ../../..

PRODUCT = sun.tools.pack.verify

include $(BUILDDIR)/common/Defs.gmk

#
# Pack verifier needs ClassInfoLib.jar
#
ifndef CLASSINFO
ifeq ($(PLATFORM), windows)
CLASSINFO=j:/classinfo1.0
else
CLASSINFO=/import/java/classinfo1.0
endif 
endif
CLASSINFO_LIB=$(CLASSINFO)/lib/ClassInfoLib.jar
.SUFFIXES: .java .class

FILES_java = \
	sun/tools/pack/verify/ClassAttributeCompare.java \
	sun/tools/pack/verify/ClassCompare.java \
	sun/tools/pack/verify/ClassMetaData.java \
	sun/tools/pack/verify/CPoolCompare.java \
	sun/tools/pack/verify/CPoolRef.java \
	sun/tools/pack/verify/FieldCompare.java \
	sun/tools/pack/verify/Globals.java \
	sun/tools/pack/verify/JarFileCompare.java \
	sun/tools/pack/verify/Main.java \
	sun/tools/pack/verify/MethodCodeCompare.java \
	sun/tools/pack/verify/MethodCompare.java \
	sun/tools/pack/verify/MethodInstructionVisitor.java \
	sun/tools/pack/verify/VerifyTreeSet.java

SOURCEPATH = .
CLASSDESTDIR = $(TMPDIR)
CLASSPATH = $(TMPDIR)$(CLASSPATH_SEPARATOR)$(CLASSINFO_LIB)
THIS_JAVAC_CMD = $(JAVAC) -classpath "$(JAVAC_CP)$(CLASSPATH_SEPARATOR)$(CLASSPATH)" $(JAVAC_SRC_FLAG) $(JAVAC_DST_FLAG)
VERIFY_CMD=$(JAVA) -classpath "$(CLASSPATH)" sun.tools.pack.verify.Main

all: build do-verify

build: 
	$(MKDIR) -p $(TMPDIR)/sun/tools/
	$(THIS_JAVAC_CMD) $(FILES_java)

do-verify: jre-verify  jdk-verify jdk-verify-strip

jre-verify:
	$(MAKE) verify VARIANT=jre CD_STRIP="" 

jdk-verify:
	$(MAKE) verify VARIANT=jdk CD_STRIP="" VARPATH=/jre 

jdk-verify-strip:
	$(MAKE) verify VARIANT=jdk CD_STRIP=_strip VARPATH=/jre JFLAGS="-c -d" NFLAGS="-c -d"

verify: build rt-jar jsse-jar charsets-jar localedata-jar  jaws-jar tools-jar

rt-jar:
	@$(ECHO) "rt.jar"
	$(UNPACK_CMD)   	$(NFLAGS) $(OUTPUTDIR)/pack/pack-$(VARIANT)-jars$(VARPATH)/lib/rt.pack \
					$(TMPDIR)/rt_$(VARIANT)$(CD_STRIP)_native.jar
	$(UNPACK_JAVA_CMD) 	$(JFLAGS) $(OUTPUTDIR)/pack/pack-$(VARIANT)-jars$(VARPATH)/lib/rt.pack \
					$(TMPDIR)/rt_$(VARIANT)$(CD_STRIP)_java.jar
	$(VERIFY_CMD) 		$(LIBDIR)/rt.jar $(TMPDIR)/rt_$(VARIANT)$(CD_STRIP)_native.jar \
					-l $(TMPDIR)/verifier_rt_$(VARIANT)$(CD_STRIP)_native.log
	$(VERIFY_CMD) 		$(LIBDIR)/rt.jar $(TMPDIR)/rt_$(VARIANT)$(CD_STRIP)_java.jar \
					-l $(TMPDIR)/verifier_rt_$(VARIANT)$(CD_STRIP)_java.log
jsse-jar:
	@$(ECHO) "jsse.jar"
	$(UNPACK_CMD)   	$(NFLAGS) $(OUTPUTDIR)/pack/pack-$(VARIANT)-jars$(VARPATH)/lib/jsse.pack \
					$(TMPDIR)/jsse_$(VARIANT)$(CD_STRIP)_native.jar
	$(UNPACK_JAVA_CMD) 	$(JFLAGS) $(OUTPUTDIR)/pack/pack-$(VARIANT)-jars$(VARPATH)/lib/jsse.pack \
					$(TMPDIR)/jsse_$(VARIANT)$(CD_STRIP)_java.jar
	$(VERIFY_CMD) 		$(LIBDIR)/jsse.jar $(TMPDIR)/jsse_$(VARIANT)$(CD_STRIP)_native.jar \
					-l $(TMPDIR)/verifier_jsse_$(VARIANT)$(CD_STRIP)_native.log
	$(VERIFY_CMD) 		$(LIBDIR)/jsse.jar $(TMPDIR)/jsse_$(VARIANT)$(CD_STRIP)_java.jar \
					-l $(TMPDIR)/verifier_jsse_$(VARIANT)$(CD_STRIP)_java.log
charsets-jar:
	@$(ECHO) "charsets.jar"
	$(UNPACK_CMD)   	$(NFLAGS) $(OUTPUTDIR)/pack/pack-$(VARIANT)-jars$(VARPATH)/lib/charsets.pack \
					$(TMPDIR)/charsets_$(VARIANT)$(CD_STRIP)_native.jar
	$(UNPACK_JAVA_CMD) 	$(JFLAGS) $(OUTPUTDIR)/pack/pack-$(VARIANT)-jars$(VARPATH)/lib/charsets.pack \
					$(TMPDIR)/charsets_$(VARIANT)$(CD_STRIP)_java.jar
	$(VERIFY_CMD) 		$(LIBDIR)/charsets.jar $(TMPDIR)/charsets_$(VARIANT)$(CD_STRIP)_native.jar \
					-l $(TMPDIR)/verifier_charsets_$(VARIANT)$(CD_STRIP)_native.log
	$(VERIFY_CMD) 		$(LIBDIR)/charsets.jar $(TMPDIR)/charsets_$(VARIANT)$(CD_STRIP)_java.jar \
					-l $(TMPDIR)/verifier_charsets_$(VARIANT)$(CD_STRIP)_java.log
localedata-jar:
	@$(ECHO) "localedata.jar"
	$(UNPACK_CMD)   	$(NFLAGS) $(OUTPUTDIR)/pack/pack-$(VARIANT)-jars$(VARPATH)/lib/ext/localedata.pack \
					$(TMPDIR)/localedata_$(VARIANT)$(CD_STRIP)_native.jar
	$(UNPACK_JAVA_CMD) 	$(JFLAGS) $(OUTPUTDIR)/pack/pack-$(VARIANT)-jars$(VARPATH)/lib/ext/localedata.pack \
					$(TMPDIR)/localedata_$(VARIANT)$(CD_STRIP)_java.jar
	$(VERIFY_CMD) 		$(LIBDIR)/ext/localedata.jar $(TMPDIR)/localedata_$(VARIANT)$(CD_STRIP)_native.jar \
					-l $(TMPDIR)/verifier_localedata_$(VARIANT)$(CD_STRIP)_native.log
	$(VERIFY_CMD) 		$(LIBDIR)/ext/localedata.jar $(TMPDIR)/localedata_$(VARIANT)$(CD_STRIP)_java.jar \
					-l $(TMPDIR)/verifier_localedata_$(VARIANT)$(CD_STRIP)_java.log

jaws-jar:
	@$(ECHO) "jaws.jar"
ifeq ($(PLATFORM), windows) 
	$(UNPACK_CMD)   	$(NFLAGS) $(OUTPUTDIR)/pack/pack-$(VARIANT)-jars$(VARPATH)/lib/jaws.pack \
					$(TMPDIR)/jaws_$(VARIANT)$(CD_STRIP)_native.jar
	$(UNPACK_JAVA_CMD) 	$(JFLAGS) $(OUTPUTDIR)/pack/pack-$(VARIANT)-jars$(VARPATH)/lib/jaws.pack \
					$(TMPDIR)/jaws_$(VARIANT)$(CD_STRIP)_java.jar
	@# FOR WINDOWS the jaws.jar is in a wierd place
	$(VERIFY_CMD) 		$(OUTPUTDIR)/tmp/plugin/lib/jaws.jar $(TMPDIR)/jaws_$(VARIANT)$(CD_STRIP)_native.jar \
					-l $(TMPDIR)/verifier_jaws_$(VARIANT)$(CD_STRIP)_native.log
	$(VERIFY_CMD) 		$(OUTPUTDIR)/tmp/plugin/lib/jaws.jar $(TMPDIR)/jaws_$(VARIANT)$(CD_STRIP)_java.jar \
					-l $(TMPDIR)/verifier_jaws_$(VARIANT)$(CD_STRIP)_java.log
else
	$(UNPACK_CMD)   	$(NFLAGS) $(OUTPUTDIR)/pack/pack-$(VARIANT)-jars$(VARPATH)/lib/javaplugin.pack \
					$(TMPDIR)/javaplugin_$(VARIANT)$(CD_STRIP)_native.jar
	$(UNPACK_JAVA_CMD) 	$(JFLAGS) $(OUTPUTDIR)/pack/pack-$(VARIANT)-jars$(VARPATH)/lib/javaplugin.pack \
					$(TMPDIR)/javaplugin_$(VARIANT)$(CD_STRIP)_java.jar
	$(VERIFY_CMD) 		$(LIBDIR)/javaplugin.jar $(TMPDIR)/javaplugin_$(VARIANT)$(CD_STRIP)_native.jar \
					-l $(TMPDIR)/verifier_javaplugin_$(VARIANT)$(CD_STRIP)_native.log
	$(VERIFY_CMD) 		$(LIBDIR)/javaplugin.jar $(TMPDIR)/javaplugin_$(VARIANT)$(CD_STRIP)_java.jar \
					-l $(TMPDIR)/verifier_javaplugin_$(VARIANT)$(CD_STRIP)_java.log
endif

tools-jar:
	@$(ECHO) "tools.jar"
ifeq ($(VARIANT), jdk) 
	$(UNPACK_CMD)   	$(NFLAGS) $(OUTPUTDIR)/pack/pack-$(VARIANT)-jars/lib/tools.pack \
					$(TMPDIR)/tools_$(VARIANT)$(CD_STRIP)_native.jar
	$(UNPACK_JAVA_CMD) 	$(JFLAGS) $(OUTPUTDIR)/pack/pack-$(VARIANT)-jars/lib/tools.pack \
					$(TMPDIR)/tools_$(VARIANT)$(CD_STRIP)_java.jar
	$(VERIFY_CMD) 		$(LIBDIR)/tools.jar $(TMPDIR)/tools_$(VARIANT)$(CD_STRIP)_native.jar \
					-l $(TMPDIR)/verifier_tools_$(VARIANT)$(CD_STRIP)_native.log
	$(VERIFY_CMD) 		$(LIBDIR)/tools.jar $(TMPDIR)/tools_$(VARIANT)$(CD_STRIP)_java.jar \
					-l $(TMPDIR)/verifier_tools_$(VARIANT)$(CD_STRIP)_java.log
endif


clean: 
	$(RM) -r $(TMPDIR)

.PHONY: all clean  do-verify jdk-verify jdk-verify-strip jre-verify rt-jar jsse-jar jaws-jar localedata-jar charsets-jar tools-jar
