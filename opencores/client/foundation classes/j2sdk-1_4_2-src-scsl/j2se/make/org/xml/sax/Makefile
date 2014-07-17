#
# @(#)Makefile	1.6 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.6 03/01/23
#
# Makefile for building SAX API classes
#

BUILDDIR = ../../..
PACKAGE = org.xml.sax
PRODUCT = xml
include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile
#
FILES_java = \
	$(PKGDIR)/AttributeList.java \
	$(PKGDIR)/Attributes.java \
	$(PKGDIR)/ContentHandler.java \
	$(PKGDIR)/DTDHandler.java \
	$(PKGDIR)/DocumentHandler.java \
	$(PKGDIR)/EntityResolver.java \
	$(PKGDIR)/ErrorHandler.java \
	$(PKGDIR)/HandlerBase.java \
	$(PKGDIR)/InputSource.java \
	$(PKGDIR)/Locator.java \
	$(PKGDIR)/Parser.java \
	$(PKGDIR)/SAXException.java \
	$(PKGDIR)/SAXNotRecognizedException.java \
	$(PKGDIR)/SAXNotSupportedException.java \
	$(PKGDIR)/SAXParseException.java \
	$(PKGDIR)/XMLFilter.java \
	$(PKGDIR)/XMLReader.java \
	\
	$(PKGDIR)/helpers/AttributeListImpl.java \
	$(PKGDIR)/helpers/AttributesImpl.java \
	$(PKGDIR)/helpers/DefaultHandler.java \
	$(PKGDIR)/helpers/LocatorImpl.java \
	$(PKGDIR)/helpers/NamespaceSupport.java \
	$(PKGDIR)/helpers/ParserAdapter.java \
	$(PKGDIR)/helpers/ParserFactory.java \
	$(PKGDIR)/helpers/XMLFilterImpl.java \
	$(PKGDIR)/helpers/XMLReaderAdapter.java \
	$(PKGDIR)/helpers/XMLReaderFactory.java \
	$(PKGDIR)/helpers/NewInstance.java \
	\
	$(PKGDIR)/ext/DeclHandler.java \
	$(PKGDIR)/ext/LexicalHandler.java

#
# Rules
#
include $(BUILDDIR)/common/Classes.gmk
