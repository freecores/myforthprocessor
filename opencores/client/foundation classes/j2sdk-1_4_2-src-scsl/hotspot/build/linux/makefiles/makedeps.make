# @(#)makedeps.make	1.5 03/01/23 10:49:26
# 
# Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

# These are the commands used externally to compile MakeDeps.
# Note use of ALT_BOOTDIR to explicitly specify location of java and
# javac; this is the same environment variable used in the JDK build
# process for overriding the default spec, which is BOOTDIR. If
# neither ALT_BOOTDIR nor BOOTDIR are defined, use search rules.

# if no vars defined, rely on builder's search rules

ifdef ALT_BOOTDIR

MAKEDEPS_JAVA  = $(ALT_BOOTDIR)/bin/java
MAKEDEPS_JAVAC = $(ALT_BOOTDIR)/bin/javac

else

ifdef BOOTDIR

MAKEDEPS_JAVA  = $(BOOTDIR)/bin/java
MAKEDEPS_JAVAC = $(BOOTDIR)/bin/javac

else

MAKEDEPS_JAVA  = java
MAKEDEPS_JAVAC = javac

endif
endif

MakeDepsSources=\
	$(GAMMADIR)/src/share/tools/MakeDeps/Database.java \
	$(GAMMADIR)/src/share/tools/MakeDeps/DirectoryTree.java \
	$(GAMMADIR)/src/share/tools/MakeDeps/DirectoryTreeNode.java \
	$(GAMMADIR)/src/share/tools/MakeDeps/FileFormatException.java \
	$(GAMMADIR)/src/share/tools/MakeDeps/FileList.java \
	$(GAMMADIR)/src/share/tools/MakeDeps/FileName.java \
	$(GAMMADIR)/src/share/tools/MakeDeps/Macro.java \
	$(GAMMADIR)/src/share/tools/MakeDeps/MacroDefinitions.java \
	$(GAMMADIR)/src/share/tools/MakeDeps/MakeDeps.java \
	$(GAMMADIR)/src/share/tools/MakeDeps/MetroWerksMacPlatform.java \
	$(GAMMADIR)/src/share/tools/MakeDeps/Platform.java \
	$(GAMMADIR)/src/share/tools/MakeDeps/UnixPlatform.java \
	$(GAMMADIR)/src/share/tools/MakeDeps/WinGammaPlatform.java

MakeDepsOptions=-firstFile functionAtStart.cpp -lastFile functionAtEnd.cpp
