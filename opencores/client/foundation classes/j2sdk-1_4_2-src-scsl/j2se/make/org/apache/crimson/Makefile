#
# @(#)Makefile	1.11 02/08/02
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.11 02/08/02
#
# Makefile for building Crimson XML parser
#

BUILDDIR = ../../..
PACKAGE = org.apache
PRODUCT = crimson
include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile
#
FILES_java = \
	$(PKGDIR)/crimson/util/MessageCatalog.java \
	$(PKGDIR)/crimson/util/XmlChars.java \
	$(PKGDIR)/crimson/util/XmlNames.java \
	\
	$(PKGDIR)/crimson/parser/AttributeDecl.java \
	$(PKGDIR)/crimson/parser/AttributesEx.java \
	$(PKGDIR)/crimson/parser/AttributesExImpl.java \
	$(PKGDIR)/crimson/parser/ContentModel.java \
	$(PKGDIR)/crimson/parser/ContentModelState.java \
	$(PKGDIR)/crimson/parser/ElementDecl.java \
	$(PKGDIR)/crimson/parser/ElementValidator.java \
	$(PKGDIR)/crimson/parser/EndOfInputException.java \
	$(PKGDIR)/crimson/parser/EntityDecl.java \
	$(PKGDIR)/crimson/parser/ExternalEntity.java \
	$(PKGDIR)/crimson/parser/InputEntity.java \
	$(PKGDIR)/crimson/parser/InternalEntity.java \
	$(PKGDIR)/crimson/parser/Parser2.java \
	$(PKGDIR)/crimson/parser/Resolver.java \
	$(PKGDIR)/crimson/parser/SimpleHashtable.java \
	$(PKGDIR)/crimson/parser/ValidatingParser.java \
	$(PKGDIR)/crimson/parser/XMLReaderImpl.java \
	$(PKGDIR)/crimson/parser/XmlReader.java \
	\
	$(PKGDIR)/crimson/tree/AttributeNode.java \
	$(PKGDIR)/crimson/tree/AttributeNode1.java \
	$(PKGDIR)/crimson/tree/AttributeSet.java \
	$(PKGDIR)/crimson/tree/CDataNode.java \
	$(PKGDIR)/crimson/tree/CommentNode.java \
	$(PKGDIR)/crimson/tree/DataNode.java \
	$(PKGDIR)/crimson/tree/Doctype.java \
	$(PKGDIR)/crimson/tree/DocumentEx.java \
	$(PKGDIR)/crimson/tree/DOMImplementationImpl.java \
	$(PKGDIR)/crimson/tree/DomEx.java \
	$(PKGDIR)/crimson/tree/ElementEx.java \
	$(PKGDIR)/crimson/tree/ElementFactory.java \
	$(PKGDIR)/crimson/tree/ElementNode.java \
	$(PKGDIR)/crimson/tree/ElementNode2.java \
	$(PKGDIR)/crimson/tree/NamespacedNode.java \
	$(PKGDIR)/crimson/tree/NodeBase.java \
	$(PKGDIR)/crimson/tree/NodeEx.java \
	$(PKGDIR)/crimson/tree/PINode.java \
	$(PKGDIR)/crimson/tree/ParentNode.java \
	$(PKGDIR)/crimson/tree/ParseContext.java \
	$(PKGDIR)/crimson/tree/SimpleElementFactory.java \
	$(PKGDIR)/crimson/tree/TextNode.java \
	$(PKGDIR)/crimson/tree/TreeWalker.java \
	$(PKGDIR)/crimson/tree/XmlDocument.java \
	$(PKGDIR)/crimson/tree/XmlDocumentBuilder.java \
	$(PKGDIR)/crimson/tree/XmlDocumentBuilderNS.java \
	$(PKGDIR)/crimson/tree/XmlWritable.java \
	$(PKGDIR)/crimson/tree/XmlWriteContext.java \
	\
	$(PKGDIR)/crimson/jaxp/DefaultValidationErrorHandler.java \
	$(PKGDIR)/crimson/jaxp/DocumentBuilderFactoryImpl.java \
	$(PKGDIR)/crimson/jaxp/DocumentBuilderImpl.java \
	$(PKGDIR)/crimson/jaxp/SAXParserFactoryImpl.java \
	$(PKGDIR)/crimson/jaxp/SAXParserImpl.java \

#
# Rules
#
include $(BUILDDIR)/common/Classes.gmk

#
# Resources
#
LOCALE_SET_DEFINITION = j2re
RESOURCE_BUNDLES_PROPERTIES = \
	crimson/parser/resources/Messages.properties \
	crimson/tree/resources/Messages.properties
RESOURCE_SRC_DIR = $(SHARE_SRC)/classes/$(PKGDIR)
RESOURCE_DEST_DIR = $(CLASSDESTDIR)/$(PKGDIR)
include $(BUILDDIR)/common/Resources.gmk
