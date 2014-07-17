#
# @(#)Makefile	1.37 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for JDBC-ODBC Bridge Driver 
#
# Note - the native library for the bridge is linked with the 
# shared library for the ODBC driver manager.  
#

BUILDDIR = ../..
PACKAGE = sun.jdbc.odbc
LIBRARY = JdbcOdbc
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files
#

include FILES_java.gmk

FILES_c = \
	JdbcOdbc.c

FILES_export = \
    sun/jdbc/odbc/JdbcOdbc.java 

ifeq ($(PLATFORM),linux)
    # on linux you may not have an odbc library
     INIT = make_libs
endif

ifeq ($(PLATFORM),windows)
  # Files built here do not compile with warning level 3
  COMPILER_WARNING_LEVEL=0
endif

#
# Rules
#
  include $(BUILDDIR)/common/Library.gmk

#
# The UNIX define specifies conditional compilation for UNIX
# The -xCC flag is need because the C code contains C++ style comments
#
ifneq ($(PLATFORM), windows)
  CFLAGS += -DUNIX
  ifneq ($(CC_VERSION),gcc)
    CFLAGS += -xCC -w
  endif 
endif 

ifeq ($(PLATFORM), linux)
    LDFLAGS += -L$(TMPDIR) -lodbcinst -lodbc
endif

ifeq ($(PLATFORM), solaris)
  ifeq ($(ARCH_DATA_MODEL), 64)
    LDFLAGS += -L$(ODBCDIR)/ISLIodbc/2.11/lib -z nodefs
  else
    LDFLAGS += -L$(ODBCDIR)/ISLIodbc/2.11/lib -lodbcinst -lodbc
  endif
endif

ifeq ($(PLATFORM), windows)
    LDLIBS += odbc32.lib odbccp32.lib
endif 

#
# The native code for the bridge uses conditional compilation to 
# support Solaris, Win95 and Mac PPC. This is the path to the shared C files
# (which unfortunately are in the same directory as shared Java files).
#
vpath %.c $(SHARE_SRC)/classes/sun/jdbc/odbc

#
# Has been converted to the JNI: generate JNI-style header files
#
JAVAHFLAGS += -jni

ifeq ($(PLATFORM), linux)

make_libs: $(TMPDIR)/libodbcinst.so $(TMPDIR)/libodbc.so

$(TMPDIR)/libodbcinst.so: dummyodbc.c  $(TMPDIR)
	$(CC) -shared -o $@ $<

$(TMPDIR)/libodbc.so: dummyodbc.c $(TMPDIR) 
	$(CC) -shared -o $@ $<

clean::
	$(RM) -f $(TMPDIR)/libodbcinst.so $(TMPDIR)/libodbc.so

endif
