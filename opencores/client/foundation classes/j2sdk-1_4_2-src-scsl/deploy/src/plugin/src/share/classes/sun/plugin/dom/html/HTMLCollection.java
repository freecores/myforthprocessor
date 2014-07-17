/*
 * Copyright (c) 2000 World Wide Web Consortium,
 * (Massachusetts Institute of Technology, Institut National de
 * Recherche en Informatique et en Automatique, Keio University). All
 * Rights Reserved. This program is distributed under the W3C's Software
 * Intellectual Property License. This program is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See W3C License http://www.w3.org/Consortium/Legal/ for more
 * details.
 */

package sun.plugin.dom.html;

import org.w3c.dom.Node;
import org.w3c.dom.html.*;
import sun.plugin.dom.*;


/**
 *  An <code>HTMLCollection</code> is a list of nodes. An individual node may 
 * be accessed by either ordinal index or the node's<code>name</code> or 
 * <code>id</code> attributes.  Note: Collections in the HTML DOM are assumed 
 * to be  live meaning that they are automatically updated when the 
 * underlying document is changed. 
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
final class HTMLCollection implements org.w3c.dom.html.HTMLCollection 
{
    // HTMLElement type
    private int type;

    // Underlying DOMObject
    private DOMObject obj;

    // Owner document
    private org.w3c.dom.html.HTMLDocument doc;

    // Owner document
    private org.w3c.dom.Element owner;

    /**
     * Construct a HTML Collection object.
     */
    public HTMLCollection(int type, DOMObject obj, 
			  org.w3c.dom.html.HTMLDocument doc)
    {
	this(type, obj, doc, null);
    }

    /**
     * Construct a HTML Collection object.
     */
    public HTMLCollection(int type, DOMObject obj, 
			  org.w3c.dom.html.HTMLDocument doc,
			  org.w3c.dom.Element owner)
    {
	this.type = type;
	this.obj = obj;
	this.doc = doc;
	this.owner = owner;
    }

    /**
     *  This attribute specifies the length or  size of the list. 
     */
    public int getLength()
    {
	return DOMObjectHelper.getIntMemberNoEx(obj, "length");	
    }

    /**
     *  This method retrieves a node specified by ordinal index. Nodes are 
     * numbered in tree order (depth-first traversal order).
     * @param index  The index of the node to be fetched. The index origin is 
     *   0.
     * @return  The <code>Node</code> at the corresponding position upon 
     *   success. A value of <code>null</code> is returned if the index is 
     *   out of range. 
     */
    public Node item(int index)
    {
	Object result = obj.getSlot(index);

	// Wrap the node with proper HTMLElement type
	return DOMObjectFactory.createHTMLElement(type, result, doc, owner);
    }

    /**
     *  This method retrieves a <code>Node</code> using a name. It first 
     * searches for a <code>Node</code> with a matching <code>id</code> 
     * attribute. If it doesn't find one, it then searches for a 
     * <code>Node</code> with a matching <code>name</code> attribute, but 
     * only on those elements that are allowed a name attribute. 
     * @param name  The name of the <code>Node</code> to be fetched.
     * @return  The <code>Node</code> with a <code>name</code> or 
     *   <code>id</code> attribute whose value corresponds to the specified 
     *   string. Upon failure (e.g., no node with this name exists), returns 
     *   <code>null</code> .
     */
    public Node namedItem(String name)
    {
	Object result = obj.getMember(name);

	// Wrap the node with proper HTMLElement type
	return DOMObjectFactory.createHTMLElement(type, result, doc, owner);
    }

}

