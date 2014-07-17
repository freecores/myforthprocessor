# 
# @(#)rules.make	1.7 03/01/23 10:50:38
# 
# Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
#

# Common rules/macros for the vm, adlc.

# Tell make that .cpp is important
.SUFFIXES: .cpp $(SUFFIXES)

# For now.  Other makefiles use CPP as the c++ compiler, but that should really
# name the preprocessor.
ifeq	($(CCC),)
CCC		= $(CPP)
endif

DEMANGLER	= c++filt
DEMANGLE	= $(DEMANGLER) < $@ > .$@ && mv -f .$@ $@

# $(CC) is the c compiler (cc/gcc), $(CCC) is the c++ compiler (CC/g++).
C_COMPILE	= $(CC) $(CPPFLAGS) $(CFLAGS)
CC_COMPILE	= $(CCC) $(CPPFLAGS) $(CFLAGS)

COMPILE.c	= $(C_COMPILE) -c
GENASM.c	= $(C_COMPILE) -S
LINK.c		= $(CC) $(LFLAGS) $(AOUT_FLAGS) $(PROF_AOUT_FLAGS)
LINK_LIB.c	= $(CC) $(LFLAGS) -G
PREPROCESS.c	= $(C_COMPILE) -E

COMPILE.CC	= $(CC_COMPILE) -c
GENASM.CC	= $(CC_COMPILE) -S
LINK.CC		= $(CCC) $(LFLAGS) $(AOUT_FLAGS) $(PROF_AOUT_FLAGS)
LINK_NOPROF.CC	= $(CCC) $(LFLAGS) $(AOUT_FLAGS)
LINK_LIB.CC	= $(CCC) $(LFLAGS) -G
PREPROCESS.CC	= $(CC_COMPILE) -E

# Effect of REMOVE_TARGET is to delete out-of-date files during "gnumake -k".
REMOVE_TARGET   = rm -f $@

# Synonyms.
COMPILE.cpp	= $(COMPILE.CC)
GENASM.cpp	= $(GENASM.CC)
LINK.cpp	= $(LINK.CC)
LINK_LIB.cpp	= $(LINK_LIB.CC)
PREPROCESS.cpp	= $(PREPROCESS.CC)

# 'gmake MAKE_VERBOSE=y' or 'gmake QUIETLY=' gives all the gory details.
QUIETLY$(MAKE_VERBOSE)	= @

# With parallel makes, print a message at the end of compilation.
ifeq	($(findstring j,$(MFLAGS)),j)
COMPILE_DONE	= && { echo Done with $<; }
endif

%.o: %.cpp
	@echo Compiling $<
	$(QUIETLY) $(REMOVE_TARGET)
	$(QUIETLY) $(COMPILE.CC) -o $@ $< $(COMPILE_DONE)

%.s: %.cpp
	@echo Generating assembly for $<
	$(QUIETLY) $(GENASM.CC) -o $@ $<
	$(QUIETLY) $(DEMANGLE) $(COMPILE_DONE)

# Intermediate files (for debugging macros)
%.i: %.cpp
	@echo Preprocessing $< to $@
	$(QUIETLY) $(PREPROCESS.CC) $< > $@ $(COMPILE_DONE)

#  Override gnumake built-in rules which do sccs get operations badly.
#  (They put the checked out code in the current directory, not in the
#  directory of the original file.)  Since this is a symptom of a teamware
#  failure, and since not all problems can be detected by gnumake due
#  to incomplete dependency checking... just complain and stop.
%:: s.%
	@echo "========================================================="
	@echo File $@
	@echo is out of date with respect to its SCCS file.
	@echo This file may be from an unresolved Teamware conflict.
	@echo This is also a symptom of a Teamware bringover/putback failure
	@echo in which SCCS files are updated but not checked out.
	@echo Check for other out of date files in your workspace.
	@echo "========================================================="
	@exit 666

%:: SCCS/s.%
	@echo "========================================================="
	@echo File $@
	@echo is out of date with respect to its SCCS file.
	@echo This file may be from an unresolved Teamware conflict.
	@echo This is also a symptom of a Teamware bringover/putback failure
	@echo in which SCCS files are updated but not checked out.
	@echo Check for other out of date files in your workspace.
	@echo "========================================================="
	@exit 666

.PHONY: default
