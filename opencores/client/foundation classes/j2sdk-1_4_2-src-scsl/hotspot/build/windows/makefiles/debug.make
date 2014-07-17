# @(#)debug.make	1.10 03/01/23 10:51:16
# 
# Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

!include compile.make

CPP_FLAGS=$(CPP_FLAGS) /Zi /Od /MDd /D "_DEBUG"
LINK_FLAGS=$(LINK_FLAGS) /debug /export:opnames
