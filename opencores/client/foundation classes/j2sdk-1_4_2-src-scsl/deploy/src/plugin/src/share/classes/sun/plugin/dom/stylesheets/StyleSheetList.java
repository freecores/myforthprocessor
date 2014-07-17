/*
 * Copyright (c) 2000 World Wide Web Consortium,
 * (Massachusetts Institute of Technology, Institut National de
 * Recherche en Informatique et en Automatique, Keio University). All
 * Rights Reserved. This program is distributed under the W3C's Software
 * Intellectual Property License. This program is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.
 * See W3C License http://www.w3.org/Consortium/Legal/ for more details.
 */

package sun.plugin.dom.stylesheets;

import org.w3c.dom.DOMException;
import org.w3c.dom.Node;
import org.w3c.dom.html.*;
import sun.plugin.dom.*;


/**
 * The <code>StyleSheetList</code> interface provides the abstraction of an 
 * ordered collection of style sheets. 
 * <p> The items in the <code>StyleSheetList</code> are accessible via an 
 * integral index, starting from 0. 
 * <p>See also the <a href='http://www.w3.org/TR/2000/REC-DOM-Level-2-Style-20001113'>Document Object Model (DOM) Level 2 Style Specification</a>.
 * @since DOM Level 2
 */
public class StyleSheetList implements org.w3c.dom.stylesheets.StyleSheetList 
{
    // HTMLElement type
    private int type;

    // Underlying DOMObject
    private DOMObject obj;

    // Owner document
    private org.w3c.dom.Node owner;

    /**
     * Construct a HTML Collection object.
     */
    public StyleSheetList(int type, DOMObject obj, org.w3c.dom.Node owner)
    {
	this.type = type;
	this.obj = obj;
	this.owner = owner;
    }

    /**
     *  The number of <code>StyleSheets</code> in the list. The range of valid 
     * child stylesheet indices is <code>0</code> to <code>length-1</code> 
     * inclusive. 
     */
    public int getLength()
    {
	return DOMObjectHelper.getIntMemberNoEx(obj, "length");	
    }

    /**
     *  Used to retrieve a style sheet by ordinal index. If index is greater 
     * than or equal to the number of style sheets in the list, this returns 
     * <code>null</code>. 
     * @param indexIndex into the collection
     * @return The style sheet at the <code>index</code> position in the 
     *   <code>StyleSheetList</code>, or <code>null</code> if that is not a 
     *   valid index. 
     */
    public org.w3c.dom.stylesheets.StyleSheet item(int index)
    {
	try
	{
	    Object result = obj.getSlot(index);

	    // Wrap the node with proper HTMLElement type
	    if (result != null && result instanceof DOMObject)
		return (org.w3c.dom.stylesheets.StyleSheet) DOMObjectFactory.createStyleSheet(type, result, owner);
	}
	catch (DOMException e)
	{
	}

	return null;
    }
}


