#
# @(#)Makefile	1.10 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.10 03/01/23
#
# Makefile to generate sun/misc/Version.java
#

BUILDDIR = ../..
include $(BUILDDIR)/common/Defs.gmk

all optimized debug: $(GENSRCDIR)/sun/misc/Version.java

$(GENSRCDIR)/sun/misc/Version.java: \
		$(SHARE_SRC)/classes/sun/misc/Version-template.java
	$(MKDIR) -p $(@D)
	$(SED) -e 's/@@java_version@@/$(RELEASE)/g' \
	    -e 's/@@java_runtime_version@@/$(FULL_VERSION)/g' $< > $@

clean clobber:
	$(RM) $(GENSRCDIR)/sun/misc/Version.java
