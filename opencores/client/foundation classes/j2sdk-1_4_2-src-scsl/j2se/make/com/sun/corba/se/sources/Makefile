#
# @(#)Makefile	1.8 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../../../../..

PACKAGE = com.sun.corba.se
PRODUCT = org

include $(BUILDDIR)/common/Defs.gmk

#
# Rules
#
all: activation.idl.compile

clean:: activation.idl.clean

#
# Files
#

include ../ActivationIDL_java.gmk

#
# Dirs
#

SRC_DIR = $(SHARE_SRC)/classes
CORBA_IDL_DIR = $(SRC_DIR)/org/omg/PortableServer

ACTIVATIONFILEDIR = $(GENSRCDIR)/com/sun/corba/se/ActivationIDL

#
# IDL compiler
#
IDLJ_MAIN = com.sun.tools.corba.se.idl.toJavaPortable.Compile
IDLJ = $(JAVA) -classpath "$(CLASSBINDIR)$(CLASSPATH_SEPARATOR)$(CLASSPATH)" $(IDLJ_MAIN) -td $(GENSRCDIR)

#
# IDLs
#

ACTIVATION.IDL = $(SRC_DIR)/com/sun/corba/se/ActivationIDL/activation.idl

#
# Activation 
#

ACTIVATIONFILES = $(ActivationIDL_java:%=$(GENSRCDIR)/%)

$(ACTIVATIONFILES): $(ACTIVATION.IDL)
	$(IDLJ) -i $(ACTIVATIONFILEDIR) -fall -oldImplBase -pkgPrefix ActivationIDL com.sun.corba.se $(ACTIVATION.IDL)

activation.idl.compile: $(ACTIVATIONFILES)

activation.idl.clean:
	$(RM) -f $(ACTIVATIONFILES)

.PHONY:  activation.idl.compile activation.idl.clean 

#
# Include
#
include $(BUILDDIR)/common/Rules.gmk
