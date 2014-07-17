# 
# @(#)vm.make	1.50 03/01/23 10:50:51
# 
# Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

# This makefile (vm.make) is included from the vm.make in the
# build directories.
# It knows how to compile and link the VM.

# It assumes the following flags are set:
# Platform_file, Src_Dirs, Obj_Files, SYSDEFS, INCLUDES, CFLAGS, LFLAGS, LIBS

# And it assumes that the deps and incls have already been built.

# -- D. Ungar (5/97) from a file by Bill Bush

# build whole VM by default, I can also build foo.o or foo.i

# Common build rules.
include $(GAMMADIR)/build/solaris/makefiles/rules.make

AOUT   = gamma$(G_SUFFIX)
GENERATED  = ../generated

# set VPATH so make knows where to look for source files
# Src_Dirs is everything in src/share/vm/*, plus the right os/*/vm and cpu/*/vm
# The incls directory contains generated header file lists for inclusion.
# The adfiles directory contains ad_<arch>.[ch]pp.
Src_Dirs_V = $(GENERATED)/adfiles ${Src_Dirs} $(GENERATED)/incls
VPATH    += $(Src_Dirs_V:%=%:)

# set INCLUDES for C preprocessor
Src_Dirs_I = $(GENERATED)/adfiles ${Src_Dirs} $(GENERATED) 
INCLUDES += $(Src_Dirs_I:%=-I%)

BUILD_VERSION = -DHOTSPOT_BUILD_VERSION="\"$(HOTSPOT_BUILD_VERSION)\""
BUILD_VERSION$(HOTSPOT_BUILD_VERSION) = 

BUILD_USER = -DHOTSPOT_BUILD_USER="$(HOTSPOT_BUILD_USER)"
BUILD_USER$(HOTSPOT_BUILD_USER) = 

CPPFLAGS = ${SYSDEFS} ${INCLUDES} ${BUILD_VERSION} ${BUILD_USER}

# CFLAGS_WARN holds compiler options to suppress/enable warnings.
CFLAGS	+= $(CFLAGS_WARN)

# Extra flags from gnumake's invocation or environment
CFLAGS += $(EXTRA_CFLAGS)

# Do not use C++ exception handling
CFLAGS += $(CFLAGS/NOEX)

# The whole megilla:
LIBS += -ldl -lthread -lsocket -lnsl -lm -lsched

JVM = jvm$(G_SUFFIX)
LIBJVM = lib$(JVM).so

ifeq ("${G_SUFFIX}", "_g")
LN_LIBJVM = libjvm.so
else
LN_LIBJVM = libjvm_g.so
endif

# By default, link the *.o into the library, not the executable.
LINK_INTO$(LINK_INTO) = LIBJVM

# making launch:
AOUT.o = $(AOUT.o/LINK_INTO_$(LINK_INTO))
AOUT.o/LINK_INTO_AOUT    = $(Obj_Files)
AOUT.o/LINK_INTO_LIBJVM  =

# For now, build a copy of the JDK1.2beta4 launcher.
# This is useful, because the development cycle goes faster
# if we can use ild to statically link the VM into the launcher.
# Eventually, we should expunge all use of $(AOUT) from the makefiles.
# The optimized (product) build should certainly avoid making a new launcher.
AOUT.o += launcher.o

LAUNCHER = $(GAMMADIR)/src/os/$(Platform_os_family)/launcher
ifdef LP64
LAUNCHERFLAGS = -xarch=v9 -I$(LAUNCHER) -I$(GAMMADIR)/src/share/vm/prims
else
LAUNCHERFLAGS = -I$(LAUNCHER) -I$(GAMMADIR)/src/share/vm/prims
endif

# libjvm-related things.
LIBJVM.o = $(LIBJVM.o/LINK_INTO_$(LINK_INTO))
LIBJVM.o/LINK_INTO_AOUT    =
LIBJVM.o/LINK_INTO_LIBJVM  = $(Obj_Files)

JSIG = jsig$(G_SUFFIX)
LIBJSIG = lib$(JSIG).so

default: $(AOUT) $(LIBJSIG)

# read a generated file defining the set of .o's and the .o .h dependencies
include $(GENERATED)/Dependencies

# read machine-specific adjustments (%%% should do this via buildATree?)
ifdef LP64
include $(GAMMADIR)/build/solaris/makefiles/sparcv9.make
else
include $(GAMMADIR)/build/solaris/makefiles/$(Platform_arch).make
endif

launcher.o: launcher.c $(LAUNCHER)/java.c $(LAUNCHER)/java_md.c
	$(CC) -g -c -o $@ launcher.c $(LAUNCHERFLAGS) $(CPPFLAGS)

launcher.c:
	@echo Generating $@
	$(QUIETLY) { \
	echo '#define debug launcher_debug'; \
	echo '#include "java.c"'; \
	echo '#include "java_md.c"'; \
	} > $@

$(AOUT): $(AOUT.o) $(LIBJVM)
	$(QUIETLY) \
	case "$(CFLAGS_BROWSE)" in \
	-sbfast|-xsbfast) \
	    ;; \
	*) \
	    echo Linking launcher...; \
	    $(LINK.CC) -o $@ $(AOUT.o) \
	        -L `pwd` -L $(JAVA_HOME)/jre/lib/$(Platform_gnu_dis_arch) \
	        -L $(JAVA_HOME)/lib/$(Platform_gnu_dis_arch) -l$(JVM) $(LIBS); \
	    ;; \
	esac


ifdef MAPFILE
LIBJVM_MAPFILE = mapfile

ifdef USE_GCC
LFLAGS_VM += -Xlinker -M $(LIBJVM_MAPFILE)
else
LFLAGS_VM += -M $(LIBJVM_MAPFILE)
endif

mapfile : $(MAPFILE) $(REORDERFILE)
	rm -f $@
	cat $^ > $@
endif


# making the library:
$(LIBJVM): $(LIBJVM.o) $(LIBJVM_MAPFILE)
	$(QUIETLY) \
	case "$(CFLAGS_BROWSE)" in \
	-sbfast|-xsbfast) \
	    ;; \
	*) \
	    echo Linking vm...; \
	    $(LINK_LIB.CC/PRE_HOOK) \
	    $(LINK_LIB.CC) $(LFLAGS_VM) -o $@ $(LIBJVM.o) $(LIBS); \
	    $(LINK_LIB.CC/POST_HOOK) \
	    rm -f $@.1; ln -s $@ $@.1; \
	    rm -f $(LN_LIBJVM); ln -s $@ $(LN_LIBJVM); \
	    rm -f $(LN_LIBJVM).1; ln -s $(LN_LIBJVM) $(LN_LIBJVM).1; \
	    ;; \
	esac

# making libjsig
JSIGSRCDIR = $(GAMMADIR)/src/os/$(Platform_os_family)/vm

ifeq ("${G_SUFFIX}", "_g")
SYMFLAG = -g
else
SYMFLAG =
endif

ifdef LP64
ARCHFLAG = -xarch=v9
else
ARCHFLAG =
endif

$(LIBJSIG): $(JSIGSRCDIR)/jsig.c
	$(QUIETLY) echo Making signal interposition lib...; \
	$(CC) $(SYMFLAG) $(ARCHFLAG) -G -D_REENTRANT $(PICFLAG) -o $@ $(JSIGSRCDIR)/jsig.c \
	    -ldl
