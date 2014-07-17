#
# @(#)Makefile	1.7 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.7 03/01/23
#
# Makefile for building security components of JNDI LDAP service provider.
#
# These go in ldapsec.jar in the extensions directory, due to dependencies
# on security components that are also in the extensions directory.
#

BUILDDIR = ../../../..
PACKAGE = com.sun.jndi.ldapsec#		a "pseudo" package name
PRODUCT = sun

#
# JAR name, contents, and version info
#
JARNAME = ldapsec.jar
JARCONTENTS =	META-INF \
		com/sun/jndi/ldap/ext \
		com/sun/jndi/ldap/sasl \
		com/sun/security/sasl
SPECVERS = 1.2.4
IMPLVERS = J2RE $(RELEASE)

#
# Isolate the files that will be packaged into ldapsec.jar.  They don't
# belong in rt.jar.
#
CLASSDESTDIR = $(TMPDIR)/classes


include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile
#
include FILES_java.gmk

#
# Rules
#
include $(BUILDDIR)/common/Classes.gmk

#
# META-INF/services
#
SERVICES       = com.sun.security.sasl.preview.SaslClientFactory
SERVICE_SRCDIR = .
SERVICE_DSTDIR = $(CLASSDESTDIR)/META-INF/services
SERVICE_FILES  = $(SERVICES:%=$(SERVICE_DSTDIR)/%)

#
# JAR file
#
JARFILE	      = $(EXTDIR)/$(JARNAME)
MANIFEST_TMPL = Manifest.template
MANIFEST      = $(TMPDIR)/MANIFEST.MF
OPTIMIZED += check-classlist $(JARFILE)
DEBUG	  += check-classlist $(JARFILE)

# Force JAR file to be rebuilt when any class is recompiled.
check-classlist:
	if [ -s $(TMPDIR)/.classes.list ]; then $(RM) $(MANIFEST); fi

$(JARFILE): $(MANIFEST) $(SERVICE_FILES)
	@$(RM) $@
	$(JAR) cfm $@ $(MANIFEST) $(JARCONTENTS:%=-C $(CLASSDESTDIR) %)

$(MANIFEST): $(MANIFEST_TMPL)
	@$(RM) $@
	$(SED) -e 's/$$SPECVERS/$(SPECVERS)/' \
	       -e 's/$$IMPLVERS/$(IMPLVERS)/' $< > $@

$(SERVICE_DSTDIR)/%: $(SERVICE_SRCDIR)/%
	$(install-file)

clean::
	$(RM) -r $(CLASSDESTDIR) $(MANIFEST) $(JARFILE)

.PHONY: check-classlist
