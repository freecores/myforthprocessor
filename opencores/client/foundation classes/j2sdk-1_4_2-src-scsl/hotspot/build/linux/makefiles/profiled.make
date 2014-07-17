# 
# @(#)profiled.make	1.3 03/01/23 10:49:35
# 
# Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

# Sets make macros for making profiled version of Gamma VM
# (It is also optimized.)

CFLAGS += -pg
AOUT_FLAGS += -pg
