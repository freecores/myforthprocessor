# 
# @(#)top.make	1.10 03/01/23 10:49:42
# 
# Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

# top.make is included in the Makefile in the build directories.
# It DOES NOT include the vm dependency info in order to be faster.
# It's main job is to implement the incremental form of make lists.
# It also builds and runs adlc.

# It assumes the following flags are set:
# CFLAGS Platform_file, Src_Dirs, SYSDEFS, AOUT, Obj_Files

# -- D. Ungar (5/97) from a file by Bill Bush

# Don't override the built-in $(MAKE).
# Instead, use "gmake" (or "gnumake") from the command line.  --Rose
#MAKE = gmake

GENERATED   = ../generated
VM          = $(GAMMADIR)/src/share/vm
Plat_File   = $(Platform_file)
CDG         = cd $(GENERATED); 

# Pick up MakeDeps' sources and definitions
include $(GAMMADIR)/build/linux/makefiles/makedeps.make
MakeDepsClass = MakeDeps.class
MakeDeps    = $(MAKEDEPS_JAVA) -classpath . MakeDeps

Include_DBs/GC          = $(VM)/includeDB_gc \
                          $(VM)/gc_implementation/includeDB_gc_parallelScavenge \
                          $(VM)/gc_implementation/includeDB_gc_shared

Include_DBs/CORE        = $(VM)/includeDB_core   $(Include_DBs/GC)   $(VM)/includeDB_coreonly
Include_DBs/CORE+CI     = $(VM)/includeDB_core   $(Include_DBs/GC)   $(VM)/includeDB_ci
Include_DBs/COMPILER1   = $(Include_DBs/CORE+CI) $(VM)/includeDB_compiler1
Include_DBs/COMPILER2   = $(Include_DBs/CORE+CI) $(VM)/includeDB_compiler2
Include_DBs = $(Include_DBs/$(TYPE))

Cached_plat = platform.current
Cached_db   = includeDB.current

Incremental_Lists =$(GENERATED)/$(Cached_db)
# list generation also creates $(GENERATED)/$(Cached_plat)


AD_Dir   = $(GENERATED)/adfiles
ADLC     = $(AD_Dir)/adlc
AD_Spec  = $(GAMMADIR)/src/cpu/$(Platform_arch)/vm/$(Platform_arch).ad
AD_Src   = $(GAMMADIR)/src/share/vm/adlc
AD_Names = ad_$(Platform_arch).hpp ad_$(Platform_arch).cpp
AD_Files = $(AD_Names:%=$(AD_Dir)/%)

# AD_Files_If_Required/COMPILER1 = ad_stuff
AD_Files_If_Required/COMPILER2 = ad_stuff
AD_Files_If_Required = $(AD_Files_If_Required/$(TYPE))

# Wierd argument adjustment for "gnumake -j..."
adjust-mflags   = $(GENERATED)/adjust-mflags
MFLAGS-adjusted = `$(adjust-mflags) "$(MFLAGS)" "$(HOTSPOT_BUILD_JOBS)"`


# default target: make makeDeps, update lists, make vm
# done in stages to force sequential order with parallel make
#

default: vm_build_preliminaries the_vm
	@echo All done.

# This is an explicit dependency for the sake of parallel makes.
vm_build_preliminaries: check_os_version $(Incremental_Lists) $(AD_Files_If_Required)
	@# We need a null action here, so implicit rules don't get consulted.

# We do not want people accidentally building on Linux 2.1.x or earlier.
check_os_version:
	@ case ${DISABLE_HOTSPOT_OS_VERSION_CHECK},`uname -r` in \
	  ,2.[0-1].*) >&2 echo "*** This OS is not supported:" `uname -a`; exit 1;; \
	  esac
# Disable this check by setting DISABLE_HOTSPOT_OS_VERSION_CHECK=ok.

# make makeDeps: (and zap the cached db files to force a nonincremental run)

$(GENERATED)/$(MakeDepsClass): $(MakeDepsSources)
	@$(MAKEDEPS_JAVAC) -classpath $(GAMMADIR)/src/share/tools/MakeDeps -g -d $(GENERATED) $(MakeDepsSources)
	@echo Removing $(Incremental_Lists) to force regeneration.
	@rm -f $(Incremental_Lists)
	@$(CDG) echo >$(Cached_plat)

# make incremental_lists, if cached files out of date, run makeDeps

$(Incremental_Lists):  $(Include_DBs) $(Plat_File) $(GENERATED)/$(MakeDepsClass)
	$(CDG)  cat $(Include_DBs) > includeDB
	$(CDG) if [ ! -r incls ] ; then \
	mkdir incls ; \
	fi
	$(CDG)  $(MakeDeps) diffs UnixPlatform $(Cached_plat) $(Cached_db) $(Plat_File) includeDB $(MakeDepsOptions)
	$(CDG)  cp includeDB    $(Cached_db)
	$(CDG)  cp $(Plat_File) $(Cached_plat)

# symbolic target for command lines
lists: $(Incremental_Lists)
	@: lists are now up to date

# make AD files as necessary
ad_stuff: $(Incremental_Lists) $(adjust-mflags)
	@$(MAKE) -f adlc.make $(MFLAGS-adjusted)

# and the VM: must use other makefile with dependencies included

# We have to go to great lengths to get control over the -jN argument
# to the recursive invocation of vm.make.  The problem is that gnumake
# resets -jN to -j1 for recursive runs.  (How helpful.)
# Note that the user must specify the desired parallelism level via a
# command-line or environment variable name HOTSPOT_BUILD_JOBS.
$(adjust-mflags): $(GAMMADIR)/build/linux/makefiles/adjust-mflags.sh
	@+rm -f $@ $@+
	@+cat $< > $@+
	@+chmod +x $@+
	@+mv $@+ $@

the_vm: vm_build_preliminaries $(adjust-mflags)
	@$(MAKE) -f vm.make $(MFLAGS-adjusted)

# next rules support "make foo.[oi]"

%.o %.i %.s:
	$(MAKE) -f vm.make $(MFLAGS) $@
	#$(MAKE) -f vm.make $@

# this should force everything to be rebuilt
clean: 
	rm -f $(GENERATED)/*.class
	$(MAKE) $(MFLAGS) $(GENERATED)/$(MakeDepsClass)
	$(MAKE) -f vm.make $(MFLAGS) clean

# just in case it doesn't, this should do it
realclean:
	$(MAKE) -f vm.make $(MFLAGS) clean
	rm -fr $(GENERATED)

.PHONY: default vm_build_preliminaries check_os_version
.PHONY: lists ad_stuff the_vm clean realclean
