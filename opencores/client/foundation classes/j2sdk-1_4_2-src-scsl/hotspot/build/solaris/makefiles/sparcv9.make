# 
# @(#)sparcv9.make	1.7 03/01/23 10:50:46
# 
# Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

# When optimized fully, stubGenerator_sparc.cpp 
# has bogus code for the routine 
# StubGenerator::generate_flush_callers_register_windows() 
OPT_CFLAGS/stubGenerator_sparc.o = $(OPT_CFLAGS/SLOWER)

# The copied fdlibm routines in sharedRuntimeTrig.o must not be optimized
OPT_CFLAGS/sharedRuntimeTrig.o = $(OPT_CFLAGS/NOOPT)

# For now ad_sparc file is compiled with -O2 %%%% remove when adlc is fixed
OPT_CFLAGS/ad_sparc.o = $(OPT_CFLAGS/SLOWER)
OPT_CFLAGS/dfa_sparc.o = $(OPT_CFLAGS/SLOWER)

# CC brings an US-II to its knees compiling the vmStructs asserts under -xO4
OPT_CFLAGS/vmStructs.o = $(OPT_CFLAGS/O2)
