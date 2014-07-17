/*
 * @(#)HTMLTextAreaElement.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import org.w3c.dom.*;
import org.w3c.dom.html.*;
import sun.plugin.dom.*;

/**
 *  A class that encapsulates an input element. 
 */
public class HTMLTextAreaElement extends sun.plugin.dom.html.HTMLElement 
			         implements org.w3c.dom.html.HTMLTextAreaElement
{
    /**
     * Construct a HTMLTextAreaElement object.
     */
    public HTMLTextAreaElement(DOMObject obj, 
			       org.w3c.dom.html.HTMLDocument doc, 
			       org.w3c.dom.html.HTMLFormElement form)
    {
	super(DOMObjectType.HTMLTextAreaElement, obj, doc, form);
    }

    /**
     *  Represents the contents of the element. The value of this attribute 
     * does not change if the contents of the corresponding form control, in 
     * an interactive user agent, changes. Changing this attribute, however, 
     * resets the contents of the form control.
     */
    public String getDefaultValue()
    {	
	return getAttribute("defaultValue");
    }

    public void setDefaultValue(String defaultValue)
    {
	setAttribute("defaultValue", defaultValue);
    }

    /**
     *  Returns the <code>FORM</code> element containing this control. Returns 
     * <code>null</code> if this control is not within the context of a form. 
     */
    public org.w3c.dom.html.HTMLFormElement getForm()
    {
	return (org.w3c.dom.html.HTMLFormElement) owner;
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
     *  Width of control (in characters). See the  cols attribute definition 
     * in HTML 4.0.
     */
    public int getCols()
    {
	return DOMObjectHelper.getIntMemberNoEx(obj, "cols");
    }

    public void setCols(int cols)
    {
	DOMObjectHelper.setIntMemberNoEx(obj, "cols", cols);
    }

    /**
     *  The control is unavailable in this context. See the  disabled 
     * attribute definition in HTML 4.0.
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
     *  This control is read-only. See the  readonly attribute definition in 
     * HTML 4.0.
     */
    public boolean getReadOnly()
    {
	return DOMObjectHelper.getBooleanMemberNoEx(obj, "readOnly");
    }

    public void setReadOnly(boolean readOnly)
    {
	DOMObjectHelper.setBooleanMemberNoEx(obj, "readOnly", readOnly);
    }

    /**
     *  Number of text rows. See the  rows attribute definition in HTML 4.0.
     */
    public int getRows()
    {
	return DOMObjectHelper.getIntMemberNoEx(obj, "rows");
    }

    public void setRows(int rows)
    {
	DOMObjectHelper.setIntMemberNoEx(obj, "rows", rows);
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
     *  The type of this form control. This the string "textarea".
     */
    public String getType()
    {
	return "textarea";
    }

    /**
     *  Represents the current contents of the corresponding form control, in 
     * an interactive user agent. Changing this attribute changes the 
     * contents of the form control, but does not change the contents of the 
     * element. If the entirety of the data can not fit into a single 
     * <code>DOMString</code> , the implementation may truncate the data.
     */
    public String getValue()
    {
	return getAttribute("value");
    }

    public void setValue(String value)
    {
	setAttribute("value", value);
    }

    /**
     *  Removes keyboard focus from this element.
     */
    public void blur()
    {
	obj.call("blur", null);
    }

    /**
     *  Gives keyboard focus to this element.
     */
    public void focus()
    {
	obj.call("focus", null);
    }

    /**
     *  Select the contents of the <code>TEXTAREA</code> .
     */
    public void select()
    {
	obj.call("select", null);
    }
}

