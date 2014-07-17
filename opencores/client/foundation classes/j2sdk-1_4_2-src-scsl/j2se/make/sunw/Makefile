#
# @(#)Makefile	1.14 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.14 03/01/23
#

BUILDDIR = ..
PACKAGE = sunw
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Rules
#
default: all


#
# Files
#
FILES_java = \
	sunw/util/EventObject.java \
        sunw/util/EventListener.java \
        sunw/io/Serializable.java

#
# include rules
#
include $(BUILDDIR)/common/Rules.gmk

all :: classes

clean :: .delete.classlist
	$(RM) -r $(CLASSBINDIR)/sunw


