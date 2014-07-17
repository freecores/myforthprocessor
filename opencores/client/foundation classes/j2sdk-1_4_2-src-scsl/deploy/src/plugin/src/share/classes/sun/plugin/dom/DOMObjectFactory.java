/*
 * @(#)DOMObjectFactory.java	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom;

import org.w3c.dom.*;
import org.w3c.dom.html.*;
import sun.plugin.dom.exception.*;

/**
 * A class the creates various types of DOM objects.
 */
public class DOMObjectFactory
{
    /**
     * Create a new HTMLElement by wrapping the object.
     */
    public static org.w3c.dom.html.HTMLElement createHTMLElement(int type, 
								 Object obj, 
								 org.w3c.dom.html.HTMLDocument doc,
								 org.w3c.dom.Element owner)
    {
	// Error checking
	if (obj == null)
	    return null;

	if (obj instanceof DOMObject)
	{
	    switch (type)
	    {
		case DOMObjectType.HTMLAnchorElement:
		{
		    return new sun.plugin.dom.html.HTMLAnchorElement((DOMObject) obj, doc);
		}

		case DOMObjectType.HTMLObjectElement:
		{
		    return new sun.plugin.dom.html.HTMLObjectElement((DOMObject) obj, doc);
		}

		case DOMObjectType.HTMLImageElement:
		{
		    return new sun.plugin.dom.html.HTMLImageElement((DOMObject) obj, doc);
		}

		case DOMObjectType.HTMLLinkElement:
		{
		    return new sun.plugin.dom.html.HTMLLinkStyleElement((DOMObject) obj, doc);
		}

		case DOMObjectType.HTMLFormElement:
		{
		    return new sun.plugin.dom.html.HTMLFormElement((DOMObject) obj, doc);
		}

		case DOMObjectType.HTMLElement:
		{
		    // Try to see if we can convert it to proper element
		    try
		    {	
			DOMObject domObject = (DOMObject) obj;
			Object result = domObject.getMember("tagName");

			if (result != null)
			{
			    if ("INPUT".equalsIgnoreCase(result.toString()))
				return new sun.plugin.dom.html.HTMLInputElement((DOMObject) obj, doc, (org.w3c.dom.html.HTMLFormElement) owner);
			    else if ("SELECT".equalsIgnoreCase(result.toString()))
				return new sun.plugin.dom.html.HTMLSelectElement((DOMObject) obj, doc, (org.w3c.dom.html.HTMLFormElement) owner);
			    else if ("TEXTAREA".equalsIgnoreCase(result.toString()))
				return new sun.plugin.dom.html.HTMLTextAreaElement((DOMObject) obj, doc, (org.w3c.dom.html.HTMLFormElement) owner);
			}
		    }
		    catch (Throwable e)
		    {
		    }

		    return new sun.plugin.dom.html.HTMLElement((DOMObject) obj, doc);
		}
	    }
	}

	// Wrong type, cannot wrap - throw exception
	throw new NotSupportedException("DOMObjectFactory::createHTMLElement() cannot wrap " + obj);
    } 

    /**
     * Create a new StyleSheet by wrapping the object.
     */
    public static org.w3c.dom.stylesheets.StyleSheet createStyleSheet(int type, 
							       Object obj, 
							       org.w3c.dom.Node owner)
    {
	// Error checking
	if (obj == null)
	    return null;

	if (obj instanceof DOMObject)
	{
	    switch (type)
	    {
		case DOMObjectType.StyleSheet:
		{
		    return new sun.plugin.dom.stylesheets.StyleSheet((DOMObject) obj, owner);
		}

		case DOMObjectType.CSSStyleSheet:
		{
		    return new sun.plugin.dom.css.CSSStyleSheet((DOMObject) obj, owner);
		}
	    }
	}

	// Wrong type, cannot wrap - throw exception
	throw new NotSupportedException("DOMObjectFactory::createStyleSheet() cannot wrap " + obj);
    } 
}

