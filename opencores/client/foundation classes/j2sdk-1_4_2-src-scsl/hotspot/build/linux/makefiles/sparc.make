# 
# @(#)sparc.make	1.3 03/01/23 10:49:37
# 
# Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

# For now ad_sparc file is compiled with -O2 %%%% remove when adlc is fixed
OPT_CFLAGS/ad_sparc.o = $(OPT_CFLAGS/SLOWER)
OPT_CFLAGS/dfa_sparc.o = $(OPT_CFLAGS/SLOWER)
# CC drops core on systemDictionary.o in -xO4 mode
OPT_CFLAGS/systemDictionary.o = $(OPT_CFLAGS/SLOWER)

# File-specific adjustments to the PICFLAG, applicable only to SPARC,
# which has a very tight limit on global constant references.

# Old files which seemed hot at one point or another:
#PICFLAG/runtime.o = $(PICFLAG/BETTER)
#PICFLAG/generateOopMap.o = $(PICFLAG/BETTER)
#PICFLAG/thread.o = $(PICFLAG/BETTER)
#PICFLAG/parse2.o = $(PICFLAG/BETTER)
#PICFLAG/parse1.o = $(PICFLAG/BETTER)
#PICFLAG/universe.o = $(PICFLAG/BETTER)
#PICFLAG/safepoint.o = $(PICFLAG/BETTER)
#PICFLAG/parse3.o = $(PICFLAG/BETTER)
#PICFLAG/compile.o = $(PICFLAG/BETTER)
#PICFLAG/codeBlob.o = $(PICFLAG/BETTER)
#PICFLAG/mutexLocker.o = $(PICFLAG/BETTER)
#PICFLAG/nativeInst_sparc.o = $(PICFLAG/BETTER)
#PICFLAG/methodLiveness.o = $(PICFLAG/BETTER)
#PICFLAG/synchronizer.o = $(PICFLAG/BETTER)
#PICFLAG/methodOop.o = $(PICFLAG/BETTER)
#PICFLAG/space.o = $(PICFLAG/BETTER)
#PICFLAG/interpreterRT_sparc.o = $(PICFLAG/BETTER)
#PICFLAG/generation.o = $(PICFLAG/BETTER)
#PICFLAG/markSweep.o = $(PICFLAG/BETTER)
#PICFLAG/parseHelper.o = $(PICFLAG/BETTER)

# Confirmed by function-level profiling:
PICFLAG/scavenge.o = $(PICFLAG/BETTER)
PICFLAG/instanceKlass.o = $(PICFLAG/BETTER)
PICFLAG/frame.o = $(PICFLAG/BETTER)
PICFLAG/phaseX.o = $(PICFLAG/BETTER)
PICFLAG/lookupCache.o = $(PICFLAG/BETTER)
PICFLAG/chaitin.o = $(PICFLAG/BETTER)
PICFLAG/type.o = $(PICFLAG/BETTER)
PICFLAG/jvm.o = $(PICFLAG/BETTER)
PICFLAG/jni.o = $(PICFLAG/BETTER)
PICFLAG/matcher.o = $(PICFLAG/BETTER)

# New from module-level profiling (trustworthy?):
PICFLAG/rememberedSet.o = $(PICFLAG/BETTER)
PICFLAG/frame_sparc.o = $(PICFLAG/BETTER)
PICFLAG/live.o = $(PICFLAG/BETTER)
PICFLAG/vectset.o = $(PICFLAG/BETTER)
PICFLAG/objArrayKlass.o = $(PICFLAG/BETTER)
PICFLAG/do_call.o = $(PICFLAG/BETTER)
PICFLAG/loopnode.o = $(PICFLAG/BETTER)
PICFLAG/cfgnode.o = $(PICFLAG/BETTER)
PICFLAG/ifg.o = $(PICFLAG/BETTER)
PICFLAG/vframe.o = $(PICFLAG/BETTER)
PICFLAG/postaloc.o = $(PICFLAG/BETTER)
PICFLAG/carRememberedSet.o = $(PICFLAG/BETTER)
PICFLAG/gcm.o = $(PICFLAG/BETTER)
PICFLAG/coalesce.o = $(PICFLAG/BETTER)
PICFLAG/oop.o = $(PICFLAG/BETTER)
PICFLAG/oopMap.o = $(PICFLAG/BETTER)
PICFLAG/resourceArea.o = $(PICFLAG/BETTER)
PICFLAG/node.o = $(PICFLAG/BETTER)
PICFLAG/dict.o = $(PICFLAG/BETTER)
PICFLAG/domgraph.o = $(PICFLAG/BETTER)
PICFLAG/dfa_sparc.o = $(PICFLAG/BETTER)
PICFLAG/block.o = $(PICFLAG/BETTER)
PICFLAG/javaClasses.o = $(PICFLAG/BETTER)

# New hot files:
PICFLAG/classes.o = $(PICFLAG/BETTER)
#PICFLAG/ad_sparc.o = $(PICFLAG/BETTER)
PICFLAG/nmethod.o = $(PICFLAG/BETTER)
PICFLAG/relocInfo.o = $(PICFLAG/BETTER)
PICFLAG/codeBuffer_sparc.o = $(PICFLAG/BETTER)
PICFLAG/callnode.o = $(PICFLAG/BETTER)
PICFLAG/multnode.o = $(PICFLAG/BETTER)
PICFLAG/os_linux.o = $(PICFLAG/BETTER)
PICFLAG/typeArrayKlass.o = $(PICFLAG/BETTER)
