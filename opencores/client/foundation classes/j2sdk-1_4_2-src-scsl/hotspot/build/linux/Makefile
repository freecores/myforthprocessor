# 
# @(#)Makefile	1.8 03/01/23 10:48:45
# 
# Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

GAMMADIR := $(shell pwd)/../..
unexport GAMMADIR

# This makefile creates a build tree and lights off a build.
# You can go back into the build tree and perform rebuilds or
# incremental builds as desired.  Be sure to reestablish
# environment variable settings for LD_LIBRARY_PATH and JAVA_HOME.

# The make process now relies on java and javac. These can be
# specified either implicitly on the PATH, by setting the
# (JDK-inherited) ALT_BOOTDIR environment variable to full path to a
# JDK in which bin/java and bin/javac are present and working (e.g.,
# /usr/local/java/jdk1.3/solaris), or via the (JDK-inherited) 
# default BOOTDIR path value.

# The <arch> below is determined by the machine you are building
# on. On a sparc system <arch> will be sparc. On an intel system
# <arch> will be i486.

# There is an irregular correspondence between make targets and actions:
#	Target		Tree Type	Build Dir
#	debug		compiler2	<os>_<arch>_compiler2/debug
#	profiled	compiler2		<os>_<arch>_compiler2/profiled
#	(none)		compiler2		<os>_<arch>_compiler2/product
#	product		compiler2		<os>_<arch>_compiler2/product
#	optimized	compiler2		<os>_<arch>_compiler2/optimized
#	debug1		compiler1	<os>_<arch>_compiler1/debug
#	profiled1	compiler1		<os>_<arch>_compiler1/profiled
#	product1	compiler1		<os>_<arch>_compiler1/product
#	optimized1	compiler1		<os>_<arch>_compiler1/optimized
#	basicdebug	core		<os>_<arch>_core/debug
# Hmm how do these work QQQ
#	(none)		core		<os>_<arch>_core/profiled
#	(none)		core		<os>_<arch>_core/product
#	basic		core		<os>_<arch>_core/optimized

# Target with suffix of 1 build the VM with Compiler1

OS      = linux
ARCH    = $(shell uname -m)
LIB_ARCH= $ARCH

ifneq ($(ARCH), sparc)
ifdef ITANIUM
    ARCH=ia64
else
ifneq ($(ARCH), ia64)
    ARCH=i486
endif
endif
endif
export ARCH

buildATree = ${GAMMADIR}/build/${OS}/makefiles/buildATree.sh

# For now, until the compiler is less wobbly:
TESTFLAGS = -Xbatch

RUN_TEST = \
	echo "Testing $$gamma...";                                      \
	echo "\#!/bin/csh" > test;                                      \
	echo "setenv LD_LIBRARY_PATH .:${JAVA_HOME}/jre/lib/${ARCH}:${JAVA_HOME}/jre/lib/${ARCH}/native_threads:${LD_LIBRARY_PATH}" >> test; \
	echo "./$$gamma < /dev/null $(TESTFLAGS) -classpath ${GAMMADIR}/build/${OS}:${JAVA_HOME}/jre/lib/lib/rt.jar:${JAVA_HOME}/classes Queens" >> test;          \
	chmod +x ./test; ./test

${OS}_${ARCH}_compiler2/debug ${OS}_${ARCH}_compiler2/optimized ${OS}_${ARCH}_compiler2/fastdebug ${OS}_${ARCH}_compiler2/jvmg ${OS}_${ARCH}_compiler2/profiled ${OS}_${ARCH}_compiler2/product : $(buildATree)
	sh $(buildATree) compiler2 ${GAMMADIR} ${OS} ${ARCH}

debug: ${OS}_${ARCH}_compiler2/debug
	(cd ${OS}_${ARCH}_compiler2/debug; $(MAKE))
#	@(cd ${OS}_${ARCH}_compiler2/debug; gamma=gamma_g; $(RUN_TEST))

optimized: ${OS}_${ARCH}_compiler2/optimized
	(cd ${OS}_${ARCH}_compiler2/optimized; $(MAKE))
#	@(cd ${OS}_${ARCH}_compiler2/optimized; gamma=gamma; $(RUN_TEST))

fastdebug: ${OS}_${ARCH}_compiler2/fastdebug
	(cd ${OS}_${ARCH}_compiler2/fastdebug; $(MAKE))
#	@(cd ${OS}_${ARCH}_compiler2/fastdebug; gamma=gamma; $(RUN_TEST))

jvmg: ${OS}_${ARCH}_compiler2/jvmg
	(cd ${OS}_${ARCH}_compiler2/jvmg; $(MAKE))
#	@(cd ${OS}_${ARCH}_compiler2/jvmg; gamma=gamma_g; $(RUN_TEST))

product: ${OS}_${ARCH}_compiler2/product
	(cd ${OS}_${ARCH}_compiler2/product; $(MAKE))
#	@(cd ${OS}_${ARCH}_compiler2/product; gamma=gamma; $(RUN_TEST))

profiled: ${OS}_${ARCH}_compiler2/profiled
	(cd ${OS}_${ARCH}_compiler2/profiled; $(MAKE))
#	@(cd ${OS}_${ARCH}_compiler2/profiled; gamma=gamma; $(RUN_TEST))

${OS}_${ARCH}_compiler1/debug ${OS}_${ARCH}_compiler1/optimized ${OS}_${ARCH}_compiler1/fastdebug ${OS}_${ARCH}_compiler1/jvmg ${OS}_${ARCH}_compiler1/profiled ${OS}_${ARCH}_compiler1/product : $(buildATree)
	sh $(buildATree) compiler1 ${GAMMADIR} ${OS} ${ARCH}

debug1: ${OS}_${ARCH}_compiler1/debug
	(cd ${OS}_${ARCH}_compiler1/debug; $(MAKE))
#	@(cd ${OS}_${ARCH}_compiler1/debug; gamma=gamma_g; $(RUN_TEST))

optimized1: ${OS}_${ARCH}_compiler1/optimized
	(cd ${OS}_${ARCH}_compiler1/optimized; $(MAKE))
#	@(cd ${OS}_${ARCH}_compiler1/optimized; gamma=gamma; $(RUN_TEST))

fastdebug1: ${OS}_${ARCH}_compiler1/fastdebug
	(cd ${OS}_${ARCH}_compiler1/fastdebug; $(MAKE))
#	@(cd ${OS}_${ARCH}_compiler1/fastdebug; gamma=gamma; $(RUN_TEST))

jvmg1: ${OS}_${ARCH}_compiler1/jvmg
	(cd ${OS}_${ARCH}_compiler1/jvmg; $(MAKE))
#	@(cd ${OS}_${ARCH}_compiler1/jvmg; gamma=gamma_g; $(RUN_TEST))

product1: ${OS}_${ARCH}_compiler1/product
	(cd ${OS}_${ARCH}_compiler1/product; $(MAKE))
#	@(cd ${OS}_${ARCH}_compiler1/product; gamma=gamma; $(RUN_TEST))

profiled1: ${OS}_${ARCH}_compiler1/profiled
	(cd ${OS}_${ARCH}_compiler1/profiled; $(MAKE))
#	@(cd ${OS}_${ARCH}_compiler1/profiled; gamma=gamma; $(RUN_TEST))

${OS}_${ARCH}_core/debug ${OS}_${ARCH}_core/optimized ${OS}_${ARCH}_core/fastdebug ${OS}_${ARCH}_core/jvmg ${OS}_${ARCH}_core/profiled ${OS}_${ARCH}_core/product : $(buildATree)
	sh $(buildATree) core ${GAMMADIR} ${OS} ${ARCH}

debugcore: ${OS}_${ARCH}_core/debug
	(cd ${OS}_${ARCH}_core/debug; $(MAKE))
#	@(cd ${OS}_${ARCH}_core/debug; gamma=gamma_g; $(RUN_TEST))

optimizedcore: ${OS}_${ARCH}_core/optimized
	(cd ${OS}_${ARCH}_core/optimized; $(MAKE))
#	@(cd ${OS}_${ARCH}_core/optimized; gamma=gamma; $(RUN_TEST))

fastdebugcore: ${OS}_${ARCH}_core/fastdebug
	(cd ${OS}_${ARCH}_core/fastdebug; $(MAKE))
#	@(cd ${OS}_${ARCH}_core/fastdebug; gamma=gamma; $(RUN_TEST))

jvmgcore: ${OS}_${ARCH}_core/jvmg
	(cd ${OS}_${ARCH}_core/jvmg; $(MAKE))
#	@(cd ${OS}_${ARCH}_core/jvmg; gamma=gamma_g; $(RUN_TEST))

productcore: ${OS}_${ARCH}_core/product
	(cd ${OS}_${ARCH}_core/product; $(MAKE))
#	@(cd ${OS}_${ARCH}_core/product; gamma=gamma; $(RUN_TEST))

profiledcore: ${OS}_${ARCH}_core/profiled
	(cd ${OS}_${ARCH}_core/profiled; $(MAKE))
#	@(cd ${OS}_${ARCH}_core/profiled; gamma=gamma; $(RUN_TEST))

basicdebug: debugcore
basic: optimizedcore

# Intel-style names:
compiler1: jvmg1 product1
compiler2: jvmg product
core: jvmgcore productcore

clean:
	(rm -fr ${OS}_${ARCH}_core; rm -fr ${OS}_${ARCH}_compiler2 ; rm -fr ${OS}_${ARCH}_compiler1)
