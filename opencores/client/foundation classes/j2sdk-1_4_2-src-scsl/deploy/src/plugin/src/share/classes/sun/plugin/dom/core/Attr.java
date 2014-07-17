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

package sun.plugin.dom.core;

import org.w3c.dom.*;
import sun.plugin.dom.*;

/**
 * Attr class implements org.w3c.dom.Attr.
 */
final class Attr extends sun.plugin.dom.core.Node 
			  implements org.w3c.dom.Attr 
{
    // Owner document
    private Document doc;	

    /**
     * Construct an Attr object.
     */
    public Attr(DOMObject obj, Document doc)
    {
	super(DOMObjectType.Attr, obj);

	this.doc = doc;
    }

    /**
     *  Returns the name of this attribute. 
     */
    public String getName()
    {
	return DOMObjectHelper.getStringMemberNoEx(obj, "name");
    }

    /**
     *  If this attribute was explicitly given a value in the original 
     * document, this is <code>true</code> ; otherwise, it is 
     * <code>false</code> . Note that the implementation is in charge of this 
     * attribute, not the user. If the user changes the value of the 
     * attribute (even if it ends up having the same value as the default 
     * value) then the <code>specified</code> flag is automatically flipped 
     * to <code>true</code> .  To re-specify the attribute as the default 
     * value from the DTD, the user must delete the attribute. The 
     * implementation will then make a new attribute available with 
     * <code>specified</code> set to <code>false</code> and the default value 
     * (if one exists).
     * <br> In summary: If the attribute has an assigned value in the document 
     * then  <code>specified</code> is <code>true</code> , and the value is 
     * the  assigned value. If the attribute has no assigned value in the 
     * document and has  a default value in the DTD, then 
     * <code>specified</code> is <code>false</code> ,  and the value is the 
     * default value in the DTD. If the attribute has no assigned value in 
     * the document and has  a value of #IMPLIED in the DTD, then the  
     * attribute does not appear  in the structure model of the document. If 
     * the <code>ownerElement</code> attribute is <code>null</code> (i.e. 
     * because it was just created or was set to <code>null</code> by the 
     * various removal and cloning operations) <code>specified</code> is 
     * <code>true</code> .
     */
    public boolean getSpecified()
    {
	return DOMObjectHelper.getBooleanMemberNoEx(obj, "specified");
    }

    /**
     *  On retrieval, the value of the attribute is returned as a string. 
     * Character and general entity references are replaced with their 
     * values. See also the method <code>getAttribute</code> on the  
     * <code>Element</code> interface.
     * <br> On setting, this creates a <code>Text</code> node with the unparsed
     *  contents of the string. I.e. any characters that an XML processor 
     * would recognize as markup are instead treated as literal text. See 
     * also the method <code>setAttribute</code> on the  <code>Element</code> 
     * interface.
     * @exception DOMException
     *    NO_MODIFICATION_ALLOWED_ERR: Raised when the node is readonly.
     */
    public String getValue()
    {
	return DOMObjectHelper.getStringMemberNoEx(obj, "value");
    }

    public void setValue(String value)
                            throws DOMException
    {
	DOMObjectHelper.setStringMember(obj, "value", value);
    }

    /**
     *  The <code>Element</code> node this attribute is attached to or 
     * <code>null</code> if this attribute is not in use.
     * @since DOM Level 2
     */
    public org.w3c.dom.Element getOwnerElement()
    {
	return null;
    }


    /**
     *  The <code>Document</code> object associated with this node. This is 
     * also the <code>Document</code> object used to create new nodes. When 
     * this node is a <code>Document</code> or a <code>DocumentType</code> 
     * which is not used with any <code>Document</code> yet, this is 
     * <code>null</code> .
     * @version DOM Level 2
     */
    public org.w3c.dom.Document getOwnerDocument()
    {
	return doc;
    }

    /**
     *  The name of this node, depending on its type; see the table above. 
     */
    public String getNodeName()
    {
	return getName();
    }

    /**
     *  The value of this node, depending on its type; see the table above. 
     * When it is defined to be <code>null</code> , setting it has no effect.
     * @exception DOMException
     *    NO_MODIFICATION_ALLOWED_ERR: Raised when the node is readonly.
     * @exception DOMException
     *    DOMSTRING_SIZE_ERR: Raised when it would return more characters 
     *   than fit in a <code>DOMString</code> variable on the implementation 
     *   platform.
     */
    public String getNodeValue() throws DOMException
    {
	return getValue();
    }

    public void setNodeValue(String nodeValue)
                                  throws DOMException
    {
	setValue(nodeValue);
    }

    /**
     *  A code representing the type of the underlying object, as defined 
     * above.
     */
    public short getNodeType()
    {
	return ATTRIBUTE_NODE;
    }
}

