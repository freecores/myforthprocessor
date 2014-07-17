#
# @(#)Makefile	1.4 03/01/23 11:04:35
#
# Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
#


all: 
	@javah -classpath ../../../../share/vm/agent/classes -jni sun.jvm.hotspot.debugger.proc.ProcDebuggerLocal
	CC -G -Kpic -I${JDK_HOME}/include -I${JDK_HOME}/include/solaris saproc.cpp -o libsaproc.so

clean:
	\rm -rf sun_jvm_hotspot_debugger_proc_ProcDebuggerLocal.h
	\rm -rf libsaproc.so
