/*
 * @(#)HTMLObjectElement.java	1.5 03/01/23
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
 *  Generic embedded object.  Note. In principle, all properties on the object 
 * element are read-write but in some environments some properties may be 
 * read-only once the underlying object is instantiated. See the  OBJECT 
 * element definition in HTML 4.0.
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public class HTMLObjectElement extends sun.plugin.dom.html.HTMLElement 
			       implements org.w3c.dom.html.HTMLObjectElement
{
    /**
     * Construct a HTMLObjectElement object.
     */
    public HTMLObjectElement(DOMObject obj, org.w3c.dom.html.HTMLDocument doc)
    {
	super(DOMObjectType.HTMLObjectElement, obj, doc);
    }
    
    /**
     *  Returns the <code>FORM</code> element containing this control. Returns 
     * <code>null</code> if this control is not within the context of a form. 
     */
    public org.w3c.dom.html.HTMLFormElement getForm()
    {
	return null;
    }

    /**
     *  Applet class file. See the <code>code</code> attribute for 
     * HTMLAppletElement. 
     */
    public String getCode()
    {
	return getAttribute("code");
    }

    public void setCode(String code)
    {
	setAttribute("code", code);
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
     *  Space-separated list of archives. See the  archive attribute definition
     *  in HTML 4.0.
     */
    public String getArchive()
    {
	return getAttribute("archive");
    }

    public void setArchive(String archive)
    {
	setAttribute("archive", archive);
    }


    /**
     *  Width of border around the object. See the  border attribute definition
     *  in HTML 4.0. This attribute is deprecated in HTML 4.0.
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
     *  Base URI for <code>classid</code> , <code>data</code> , and
     * <code>archive</code> attributes. See the  codebase attribute definition
     *  in HTML 4.0.
     */
    public String getCodeBase()
    {
	return getAttribute("codeBase");
    }

    public void setCodeBase(String codeBase)
    {
	setAttribute("codeBase", codeBase);
    }

    /**
     *  Content type for data downloaded via <code>classid</code> attribute. 
     * See the  codetype attribute definition in HTML 4.0.
     */
    public String getCodeType()
    {
	return getAttribute("codeType");
    }

    public void setCodeType(String codeType)
    {
	setAttribute("codeType", codeType);
    }

    /**
     *  A URI specifying the location of the object's data.  See the  data 
     * attribute definition in HTML 4.0.
     */
    public String getData()
    {
	return getAttribute("data");
    }

    public void setData(String data)
    {
	setAttribute("data", data);
    }


    /**
     *  Declare (for future reference), but do not instantiate, this object. 
     * See the  declare attribute definition in HTML 4.0.
     */
    public boolean getDeclare()
    {
	return DOMObjectHelper.getBooleanMemberNoEx(obj, "declare");
    }

    public void setDeclare(boolean declare)
    {
	DOMObjectHelper.setBooleanMemberNoEx(obj, "declare", declare);
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
     *  Horizontal space to the left and right of this image, applet, or 
     * object. See the  hspace attribute definition in HTML 4.0. This 
     * attribute is deprecated in HTML 4.0.
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
     *  Form control or object name when submitted with a form. See the  name 
     * attribute definition in HTML 4.0.
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
     *  Message to render while loading the object. See the  standby attribute 
     * definition in HTML 4.0.
     */
    public String getStandby()
    {
	return getAttribute("standby");
    }

    public void setStandby(String standby)
    {
	setAttribute("standby", standby);
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
     *  Content type for data downloaded via <code>data</code> attribute. See 
     * the  type attribute definition in HTML 4.0.
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
     *  Vertical space above and below this image, applet, or object. See the  
     * vspace attribute definition in HTML 4.0. This attribute is deprecated 
     * in HTML 4.0.
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

    /**
     *  The document this object contains, if there is any and it is 
     * available, or <code>null</code> otherwise.
     * @since DOM Level 2
     */
    public org.w3c.dom.Document getContentDocument()
    {
	return getOwnerDocument();
    }
}

