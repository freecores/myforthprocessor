#
# @(#)Makefile	1.0 02/10/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.0 02/10/23
#
# Makefile for building Xalan
#

BUILDDIR = ../../..
PACKAGE = org.apache
PRODUCT = xml
include $(BUILDDIR)/common/Defs.gmk

#
# Take out assertions flag until source code is updated
#
#LANGUAGE_VERSION = -source 1.3

#
# Files to compile
#
FILES_java = \
	$(PKGDIR)/xml/dtm/Axis.java \
	$(PKGDIR)/xml/dtm/DTM.java \
	$(PKGDIR)/xml/dtm/DTMException.java \
	$(PKGDIR)/xml/dtm/DTMAxisIterator.java \
	$(PKGDIR)/xml/dtm/DTMAxisTraverser.java \
	$(PKGDIR)/xml/dtm/DTMConfigurationException.java \
	$(PKGDIR)/xml/dtm/DTMDOMException.java \
	$(PKGDIR)/xml/dtm/DTMFilter.java \
	$(PKGDIR)/xml/dtm/DTMIterator.java \
	$(PKGDIR)/xml/dtm/DTMManager.java \
	$(PKGDIR)/xml/dtm/DTMWSFilter.java \
	\
	$(PKGDIR)/xml/dtm/ref/DTMAxisIteratorBase.java \
	$(PKGDIR)/xml/dtm/ref/ChunkedIntArray.java \
	$(PKGDIR)/xml/dtm/ref/CoroutineManager.java \
	$(PKGDIR)/xml/dtm/ref/CoroutineParser.java \
	$(PKGDIR)/xml/dtm/ref/CustomStringPool.java \
	$(PKGDIR)/xml/dtm/ref/DTMDefaultBase.java \
	$(PKGDIR)/xml/dtm/ref/DTMDocumentImpl.java \
	\
	$(PKGDIR)/xml/dtm/ref/dom2dtm/DOM2DTM.java \
	$(PKGDIR)/xml/dtm/ref/dom2dtm/DOM2DTMdefaultNamespaceDeclarationNode.java \
	\
	$(PKGDIR)/xml/dtm/ref/sax2dtm/SAX2RTFDTM.java \
	$(PKGDIR)/xml/dtm/ref/sax2dtm/SAX2DTM.java \
	\
	$(PKGDIR)/xml/dtm/ref/DTMDefaultBaseIterators.java \
	$(PKGDIR)/xml/dtm/ref/DTMDefaultBaseTraversers.java \
	$(PKGDIR)/xml/dtm/ref/DTMManagerDefault.java \
	$(PKGDIR)/xml/dtm/ref/DTMNamedNodeMap.java \
	$(PKGDIR)/xml/dtm/ref/DTMNodeIterator.java \
	$(PKGDIR)/xml/dtm/ref/DTMNodeList.java \
	$(PKGDIR)/xml/dtm/ref/DTMNodeProxy.java \
	$(PKGDIR)/xml/dtm/ref/DTMSafeStringPool.java \
	$(PKGDIR)/xml/dtm/ref/DTMStringPool.java \
	$(PKGDIR)/xml/dtm/ref/DTMTreeWalker.java \
	$(PKGDIR)/xml/dtm/ref/ExpandedNameTable.java \
	$(PKGDIR)/xml/dtm/ref/IncrementalSAXSource.java \
	$(PKGDIR)/xml/dtm/ref/IncrementalSAXSource_Filter.java \
	$(PKGDIR)/xml/dtm/ref/NodeLocator.java \
	\
	$(PKGDIR)/xml/utils/DOMBuilder.java \
	$(PKGDIR)/xml/utils/AttList.java \
	$(PKGDIR)/xml/utils/BoolStack.java \
	$(PKGDIR)/xml/utils/CharKey.java \
	$(PKGDIR)/xml/utils/DefaultErrorHandler.java \
	$(PKGDIR)/xml/utils/ElemDesc.java \
	$(PKGDIR)/xml/utils/FastStringBuffer.java \
	$(PKGDIR)/xml/utils/Hashtree2Node.java \
	$(PKGDIR)/xml/utils/IntStack.java \
	$(PKGDIR)/xml/utils/IntVector.java \
	$(PKGDIR)/xml/utils/ListingErrorHandler.java \
	$(PKGDIR)/xml/utils/MutableAttrListImpl.java \
	$(PKGDIR)/xml/utils/NSInfo.java \
	$(PKGDIR)/xml/utils/NameSpace.java \
	$(PKGDIR)/xml/utils/NamespaceSupport2.java \
	$(PKGDIR)/xml/utils/NodeConsumer.java \
	$(PKGDIR)/xml/utils/PrefixResolverDefault.java \
	$(PKGDIR)/xml/utils/NodeVector.java \
	$(PKGDIR)/xml/utils/ObjectPool.java \
	$(PKGDIR)/xml/utils/PrefixResolver.java \
	$(PKGDIR)/xml/utils/QName.java \
	$(PKGDIR)/xml/utils/StringVector.java \
	$(PKGDIR)/xml/utils/SuballocatedIntVector.java \
	$(PKGDIR)/xml/utils/RawCharacterHandler.java \
	$(PKGDIR)/xml/utils/SAXSourceLocator.java \
	$(PKGDIR)/xml/utils/SerializableLocatorImpl.java \
	$(PKGDIR)/xml/utils/StringBufferPool.java \
	$(PKGDIR)/xml/utils/StringToIntTable.java \
	$(PKGDIR)/xml/utils/StringToStringTable.java \
	$(PKGDIR)/xml/utils/StringToStringTableVector.java \
	$(PKGDIR)/xml/utils/WrongParserException.java \
	$(PKGDIR)/xml/utils/SuballocatedByteVector.java \
	$(PKGDIR)/xml/utils/SystemIDResolver.java \
	$(PKGDIR)/xml/utils/TreeWalker.java \
	$(PKGDIR)/xml/utils/Trie.java \
	$(PKGDIR)/xml/utils/URI.java \
	$(PKGDIR)/xml/utils/UnImplNode.java \
	$(PKGDIR)/xml/utils/WrappedRuntimeException.java \
	$(PKGDIR)/xml/utils/XMLChar.java \
	$(PKGDIR)/xml/utils/XMLCharacterRecognizer.java \
	$(PKGDIR)/xml/utils/XMLString.java \
	$(PKGDIR)/xml/utils/XMLStringFactory.java \
	$(PKGDIR)/xml/utils/ObjectStack.java \
	$(PKGDIR)/xml/utils/ObjectVector.java \
        \
	$(PKGDIR)/xml/utils/res/XResources_ja_JP_A.java \
	$(PKGDIR)/xml/utils/res/XResourceBundle.java \
	$(PKGDIR)/xml/utils/res/XResources_cy.java \
	$(PKGDIR)/xml/utils/res/XResources_el.java \
	$(PKGDIR)/xml/utils/res/XResources_en.java \
	$(PKGDIR)/xml/utils/res/XResources_he.java \
	$(PKGDIR)/xml/utils/res/XResources_hy.java \
	$(PKGDIR)/xml/utils/res/XResources_ja_JP_HA.java \
	$(PKGDIR)/xml/utils/res/XResources_ja_JP_HI.java \
	$(PKGDIR)/xml/utils/res/XResources_ja_JP_I.java \
	$(PKGDIR)/xml/utils/res/XResources_ka.java \
	$(PKGDIR)/xml/utils/res/XResources_zh_CN.java \
	$(PKGDIR)/xml/utils/res/XResources_zh_TW.java \
	\
	$(PKGDIR)/xml/utils/synthetic/JavaUtils.java \
	$(PKGDIR)/xml/utils/synthetic/Class.java \
        $(PKGDIR)/xml/utils/synthetic/reflection/Constructor.java \
	$(PKGDIR)/xml/utils/synthetic/reflection/EntryPoint.java \
	$(PKGDIR)/xml/utils/synthetic/reflection/Field.java \
	$(PKGDIR)/xml/utils/synthetic/reflection/Member.java \
	$(PKGDIR)/xml/utils/synthetic/reflection/Method.java \
	$(PKGDIR)/xml/utils/synthetic/SynthesisException.java \
	$(PKGDIR)/xml/utils/synthetic/TestDriver.java \

# Rules
#
include $(BUILDDIR)/common/Classes.gmk

## Messages are in $(PKGDIR)/xalan/res/XSLTErrorResources.java 

#
# Resources
#
# RESOURCE_FILES = \
#OTHER_PROPERTIES = \
LOCALE_SET_DEFINITION = j2re
#NEW_RESOURCE_BUNDLES_PROPERTIES = \
#   xalan/res/XSLTInfo.properties \
#   xalan/serialize/serializer.properties \
#   xalan/serialize/XMLEntities.res \
#   xalan/serialize/HTMLEntities.res \
#   xalan/templates/output_html.properties \
#   xalan/templates/output_text.properties \
#   xalan/templates/output_xml.properties 


RESOURCE_SRC_DIR = $(SHARE_SRC)/classes/$(PKGDIR)
RESOURCE_DEST_DIR = $(CLASSDESTDIR)/$(PKGDIR)


## Xalan is using a special ".res" extension for entities.
$(RESOURCE_DEST_DIR)/%.res:	$(RESOURCE_SRC_DIR)/%.res
	$(install-file)

include $(BUILDDIR)/common/Resources.gmk
