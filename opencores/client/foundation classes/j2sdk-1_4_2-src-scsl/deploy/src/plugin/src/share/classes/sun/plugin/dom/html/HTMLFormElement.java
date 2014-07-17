/*
 * @(#)HTMLFormElement.java	1.4 03/01/23
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
 *  The <code>FORM</code> element encompasses behavior similar to a collection 
 * and an element. It provides direct access to the contained input elements 
 * as well as the attributes of the form element. See the  FORM element 
 * definition in HTML 4.0.
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public class HTMLFormElement extends sun.plugin.dom.html.HTMLElement 
			     implements org.w3c.dom.html.HTMLFormElement
{
    /**
     * Construct a HTMLFormElement object.
     */
    public HTMLFormElement(DOMObject obj, org.w3c.dom.html.HTMLDocument doc)
    {
	super(DOMObjectType.HTMLFormElement, obj, doc);
    }

    /**
     *  Returns a collection of all control elements in the form. 
     */
    public org.w3c.dom.html.HTMLCollection getElements()
    {
	Object result = obj.getMember("elements");

	if (result != null && result instanceof DOMObject)
	{
	    return new sun.plugin.dom.html.HTMLCollection(DOMObjectType.HTMLElement, 
							  (DOMObject)result, 
							  (org.w3c.dom.html.HTMLDocument) getOwnerDocument());
	}

	return null;
    }

    /**
     *  The number of form controls in the form.
     */
    public int getLength()
    {
	return DOMObjectHelper.getIntMemberNoEx(obj, "length");
    }

    /**
     *  Names the form. 
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
     *  List of character sets supported by the server. See the  
     * accept-charset attribute definition in HTML 4.0.
     */
    public String getAcceptCharset()
    {
	return getAttribute("acceptCharset");
    }

    public void setAcceptCharset(String acceptCharset)
    {
	setAttribute("acceptCharset", acceptCharset);
    }

    /**
     *  Server-side form handler. See the  action attribute definition in HTML 
     * 4.0.
     */
    public String getAction()
    {
	return getAttribute("action");
    }

    public void setAction(String action)
    {
	setAttribute("action", action);
    }

    /**
     *  The content type of the submitted form,  generally 
     * "application/x-www-form-urlencoded".  See the  enctype attribute 
     * definition in HTML 4.0.
     */
    public String getEnctype()
    {
	return getAttribute("enctype");
    }

    public void setEnctype(String enctype)
    {
	setAttribute("enctype", enctype);
    }

    /**
     *  HTTP method used to submit form. See the  method attribute definition 
     * in HTML 4.0.
     */
    public String getMethod()
    {
	return getAttribute("method");
    }

    public void setMethod(String method)
    {
	setAttribute("method", method);
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
     *  Submits the form. It performs the same action as a  submit button.
     */
    public void submit()
    {
	obj.call("submit", new Object[0]);
    }

    /**
     *  Restores a form element's default values. It performs  the same action 
     * as a reset button.
     */
    public void reset()
    {
	obj.call("reset", new Object[0]);
    }
}

