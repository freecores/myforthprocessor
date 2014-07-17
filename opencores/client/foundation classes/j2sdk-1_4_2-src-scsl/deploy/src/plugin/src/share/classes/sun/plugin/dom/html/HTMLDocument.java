/*
 * @(#)HTMLDocument.java	1.12 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import org.w3c.dom.*;
import org.w3c.dom.html.*;
import org.w3c.dom.stylesheets.*;
import org.w3c.dom.css.*;
import sun.plugin.dom.*;
import sun.plugin.dom.core.*;
import sun.plugin.dom.views.*;
import sun.plugin.dom.exception.NotSupportedException;

/**
 *  An <code>HTMLDocument</code> is the root of the HTML hierarchy and holds 
 * the entire content. Besides providing access to the hierarchy, it also 
 * provides some convenience methods for accessing certain sets of 
 * information from the document.
 * <p> The following properties have been deprecated in favor of the 
 * corresponding ones for the <code>BODY</code> element: alinkColor background
 *  bgColor fgColor linkColor vlinkColor In DOM Level 2, the method 
 * <code>getElementById</code> is inherited from the <code>Document</code> 
 * interface where it was moved.
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public class HTMLDocument extends sun.plugin.dom.core.Document 
			  implements org.w3c.dom.html.HTMLDocument,
				     org.w3c.dom.views.DocumentView,
				     org.w3c.dom.stylesheets.DocumentStyle,
				     org.w3c.dom.css.DocumentCSS
{
    /**
     * Construct a HTMLDocument object.
     */
    public HTMLDocument(DOMObject obj)
    {
	super(DOMObjectType.HTMLDocument, obj);
    }


    /**
     *  The title of a document as specified by the <code>TITLE</code> element 
     * in the head of the document. 
     */
    public String getTitle()
    {
	return getAttribute("title");
    }

    public void setTitle(String title)
    {
	setAttribute("title", title);
    }

    /**
     *  Returns the URI  of the page that linked to this page. The value is an 
     * empty string if the user navigated to the page directly (not through a 
     * link, but, for example, via a bookmark). 
     */
    public String getReferrer()
    {
	return getAttribute("referrer");
    }

    /**
     *  The domain name of the server that served the document, or 
     * <code>null</code> if the server cannot be identified by a domain name. 
     */
    public String getDomain()
    {
	return getAttribute("domain");
    }

    /**
     *  The complete URI  of the document. 
     */
    public String getURL()
    {
	return getAttribute("URL");
    }

    /**
     *  The element that contains the content for the document. In documents 
     * with <code>BODY</code> contents, returns the <code>BODY</code> 
     * element. In frameset documents, this returns the outermost
     * <code>FRAMESET</code> element. 
     */
    public org.w3c.dom.html.HTMLElement getBody()
    {
	return null;
    }

    public void setBody(org.w3c.dom.html.HTMLElement body)
    {	
	//
    }

    /**
     *  A collection of all the <code>IMG</code> elements in a document. The 
     * behavior is limited to <code>IMG</code> elements for backwards 
     * compatibility. 
     */
    public org.w3c.dom.html.HTMLCollection getImages()
    {
	Object result = obj.getMember("images");

	if (result != null && result instanceof DOMObject)
	{
	    return new sun.plugin.dom.html.HTMLCollection(DOMObjectType.HTMLImageElement, (DOMObject) result, this);
	}

	return null;
    }

    /**
     *  A collection of all the <code>OBJECT</code> elements that include 
     * applets and <code>APPLET</code> ( deprecated ) elements in a document. 
     */
    public org.w3c.dom.html.HTMLCollection getApplets()
    {
	// We don't support it for now
/*
	Object result = obj.getMember("applets");

	if (result != null && result instanceof DOMObject)
	{
	    return new sun.plugin.dom.html.HTMLCollection(DOMObjectType.HTMLObjectElement, (DOMObject) result, this);
	}
*/
	return null;
    }

    /**
     *  A collection of all <code>AREA</code> elements and anchor (
     * <code>A</code> ) elements in a document with a value for the 
     * <code>href</code> attribute. 
     */
    public org.w3c.dom.html.HTMLCollection getLinks()
    {
	Object result = obj.getMember("links");

	if (result != null && result instanceof DOMObject)
	{
	    return new sun.plugin.dom.html.HTMLCollection(DOMObjectType.HTMLLinkElement, (DOMObject) result, this);
	}

	return null;
    }

    /**
     *  A collection of all the forms of a document. 
     */
    public org.w3c.dom.html.HTMLCollection getForms()
    {
	Object result = obj.getMember("forms");

	if (result != null && result instanceof DOMObject)
	{
	    return new sun.plugin.dom.html.HTMLCollection(DOMObjectType.HTMLFormElement, (DOMObject) result, this);
	}

	return null;
    }

    /**
     *  A collection of all the anchor (<code>A</code> ) elements in a document
     *  with a value for the <code>name</code> attribute. Note. For reasons 
     * of backwards compatibility, the returned set of anchors only contains 
     * those anchors created with the <code>name</code>  attribute, not those 
     * created with the <code>id</code> attribute. 
     */
    public org.w3c.dom.html.HTMLCollection getAnchors()
    {
	Object result = obj.getMember("anchors");

	if (result != null && result instanceof DOMObject)
	{
	    return new sun.plugin.dom.html.HTMLCollection(DOMObjectType.HTMLAnchorElement, (DOMObject) result, this);
	}

	return null;
    }

    /**
     *  The cookies associated with this document. If there are none, the 
     * value is an empty string. Otherwise, the value is a string: a 
     * semicolon-delimited list of "name, value" pairs for all the cookies 
     * associated with the page. For example, 
     * <code>name=value;expires=date</code> . 
     */
    public String getCookie()
    {
	return getAttribute("cookie");
    }

    public void setCookie(String cookie)
    {
	setAttribute("cookie", cookie);
    }

    /**
     *  Note. This method and the ones following  allow a user to add to or 
     * replace the structure model of a document using strings of unparsed 
     * HTML. At the time of  writing alternate methods for providing similar 
     * functionality for  both HTML and XML documents were being considered. 
     * The following methods may be deprecated at some point in the future in 
     * favor of a more general-purpose mechanism.
     * <br> Open a document stream for writing. If a document exists in the 
     * target, this method clears it.
     */
    public void open()
    {
        throw new NotSupportedException("HTMLDocument.open() is not supported");
    }

    /**
     *  Closes a document stream opened by <code>open()</code> and forces 
     * rendering.
     */
    public void close()
    {
        throw new NotSupportedException("HTMLDocument.close() is not supported");
    }

    /**
     *  Write a string of text to a document stream opened by
     * <code>open()</code> . The text is parsed into the document's structure 
     * model.
     * @param text  The string to be parsed into some structure in the 
     *   document structure model.
     */
    public void write(String text)
    {
        throw new NotSupportedException("HTMLDocument.write() is not supported");
    }

    /**
     *  Write a string of text followed by a newline character to a document 
     * stream opened by <code>open()</code> . The text is parsed into the 
     * document's structure model.
     * @param text  The string to be parsed into some structure in the 
     *   document structure model.
     */
    public void writeln(String text)
    {
        throw new NotSupportedException("HTMLDocument.writeln() is not supported");
    }


    /**
     *  Returns the (possibly empty) collection of elements whose
     * <code>name</code> value is given by <code>elementName</code> .
     * @param elementName  The <code>name</code> attribute value for an 
     *   element.
     * @return  The matching elements.
     */
    public org.w3c.dom.NodeList getElementsByName(String elementName)
    {
	return null;
    }


    /**
     *  Returns a <code>NodeList</code> of all the <code>Elements</code> with 
     * a given tag name in the order in which they are encountered in a 
     * preorder traversal of the <code>Document</code> tree. 
     * @param tagname  The name of the tag to match on. The special value "*" 
     *   matches all tags.
     * @return  A new <code>NodeList</code> object containing all the matched 
     *   <code>Elements</code> .
     */
    public org.w3c.dom.NodeList getElementsByTagName(String tagname)
    {
	return null;
    }


    /**
     *  This is a convenience attribute that allows direct access to the child 
     * node that is the root element of  the document. For HTML documents, 
     * this is the element with the tagName "HTML".
     */
    public org.w3c.dom.Element getDocumentElement()
    {
	Object result = obj.getMember("documentElement");

	if (result != null && result instanceof DOMObject)
	{
    	    return new sun.plugin.dom.html.HTMLHtmlElement((DOMObject) result, this);
	}

	return null;
    }

    /**
     *  Creates an element of the type specified. Note that the instance 
     * returned implements the <code>Element</code> interface, so attributes 
     * can be specified directly  on the returned object.
     * <br> In addition, if there are known attributes with default values, 
     * <code>Attr</code> nodes representing them are automatically created and
     *  attached to the element.
     * <br> To create an element with a qualified name and namespace URI, use 
     * the <code>createElementNS</code> method.
     * @param tagName  The name of the element type to instantiate. For XML, 
     *   this is case-sensitive. For HTML, the  <code>tagName</code> 
     *   parameter may be provided in any case,  but it must be mapped to the 
     *   canonical uppercase form by  the DOM implementation. 
     * @return  A new <code>Element</code> object with the 
     *   <code>nodeName</code> attribute set to <code>tagName</code> , and 
     *   <code>localName</code> , <code>prefix</code> , and 
     *   <code>namespaceURI</code> set to <code>null</code> .
     * @exception DOMException
     *    INVALID_CHARACTER_ERR: Raised if the specified name contains an 
     *   illegal character.
     */
    public org.w3c.dom.Element createElement(String tagName)
					    throws DOMException
    {
	return null;
    }

    /**
     *  Returns the <code>Element</code> whose <code>ID</code> is given by 
     * <code>elementId</code> . If no such element exists, returns 
     * <code>null</code> . Behavior is not defined if more than one element 
     * has this <code>ID</code> .  The DOM implementation must have 
     * information that says which attributes are of type ID. Attributes with 
     * the name "ID" are not of type ID unless so defined. Implementations 
     * that do not know whether attributes are of type ID or not are expected 
     * to return <code>null</code> .
     * @param elementId  The unique <code>id</code> value for an element.
     * @return  The matching element.
     * @since DOM Level 2
     */
    public org.w3c.dom.Element getElementById(String elementId)
    {
	return null;
    }	
  

    private String getAttribute(String name)
    {
	return DOMObjectHelper.getStringMemberNoEx(obj, name);
    }


    private void setAttribute(String name, String value)
    {
	DOMObjectHelper.setStringMemberNoEx(obj, name, value);
    }


    //------------------------------------------------------------
    // Method from org.w3c.dom.views.DocumentView
    //------------------------------------------------------------

    /**
     *  The default <code>AbstractView</code> for this <code>Document</code> , 
     * or <code>null</code> if none available.
     */
    public org.w3c.dom.views.AbstractView getDefaultView()
    {
	return new sun.plugin.dom.css.ViewCSS(this);
    }


    //------------------------------------------------------------
    // Method from org.w3c.dom.stylesheets.DocumentStyle
    //------------------------------------------------------------

    /**
     *  A list containing all the style sheets explicitly linked into or 
     * embedded in a document. For HTML documents, this includes external 
     * style sheets, included via the HTML  LINK element, and inline  STYLE 
     * elements. In XML, this includes external style sheets, included via 
     * style sheet processing instructions (see ). 
     */
    public org.w3c.dom.stylesheets.StyleSheetList getStyleSheets()
    {
	try
	{
	    Object result = obj.getMember("stylesheets");

	    if (result != null && result instanceof DOMObject)
	    {
    		return new sun.plugin.dom.stylesheets.StyleSheetList(DOMObjectType.CSSStyleSheet,
								    (DOMObject) result, this);
	    }
	}
	catch (DOMException e)
	{
	}

	return null;
    }


    //------------------------------------------------------------
    // Method from org.w3c.dom.css.DocumentCSS
    //------------------------------------------------------------

    /**
     *  This method is used to retrieve the override style declaration for a 
     * specified element and a specified pseudo-element. 
     * @param elt The element whose style is to be modified. This parameter 
     *   cannot be null. 
     * @param pseudoElt The pseudo-element or <code>null</code> if none. 
     * @return  The override style declaration. 
     */
    public org.w3c.dom.css.CSSStyleDeclaration getOverrideStyle(org.w3c.dom.Element elt, 
                                                String pseudoElt)
    {
	if (elt instanceof org.w3c.dom.css.ElementCSSInlineStyle)
	{
	    org.w3c.dom.css.ElementCSSInlineStyle element = (org.w3c.dom.css.ElementCSSInlineStyle) elt;

	    return element.getStyle();
	}
	else
	{
	    return null;
	}
    }
}

