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
PRODUCT = xpath
include $(BUILDDIR)/common/Defs.gmk

#
# Take out assertions flag until source code is updated
#
#LANGUAGE_VERSION = -source 1.3

#
# Files to compile
#
FILES_java = \
	$(PKGDIR)/xpath/DOM2Helper.java \
	$(PKGDIR)/xpath/Arg.java \
	$(PKGDIR)/xpath/ExtensionsProvider.java \
	$(PKGDIR)/xpath/CachedXPathAPI.java \
	$(PKGDIR)/xpath/DOMHelper.java \
	$(PKGDIR)/xpath/DOMOrder.java \
	$(PKGDIR)/xpath/Expression.java \
	$(PKGDIR)/xpath/ExpressionNode.java \
	$(PKGDIR)/xpath/ExpressionOwner.java \
	$(PKGDIR)/xpath/FoundIndex.java \
	$(PKGDIR)/xpath/NodeSet.java \
	$(PKGDIR)/xpath/NodeSetDTM.java \
	$(PKGDIR)/xpath/SourceTree.java \
	$(PKGDIR)/xpath/SourceTreeManager.java \
	$(PKGDIR)/xpath/VariableStack.java \
	$(PKGDIR)/xpath/XPathAPI.java \
	$(PKGDIR)/xpath/XPath.java \
	$(PKGDIR)/xpath/XPathProcessorException.java \
	$(PKGDIR)/xpath/WhitespaceStrippingElementMatcher.java \
	$(PKGDIR)/xpath/XPathContext.java \
	$(PKGDIR)/xpath/XPathException.java \
	$(PKGDIR)/xpath/XPathFactory.java \
	\
	$(PKGDIR)/xpath/axes/DescendantIterator.java \
	$(PKGDIR)/xpath/axes/AttributeIterator.java \
	$(PKGDIR)/xpath/axes/AxesWalker.java \
	$(PKGDIR)/xpath/axes/BasicTestIterator.java \
	$(PKGDIR)/xpath/axes/ChildIterator.java \
	$(PKGDIR)/xpath/axes/ChildTestIterator.java \
	$(PKGDIR)/xpath/axes/ContextNodeList.java \
	$(PKGDIR)/xpath/axes/FilterExprIterator.java \
	$(PKGDIR)/xpath/axes/FilterExprIteratorSimple.java \
	$(PKGDIR)/xpath/axes/FilterExprWalker.java \
	$(PKGDIR)/xpath/axes/HasPositionalPredChecker.java \
	$(PKGDIR)/xpath/axes/IteratorPool.java \
	$(PKGDIR)/xpath/axes/LocPathIterator.java \
	$(PKGDIR)/xpath/axes/MatchPatternIterator.java \
	$(PKGDIR)/xpath/axes/NodeSequence.java \
	$(PKGDIR)/xpath/axes/OneStepIterator.java \
	$(PKGDIR)/xpath/axes/OneStepIteratorForward.java \
	$(PKGDIR)/xpath/axes/PathComponent.java \
	$(PKGDIR)/xpath/axes/PredicatedNodeTest.java \
	$(PKGDIR)/xpath/axes/RTFIterator.java \
	$(PKGDIR)/xpath/axes/ReverseAxesWalker.java \
	$(PKGDIR)/xpath/axes/SelfIteratorNoPredicate.java \
	$(PKGDIR)/xpath/axes/SubContextList.java \
	$(PKGDIR)/xpath/axes/UnionChildIterator.java \
	$(PKGDIR)/xpath/axes/UnionPathIterator.java \
	$(PKGDIR)/xpath/axes/WalkerFactory.java \
	$(PKGDIR)/xpath/axes/WalkingIterator.java \
	$(PKGDIR)/xpath/axes/WalkingIteratorSorted.java \
	\
	$(PKGDIR)/xpath/compiler/FuncLoader.java \
	$(PKGDIR)/xpath/compiler/Compiler.java \
	$(PKGDIR)/xpath/compiler/FunctionTable.java \
	$(PKGDIR)/xpath/compiler/Keywords.java \
	$(PKGDIR)/xpath/compiler/Lexer.java \
	$(PKGDIR)/xpath/compiler/OpCodes.java \
	$(PKGDIR)/xpath/compiler/OpMap.java \
	$(PKGDIR)/xpath/compiler/PsuedoNames.java \
	$(PKGDIR)/xpath/compiler/XPathDumper.java \
	$(PKGDIR)/xpath/compiler/XPathParser.java \
	$(PKGDIR)/xpath/compiler/OpMapVector.java \
	\
	$(PKGDIR)/xpath/XPathVisitable.java \
	$(PKGDIR)/xpath/XPathVisitor.java \
	\
	$(PKGDIR)/xpath/functions/FuncDoclocation.java \
	$(PKGDIR)/xpath/functions/FuncBoolean.java \
	$(PKGDIR)/xpath/functions/FuncCeiling.java \
	$(PKGDIR)/xpath/functions/FuncConcat.java \
	$(PKGDIR)/xpath/functions/FuncContains.java \
	$(PKGDIR)/xpath/functions/FuncCount.java \
	$(PKGDIR)/xpath/functions/FuncCurrent.java \
	$(PKGDIR)/xpath/functions/FuncLocalPart.java \
	$(PKGDIR)/xpath/functions/FuncExtElementAvailable.java \
	$(PKGDIR)/xpath/functions/FuncExtFunction.java \
	$(PKGDIR)/xpath/functions/FuncExtFunctionAvailable.java \
	$(PKGDIR)/xpath/functions/FuncFalse.java \
	$(PKGDIR)/xpath/functions/FuncFloor.java \
	$(PKGDIR)/xpath/functions/FuncGenerateId.java \
	$(PKGDIR)/xpath/functions/FuncId.java \
	$(PKGDIR)/xpath/functions/FuncLang.java \
	$(PKGDIR)/xpath/functions/FuncLast.java \
	$(PKGDIR)/xpath/functions/FuncNormalizeSpace.java \
	$(PKGDIR)/xpath/functions/FuncNamespace.java \
	$(PKGDIR)/xpath/functions/FuncNumber.java \
	$(PKGDIR)/xpath/functions/FuncNot.java \
	$(PKGDIR)/xpath/functions/FuncSubstringAfter.java \
	$(PKGDIR)/xpath/functions/FuncPosition.java \
	$(PKGDIR)/xpath/functions/FuncQname.java \
	$(PKGDIR)/xpath/functions/FuncRound.java \
	$(PKGDIR)/xpath/functions/FuncStartsWith.java \
	$(PKGDIR)/xpath/functions/FuncString.java \
	$(PKGDIR)/xpath/functions/FuncStringLength.java \
	$(PKGDIR)/xpath/functions/FuncSubstring.java \
	$(PKGDIR)/xpath/functions/FuncSubstringBefore.java \
	$(PKGDIR)/xpath/functions/FuncSum.java \
	$(PKGDIR)/xpath/functions/FuncSystemProperty.java \
	$(PKGDIR)/xpath/functions/FuncTranslate.java \
	$(PKGDIR)/xpath/functions/FuncTrue.java \
	$(PKGDIR)/xpath/functions/WrongNumberArgsException.java \
	$(PKGDIR)/xpath/functions/FuncUnparsedEntityURI.java \
	$(PKGDIR)/xpath/functions/Function.java \
	$(PKGDIR)/xpath/functions/Function2Args.java \
	$(PKGDIR)/xpath/functions/Function3Args.java \
	$(PKGDIR)/xpath/functions/FunctionDef1Arg.java \
	$(PKGDIR)/xpath/functions/FunctionMultiArgs.java \
	$(PKGDIR)/xpath/functions/FunctionOneArg.java \
	\
	$(PKGDIR)/xpath/objects/XBooleanStatic.java \
	$(PKGDIR)/xpath/objects/XBoolean.java \
	$(PKGDIR)/xpath/objects/XRTreeFragSelectWrapper.java \
	$(PKGDIR)/xpath/objects/XMLStringFactoryImpl.java \
	$(PKGDIR)/xpath/objects/XNodeSet.java \
	$(PKGDIR)/xpath/objects/XNodeSetForDOM.java \
	$(PKGDIR)/xpath/objects/XNull.java \
	$(PKGDIR)/xpath/objects/XNumber.java \
	$(PKGDIR)/xpath/objects/XObject.java \
	$(PKGDIR)/xpath/objects/XObjectFactory.java \
	$(PKGDIR)/xpath/objects/XRTreeFrag.java \
	$(PKGDIR)/xpath/objects/XStringForFSB.java \
	$(PKGDIR)/xpath/objects/XString.java \
	$(PKGDIR)/xpath/objects/XStringForChars.java \
	\
	$(PKGDIR)/xpath/operations/Equals.java \
	$(PKGDIR)/xpath/operations/And.java \
	$(PKGDIR)/xpath/operations/Bool.java \
	$(PKGDIR)/xpath/operations/Div.java \
	$(PKGDIR)/xpath/operations/UnaryOperation.java \
	$(PKGDIR)/xpath/operations/Gt.java \
	$(PKGDIR)/xpath/operations/Gte.java \
	$(PKGDIR)/xpath/operations/Lt.java \
	$(PKGDIR)/xpath/operations/Lte.java \
	$(PKGDIR)/xpath/operations/Minus.java \
	$(PKGDIR)/xpath/operations/Mod.java \
	$(PKGDIR)/xpath/operations/Mult.java \
	$(PKGDIR)/xpath/operations/Neg.java \
	$(PKGDIR)/xpath/operations/NotEquals.java \
	$(PKGDIR)/xpath/operations/Number.java \
	$(PKGDIR)/xpath/operations/Operation.java \
	$(PKGDIR)/xpath/operations/Or.java \
	$(PKGDIR)/xpath/operations/Plus.java \
	$(PKGDIR)/xpath/operations/Quo.java \
	$(PKGDIR)/xpath/operations/String.java \
	$(PKGDIR)/xpath/operations/Variable.java \
	$(PKGDIR)/xpath/operations/VariableSafeAbsRef.java \
	\
	$(PKGDIR)/xpath/patterns/ContextMatchStepPattern.java \
	$(PKGDIR)/xpath/patterns/FunctionPattern.java \
	$(PKGDIR)/xpath/patterns/NodeTest.java \
	$(PKGDIR)/xpath/patterns/NodeTestFilter.java \
	$(PKGDIR)/xpath/patterns/StepPattern.java \
	$(PKGDIR)/xpath/patterns/UnionPattern.java \
	\
	$(PKGDIR)/xpath/res/XPATHErrorResources.java \
	$(PKGDIR)/xpath/res/XPATHErrorResources_en.java \
	$(PKGDIR)/xpath/res/XPATHErrorResources_de.java \
	$(PKGDIR)/xpath/res/XPATHErrorResources_es.java \
	$(PKGDIR)/xpath/res/XPATHErrorResources_fr.java \
	$(PKGDIR)/xpath/res/XPATHErrorResources_it.java \
	$(PKGDIR)/xpath/res/XPATHErrorResources_ja.java \
	$(PKGDIR)/xpath/res/XPATHErrorResources_ko.java \
	$(PKGDIR)/xpath/res/XPATHErrorResources_sv.java \
	$(PKGDIR)/xpath/res/XPATHErrorResources_zh_CN.java \
	$(PKGDIR)/xpath/res/XPATHErrorResources_zh_TW.java \
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
NEW_RESOURCE_BUNDLES_PROPERTIES = \
#   xalan/res/XSLTInfo.properties \
#   xalan/serialize/serializer.properties \
#   xalan/serialize/XMLEntities.res \
#   xalan/serialize/HTMLEntities.res \
#   xalan/templates/output_html.properties \
#   xalan/templates/output_text.properties \
#   xalan/templates/output_xml.properties \
	xpath/res/XPATHErrorResources.properties

RESOURCE_SRC_DIR = $(SHARE_SRC)/classes/$(PKGDIR)
RESOURCE_DEST_DIR = $(CLASSDESTDIR)/$(PKGDIR)


## Xalan is using a special ".res" extension for entities.
$(RESOURCE_DEST_DIR)/%.res:	$(RESOURCE_SRC_DIR)/%.res
	$(install-file)

include $(BUILDDIR)/common/Resources.gmk
