#
# @(#)Makefile	1.21 02/08/02
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.21 02/08/02
#
# Makefile for building Xalan
#

BUILDDIR = ../../..
PACKAGE = org.apache
PRODUCT = xalan
include $(BUILDDIR)/common/Defs.gmk

#
# Take out assertions flag until source code is updated
#
#LANGUAGE_VERSION = -source 1.3

#
# Files to compile
#
#       $(PKGDIR)/xalan/extensions/extensions.dfPackage 
#        $(PKGDIR)/xalan/templates/FuncKey.class 
FILES_java =\
        $(PKGDIR)/xalan/client/XSLTProcessorApplet.java \
        \
        $(PKGDIR)/xalan/Version.java \
        \
        $(PKGDIR)/xalan/extensions/ExtensionHandlerJava.java \
        $(PKGDIR)/xalan/extensions/ExpressionContext.java \
        $(PKGDIR)/xalan/extensions/ExpressionVisitor.java \
        $(PKGDIR)/xalan/extensions/ExtensionHandler.java \
        $(PKGDIR)/xalan/extensions/XSLProcessorContext.java \
        $(PKGDIR)/xalan/extensions/ExtensionHandlerExsltFunction.java \
        $(PKGDIR)/xalan/extensions/ExtensionHandlerGeneral.java \
        $(PKGDIR)/xalan/extensions/ExtensionHandlerJavaClass.java \
        $(PKGDIR)/xalan/extensions/ExtensionHandlerJavaPackage.java \
        $(PKGDIR)/xalan/extensions/ExtensionNamespaceSupport.java \
        $(PKGDIR)/xalan/extensions/ExtensionNamespacesManager.java \
        $(PKGDIR)/xalan/extensions/ExtensionsTable.java \
        $(PKGDIR)/xalan/extensions/MethodResolver.java \
        $(PKGDIR)/xalan/lib/sql/ConnectionPoolManager.java \
        $(PKGDIR)/xalan/lib/sql/ConnectionPool.java \
        $(PKGDIR)/xalan/lib/sql/DTMDocument.java \
        $(PKGDIR)/xalan/lib/sql/ObjectArray.java \
        $(PKGDIR)/xalan/lib/sql/DefaultConnectionPool.java \
        $(PKGDIR)/xalan/lib/sql/PooledConnection.java \
        $(PKGDIR)/xalan/lib/sql/QueryParameter.java \
        $(PKGDIR)/xalan/lib/sql/SQLDocument.java \
        $(PKGDIR)/xalan/lib/sql/SQLErrorDocument.java \
        $(PKGDIR)/xalan/lib/sql/XConnection.java \
        $(PKGDIR)/xalan/lib/ExsltCommon.java \
        $(PKGDIR)/xalan/lib/ExsltDatetime.java \
        $(PKGDIR)/xalan/lib/ExsltMath.java \
        $(PKGDIR)/xalan/lib/ExsltSets.java \
        $(PKGDIR)/xalan/lib/Extensions.java \
        $(PKGDIR)/xalan/lib/NodeInfo.java \
        $(PKGDIR)/xalan/lib/PipeDocument.java \
        $(PKGDIR)/xalan/lib/Redirect.java \
        $(PKGDIR)/xalan/lib/ExsltDynamic.java \
        $(PKGDIR)/xalan/lib/ExsltStrings.java \
        $(PKGDIR)/xalan/lib/ExsltBase.java \
        \
        $(PKGDIR)/xalan/processor/ProcessorDecimalFormat.java \
        $(PKGDIR)/xalan/processor/ProcessorAttributeSet.java \
        $(PKGDIR)/xalan/processor/ProcessorCharacters.java \
        $(PKGDIR)/xalan/processor/ProcessorExsltFuncResult.java \
        $(PKGDIR)/xalan/processor/ProcessorExsltFunction.java \
        $(PKGDIR)/xalan/processor/ProcessorGlobalParamDecl.java \
        $(PKGDIR)/xalan/processor/ProcessorGlobalVariableDecl.java \
        $(PKGDIR)/xalan/processor/ProcessorImport.java \
        $(PKGDIR)/xalan/processor/ProcessorInclude.java \
        $(PKGDIR)/xalan/processor/ProcessorKey.java \
        $(PKGDIR)/xalan/processor/ProcessorLRE.java \
        $(PKGDIR)/xalan/processor/ProcessorNamespaceAlias.java \
        $(PKGDIR)/xalan/processor/ProcessorOutputElem.java \
        $(PKGDIR)/xalan/processor/ProcessorPreserveSpace.java \
        $(PKGDIR)/xalan/processor/ProcessorStripSpace.java \
        $(PKGDIR)/xalan/processor/ProcessorStylesheetDoc.java \
        $(PKGDIR)/xalan/processor/ProcessorStylesheetElement.java \
        $(PKGDIR)/xalan/processor/ProcessorTemplate.java \
        $(PKGDIR)/xalan/processor/ProcessorTemplateElem.java \
        $(PKGDIR)/xalan/processor/ProcessorText.java \
        $(PKGDIR)/xalan/processor/ProcessorUnknown.java \
        $(PKGDIR)/xalan/processor/StopParseException.java \
        $(PKGDIR)/xalan/processor/StylesheetHandler.java \
        $(PKGDIR)/xalan/processor/StylesheetPIHandler.java \
        $(PKGDIR)/xalan/processor/TransformerFactoryImpl.java \
        $(PKGDIR)/xalan/processor/WhitespaceInfoPaths.java \
        $(PKGDIR)/xalan/processor/XSLTElementDef.java \
        $(PKGDIR)/xalan/processor/XSLProcessorVersion.java \
        $(PKGDIR)/xalan/processor/XSLTAttributeDef.java \
        $(PKGDIR)/xalan/processor/XSLTElementProcessor.java \
        $(PKGDIR)/xalan/processor/XSLTSchema.java \
        \
        $(PKGDIR)/xalan/res/XSLTErrorResources.java \
        $(PKGDIR)/xalan/res/XSLTErrorResources_de.java \
        $(PKGDIR)/xalan/res/XSLTErrorResources_en.java \
        $(PKGDIR)/xalan/res/XSLTErrorResources_es.java \
        $(PKGDIR)/xalan/res/XSLTErrorResources_fr.java \
        $(PKGDIR)/xalan/res/XSLTErrorResources_it.java \
        $(PKGDIR)/xalan/res/XSLTErrorResources_ja.java \
        $(PKGDIR)/xalan/res/XSLTErrorResources_ko.java \
        $(PKGDIR)/xalan/res/XSLTErrorResources_sv.java \
        $(PKGDIR)/xalan/res/XSLTErrorResources_zh_CN.java \
        $(PKGDIR)/xalan/res/XSLTErrorResources_zh_TW.java \
        $(PKGDIR)/xalan/res/XSLMessages.java \
        \
        $(PKGDIR)/xalan/serialize/DOMSerializer.java \
        $(PKGDIR)/xalan/serialize/CharInfo.java \
        $(PKGDIR)/xalan/serialize/EncodingInfo.java \
        $(PKGDIR)/xalan/serialize/ElemDesc.java \
        $(PKGDIR)/xalan/serialize/Encodings.java \
        $(PKGDIR)/xalan/serialize/Method.java \
        $(PKGDIR)/xalan/serialize/Serializer.java \
        $(PKGDIR)/xalan/serialize/SerializerFactory.java \
        $(PKGDIR)/xalan/serialize/SerializerToHTML.java \
        $(PKGDIR)/xalan/serialize/SerializerToText.java \
        $(PKGDIR)/xalan/serialize/SerializerToXML.java \
        $(PKGDIR)/xalan/serialize/WriterToASCI.java \
        $(PKGDIR)/xalan/serialize/WriterToUTF8.java \
        $(PKGDIR)/xalan/serialize/WriterToUTF8Buffered.java \
        \
        $(PKGDIR)/xalan/templates/AVTPart.java \
        $(PKGDIR)/xalan/templates/AVT.java \
        $(PKGDIR)/xalan/templates/AbsPathChecker.java \
        $(PKGDIR)/xalan/templates/AVTPartSimple.java \
        $(PKGDIR)/xalan/templates/AVTPartXPath.java \
        $(PKGDIR)/xalan/templates/Constants.java \
        $(PKGDIR)/xalan/templates/ElemAttribute.java \
        $(PKGDIR)/xalan/templates/ElemIf.java \
        $(PKGDIR)/xalan/templates/DecimalFormatProperties.java \
        $(PKGDIR)/xalan/templates/ElemApplyImport.java \
        $(PKGDIR)/xalan/templates/ElemApplyTemplates.java \
        $(PKGDIR)/xalan/templates/ElemAttributeSet.java \
        $(PKGDIR)/xalan/templates/ElemCallTemplate.java \
        $(PKGDIR)/xalan/templates/ElemChoose.java \
        $(PKGDIR)/xalan/templates/ElemComment.java \
        $(PKGDIR)/xalan/templates/ElemCopy.java \
        $(PKGDIR)/xalan/templates/ElemTextLiteral.java \
        $(PKGDIR)/xalan/templates/ElemCopyOf.java \
        $(PKGDIR)/xalan/templates/ElemElement.java \
        $(PKGDIR)/xalan/templates/ElemEmpty.java \
        $(PKGDIR)/xalan/templates/ElemExsltFuncResult.java \
        $(PKGDIR)/xalan/templates/ElemExsltFunction.java \
        $(PKGDIR)/xalan/templates/ElemExtensionCall.java \
        $(PKGDIR)/xalan/templates/ElemExtensionDecl.java \
        $(PKGDIR)/xalan/templates/ElemExtensionScript.java \
        $(PKGDIR)/xalan/templates/ElemFallback.java \
        $(PKGDIR)/xalan/templates/ElemForEach.java \
        $(PKGDIR)/xalan/templates/ElemMessage.java \
        $(PKGDIR)/xalan/templates/ElemLiteralResult.java \
        $(PKGDIR)/xalan/templates/ElemNumber.java \
        $(PKGDIR)/xalan/templates/ElemOtherwise.java \
        $(PKGDIR)/xalan/templates/ElemPI.java \
        $(PKGDIR)/xalan/templates/ElemParam.java \
        $(PKGDIR)/xalan/templates/ElemSort.java \
        $(PKGDIR)/xalan/templates/ElemTemplate.java \
        $(PKGDIR)/xalan/templates/ElemText.java \
        $(PKGDIR)/xalan/templates/ElemTemplateElement.java \
        $(PKGDIR)/xalan/templates/ElemUnknown.java \
        $(PKGDIR)/xalan/templates/ElemUse.java \
        $(PKGDIR)/xalan/templates/ElemValueOf.java \
        $(PKGDIR)/xalan/templates/ElemVariable.java \
        $(PKGDIR)/xalan/templates/ElemVariablePsuedo.java \
        $(PKGDIR)/xalan/templates/ElemWhen.java \
        $(PKGDIR)/xalan/templates/ElemWithParam.java \
        $(PKGDIR)/xalan/templates/FuncDocument.java \
        $(PKGDIR)/xalan/templates/FuncFormatNumb.java \
        $(PKGDIR)/xalan/templates/FuncKey.java \
        $(PKGDIR)/xalan/templates/KeyDeclaration.java \
        $(PKGDIR)/xalan/templates/NamespaceAlias.java \
        $(PKGDIR)/xalan/templates/OutputProperties.java \
        $(PKGDIR)/xalan/templates/RedundentExprEliminator.java \
        $(PKGDIR)/xalan/templates/Stylesheet.java \
        $(PKGDIR)/xalan/templates/StylesheetComposed.java \
        $(PKGDIR)/xalan/templates/StylesheetRoot.java \
        $(PKGDIR)/xalan/templates/TemplateList.java \
        $(PKGDIR)/xalan/templates/TemplateSubPatternAssociation.java \
        $(PKGDIR)/xalan/templates/VarNameCollector.java \
        $(PKGDIR)/xalan/templates/WhiteSpaceInfo.java \
        $(PKGDIR)/xalan/templates/XMLNSDecl.java \
        $(PKGDIR)/xalan/templates/XSLTVisitable.java \
        $(PKGDIR)/xalan/templates/XSLTVisitor.java \
        $(PKGDIR)/xalan/templates/XUnresolvedVariable.java \
        $(PKGDIR)/xalan/templates/XUnresolvedVariableSimple.java \
        \
        $(PKGDIR)/xalan/trace/PrintTraceListener.java \
        $(PKGDIR)/xalan/trace/EndSelectionEvent.java \
        $(PKGDIR)/xalan/trace/GenerateEvent.java \
        $(PKGDIR)/xalan/trace/SelectionEvent.java \
        $(PKGDIR)/xalan/trace/TraceListener.java \
        $(PKGDIR)/xalan/trace/TraceListenerEx.java \
        $(PKGDIR)/xalan/trace/TraceListenerEx2.java \
        $(PKGDIR)/xalan/trace/TraceManager.java \
        $(PKGDIR)/xalan/trace/TracerEvent.java \
        \
        $(PKGDIR)/xalan/transformer/TransformerHandlerImpl.java \
        $(PKGDIR)/xalan/transformer/ClonerToResultTree.java \
        $(PKGDIR)/xalan/transformer/Counter.java \
        $(PKGDIR)/xalan/transformer/CountersTable.java \
        $(PKGDIR)/xalan/transformer/DecimalToRoman.java \
        $(PKGDIR)/xalan/transformer/KeyIterator.java \
        $(PKGDIR)/xalan/transformer/KeyManager.java \
        $(PKGDIR)/xalan/transformer/KeyRefIterator.java \
        $(PKGDIR)/xalan/transformer/KeyTable.java \
        $(PKGDIR)/xalan/transformer/MsgMgr.java \
        $(PKGDIR)/xalan/transformer/NodeSortKey.java \
        $(PKGDIR)/xalan/transformer/NodeSorter.java \
        $(PKGDIR)/xalan/transformer/NumeratorFormatter.java \
        $(PKGDIR)/xalan/transformer/QueuedEvents.java \
        $(PKGDIR)/xalan/transformer/ResultNameSpace.java \
        $(PKGDIR)/xalan/transformer/ResultTreeHandler.java \
        $(PKGDIR)/xalan/transformer/StackGuard.java \
        $(PKGDIR)/xalan/transformer/SerializerSwitcher.java \
        $(PKGDIR)/xalan/transformer/TrAXFilter.java \
        $(PKGDIR)/xalan/transformer/TransformSnapshot.java \
        $(PKGDIR)/xalan/transformer/TransformSnapshotImpl.java \
        $(PKGDIR)/xalan/transformer/TransformState.java \
        $(PKGDIR)/xalan/transformer/TransformerClient.java \
        $(PKGDIR)/xalan/transformer/TransformerIdentityImpl.java \
        $(PKGDIR)/xalan/transformer/TransformerImpl.java \
        $(PKGDIR)/xalan/transformer/TreeWalker2Result.java \
        $(PKGDIR)/xalan/transformer/XSLInfiniteLoopException.java \
        $(PKGDIR)/xalan/transformer/XalanProperties.java \
        \
        $(PKGDIR)/xalan/xslt/EnvironmentCheck.java \
        $(PKGDIR)/xalan/xslt/Process.java \
# Rule
#
include $(BUILDDIR)/common/Classes.gmk

## Messages are in $(PKGDIR)/xalan/res/XSLTErrorResources.java 

#
# Resources
#
# RESOURCE_FILES = \
#OTHER_PROPERTIES = \
LOCALE_SET_DEFINITION = j2re
NEW_RESOURCE_BUNDLES_PROPERTIES = \
   xalan/res/XSLTInfo.properties \
   xalan/serialize/serializer.properties \
   xalan/serialize/XMLEntities.res \
   xalan/serialize/HTMLEntities.res \
   xalan/serialize/Encodings.properties \
   xalan/templates/output_html.properties \
   xalan/templates/output_text.properties \
   xalan/templates/output_xml.properties 


RESOURCE_SRC_DIR = $(SHARE_SRC)/classes/$(PKGDIR)
RESOURCE_DEST_DIR = $(CLASSDESTDIR)/$(PKGDIR)


## Xalan is using a special ".res" extension for entities.
$(RESOURCE_DEST_DIR)/%.res:        $(RESOURCE_SRC_DIR)/%.res
	$(install-file)

include $(BUILDDIR)/common/Resources.gmk
