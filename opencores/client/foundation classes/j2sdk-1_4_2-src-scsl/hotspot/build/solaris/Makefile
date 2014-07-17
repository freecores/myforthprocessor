# 
# @(#)Makefile	1.32 03/01/23 10:49:47
# 
# Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

# This makefile creates a build tree and lights off a build.
# You can go back into the build tree and perform rebuilds or
# incremental builds as desired.

# The make process now relies on java and javac. These can be
# specified implicitly on the PATH, by setting the
# (JDK-inherited) ALT_BOOTDIR environment variable to a full path to a
# JDK in which bin/java and bin/javac are present and working (e.g.,
# /usr/local/java/jdk1.3/solaris), or via the (JDK-inherited) default
# BOOTDIR path to a bootsrap java and javac.

# Unless GAMMADIR is set on the command line, search upward from
# the current directory for a parent directory containing "src/share/vm".
# If that fails, look for $GAMMADIR in the environment.
# When the tree of subdirs is built, this setting is stored in each flags.make.
GAMMADIR := $(shell until ([ -d dev ]&&echo $${GAMMADIR:-/GAMMADIR/}) || ([ -d src/share/vm ]&&pwd); do cd ..; done)
unexport GAMMADIR

# The <arch> below is determined by the machine you are building
# on. On a sparc system <arch> will be sparc. On an intel system
# <arch> will be i486.

OS		= $(subst SunOS,solaris,$(shell uname))
ARCH		= $(subst i386,i486,$(shell uname -p))
ifdef LP64
# For the 64 bit Sparc build, we want to
# build it into a unique directory (solaris_sparcv9_xxxx)
# even though the platform is still sparc
BUILDARCH = sparcv9
else
BUILDARCH = $(ARCH)
endif


# There is a (semi-) regular correspondence between make targets and actions:
#
#	Target		Tree Type	Build Dir
#
#	debug		compiler2	<os>_<arch>_compiler2/debug
#	fastdebug	compiler2	<os>_<arch>_compiler2/fastdebug
#	jvmg		compiler2	<os>_<arch>_compiler2/jvmg
#	optimized	compiler2	<os>_<arch>_compiler2/optimized
#	profiled	compiler2	<os>_<arch>_compiler2/profiled
#	product		compiler2	<os>_<arch>_compiler2/product
#
#	debug1		compiler1	<os>_<arch>_compiler1/debug
#	fastdebug1	compiler1	<os>_<arch>_compiler1/fastdebug
#	jvmg1		compiler1	<os>_<arch>_compiler1/jvmg
#	optimized1	compiler1	<os>_<arch>_compiler1/optimized
#	profiled1	compiler1	<os>_<arch>_compiler1/profiled
#	product1	compiler1	<os>_<arch>_compiler1/product
#
#	debugcore	core		<os>_<arch>_core/debug
#	fastdebugcore	core		<os>_<arch>_core/fastdebug
#	jvmgcore	core		<os>_<arch>_core/jvmg
#	optimizedcore	core		<os>_<arch>_core/optimized
#	profiledcore	core		<os>_<arch>_core/profiled
#	productcore	core		<os>_<arch>_core/product
#
# What you get with each target:
#
# debug*	- "thin" libjvm_g - debug info linked into the gamma_g launcher
# fastdebug*	- optimized compile, but with asserts enabled
# jvmg*		- "fat" libjvm_g - debug info linked into libjvm_g.so
# optimized*	- optimized compile, no asserts
# profiled*	- gprof
# product*	- the shippable thing:  optimized compile, no asserts, -DPRODUCT

# This target list needs to be coordinated with the usage message
# in the build.sh script:
TARGETS		= debug jvmg fastdebug optimized profiled product

OS      = solaris

SUBDIRS_C1	= $(addprefix $(OS)_$(BUILDARCH)_compiler1/,$(TARGETS))
SUBDIRS_C2	= $(addprefix $(OS)_$(BUILDARCH)_compiler2/,$(TARGETS))
SUBDIRS_CORE	= $(addprefix $(OS)_$(BUILDARCH)_core/,$(TARGETS))

TARGETS_C2	= $(TARGETS)
TARGETS_C1	= $(addsuffix 1,$(TARGETS))
TARGETS_CORE	= $(addsuffix core,$(TARGETS))

BUILDTREE_MAKE	= $(GAMMADIR)/build/solaris/makefiles/buildtree.make
BUILDTREE_VARS	= GAMMADIR=$(GAMMADIR) OS_FAMILY=$(OS) ARCH=$(ARCH)
BUILDTREE	= $(MAKE) -f $(BUILDTREE_MAKE) $(BUILDTREE_VARS)

.PHONY: $(TARGETS_C2) $(TARGETS_C1) $(TARGETS_CORE)
.PHONY: tree tree1 treecore
.PHONY: all compiler1 compiler2 core
.PHONY: clean clean_compiler1 clean_compiler2 clean_core

# Could make everything by default, but that would take a while.
all:
	@echo "Try '$(MAKE) <target> ...'  where <target> is one or more of"
	@echo "  $(TARGETS_C2)"
	@echo "  $(TARGETS_C1)"
	@echo "  $(TARGETS_CORE)"

$(SUBDIRS_C2): $(BUILDTREE_MAKE)
	$(BUILDTREE) VARIANT=compiler2

$(SUBDIRS_C1): $(BUILDTREE_MAKE)
	$(BUILDTREE) VARIANT=compiler1

$(SUBDIRS_CORE): $(BUILDTREE_MAKE)
	$(BUILDTREE) VARIANT=core

$(TARGETS_C2):  $(SUBDIRS_C2)
	cd $(OS)_$(BUILDARCH)_compiler2/$@ && $(MAKE) $(MFLAGS)
	cd $(OS)_$(BUILDARCH)_compiler2/$@ && ./test_gamma

$(TARGETS_C1):  $(SUBDIRS_C1)
	cd $(OS)_$(BUILDARCH)_compiler1/$(patsubst %1,%,$@) && $(MAKE) $(MFLAGS)
	-cd $(OS)_$(BUILDARCH)_compiler1/$(patsubst %1,%,$@) && ./test_gamma

$(TARGETS_CORE):  $(SUBDIRS_CORE)
	cd $(OS)_$(BUILDARCH)_core/$(patsubst %core,%,$@) && $(MAKE) $(MFLAGS)
	cd $(OS)_$(BUILDARCH)_core/$(patsubst %core,%,$@) && ./test_gamma

# Just build the tree, and nothing else:
tree:      $(SUBDIRS_C2)
tree1:     $(SUBDIRS_C1)
treecore:  $(SUBDIRS_CORE)


# Synonyms for win32-like targets.
compiler2:  jvmg product

compiler1:  jvmg1 product1

core: jvmgcore productcore

clean_compiler1 clean_compiler2 clean_core:
	rm -rf $(OS)_$(ARCH)_$(subst clean_,,$@)
	rm -rf $(OS)_$(ARCH)v9_$(subst clean_,,$@)

clean:  clean_compiler2 clean_compiler1 clean_core

include $(GAMMADIR)/build/solaris/makefiles/cscope.make
