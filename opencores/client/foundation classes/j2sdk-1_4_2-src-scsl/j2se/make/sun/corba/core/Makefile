#
# @(#)Makefile	1.67 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building RMI/IIOP
#

BUILDDIR = ../../..
PACKAGE = com.sun.corba.se.internal.io
PRODUCT = sun
ifndef PUREORB_SUPPORT
LIBRARY = ioser12
endif
include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile.
#

include FILES_java.gmk
ifndef PUREORB_SUPPORT
JAVAX_RMI_APIS_java = \
		$(UTIL) \
		$(IO)
endif

ifdef STANDALONE_CORBA_WS
OTHER_LDFLAGS=-L$(BOOTDIR)/jre/lib/$(LIBARCH) -L$(BOOTDIR)/jre/lib/$(LIBARCH)/native_threads -ljvm
OTHER_INCLUDES+=-ICClassHeaders -I$(BOOTDIR)/include -I$(BOOTDIR)/include/$(PLATFORM)
else
OTHER_LDLIBS=$(JVMLIB)
endif

FILES_c = ioser.c

JAVAX_java = \
	$(RMI) \
	$(CORBA) \
	$(TRANSACTION)

#ifdef STANDALONE_CORBA_WS
#include $(MINCLUDES)javax_jts.jmk
#JAVAX_ALL_java = \
#       $(JAVAX_java) \
#       $(JAVAX_JTS_java)
#else
JAVAX_ALL_java = \
	$(JAVAX_java)
#endif

FILES_java =    $(JAVAX_ALL_java) \
		$(JAVAX_RMI_APIS_java) \
#
# Generate header files for.
#
FILES_export = \
	com/sun/corba/se/internal/io/IIOPInputStream.java \
	com/sun/corba/se/internal/io/IIOPOutputStream.java \
	com/sun/corba/se/internal/io/ObjectStreamClass.java \
	com/sun/corba/se/internal/io/LibraryManager.java \
	com/sun/corba/se/internal/util/JDKClassLoader.java

ifeq ($(PLATFORM), windows)
  # Files built here do not compile with warning level 3
  COMPILER_WARNING_LEVEL=2
endif

#
# Resources
#
LOCALE_SET_DEFINITION = j2re
RESOURCE_BUNDLES_COMPILED_PROPERTIES = com/sun/corba/se/internal/orbutil/resources/sunorb.properties
RESOURCE_SRC_DIR  = $(SHARE_SRC)/classes
include $(BUILDDIR)/common/Resources.gmk


#
# Rules.
#
ifndef PUREORB_SUPPORT
  include $(BUILDDIR)/common/Library.gmk
else
  include $(BUILDDIR)/common/Classes.gmk
endif

#
# Extra clean rules because we build more than one package.
#
clean:: classheaders.clean objects.clean
	$(RM) -r $(CLASSBINDIR)/javax/rmi
	$(RM) -r $(CLASSBINDIR)/javax/transaction
	$(RM) -r $(CLASSBINDIR)/com/sun/corba/se/internal

