/*
 * @(#)HTMLElement.java	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import org.w3c.dom.*;
import org.w3c.dom.html.*;
import org.w3c.dom.css.*;
import sun.plugin.dom.*;
import sun.plugin.dom.core.*;

/**
 *  A class that encapsulates any HTML element.
 */
public class HTMLElement extends sun.plugin.dom.core.Element 
			 implements org.w3c.dom.html.HTMLElement,
				    org.w3c.dom.css.ElementCSSInlineStyle
{
    /**
     * Construct a HTMLElement object.
     */
    public HTMLElement(DOMObject obj, org.w3c.dom.html.HTMLDocument doc)
    { 
	super(DOMObjectType.HTMLElement, obj, doc, null);
    }

    /**
     * Construct a HTMLElement object.
     */
    protected HTMLElement(int type, DOMObject obj, org.w3c.dom.html.HTMLDocument doc)
    {
	super(type, obj, doc, null);
    }
    
    /**
     * Construct a HTMLElement object.
     */
    protected HTMLElement(int type, DOMObject obj, 
			  org.w3c.dom.html.HTMLDocument doc,
			  org.w3c.dom.Element owner)
    {
	super(type, obj, doc, owner);
    }

    /**
     *  The element's identifier. See the  id attribute definition in HTML 4.0.
     */
    public String getId()
    {
	return getAttribute("id");	
    }

    public void setId(String id)
    {
	setAttribute("id", id);
    }

    /**
     *  The element's advisory title. See the  title attribute definition in 
     * HTML 4.0.
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
     *  Language code defined in RFC 1766. See the  lang attribute definition 
     * in HTML 4.0.
     */
    public String getLang()
    {
	return getAttribute("lang");
    }

    public void setLang(String lang)
    {
	setAttribute("lang", lang);
    }

    /**
     *  Specifies the base direction of directionally neutral text and the 
     * directionality of tables. See the  dir attribute definition in HTML 
     * 4.0.
     */
    public String getDir()
    {
	return getAttribute("dir");
    }

    public void setDir(String dir)
    {
	setAttribute("dir", dir);
    }

    /**
     *  The class attribute of the element. This attribute has been renamed 
     * due to conflicts with the "class" keyword exposed by many languages. 
     * See the  class attribute definition in HTML 4.0.
     */
    public String getClassName()
    {
	return getAttribute("className");
    }

    public void setClassName(String className)
    {
	setAttribute("className", className);
    }


    //------------------------------------------------------------
    // Method from org.w3c.dom.css.CSSStyleDeclaration
    //------------------------------------------------------------

    /**
     *  The style attribute. 
     */
    public org.w3c.dom.css.CSSStyleDeclaration getStyle()
    {
	try
	{
	    Object result = obj.getMember("style");

	    if (result != null && result instanceof DOMObject)
	    {
		return new sun.plugin.dom.css.CSSStyleDeclaration((DOMObject) result, null, null);
	    }
	}
	catch (DOMException e)
	{
	}

	return null;
    }
}

