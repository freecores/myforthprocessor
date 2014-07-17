/*
 * @(#)HTMLAnchorElement.java	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import org.w3c.dom.*;
import org.w3c.dom.html.*;
import sun.plugin.dom.*;

/**
 *  A class that encapsulates an anchor element. 
 */
public class HTMLAnchorElement extends sun.plugin.dom.html.HTMLElement 
			       implements org.w3c.dom.html.HTMLAnchorElement
{
    /**
     * Construct a HTMLAnchorElement object.
     */
    public HTMLAnchorElement(DOMObject obj, org.w3c.dom.html.HTMLDocument doc)
    {
	super(DOMObjectType.HTMLAnchorElement, obj, doc);
    }

    /**
     *  A single character access key to give access to the form control. See 
     * the  accesskey attribute definition in HTML 4.0.
     */
    public String getAccessKey()
    {
	return getAttribute("accessKey");
    }

    public void setAccessKey(String accessKey)
    {
	setAttribute("accessKey", accessKey);
    }

    /**
     *  The character encoding of the linked resource. See the  charset 
     * attribute definition in HTML 4.0.
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
     *  Comma-separated list of lengths, defining an active region geometry. 
     * See also <code>shape</code> for the shape of the region. See the  
     * coords attribute definition in HTML 4.0.
     */
    public String getCoords()
    {
	return getAttribute("coords");
    }

    public void setCoords(String coords)
    {
	setAttribute("coords", coords);
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
     *  Anchor name. See the  name attribute definition in HTML 4.0.
     */
    public String getName()
    {
	return getAttribute("name");
    }

    public void setName(String name)
    {
	setAttribute("name", name);
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
     *  The shape of the active area. The coordinates are given by 
     * <code>coords</code> . See the  shape attribute definition in HTML 4.0.
     */
    public String getShape()
    {
	return getAttribute("shape");
    }

    public void setShape(String shape)
    {
	setAttribute("shape", shape);
    }

    /**
     *  Index that represents the element's position in the tabbing order. See 
     * the  tabindex attribute definition in HTML 4.0.
     */
    public int getTabIndex()
    {
	return DOMObjectHelper.getIntMemberNoEx(obj, "tabIndex");
    }

    public void setTabIndex(int tabIndex)
    {
	DOMObjectHelper.setIntMemberNoEx(obj, "tabIndex", tabIndex);
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

    /**
     *  Removes keyboard focus from this element.
     */
    public void blur()
    {
	obj.call("blur", new Object[0]);
    }

    /**
     *  Gives keyboard focus to this element.
     */
    public void focus()
    {
	obj.call("focus", new Object[0]);
    }
}

