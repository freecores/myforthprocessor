#
# @(#)Makefile	1.10 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.10 03/01/23
#
# Makefile for building W3C DOM API classes
#

BUILDDIR = ../../..
PACKAGE = org.w3c.dom
PRODUCT = xml
include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile
#
FILES_java = \
	$(PKGDIR)/Attr.java \
	$(PKGDIR)/CDATASection.java \
	$(PKGDIR)/CharacterData.java \
	$(PKGDIR)/Comment.java \
	$(PKGDIR)/DOMException.java \
	$(PKGDIR)/DOMImplementation.java \
	$(PKGDIR)/Document.java \
	$(PKGDIR)/DocumentFragment.java \
	$(PKGDIR)/DocumentType.java \
	$(PKGDIR)/Element.java \
	$(PKGDIR)/Entity.java \
	$(PKGDIR)/EntityReference.java \
	$(PKGDIR)/NamedNodeMap.java \
	$(PKGDIR)/Node.java \
	$(PKGDIR)/NodeList.java \
	$(PKGDIR)/Notation.java \
	$(PKGDIR)/ProcessingInstruction.java \
	$(PKGDIR)/Text.java \
	\
	$(PKGDIR)/css/Counter.java \
	$(PKGDIR)/css/CSS2Properties.java \
	$(PKGDIR)/css/CSSCharsetRule.java \
	$(PKGDIR)/css/CSSFontFaceRule.java \
	$(PKGDIR)/css/CSSImportRule.java \
	$(PKGDIR)/css/CSSMediaRule.java \
	$(PKGDIR)/css/CSSPageRule.java \
	$(PKGDIR)/css/CSSPrimitiveValue.java \
	$(PKGDIR)/css/CSSRule.java \
	$(PKGDIR)/css/CSSRuleList.java \
	$(PKGDIR)/css/CSSStyleDeclaration.java \
	$(PKGDIR)/css/CSSStyleRule.java \
	$(PKGDIR)/css/CSSStyleSheet.java \
	$(PKGDIR)/css/CSSUnknownRule.java \
	$(PKGDIR)/css/CSSValue.java \
	$(PKGDIR)/css/CSSValueList.java \
	$(PKGDIR)/css/DocumentCSS.java \
	$(PKGDIR)/css/DOMImplementationCSS.java \
	$(PKGDIR)/css/ElementCSSInlineStyle.java \
	$(PKGDIR)/css/Rect.java \
	$(PKGDIR)/css/RGBColor.java \
	$(PKGDIR)/css/ViewCSS.java \
	\
	$(PKGDIR)/events/DocumentEvent.java \
	$(PKGDIR)/events/Event.java \
	$(PKGDIR)/events/EventException.java \
	$(PKGDIR)/events/EventListener.java \
	$(PKGDIR)/events/EventTarget.java \
	$(PKGDIR)/events/MouseEvent.java \
	$(PKGDIR)/events/MutationEvent.java \
	$(PKGDIR)/events/UIEvent.java \
	\
	$(PKGDIR)/html/HTMLAnchorElement.java \
	$(PKGDIR)/html/HTMLAppletElement.java \
	$(PKGDIR)/html/HTMLAreaElement.java \
	$(PKGDIR)/html/HTMLBRElement.java \
	$(PKGDIR)/html/HTMLBaseElement.java \
	$(PKGDIR)/html/HTMLBaseFontElement.java \
	$(PKGDIR)/html/HTMLBodyElement.java \
	$(PKGDIR)/html/HTMLButtonElement.java \
	$(PKGDIR)/html/HTMLCollection.java \
	$(PKGDIR)/html/HTMLDListElement.java \
	$(PKGDIR)/html/HTMLDOMImplementation.java \
	$(PKGDIR)/html/HTMLDirectoryElement.java \
	$(PKGDIR)/html/HTMLDivElement.java \
	$(PKGDIR)/html/HTMLDocument.java \
	$(PKGDIR)/html/HTMLElement.java \
	$(PKGDIR)/html/HTMLFieldSetElement.java \
	$(PKGDIR)/html/HTMLFontElement.java \
	$(PKGDIR)/html/HTMLFormElement.java \
	$(PKGDIR)/html/HTMLFrameElement.java \
	$(PKGDIR)/html/HTMLFrameSetElement.java \
	$(PKGDIR)/html/HTMLHRElement.java \
	$(PKGDIR)/html/HTMLHeadElement.java \
	$(PKGDIR)/html/HTMLHeadingElement.java \
	$(PKGDIR)/html/HTMLHtmlElement.java \
	$(PKGDIR)/html/HTMLIFrameElement.java \
	$(PKGDIR)/html/HTMLImageElement.java \
	$(PKGDIR)/html/HTMLInputElement.java \
	$(PKGDIR)/html/HTMLIsIndexElement.java \
	$(PKGDIR)/html/HTMLLIElement.java \
	$(PKGDIR)/html/HTMLLabelElement.java \
	$(PKGDIR)/html/HTMLLegendElement.java \
	$(PKGDIR)/html/HTMLLinkElement.java \
	$(PKGDIR)/html/HTMLMapElement.java \
	$(PKGDIR)/html/HTMLMenuElement.java \
	$(PKGDIR)/html/HTMLMetaElement.java \
	$(PKGDIR)/html/HTMLModElement.java \
	$(PKGDIR)/html/HTMLOListElement.java \
	$(PKGDIR)/html/HTMLObjectElement.java \
	$(PKGDIR)/html/HTMLOptGroupElement.java \
	$(PKGDIR)/html/HTMLOptionElement.java \
	$(PKGDIR)/html/HTMLParagraphElement.java \
	$(PKGDIR)/html/HTMLParamElement.java \
	$(PKGDIR)/html/HTMLPreElement.java \
	$(PKGDIR)/html/HTMLQuoteElement.java \
	$(PKGDIR)/html/HTMLScriptElement.java \
	$(PKGDIR)/html/HTMLSelectElement.java \
	$(PKGDIR)/html/HTMLStyleElement.java \
	$(PKGDIR)/html/HTMLTableCaptionElement.java \
	$(PKGDIR)/html/HTMLTableCellElement.java \
	$(PKGDIR)/html/HTMLTableColElement.java \
	$(PKGDIR)/html/HTMLTableElement.java \
	$(PKGDIR)/html/HTMLTableRowElement.java \
	$(PKGDIR)/html/HTMLTableSectionElement.java \
	$(PKGDIR)/html/HTMLTextAreaElement.java \
	$(PKGDIR)/html/HTMLTitleElement.java \
	$(PKGDIR)/html/HTMLUListElement.java \
	\
	$(PKGDIR)/stylesheets/DocumentStyle.java \
	$(PKGDIR)/stylesheets/LinkStyle.java \
	$(PKGDIR)/stylesheets/MediaList.java \
	$(PKGDIR)/stylesheets/StyleSheet.java \
	$(PKGDIR)/stylesheets/StyleSheetList.java \
	\
        $(PKGDIR)/traversal/DocumentTraversal.java \
        $(PKGDIR)/traversal/NodeFilter.java \
        $(PKGDIR)/traversal/NodeIterator.java \
        $(PKGDIR)/traversal/TreeWalker.java \
	\
	$(PKGDIR)/views/AbstractView.java \
	$(PKGDIR)/views/DocumentView.java 

#
# Rules
#
include $(BUILDDIR)/common/Classes.gmk
