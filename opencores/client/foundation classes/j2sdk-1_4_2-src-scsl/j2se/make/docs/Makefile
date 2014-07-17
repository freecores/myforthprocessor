#
# @(#)Makefile	1.141 02/11/13
#
# Makefile for javadoc-generated documentation. To be included into
# the top level makefile.
#

#
# Definitions for $(DOCSDIR), $(MKDIR), $(BINDIR), etc.
#
BUILDDIR=..
PRODUCT=docs
include $(BUILDDIR)/common/Defs.gmk

#
# Variables used by docs target
#
DOCSTMPDIR          = $(TMPDIR)/doctmp

#
# Set higher max memory limit for 64-bit architecture
#
ifeq ($(ARCH_DATA_MODEL), 32)
  JAVADOC_VM_MAX_MEMORY_FLAG = -J-Xmx256M
else
  JAVADOC_VM_MAX_MEMORY_FLAG = -J-Xmx512M
endif


JAVADOCFLAGS  = $(JAVADOC_VM_MAX_MEMORY_FLAG)      \
                -source 1.4                        \
                -tag beaninfo:X                    \
                -quiet                             \
                -splitIndex                        \
                -use                               \
                -doctitle $(JAVADOCTITLE)          \
                -windowtitle $(JAVADOCWINDOWTITLE) \
                -header $(JAVADOCHEADER)           \
                -bottom $(JAVADOCBOTTOM)           \
                -group $(GROUPNAME) $(REGEXP)      \
                -overview $(SHARE_SRC)/classes/overview-core.html
JAVADOCTITLE  = 'Java<sup><font size=-2>TM</font></sup> 2 Platform, Standard Edition, v $(JDK_VERSION)<br>API Specification'
JAVADOCWINDOWTITLE  = 'Java 2 Platform SE v$(JDK_VERSION)'
JAVADOCHEADER = '<b>Java<sup><font size=-2>TM</font></sup>&nbsp;2&nbsp;Platform<br>Std.&nbsp;Ed. v$(JDK_VERSION)</b>'
JAVADOCBOTTOM = '<font size="-1"><a href="http://java.sun.com/cgi-bin/bugreport.cgi">Submit a bug or feature</a><br>For further API reference and developer documentation, see <a href="{@docroot}/../relnotes/devdocs-vs-specs.html">Java 2 SDK SE Developer Documentation</a>. That documentation contains more detailed, developer-targeted descriptions, with conceptual overviews, definitions of terms, workarounds, and working code examples. <p>Copyright 2003 Sun Microsystems, Inc. All rights reserved. Use is subject to <a href="{@docroot}/../relnotes/license.html">license terms</a>. Also see the <a href="http://java.sun.com/docs/redist.html">documentation redistribution policy</a>.</font>'
GROUPNAME = "Java 2 Platform Packages"
REGEXP = "java.*:javax.swing*:javax.accessibility:javax.imageio*:javax.naming*:javax.net*:javax.print*:javax.sound*:org.omg.*:javax.rmi*:javax.sql*:javax.transaction*:javax.xml*:org.w3c.dom*:org.xml.sax*:javax.security*:org.ietf*:javax.crypto*"

#
# Variables used by domapidocs target
#

DOMAPI_JAVADOCFLAGS = -source 1.4 $(JAVADOC_VM_MAX_MEMORY_FLAG)           \
                    -quiet                                    \
                    -splitIndex                               \
                    -use                                      \
                    -doctitle $(DOMAPI_JAVADOCTITLE)          \
                    -windowtitle $(DOMAPI_JAVADOCWINDOWTITLE) \
                    -header $(DOMAPI_JAVADOCHEADER)           \
                    -bottom $(DOMAPI_JAVADOCBOTTOM)           \
                    -group $(DOMAPI_GROUPNAME) $(DOMAPI_REGEXP)
DOMAPI_JAVADOCTITLE  = 'Common DOM API'
DOMAPI_JAVADOCWINDOWTITLE  = 'Common DOM API'
DOMAPI_JAVADOCHEADER = '<b>Common DOM API</b>'
DOMAPI_JAVADOCBOTTOM = '<font size="-1"><a href="http://java.sun.com/cgi-bin/bugreport.cgi">Submit a bug or feature</a><br>Java is a trademark or registered trademark of Sun Microsystems, Inc. in the US and other countries.<br>Copyright 2003 Sun Microsystems, Inc. 4150 Network Circle<br>Santa Clara, California, 95054, U.S.A.  All Rights Reserved.</font>'
DOMAPI_GROUPNAME = "Packages"
DOMAPI_REGEXP = "com.sun.java.browser.dom:org.w3c.dom*"
# DOMAPI_PKGS is located in NON_CORE_PKGS.gmk

#
# Variables used by docletapi target
#

DOCLETAPI_JAVADOCFLAGS = -source 1.4 $(JAVADOC_VM_MAX_MEMORY_FLAG)           \
                    -quiet                                       \
                    -use                                         \
                    -doctitle $(DOCLETAPI_JAVADOCTITLE)          \
                    -windowtitle $(DOCLETAPI_JAVADOCWINDOWTITLE) \
                    -header $(DOCLETAPI_JAVADOCHEADER)           \
                    -bottom $(DOCLETAPI_JAVADOCBOTTOM)           \
                    -group $(DOCLETAPI_GROUPNAME) $(DOCLETAPI_REGEXP)
DOCLETAPI_JAVADOCTITLE  = 'Doclet API'
DOCLETAPI_JAVADOCWINDOWTITLE  = 'Doclet API'
DOCLETAPI_JAVADOCHEADER = '<b>Doclet API</b>'
DOCLETAPI_JAVADOCBOTTOM = '<font size="-1"><a href="http://java.sun.com/cgi-bin/bugreport.cgi">Submit a bug or feature</a><br>Java is a trademark or registered trademark of Sun Microsystems, Inc. in the US and other countries.<br>Copyright 1993-2003 Sun Microsystems, Inc. 4150 Network Circle<br>Santa Clara, California, 95054, U.S.A.  All Rights Reserved.</font>'
DOCLETAPI_GROUPNAME = "Packages"
DOCLETAPI_REGEXP = "com.sun.javadoc"
# DOCLETAPI_PKGS is located in NON_CORE_PKGS.gmk

#
# Variables used by tagletapi target
#

TAGLETAPI_JAVADOCFLAGS = $(JAVADOC_VM_MAX_MEMORY_FLAG)           \
                    -quiet                                       \
                    -nonavbar                                    \
                    -noindex                                     \
                    -bottom $(TAGLETAPI_JAVADOCBOTTOM)           
TAGLETAPI_JAVADOCBOTTOM = '<font size="-1"><a href="http://java.sun.com/cgi-bin/bugreport.cgi">Submit a bug or feature</a><br>Java is a trademark or registered trademark of Sun Microsystems, Inc. in the US and other countries.<br>Copyright 1993-2003 Sun Microsystems, Inc. 4150 Network Circle<br>Santa Clara, California, 95054, U.S.A.  All Rights Reserved.</font>'
# TAGLETAPI_PKGS is located in NON_CORE_PKGS.gmk

#
# Variables used by jpeg target
#

JPEG_JAVADOCFLAGS = -source 1.4 $(JAVADOC_VM_MAX_MEMORY_FLAG)           \
                    -quiet                                  \
                    -splitIndex                             \
                    -use                                    \
                    -doctitle $(JPEG_JAVADOCTITLE)          \
                    -windowtitle $(JPEG_JAVADOCWINDOWTITLE) \
                    -header $(JPEG_JAVADOCHEADER)           \
                    -bottom $(JPEG_JAVADOCBOTTOM)           \
                    -group $(JPEG_GROUPNAME) $(JPEG_REGEXP) \
                    -overview $(SHARE_SRC)/classes/overview-bundled.html
JPEG_JAVADOCTITLE  = 'Other API Bundled with the Java 2 SDK'
JPEG_JAVADOCWINDOWTITLE  = 'Other API Bundled with the Java 2 SDK'
JPEG_JAVADOCHEADER = '<b></b>'
JPEG_JAVADOCBOTTOM = '<font size="-1"><a href="http://java.sun.com/cgi-bin/bugreport.cgi">Submit a bug or feature</a><br>Java is a trademark or registered trademark of Sun Microsystems, Inc. in the US and other countries.<br>Copyright 1993-2003 Sun Microsystems, Inc. 4150 Network Circle<br>Santa Clara, California, 95054, U.S.A.  All Rights Reserved.</font>'
JPEG_GROUPNAME = "Packages"
JPEG_REGEXP = "com.sun.image.codec.jpeg"

# 
# Variables used by jdi target
# 

JPDA_SOURCEPATH  = $(TOPDIR)/src/share/classes

JDI_JAVADOCFLAGS = -source 1.4 $(JAVADOC_VM_MAX_MEMORY_FLAG)                 \
                   -quiet                                        \
                   -use                                          \
                   -nodeprecatedlist                             \
                   -d $(DOCSDIR)/guide/jpda/jdi                  \
                   -sourcepath $(JPDA_SOURCEPATH)                \
                   -windowtitle $(JDI_WINDOWTITLE)               \
                   -doctitle $(JDI_DOCTITLE)                     \
                   -header $(JDI_HEADER)                         \
                   -linkoffline ../../../api $(DOCSDIR)/api/     \
                   -overview $(JPDA_SOURCEPATH)/jdi-overview.html
JDI_WINDOWTITLE = "Java Debug Interface"
JDI_DOCTITLE    = "Java<font size="-2"><sup>TM</sup></font> Debug Interface"
JDI_HEADER      = "Java Debug Interface"
# JDI_PKGS is located in NON_CORE_PKGS.gmk

# 
# Variables used by JAAS target
# 
# NOTE: Quotes are required around sourcepath argument only on Windows.  Otherwise, 
#       you get "No packages or classes specified." due to $(CLASSPATH_SEPARATOR)
#       being interpreted as an end of command (newline)

JAAS_SOURCEPATH  = "$(TOPDIR)/src/share/classes$(CLASSPATH_SEPARATOR)$(TOPDIR)/src/solaris/classes$(CLASSPATH_SEPARATOR)$(TOPDIR)/src/windows/classes$(CLASSPATH_SEPARATOR)$(TOPDIR)/src/linux/classes"
JAAS_DOCDIR	 = $(DOCSDIR)/guide/security/jaas/spec
JAAS_JAVADOCFLAGS = -source 1.4 $(JAVADOC_VM_MAX_MEMORY_FLAG)                \
                   -quiet                                        \
                   -use                                          \
                   -nodeprecatedlist                             \
                   -d $(JAAS_DOCDIR)				 \
                   -sourcepath $(JAAS_SOURCEPATH)                \
                   -windowtitle $(JAAS_WINDOWTITLE)              \
                   -doctitle $(JAAS_DOCTITLE)                    \
                   -header $(JAAS_JAVADOCHEADER)                 \
                   -linkoffline ../../../../api $(DOCSDIR)/api/ \
                   -overview $(TOPDIR)/src/share/classes/com/sun/security/auth/jaas-overview.html
JAAS_WINDOWTITLE = "Java Authentication and Authorization Service "
JAAS_DOCTITLE    = "Java<font size="-2"><sup>TM</sup></font> Authentication and Authorization Service" 
JAAS_JAVADOCHEADER      = "JAAS"
# JAAS_PKGS is located in NON_CORE_PKGS.gmk

# 
# Variables used by JGSS target
# 

JGSS_SOURCEPATH  = $(TOPDIR)/src/share/classes
JGSS_DOCDIR	 = $(DOCSDIR)/guide/security/jgss/spec

JGSS_JAVADOCFLAGS = -source 1.4 $(JAVADOC_VM_MAX_MEMORY_FLAG)                \
                   -quiet                                        \
                   -use                                          \
                   -nodeprecatedlist                             \
                   -d $(JGSS_DOCDIR)				 \
                   -sourcepath $(JGSS_SOURCEPATH)                \
                   -windowtitle $(JGSS_WINDOWTITLE)              \
                   -doctitle $(JGSS_DOCTITLE)                    \
                   -header $(JGSS_JAVADOCHEADER)                 \
                   -linkoffline ../../../../api $(DOCSDIR)/api/ \
                   -overview $(JGSS_SOURCEPATH)/com/sun/security/jgss/jgss-overview.html

JGSS_WINDOWTITLE = "Java GSS-API Utilities "
JGSS_DOCTITLE    = "Java<font size="-2"><sup>TM</sup></font> GSS-API Utilities"
JGSS_JAVADOCHEADER      = "Java GSS-API Utilities"
# JGSS_PKGS is located in NON_CORE_PKGS.gmk

#
# Variables used by jdwp target
#
JDWP_SPEC       = $(JPDA_SOURCEPATH)/com/sun/tools/jdwpgen/jdwp.spec

#
# Path where javadoc should find source files for release docs
#
RELEASEDOCS_SRCPATH = "$(SHARE_SRC)/classes$(CLASSPATH_SEPARATOR)$(GENSRCDIR)$(CLASSPATH_SEPARATOR)$(SHARE_SRC)/doc/stub"

#
# Needed above if extensions are added
#
# RELEASEDOCS_EXT_SRCPATH = $(TOPDIR)/ext/servlet/src/share
# RELEASEDOCS_SRCPATH = "$(SHARE_SRC)/classes$(CLASSPATH_SEPARATOR)$(RELEASEDOCS_EXT_SRCPATH)"
#

#
# CORE_PKGS environment variable has been moved to the following file
#
include CORE_PKGS.gmk

#
# Load environment variables for API package names that are not part of
# the J2SE platform
#
include NON_CORE_PKGS.gmk

#
# Generates the release doc for the JDK.
# The "all" target should take about 30 minutes on an Ultra 1 with 128M RAM
# and 240MB swap space.
# The first part generates the docs and serialized-form.html for the Java platform packages.
# The following parts are for generating jpeg and jdi.
#
all: coredocs docletapidocs domapidocs jpegdocs jpdadocs jaasdocs jgssdocs

coredocs $(DOCSDIR)/api/package-list:
	@# ######## core packages #######################
	$(RM) -r $(DOCSDIR)/api
	$(MKDIR) -p $(DOCSDIR)/api
	$(JAVADOC) $(JAVADOCFLAGS)                     \
                   -d $(DOCSDIR)/api                   \
                   -sourcepath $(RELEASEDOCS_SRCPATH)  \
                   $(CORE_PKGS)

docletapidocs:
	@# ######## doclet api ############################
	$(RM) -r $(DOCSDIR)/tooldocs/javadoc/doclet
	$(MKDIR) -p $(DOCSDIR)/tooldocs/javadoc/doclet
	$(JAVADOC) $(DOCLETAPI_JAVADOCFLAGS)                 \
                   -d $(DOCSDIR)/tooldocs/javadoc/doclet     \
                   -sourcepath $(RELEASEDOCS_SRCPATH)        \
                   -linkoffline ../../../api $(DOCSDIR)/api/ \
                   $(DOCLETAPI_PKGS)

tagletapidocs:
	@# ######## taglet api ############################
	$(RM) -r $(DOCSDIR)/tooldocs/javadoc/taglet
	$(MKDIR) -p $(DOCSDIR)/tooldocs/javadoc/taglet
	$(RM) -r $(DOCSTMPDIR)
	mkdir -p $(DOCSTMPDIR)
	$(JAVADOC) $(TAGLETAPI_JAVADOCFLAGS)                 \
                   -d $(DOCSTMPDIR)                          \
                   -sourcepath         \
                   -linkoffline ../../../api $(DOCSDIR)/api/ \
                   $(RELEASEDOCS_SRCPATH)/$(TAGLETAPI_FILE)
	cp -r $(DOCSTMPDIR)/com $(DOCSDIR)/tooldocs/javadoc/taglet
	cp $(DOCSTMPDIR)/stylesheet.css $(DOCSDIR)/tooldocs/javadoc/taglet
	$(RM) -r $(DOCSTMPDIR)

domapidocs:
	@# ######## dom api ############################
	$(RM) -r $(DOCSDIR)/guide/plugin/dom
	$(MKDIR) -p $(DOCSDIR)/guide/plugin/dom
	$(JAVADOC) $(DOMAPI_JAVADOCFLAGS)                 \
                   -d $(DOCSDIR)/guide/plugin/dom            \
                   -sourcepath $(RELEASEDOCS_SRCPATH)        \
                   -linkoffline ../../../api $(DOCSDIR)/api/ \
                   $(DOMAPI_PKGS)
jpegdocs:
	@# ######## api-jpeg ############################
	$(RM) -r $(DOCSDIR)/guide/2d/api-jpeg
	$(MKDIR) -p $(DOCSDIR)/guide/2d/api-jpeg
	$(JAVADOC) $(JPEG_JAVADOCFLAGS)                      \
                   -d $(DOCSDIR)/guide/2d/api-jpeg           \
                   -sourcepath $(RELEASEDOCS_SRCPATH)        \
                   -linkoffline ../../../api $(DOCSDIR)/api/ \
                   $(JPEG_PKGS)

jpdadocs: jdidocs jdwpdocs

jdidocs: $(DOCSDIR)/api/package-list
	@# ######## jdi #################################
	$(RM) -r $(DOCSDIR)/guide/jpda/jdi
	$(MKDIR) -p $(DOCSDIR)/guide/jpda/jdi
	$(JAVADOC) $(JDI_JAVADOCFLAGS)                       \
                   $(JDI_PKGS)

jdwpdocs:
	@# ######## jdwp #################################
	$(CD) $(TOPDIR)/make/jpda/jdwpgen/; $(MAKE) all
	$(RM) -r $(DOCSDIR)/guide/jpda/jdwp
	$(MKDIR) -p $(DOCSDIR)/guide/jpda/jdwp
	$(JAVA) -classpath $(CLASSBINDIR)                   \
                com.sun.tools.jdwpgen.Main                  \
                $(JDWP_SPEC)                                \
                -jvmdi $(BOOTDIR)/include/jvmdi.h           \
                -doc $(DOCSDIR)/guide/jpda/jdwp/jdwp-protocol.html

jaasdocs: $(DOCSDIR)/api/package-list
	@# ######## api-jaas ############################
	$(RM) -r $(JAAS_DOCDIR)
	$(MKDIR) -p $(JAAS_DOCDIR) 
	$(JAVADOC) $(JAAS_JAVADOCFLAGS)                     \
		   $(JAAS_PKGS)

jgssdocs: $(DOCSDIR)/api/package-list
	@# ######## api-jgss ############################
	$(RM) -r $(JGSS_DOCDIR)
	$(MKDIR) -p $(JGSS_DOCDIR) 
	$(JAVADOC) $(JGSS_JAVADOCFLAGS)                     \
		   $(JGSS_PKGS)
#
# Generate docs for all packages in the JDK build/solaris/classes
# directory: sun.*, sunw.*, com.sun.* and java.*, but no extensions
# javax.* (for now)
#

ALLDOCS_DIR    = $(OUTPUTDIR)/all-docs
ALLDOCS_TITLE  = 'Java Core and Proprietary Packages'
ALLDOCS_HEADER = 'Core/Proprietary'
ALLDOCS_BOTTOM = '<font size="-1">Java, Java 2D, and JDBC are trademarks or registered trademarks of Sun Microsystems, Inc. in the US and other countries.<br>Copyright 1993-2003 Sun Microsystems, Inc. 4150 Network Circle<br>Santa Clara, California, 95054, U.S.A.  All Rights Reserved.</font>'
ALLDOCS_FLAGS  = -source 1.4 $(JAVADOC_VM_MAX_MEMORY_FLAG) -quiet -splitIndex -doctitle $(ALLDOCS_TITLE) -windowtitle $(ALLDOCS_TITLE) -header $(ALLDOCS_HEADER) -footer $(ALLDOCS_HEADER) -bottom $(ALLDOCS_BOTTOM)

alldocs: .alldocs.pkg.list
	$(RM) -r $(ALLDOCS_DIR)/api
	$(MKDIR) -p $(ALLDOCS_DIR)/api
	$(JAVADOC) $(ALLDOCS_FLAGS)         \
                   -d $(ALLDOCS_DIR)/api    \
                   -sourcepath "$(SHARE_SRC)/classes$(CLASSPATH_SEPARATOR)$(PLATFORM_SRC)/classes" \
                   `$(CAT) .alldocs.pkg.list`

# File that lists all packages to be documented, located at build/solaris/.alldocs.pkg.list
.alldocs.pkg.list:
	($(CD) $(CLASSBINDIR); $(FIND) . -name '*.class' -print) | \
	    $(SED) 's/..//' | $(SED) 's%/[^/]*.class%%' | $(SED) 's%/%.%g' | $(SORT) | $(UNIQ) \
	    > $@


#
# Avoid accidents.
#
.PHONY: docs alldocs  \
	releasedocs-serialpublicprivatepage \
	releasedocs-serialprivateonlypage
