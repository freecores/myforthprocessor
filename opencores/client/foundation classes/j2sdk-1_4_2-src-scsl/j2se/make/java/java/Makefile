#
# @(#)Makefile	1.79 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for core Java libraries
# (java.lang, java.lang.ref, java.lang.reflect, java.io, java.util)
#

BUILDDIR = ../..
PACKAGE = java.lang
LIBRARY = java
PRODUCT = java
include $(BUILDDIR)/common/Defs.gmk

# windows compiler flags
ifeq ($(PLATFORM),windows)
  OTHER_CFLAGS =
  # Files built here do not compile with warning level 3
  ifeq ($(ARCH_DATA_MODEL), 32)
    COMPILER_WARNING_LEVEL=2
  else
    COMPILER_WARNING_LEVEL=0
  endif
  # build directly into BINDIR...
  LIB_LOCATION = $(BINDIR)
endif

#
# Files to compile.
#
include FILES_c.gmk
include FILES_java.gmk
include Exportedfiles.gmk

ifeq ($(PLATFORM),windows)
FILES_java += 	$(TARGDIR)java/lang/Win32Process.java \
		$(TARGDIR)java/io/Win32FileSystem.java \
		$(TARGDIR)java/io/WinNTFileSystem.java \
		$(TARGDIR)java/util/prefs/WindowsPreferences.java \
                $(TARGDIR)java/util/prefs/WindowsPreferencesFactory.java

FILES_c    +=   $(TARGDIR)Win32Process_md.c \
		$(TARGDIR)Win32FileSystem_md.c \
		$(TARGDIR)WinNTFileSystem_md.c \
		$(TARGDIR)canonicalize_md.c \
		$(TARGDIR)dirent_md.c \
		$(TARGDIR)TimeZone.c \
		$(TARGDIR)TimeZone_md.c \
		$(TARGDIR)WindowsPreferences.c \
		$(TARGDIR)sun/security/provider/WinCAPISeedGenerator.c

else # PLATFORM
FILES_java += 	$(TARGDIR)java/lang/UNIXProcess.java \
		$(TARGDIR)java/io/UnixFileSystem.java \
		$(TARGDIR)java/util/prefs/FileSystemPreferences.java \
                $(TARGDIR)java/util/prefs/FileSystemPreferencesFactory.java \
                $(TARGDIR)sun/io/ByteToCharCOMPOUND_TEXT.java \
                $(TARGDIR)sun/io/CharToByteCOMPOUND_TEXT.java \
                $(TARGDIR)sun/io/CompoundTextSupport.java

FILES_c    +=   $(OBJDIR)/UNIXProcess_md.c \
		$(TARGDIR)UnixFileSystem_md.c \
		$(TARGDIR)canonicalize_md.c \
		$(TARGDIR)TimeZone.c \
		$(TARGDIR)TimeZone_md.c \
		$(TARGDIR)FileSystemPreferences.c

INIT += $(OBJDIR)/UNIXProcess_md.c $(GENSRCDIR)/java/lang/UNIXProcess.java

endif # PLATFORM

#
# Source files to generate before we try to compile anything
#

CLASSES_INIT = gencharsrc niosrc

niosrc: ; ($(CD) ../nio; $(MAKE) sources)
gencharsrc: $(GENSRCDIR)/java/lang/Character.java $(GENSRCDIR)/java/lang/CharacterData.java $(GENSRCDIR)/java/lang/CharacterDataLatin1.java

#
# Add to the default C file search paths
#
vpath %.c $(PLATFORM_SRC)/native/java/lang:$(SHARE_SRC)/native/java/lang
vpath %.c $(SHARE_SRC)/native/java/lang/ref
vpath %.c $(SHARE_SRC)/native/java/lang/reflect
vpath %.c $(SHARE_SRC)/native/java/io
vpath %.c $(PLATFORM_SRC)/native/java/io
vpath %.c $(SHARE_SRC)/native/java/nio
vpath %.c $(SHARE_SRC)/native/java/security
vpath %.c $(SHARE_SRC)/native/common
vpath %.c $(SHARE_SRC)/native/sun/misc
vpath %.c $(SHARE_SRC)/native/sun/reflect
vpath %.c $(SHARE_SRC)/native/java/sql
vpath %.c $(SHARE_SRC)/native/java/util
vpath %.c $(PLATFORM_SRC)/native/java/util
vpath %.c $(PLATFORM_SRC)/native/sun/security/provider

#
# Includes the fdlibm header file.
#
OTHER_INCLUDES += -I$(SHARE_SRC)/native/java/lang/fdlibm/include

#
# Also include the io_util.h header file.
#
OTHER_INCLUDES += -I$(SHARE_SRC)/native/java/io

#
# Also include the TimeZone_md.h header file.
#
OTHER_INCLUDES += -I$(PLATFORM_SRC)/native/java/util

#
# Is the altzone extern documented in ctime(3C) available?
#
ifneq ($(PLATFORM), windows)
HAVE_ALTZONE=true
endif

ifeq ($(HAVE_ALTZONE),true)
OTHER_CPPFLAGS += -DHAVE_ALTZONE
endif

#
# System.c needs RELEASE.
# java_props_md.c needs ARCH on non-Solaris platforms.
#
OTHER_CPPFLAGS += $(VERSION_DEFINES)

ifeq ($(CC_VERSION),gcc)
  # The setting of ARCH on 32-bit linux needs to be 'i386' since it
  # defines the value of the os.arch System Property.
  ifeq ($(ARCH_DATA_MODEL), 32)
    OTHER_CPPFLAGS += -DARCH='"i386"'
  else
    OTHER_CPPFLAGS += -DARCH='"$(ARCH)"'
  endif
endif

#
# Is the key in the call nl_langinfo(CODESET) called something other than
# CODESET?
#
ifdef ALT_CODESET_KEY
OTHER_CPPFLAGS += -DALT_CODESET_KEY=$(ALT_CODESET_KEY)
endif

#
# Install .lib file.
#
INSTALL_DOT_LIB = true

#
# What to link?
#
ifeq ($(PLATFORM),windows)
OTHER_LDLIBS += $(JVMLIB) -libpath:$(OBJDIR)/../../../fdlibm/$(OBJDIRNAME) fdlibm$(SUFFIX).lib \
		       -libpath:$(OBJDIR)/../../../verify/$(OBJDIRNAME) verify$(SUFFIX).lib
else
LIBSOCKET = -lsocket
OTHER_LDLIBS += $(JVMLIB) -lverify$(SUFFIX) $(LIBSOCKET) -lnsl -ldl \
		-L$(OBJDIR)/../../../fdlibm/$(OBJDIRNAME) -lfdlibm$(SUFFIX).$(ARCH)
endif

#
# Rules.
#
ifeq ($(PLATFORM), solaris)
  FILES_reorder += reorder-$(ARCH)
endif
include $(BUILDDIR)/common/Mapfile-vers.gmk
include $(BUILDDIR)/common/Library.gmk

ifneq ($(PLATFORM),windows)

# UNIXProcess.java and UNIXProcess_md.c are different for
# solaris and linux. We need to copy the correct UNIXProcess.java 
# over to $(GENSRCDIR)/java/lang/ and the correct UNIXProcess_md.c
# to $(OBJDIR). 

$(GENSRCDIR)/java/lang:
	$(MKDIR) -p $@

$(OBJDIR)/UNIXProcess_md.c: $(OBJDIR)
	@# try to remove it just in case there is a clock-skew 
	@# problem that would force a second copy...
	-$(RM) $@
	$(CP) $(PLATFORM_SRC)/native/java/lang/$(@F).$(PLATFORM) $@

$(GENSRCDIR)/java/lang/UNIXProcess.java:  $(GENSRCDIR)/java/lang
	@# try to remove it just in case there is a clock-skew 
	@# problem that would force a second copy...
	-$(RM) $@
	$(CP) $(PLATFORM_SRC)/classes/java/lang/$(@F).$(PLATFORM) $@

clean:: 
	$(RM) $(OBJDIR)/UNIXProcess_md.c
	$(RM) $(GENSRCDIR)/java/lang/UNIXProcess.java
endif


#
# Because we can't link against ourselves!
#
JAVALIB =

#
# Bootstrap stage.
#
JAVAC_CMD = $(JAVAC_BOOT_CMD) -source 1.4
JAVAH_CMD = $(JAVAH_BOOT_CMD)

#
# Has been converted to the JNI: generate JNI-style header files
#
JAVAHFLAGS += -jni

#
# Special rules.
#
clean:: 
	$(RM) -r $(CLASSHDRDIR)

clobber:: 
	$(RM) $(OPTIMIZED) $(DEBUG) $(PRODUCT) $(PRODUCT)_g
	$(RM) -r $(CLASSBINDIR)/java/io $(CLASSBINDIR)/java/lang \
		$(CLASSBINDIR)/java/security $(CLASSBINDIR)/java/util \
		$(CLASSBINDIR)/sun/io $(CLASSBINDIR)/sun/misc

#
# Additional rule from sun/net/GNUmakefile to copy content-type.properties
# file so that the MimeTable class won't complain while bootstrapping...
#
PROPS = content-types.properties

#
# Rule to copy tzmappings file on Windows
#
ifeq ($(PLATFORM), windows)
TZMAP = $(LIBDIR)/tzmappings
TZMAPFILE = $(PLATFORM_SRC)/lib/tzmappings

$(TZMAP): $(TZMAPFILE)
	$(RM) $@
	$(CP) $< $@
	$(CHMOD) 444 $@
endif

$(OPTIMIZED) $(DEBUG):: $(TMPDIR)/.lib.dirs $(LIBDIR)/$(PROPS) $(TZMAP)

$(LIBDIR)/$(PROPS): $(PLATFORM_SRC)/lib/$(PROPS)
	$(RM) -r $@
	$(CP) $< $@

clean:: 
	$(RM) -r $(TMPDIR)/.lib.dirs $(LIBDIR)/$(PROPS) $(TZMAP)

$(TMPDIR)/.lib.dirs:
	$(MKDIR) -p $(LIBDIR)
	$(TOUCH) $(TMPDIR)/.lib.dirs

#
# Rules to create java/util/CurrencyData.java
#

$(GENSRCDIR)/java/util:
	$(MKDIR) -p $@

$(GENSRCDIR)/java/util/CurrencyData.java: $(GENSRCDIR)/java/util $(BUILDDIR)/tools/GenerateCurrencyData/GenerateCurrencyData.java $(SHARE_SRC)/classes/java/util/CurrencyData.properties
	-$(RM) $@
	$(RM) -r $(TMPDIR)/GenerateCurrencyData
	$(MKDIR) -p $(TMPDIR)/GenerateCurrencyData
	$(JAVAC_BOOT) -d $(TMPDIR)/GenerateCurrencyData $(BUILDDIR)/tools/GenerateCurrencyData/GenerateCurrencyData.java
	$(JAVA_BOOT) -classpath $(TMPDIR)/GenerateCurrencyData GenerateCurrencyData < $(SHARE_SRC)/classes/java/util/CurrencyData.properties > $@

clean:: 
	$(RM) $(GENSRCDIR)/java/util/CurrencyData.java
	$(RM) -r $(TMPDIR)/GenerateCurrencyData


#
# Rules to create $(GENSRCDIR)/java/lang/CharacterData.java 
#
GENCHARSRC = $(BUILDDIR)/tools/GenerateCharacter
GENCHAROUT = $(TMPDIR)/GenerateCharacter

$(GENSRCDIR)/java/lang:
	$(MKDIR) -p $(GENSRCDIR)/java/lang

$(GENSRCDIR)/java/lang/Character.java $(GENSRCDIR)/java/lang/CharacterData.java $(GENSRCDIR)/java/lang/CharacterDataLatin1.java: $(GENSRCDIR)/java/lang $(wildcard $(GENCHARSRC)/*)
	-$(RM) $@
	$(MKDIR) -p $(GENSRCDIR)/java/lang
	$(MKDIR) -p $(GENCHAROUT)
	$(JAVAC_BOOT) -d $(GENCHAROUT) -sourcepath $(GENCHARSRC) $(GENCHARSRC)/GenerateCharacter.java
	$(JAVA_BOOT) -classpath $(GENCHAROUT) GenerateCharacter -template $(GENCHARSRC)/Character.java.template \
		-spec $(GENCHARSRC)/UnicodeData.txt -specialcasing $(GENCHARSRC)/SpecialCasing.txt \
		-o $(GENSRCDIR)/java/lang/Character.java -string -usecharforbyte 11 4 1
	$(JAVA_BOOT) -classpath $(GENCHAROUT) GenerateCharacter -template $(GENCHARSRC)/CharacterData.java.template \
		-spec $(GENCHARSRC)/UnicodeData.txt -specialcasing $(GENCHARSRC)/SpecialCasing.txt \
		-o $(GENSRCDIR)/java/lang/CharacterData.java -string -usecharforbyte 11 4 1
	$(JAVA_BOOT) -classpath $(GENCHAROUT) GenerateCharacter -template $(GENCHARSRC)/CharacterDataLatin1.java.template \
		-spec $(GENCHARSRC)/UnicodeData.txt -specialcasing $(GENCHARSRC)/SpecialCasing.txt \
		-o $(GENSRCDIR)/java/lang/CharacterDataLatin1.java -string -usecharforbyte -latin1 8

clean::
	$(RM) $(GENSRCDIR)/java/lang/CharacterData.java $(GENSRCDIR)/java/lang/CharacterDataLatin1.java
	$(RM) -r $(GENCHAROUT)

#
# End of rules to create $(GENSRCDIR)/java/lang/CharacterData.java
#

.PHONY: clean clobber









