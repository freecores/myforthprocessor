/*
 * @(#)HTMLLinkStyleElement.java	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import org.w3c.dom.*;
import org.w3c.dom.html.*;
import sun.plugin.dom.*;
import sun.plugin.dom.core.*;

 
/**
 *  The <code>LINK</code> element specifies a link to an external resource, 
 * and defines this document's relationship to that resource (or vice versa). 
 *  See the  LINK element definition in HTML 4.0  (see also the 
 * <code>LinkStyle</code> interface in the  module).
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
/**
 *  Style information. See the  STYLE element definition in HTML 4.0, the  
 * module and the <code>LinkStyle</code> interface in the  module. 
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public class HTMLLinkStyleElement extends sun.plugin.dom.html.HTMLElement 
			          implements org.w3c.dom.html.HTMLLinkElement, 
					     org.w3c.dom.html.HTMLStyleElement,
					     org.w3c.dom.stylesheets.LinkStyle
{
    /**
     * Construct a HTMLObjectElement object.
     */
    public HTMLLinkStyleElement(DOMObject obj, org.w3c.dom.html.HTMLDocument doc)
    {
	super(DOMObjectType.HTMLLinkElement, obj, doc);
    }

    /**
     *  Enables/disables the link. This is currently only used for style sheet 
     * links, and may be used to activate or deactivate style sheets. 
     */
    public boolean getDisabled()
    {
	return DOMObjectHelper.getBooleanMemberNoEx(obj, "disabled");
    }

    public void setDisabled(boolean disabled)
    {
	DOMObjectHelper.setBooleanMemberNoEx(obj, "disabled", disabled);
    }

    /**
     *  The character encoding of the resource being linked to. See the  
     * charset attribute definition in HTML 4.0.
     */
    public String getCharset()
    {
	return getAttribute("charset");
    }

    public void setCharset(String charset)
    {
	setAttribute("charset", charset);
    }

    /**
     *  The URI of the linked resource. See the  href attribute definition in 
     * HTML 4.0.
     */
    public String getHref()
    {
	return getAttribute("href");
    }

    public void setHref(String href)
    {
	setAttribute("href", href);
    }

    /**
     *  Language code of the linked resource. See the  hreflang attribute 
     * definition in HTML 4.0.
     */
    public String getHreflang()
    {
	return getAttribute("hreflang");
    }

    public void setHreflang(String hreflang)
    {
	setAttribute("hreflang", hreflang);
    }


    /**
     *  Designed for use with one or more target media. See the  media 
     * attribute definition in HTML 4.0.
     */
    public String getMedia()
    {
	return getAttribute("media");
    }

    public void setMedia(String media)
    {
	setAttribute("media", media);
    }

    /**
     *  Forward link type. See the  rel attribute definition in HTML 4.0.
     */
    public String getRel()
    {
	return getAttribute("rel");
    }

    public void setRel(String rel)
    {
	setAttribute("rel", rel);
    }

    /**
     *  Reverse link type. See the  rev attribute definition in HTML 4.0.
     */
    public String getRev()
    {
	return getAttribute("rev");
    }

    public void setRev(String rev)
    {
	setAttribute("rev", rev);
    }

    /**
     *  Frame to render the resource in. See the  target attribute definition 
     * in HTML 4.0.
     */
    public String getTarget()
    {
	return getAttribute("target");
    }

    public void setTarget(String target)
    {
	setAttribute("target", target);
    }

    /**
     *  Advisory content type. See the  type attribute definition in HTML 4.0.
     */
    public String getType()
    {
	return getAttribute("type");
    }

    public void setType(String type)
    {
	setAttribute("type", type);
    }

    //------------------------------------------------------------
    // Method from org.w3c.dom.stylesheets.LinkStyle
    //------------------------------------------------------------

    /**
     *  The style sheet. 
     */
    public org.w3c.dom.stylesheets.StyleSheet getSheet()
    {
	try
	{
	    Object result = null;

	    try
	    {
		result = obj.getMember("stylesheet");
	    }
	    catch (Throwable e)
	    {
	    }

	    if (result == null)
	    {
		try
		{
		    result = obj.getMember("sheet");
		}
		catch (Throwable e)
		{
		}
	    }

	    if (result != null && result instanceof DOMObject)
	    {
    		return new sun.plugin.dom.stylesheets.StyleSheet((DOMObject) result, getOwnerDocument());
	    }
	}
	catch (DOMException e)
	{
	}

	return null;
    }
}

