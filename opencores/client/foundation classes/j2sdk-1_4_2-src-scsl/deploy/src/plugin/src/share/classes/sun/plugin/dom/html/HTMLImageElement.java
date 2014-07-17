/*
 * @(#)HTMLImageElement.java	1.4 03/01/23
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
 *  Embedded image. See the  IMG element definition in HTML 4.0.
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public class HTMLImageElement extends sun.plugin.dom.html.HTMLElement 
			      implements org.w3c.dom.html.HTMLImageElement
{
    /**
     * Construct a HTMLImageElement object.
     */
    public HTMLImageElement(DOMObject obj, org.w3c.dom.html.HTMLDocument doc)
    {
	super(DOMObjectType.HTMLImageElement, obj, doc);
    }

    /**
     *  URI designating the source of this image, for low-resolution output. 
     */
    public String getLowSrc()
    {
	return getAttribute("lowSrc");
    }


    public void setLowSrc(String lowSrc)
    {
	setAttribute("lowSrc", lowSrc);
    }

    /**
     *  The name of the element (for backwards compatibility). 
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
     *  Aligns this object (vertically or horizontally)  with respect to its 
     * surrounding text. See the  align attribute definition in HTML 4.0. 
     * This attribute is deprecated in HTML 4.0.
     */
    public String getAlign()
    {
	return getAttribute("align");
    }

    public void setAlign(String align)
    {
	setAttribute("align", align);
    }

    /**
     *  Alternate text for user agents not rendering the normal content of 
     * this element. See the  alt attribute definition in HTML 4.0.
     */
    public String getAlt()
    {
	return getAttribute("alt");
    }

    public void setAlt(String alt)
    {
	setAttribute("alt", alt);
    }

    /**
     *  Width of border around image. See the  border attribute definition in 
     * HTML 4.0. This attribute is deprecated in HTML 4.0.
     */
    public String getBorder()
    {
	return getAttribute("border");
    }

    public void setBorder(String border)
    {
	setAttribute("border", border);
    }

    /**
     *  Override height. See the  height attribute definition in HTML 4.0.
     */
    public String getHeight()
    {
	return getAttribute("height");
    }

    public void setHeight(String height)
    {
	setAttribute("height", height);
    }
    /**
     *  Horizontal space to the left and right of this image. See the  hspace 
     * attribute definition in HTML 4.0. This attribute is deprecated in HTML 
     * 4.0.
     */
    public String getHspace()
    {
	return getAttribute("hspace");
    }

    public void setHspace(String hspace)
    {
	setAttribute("hspace", hspace);
    }

    /**
     *  Use server-side image map. See the  ismap attribute definition in HTML 
     * 4.0.
     */
    public boolean getIsMap()
    {
	return DOMObjectHelper.getBooleanMemberNoEx(obj, "isMap");
    }

    public void setIsMap(boolean isMap)
    {
	DOMObjectHelper.setBooleanMemberNoEx(obj, "isMap", isMap);
    }

    /**
     *  URI designating a long description of this image or frame. See the  
     * longdesc attribute definition in HTML 4.0.
     */
    public String getLongDesc()
    {
	return getAttribute("longDesc");
    }

    public void setLongDesc(String longDesc)
    {
	setAttribute("longDesc", longDesc);
    }

    /**
     *  URI designating the source of this image. See the  src attribute 
     * definition in HTML 4.0.
     */
    public String getSrc()
    {
	return getAttribute("src");
    }

    public void setSrc(String src)
    {
	setAttribute("src", src);
    }

    /**
     *  Use client-side image map. See the  usemap attribute definition in 
     * HTML 4.0.
     */
    public String getUseMap()
    {
	return getAttribute("useMap");
    }

    public void setUseMap(String useMap)
    {
	setAttribute("useMap", useMap);
    }

    /**
     *  Vertical space above and below this image. See the  vspace attribute 
     * definition in HTML 4.0. This attribute is deprecated in HTML 4.0.
     */
    public String getVspace()
    {
	return getAttribute("vspace");
    }

    public void setVspace(String vspace)
    {
	setAttribute("vspace", vspace);
    }

    /**
     *  Override width. See the  width attribute definition in HTML 4.0.
     */
    public String getWidth()
    {
	return getAttribute("width");
    }

    public void setWidth(String width)
    {
	setAttribute("width", width);
    }
}

