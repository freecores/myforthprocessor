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

package sun.plugin.dom.css;

import org.w3c.dom.DOMException;
import org.w3c.dom.stylesheets.StyleSheet;
import org.w3c.dom.css.*;
import sun.plugin.dom.*;


/**
 *  The <code>CSSStyleSheet</code> interface is a concrete interface used to 
 * represent a CSS style sheet i.e., a style sheet whose content type is 
 * "text/css". 
 * <p>See also the <a href='http://www.w3.org/TR/2000/REC-DOM-Level-2-Style-20001113'>Document Object Model (DOM) Level 2 Style Specification</a>.
 * @since DOM Level 2
 */
public class CSSStyleSheet extends sun.plugin.dom.stylesheets.StyleSheet
			   implements org.w3c.dom.css.CSSStyleSheet 
{
    /**
     * Construct a HTML CSSStyleSheet object.
     */
    public CSSStyleSheet(DOMObject obj, org.w3c.dom.Node owner)
    {
	super(obj, owner);
    }

    /**
     *  If this style sheet comes from an <code>@import</code> rule, the 
     * <code>ownerRule</code> attribute will contain the 
     * <code>CSSImportRule</code>. In that case, the <code>ownerNode</code> 
     * attribute in the <code>StyleSheet</code> interface will be 
     * <code>null</code>. If the style sheet comes from an element or a 
     * processing instruction, the <code>ownerRule</code> attribute will be 
     * <code>null</code> and the <code>ownerNode</code> attribute will 
     * contain the <code>Node</code>. 
     */
    public org.w3c.dom.css.CSSRule getOwnerRule()
    {
	return null;
    }
    
    /**
     *  The list of all CSS rules contained within the style sheet. This 
     * includes both rule sets and at-rules. 
     */
    public org.w3c.dom.css.CSSRuleList getCssRules()
    {
	try
	{
	    Object result = null;
	    	    
	    try
	    {
		result = obj.getMember("cssRules");
	    }
	    catch (Throwable e)
	    {
	    }

	    if (result == null)
	    {
		try
		{
		    result = obj.getMember("rules");
		}
		catch (Throwable e)
		{
		}
	    }

	    if (result != null && result instanceof DOMObject)
	    {
		return new sun.plugin.dom.css.CSSRuleList((DOMObject) result, this, null);
	    }
	}
	catch (DOMException e)
	{
	    e.printStackTrace();
	}

	return null;
    }

    /**
     *  Used to insert a new rule into the style sheet. The new rule now 
     * becomes part of the cascade. 
     * @param rule The parsable text representing the rule. For rule sets 
     *   this contains both the selector and the style declaration. For 
     *   at-rules, this specifies both the at-identifier and the rule 
     *   content. 
     * @param index The index within the style sheet's rule list of the rule 
     *   before which to insert the specified rule. If the specified index 
     *   is equal to the length of the style sheet's rule collection, the 
     *   rule will be added to the end of the style sheet. 
     * @return  The index within the style sheet's rule collection of the 
     *   newly inserted rule. 
     * @exception DOMException
     *   HIERARCHY_REQUEST_ERR: Raised if the rule cannot be inserted at the 
     *   specified index e.g. if an <code>@import</code> rule is inserted 
     *   after a standard rule set or other at-rule.
     *   <br>INDEX_SIZE_ERR: Raised if the specified index is not a valid 
     *   insertion point.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this style sheet is 
     *   readonly.
     *   <br>SYNTAX_ERR: Raised if the specified rule has a syntax error and 
     *   is unparsable.
     */
    public int insertRule(String rule, 
                          int index)
                          throws DOMException
    {
	Object[] args = new Object[2];
	args[0] = rule;
	args[1] = new Integer(index);

	String result = DOMObjectHelper.callStringMethod(obj, "insertRule", args);

	if (result != null)
	    return new Integer(result).intValue();
	else
	    return 0;
    }

    /**
     *  Used to delete a rule from the style sheet. 
     * @param index The index within the style sheet's rule list of the rule 
     *   to remove. 
     * @exception DOMException
     *   INDEX_SIZE_ERR: Raised if the specified index does not correspond to 
     *   a rule in the style sheet's rule list.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this style sheet is 
     *   readonly.
     */
    public void deleteRule(int index)
                           throws DOMException
    {
	Object[] args = new Object[1];
	args[0] = new Integer(index);

	DOMObjectHelper.callStringMethod(obj, "deleteRule", args);
    }
}
