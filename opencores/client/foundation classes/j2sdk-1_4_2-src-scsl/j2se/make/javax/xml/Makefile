#
# @(#)Makefile	1.14 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.14 03/01/23
#
# Makefile for building packages under javax.xml
#

BUILDDIR = ../..
PACKAGE = javax.xml
PRODUCT = xml
include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile
#
FILES_java = \
	$(PKGDIR)/parsers/DocumentBuilder.java \
	$(PKGDIR)/parsers/DocumentBuilderFactory.java \
	$(PKGDIR)/parsers/FactoryConfigurationError.java \
	$(PKGDIR)/parsers/FactoryFinder.java \
	$(PKGDIR)/parsers/ParserConfigurationException.java \
	$(PKGDIR)/parsers/SAXParser.java \
	$(PKGDIR)/parsers/SAXParserFactory.java \
	\
	$(PKGDIR)/transform/ErrorListener.java \
	$(PKGDIR)/transform/FactoryFinder.java \
	$(PKGDIR)/transform/OutputKeys.java \
	$(PKGDIR)/transform/Result.java \
	$(PKGDIR)/transform/Source.java \
	$(PKGDIR)/transform/SourceLocator.java \
	$(PKGDIR)/transform/Templates.java \
	$(PKGDIR)/transform/Transformer.java \
	$(PKGDIR)/transform/TransformerConfigurationException.java \
	$(PKGDIR)/transform/TransformerException.java \
	$(PKGDIR)/transform/TransformerFactory.java \
	$(PKGDIR)/transform/TransformerFactoryConfigurationError.java \
	$(PKGDIR)/transform/URIResolver.java \
	$(PKGDIR)/transform/dom/DOMLocator.java \
	$(PKGDIR)/transform/dom/DOMResult.java \
	$(PKGDIR)/transform/dom/DOMSource.java \
	$(PKGDIR)/transform/sax/SAXResult.java \
	$(PKGDIR)/transform/sax/SAXSource.java \
	$(PKGDIR)/transform/sax/SAXTransformerFactory.java \
	$(PKGDIR)/transform/sax/TemplatesHandler.java \
	$(PKGDIR)/transform/sax/TransformerHandler.java \
	$(PKGDIR)/transform/stream/StreamResult.java \
	$(PKGDIR)/transform/stream/StreamSource.java


#
# Rules
#
include $(BUILDDIR)/common/Classes.gmk
