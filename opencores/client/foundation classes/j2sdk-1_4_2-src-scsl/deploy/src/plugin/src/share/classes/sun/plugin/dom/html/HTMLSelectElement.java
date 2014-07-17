/*
 * @(#)HTMLSelectElement.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import org.w3c.dom.*;
import org.w3c.dom.html.*;
import sun.plugin.dom.*;

/**
 *  A class that encapsulates an select element. 
 */
public class HTMLSelectElement extends sun.plugin.dom.html.HTMLElement
			       implements org.w3c.dom.html.HTMLSelectElement
{
    /**
     * Construct a HTMLSelectElement object.
     */
    public HTMLSelectElement(DOMObject obj, 
			    org.w3c.dom.html.HTMLDocument doc, 
			    org.w3c.dom.html.HTMLFormElement form)
    {
	super(DOMObjectType.HTMLSelectElement, obj, doc, form);
    }
    
    /**
     *  The type of this form control. This is the string "select-multiple" 
     * when the multiple attribute is <code>true</code> and the string 
     * "select-one" when <code>false</code> .
     */
    public String getType()
    {
	return getAttribute("type");
    }

    /**
     *  The ordinal index of the selected option, starting from 0. The value 
     * -1 is returned if no element is selected. If multiple options are 
     * selected, the index of the first selected option is returned. 
     */
    public int getSelectedIndex()
    {
	return DOMObjectHelper.getIntMemberNoEx(obj, "selectedIndex");
    }

    public void setSelectedIndex(int selectedIndex)
    {
	DOMObjectHelper.setIntMemberNoEx(obj, "selectedIndex", selectedIndex);
    }

    /**
     *  The current form control value. 
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
     *  The number of options in this <code>SELECT</code> . 
     */
    public int getLength()
    {
	return DOMObjectHelper.getIntMemberNoEx(obj, "length");
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
     *  The collection of <code>OPTION</code> elements contained by this 
     * element. 
     */
    public org.w3c.dom.html.HTMLCollection getOptions()
    {
	return null;
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
     *  If true, multiple <code>OPTION</code> elements may  be selected in 
     * this <code>SELECT</code> . See the  multiple attribute definition in 
     * HTML 4.0.
     */
    public boolean getMultiple()
    {
	return DOMObjectHelper.getBooleanMemberNoEx(obj, "multiple");
    }

    public void setMultiple(boolean multiple)
    {
	DOMObjectHelper.setBooleanMemberNoEx(obj, "multiple", multiple);
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
     *  Number of visible rows. See the  size attribute definition in HTML 4.0.
     */
    public int getSize()
    {
	return DOMObjectHelper.getIntMemberNoEx(obj, "size");
    }

    public void setSize(int size)
    {
	DOMObjectHelper.setIntMemberNoEx(obj, "size", size);
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
     *  Add a new element to the collection of <code>OPTION</code> elements 
     * for this <code>SELECT</code> . This method is the equivalent of the 
     * <code>appendChild</code> method of the <code>Node</code> interface if 
     * the <code>before</code> parameter is <code>null</code> . It is 
     * equivalent to the <code>insertBefore</code> method on the parent of 
     * <code>before</code> in all other cases.
     * @param element  The element to add.
     * @param before  The element to insert before, or <code>null</code> for 
     *   the tail of the list.
     * @exception DOMException
     *    NOT_FOUND_ERR: Raised if <code>before</code> is not a descendant of 
     *   the <code>SELECT</code> element. 
     */
    public void add(org.w3c.dom.html.HTMLElement element, 
                    org.w3c.dom.html.HTMLElement before)
                    throws DOMException
    {
    }


    /**
     *  Remove an element from the collection of <code>OPTION</code> elements 
     * for this <code>SELECT</code> . Does nothing if no element has the given
     *  index.
     * @param index  The index of the item to remove, starting from 0.
     */
    public void remove(int index)
    {
	Object args[] = new Object[1];
	args[0] = new Integer(index);

	obj.call("remove", args);
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
}

