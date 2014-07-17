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

import org.w3c.dom.*;
import org.w3c.dom.css.*;
import sun.plugin.dom.*;

/**
 *  The <code>CSSStyleDeclaration</code> interface represents a single CSS 
 * declaration block. This interface may be used to determine the style 
 * properties currently set in a block or to set style properties explicitly 
 * within the block. 
 * <p> While an implementation may not recognize all CSS properties within a 
 * CSS declaration block, it is expected to provide access to all specified 
 * properties in the style sheet through the <code>CSSStyleDeclaration</code>
 *  interface. Furthermore, implementations that support a specific level of 
 * CSS should correctly handle CSS shorthand properties for that level. For 
 * a further discussion of shorthand properties, see the 
 * <code>CSS2Properties</code> interface. 
 * <p> This interface is also used to provide a read-only access to the 
 * computed values of an element. See also the <code>ViewCSS</code> 
 * interface.  The CSS Object Model doesn't provide an access to the 
 * specified or actual values of the CSS cascade. 
 * <p>See also the <a href='http://www.w3.org/TR/2000/REC-DOM-Level-2-Style-20001113'>Document Object Model (DOM) Level 2 Style Specification</a>.
 * @since DOM Level 2
 */
public class CSSStyleDeclaration implements org.w3c.dom.css.CSSStyleDeclaration,
					    org.w3c.dom.css.CSS2Properties
{
    // Underlying DOMObject
    protected DOMObject obj;

    // Owner document
    private org.w3c.dom.css.CSSStyleSheet parentStyleSheet;

    // Parent rule
    private org.w3c.dom.css.CSSRule parentRule;

    /**
     * Construct a CSSRuleList object.
     */
    public CSSStyleDeclaration(DOMObject obj, 
			       org.w3c.dom.css.CSSStyleSheet parentStyleSheet, 
			       org.w3c.dom.css.CSSRule parentRule)
    {
	this.obj = obj;
	this.parentStyleSheet = parentStyleSheet;
	this.parentRule = parentRule;
    }

    //------------------------------------------------------------
    // Method from org.w3c.dom.css.CSSStyleDeclaration
    //------------------------------------------------------------

    /**
     *  The parsable textual representation of the declaration block 
     * (excluding the surrounding curly braces). Setting this attribute will 
     * result in the parsing of the new value and resetting of all the 
     * properties in the declaration block including the removal or addition 
     * of properties. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the specified CSS string value has a syntax 
     *   error and is unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this declaration is 
     *   readonly or a property is readonly.
     */
    public String getCssText()
    {
	return getPropertyValue("cssText");
    }

    public void setCssText(String cssText)
                       throws DOMException
    {
	setProperty("cssText", cssText, null);
    }

    /**
     *  Used to retrieve the value of a CSS property if it has been explicitly 
     * set within this declaration block. 
     * @param propertyName The name of the CSS property. See the CSS property 
     *   index. 
     * @return  Returns the value of the property if it has been explicitly 
     *   set for this declaration block. Returns the empty string if the 
     *   property has not been set. 
     */
    public String getPropertyValue(String propertyName)
    {
	return DOMObjectHelper.getStringMemberNoEx(obj, propertyName);
    }

    /**
     *  Used to retrieve the object representation of the value of a CSS 
     * property if it has been explicitly set within this declaration block. 
     * This method returns <code>null</code> if the property is a shorthand 
     * property. Shorthand property values can only be accessed and modified 
     * as strings, using the <code>getPropertyValue</code> and 
     * <code>setProperty</code> methods. 
     * @param propertyName The name of the CSS property. See the CSS property 
     *   index. 
     * @return  Returns the value of the property if it has been explicitly 
     *   set for this declaration block. Returns <code>null</code> if the 
     *   property has not been set. 
     */
    public org.w3c.dom.css.CSSValue getPropertyCSSValue(String propertyName)
    {
	return null;	    
    }

    /**
     *  Used to remove a CSS property if it has been explicitly set within 
     * this declaration block. 
     * @param propertyName The name of the CSS property. See the CSS property 
     *   index. 
     * @return  Returns the value of the property if it has been explicitly 
     *   set for this declaration block. Returns the empty string if the 
     *   property has not been set or the property name does not correspond 
     *   to a known CSS property. 
     * @exception DOMException
     *   NO_MODIFICATION_ALLOWED_ERR: Raised if this declaration is readonly 
     *   or the property is readonly.
     */
    public String removeProperty(String propertyName)
                                 throws DOMException
    {
	return null;
    }

    /**
     *  Used to retrieve the priority of a CSS property (e.g. the 
     * <code>"important"</code> qualifier) if the property has been 
     * explicitly set in this declaration block. 
     * @param propertyName The name of the CSS property. See the CSS property 
     *   index. 
     * @return  A string representing the priority (e.g. 
     *   <code>"important"</code>) if one exists. The empty string if none 
     *   exists. 
     */
    public String getPropertyPriority(String propertyName)
    {
	return "";
    }

    /**
     *  Used to set a property value and priority within this declaration 
     * block. 
     * @param propertyName The name of the CSS property. See the CSS property 
     *   index. 
     * @param value The new value of the property. 
     * @param priority The new priority of the property (e.g. 
     *   <code>"important"</code>).  
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the specified value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this declaration is 
     *   readonly or the property is readonly.
     */
    public void setProperty(String propertyName, 
                            String value, 
                            String priority)
                            throws DOMException
    {
	DOMObjectHelper.setStringMember(obj, propertyName, value);
    }

    /**
     *  The number of properties that have been explicitly set in this 
     * declaration block. The range of valid indices is 0 to length-1 
     * inclusive. 
     */
    public int getLength()
    {
	return 0;
    }

    /**
     *  Used to retrieve the properties that have been explicitly set in this 
     * declaration block. The order of the properties retrieved using this 
     * method does not have to be the order in which they were set. This 
     * method can be used to iterate over all properties in this declaration 
     * block. 
     * @param index Index of the property name to retrieve. 
     * @return  The name of the property at this ordinal position. The empty 
     *   string if no property exists at this position. 
     */
    public String item(int index)
    {
	return "";
    }	

    /**
     *  The CSS rule that contains this declaration block or <code>null</code> 
     * if this <code>CSSStyleDeclaration</code> is not attached to a 
     * <code>CSSRule</code>. 
     */
    public org.w3c.dom.css.CSSRule getParentRule()
    {
	return parentRule;
    }
    
    
    //------------------------------------------------------------
    // Method from org.w3c.dom.css.CSS2Properties
    //------------------------------------------------------------

    /**
     *  See the azimuth property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getAzimuth()
    {
	return getPropertyValue("azimuth");
    }

    public void setAzimuth(String azimuth)
                           throws DOMException
    {
	setProperty("azimuth", azimuth, null);
    }


    /**
     *  See the background property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBackground()
    {
	return getPropertyValue("background");
    }

    public void setBackground(String background)
                                             throws DOMException
    {
	setProperty("background", background, null);
    }


    /**
     *  See the background-attachment property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBackgroundAttachment()
    {
	return getPropertyValue("backgroundAttachment");
    }

    public void setBackgroundAttachment(String backgroundAttachment)
                                             throws DOMException
    {
	setProperty("backgroundAttachment", backgroundAttachment, null);
    }


    /**
     *  See the background-color property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBackgroundColor()
    {
	return getPropertyValue("backgroundColor");
    }

    public void setBackgroundColor(String backgroundColor)
                                             throws DOMException
    {
	setProperty("backgroundColor", backgroundColor, null);
    }


    /**
     *  See the background-image property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBackgroundImage()
    {
	return getPropertyValue("backgroundImage");
    }

    public void setBackgroundImage(String backgroundImage)
                                             throws DOMException
    {
	setProperty("backgroundImage", backgroundImage, null);
    }


    /**
     *  See the background-position property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBackgroundPosition()
    {
	return getPropertyValue("backgroundPosition");
    }

    public void setBackgroundPosition(String backgroundPosition)
                                             throws DOMException
    {
	setProperty("backgroundPosition", backgroundPosition, null);
    }


    /**
     *  See the background-repeat property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBackgroundRepeat()
    {
	return getPropertyValue("backgroundRepeat");
    }

    public void setBackgroundRepeat(String backgroundRepeat)
                                             throws DOMException
    {
	setProperty("backgroundRepeat", backgroundRepeat, null);
    }


    /**
     *  See the border property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorder()
    {
	return getPropertyValue("border");
    }

    public void setBorder(String border)
                                             throws DOMException
    {
	setProperty("border", border, null);
    }


    /**
     *  See the border-collapse property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderCollapse()
    {
	return getPropertyValue("borderCollapse");
    }

    public void setBorderCollapse(String borderCollapse)
                                             throws DOMException
    {
	setProperty("borderCollapse", borderCollapse, null);
    }


    /**
     *  See the border-color property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderColor()
    {
	return getPropertyValue("borderColor");
    }

    public void setBorderColor(String borderColor)
                                             throws DOMException
    {
	setProperty("borderColor", borderColor, null);
    }


    /**
     *  See the border-spacing property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderSpacing()
    {
	return getPropertyValue("borderSpacing");
    }

    public void setBorderSpacing(String borderSpacing)
                                             throws DOMException
    {
	setProperty("borderSpacing", borderSpacing, null);
    }


    /**
     *  See the border-style property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderStyle()
    {
	return getPropertyValue("borderStyle");
    }

    public void setBorderStyle(String borderStyle)
                                             throws DOMException
    {
	setProperty("borderStyle", borderStyle, null);
    }


    /**
     *  See the border-top property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderTop()
    {
	return getPropertyValue("borderTop");
    }

    public void setBorderTop(String borderTop)
                                             throws DOMException
    {
	setProperty("borderTop", borderTop, null);
    }

    /**
     *  See the border-right property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderRight()
    {
	return getPropertyValue("borderRight");
    }

    public void setBorderRight(String borderRight)
                                             throws DOMException
    {
	setProperty("borderRight", borderRight, null);
    }

    /**
     *  See the border-bottom property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderBottom()
    {
	return getPropertyValue("borderBottom");
    }

    public void setBorderBottom(String borderBottom)
                                             throws DOMException
    {
	setProperty("borderBottom", borderBottom, null);
    }

    /**
     *  See the border-left property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderLeft()
    {
	return getPropertyValue("borderLeft");
    }

    public void setBorderLeft(String borderLeft)
                                             throws DOMException
    {
	setProperty("borderLeft", borderLeft, null);
    }

    /**
     *  See the border-top-color property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderTopColor()
    {
	return getPropertyValue("borderTopColor");
    }

    public void setBorderTopColor(String borderTopColor)
                                             throws DOMException
    {
	setProperty("borderTopColor", borderTopColor, null);
    }

    /**
     *  See the border-right-color property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderRightColor()
    {
	return getPropertyValue("borderRightColor");
    }

    public void setBorderRightColor(String borderRightColor)
                                             throws DOMException
    {
	setProperty("borderRightColor", borderRightColor, null);
    }

    /**
     *  See the border-bottom-color property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderBottomColor()
    {
	return getPropertyValue("borderBottomColor");
    }

    public void setBorderBottomColor(String borderBottomColor)
                                             throws DOMException
    {
	setProperty("borderBottomColor", borderBottomColor, null);
    }

    /**
     *  See the border-left-color property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderLeftColor()
    {
	return getPropertyValue("borderLeftColor");
    }

    public void setBorderLeftColor(String borderLeftColor)
                                             throws DOMException
    {
	setProperty("borderLeftColor", borderLeftColor, null);
    }

    /**
     *  See the border-top-style property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderTopStyle()
    {
	return getPropertyValue("borderTopStyle");
    }

    public void setBorderTopStyle(String borderTopStyle)
                                             throws DOMException
    {
	setProperty("borderTopStyle", borderTopStyle, null);
    }

    /**
     *  See the border-right-style property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderRightStyle()
    {
	return getPropertyValue("borderRightStyle");
    }

    public void setBorderRightStyle(String borderRightStyle)
                                             throws DOMException
    {
	setProperty("borderRightStyle", borderRightStyle, null);
    }

    /**
     *  See the border-bottom-style property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderBottomStyle()
    {
	return getPropertyValue("borderBottomStyle");
    }

    public void setBorderBottomStyle(String borderBottomStyle)
                                             throws DOMException
    {
	setProperty("borderBottomStyle", borderBottomStyle, null);
    }

    /**
     *  See the border-left-style property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderLeftStyle()
    {
	return getPropertyValue("borderLeftStyle");
    }

    public void setBorderLeftStyle(String borderLeftStyle)
                                             throws DOMException
    {
	setProperty("borderLeftStyle", borderLeftStyle, null);
    }

    /**
     *  See the border-top-width property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderTopWidth()
    {
	return getPropertyValue("borderTopWidth");
    }

    public void setBorderTopWidth(String borderTopWidth)
                                             throws DOMException
    {
	setProperty("borderTopWidth", borderTopWidth, null);
    }

    /**
     *  See the border-right-width property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderRightWidth()
    {
	return getPropertyValue("borderRightWidth");
    }

    public void setBorderRightWidth(String borderRightWidth)
                                             throws DOMException
    {
	setProperty("borderRightWidth", borderRightWidth, null);
    }

    /**
     *  See the border-bottom-width property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderBottomWidth()
    {
	return getPropertyValue("borderBottomWidth");
    }

    public void setBorderBottomWidth(String borderBottomWidth)
                                             throws DOMException
    {
	setProperty("borderBottomWidth", borderBottomWidth, null);
    }

    /**
     *  See the border-left-width property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderLeftWidth()
    {
	return getPropertyValue("borderLeftWidth");
    }

    public void setBorderLeftWidth(String borderLeftWidth)
                                             throws DOMException
    {
	setProperty("borderLeftWidth", borderLeftWidth, null);
    }

    /**
     *  See the border-width property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderWidth()
    {
	return getPropertyValue("borderWidth");
    }

    public void setBorderWidth(String borderWidth)
                                             throws DOMException
    {
	setProperty("borderWidth", borderWidth, null);
    }

    /**
     *  See the bottom property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBottom()
    {
	return getPropertyValue("bottom");
    }

    public void setBottom(String bottom)
                                             throws DOMException
    {
	setProperty("bottom", bottom, null);
    }

    /**
     *  See the caption-side property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getCaptionSide()
    {
	return getPropertyValue("captionSide");
    }

    public void setCaptionSide(String captionSide)
                                             throws DOMException
    {
	setProperty("captionSide", captionSide, null);
    }

    /**
     *  See the clear property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getClear()
    {
	return getPropertyValue("clear");
    }

    public void setClear(String clear)
                                             throws DOMException
    {
	setProperty("clear", clear, null);
    }

    /**
     *  See the clip property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getClip()
    {
	return getPropertyValue("clip");
    }

    public void setClip(String clip)
                                             throws DOMException
    {
	setProperty("clip", clip, null);
    }

    /**
     *  See the color property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getColor()
    {
	return getPropertyValue("color");
    }

    public void setColor(String color)
                                             throws DOMException
    {
	setProperty("color", color, null);
    }

    /**
     *  See the content property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getContent()
    {
	return getPropertyValue("content");
    }

    public void setContent(String content)
                                             throws DOMException
    {
	setProperty("content", content, null);
    }

    /**
     *  See the counter-increment property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getCounterIncrement()
    {
	return getPropertyValue("counterIncrement");
    }

    public void setCounterIncrement(String counterIncrement)
                                             throws DOMException
    {
	setProperty("counterIncrement", counterIncrement, null);
    }

    /**
     *  See the counter-reset property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getCounterReset()
    {
	return getPropertyValue("counterReset");
    }


    public void setCounterReset(String counterReset)
                                             throws DOMException
    {
	setProperty("counterReset", counterReset, null);
    }

    /**
     *  See the cue property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getCue()
    {
	return getPropertyValue("cue");
    }

    public void setCue(String cue)
                                             throws DOMException
    {
	setProperty("cue", cue, null);
    }

    /**
     *  See the cue-after property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getCueAfter()
    {
	return getPropertyValue("cueAfter");
    }

    public void setCueAfter(String cueAfter)
                                             throws DOMException
    {
	setProperty("cueAfter", cueAfter, null);
    }

    /**
     *  See the cue-before property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getCueBefore()
    {
	return getPropertyValue("cueBefore");
    }

    public void setCueBefore(String cueBefore)
                                             throws DOMException
    {
	setProperty("cueBefore", cueBefore, null);
    }

    /**
     *  See the cursor property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getCursor()
    {
	return getPropertyValue("cursor");
    }

    public void setCursor(String cursor)
                                             throws DOMException
    {
	setProperty("cursor", cursor, null);
    }

    /**
     *  See the direction property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getDirection()
    {
	return getPropertyValue("direction");
    }

    public void setDirection(String direction)
                                             throws DOMException
    {
	setProperty("direction", direction, null);
    }

    /**
     *  See the display property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getDisplay()
    {
	return getPropertyValue("display");
    }

    public void setDisplay(String display)
                                             throws DOMException
    {
	setProperty("display", display, null);
    }

    /**
     *  See the elevation property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getElevation()
    {
	return getPropertyValue("elevation");
    }

    public void setElevation(String elevation)
                                             throws DOMException
    {
	setProperty("elevation", elevation, null);
    }

    /**
     *  See the empty-cells property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getEmptyCells()
    {
	return getPropertyValue("emptyCells");
    }

    public void setEmptyCells(String emptyCells)
                                             throws DOMException
    {
	setProperty("emptyCells", emptyCells, null);
    }

    /**
     *  See the float property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getCssFloat()
    {
	return getPropertyValue("cssFloat");
    }

    public void setCssFloat(String cssFloat)
                                             throws DOMException
    {
	setProperty("cssFloat", cssFloat, null);
    }

    /**
     *  See the font property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getFont()
    {
	return getPropertyValue("font");
    }

    public void setFont(String font)
                                             throws DOMException
    {
	setProperty("font", font, null);
    }

    /**
     *  See the font-family property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getFontFamily()
    {
	return getPropertyValue("fontFamily");
    }

    public void setFontFamily(String fontFamily)
                                             throws DOMException
    {
	setProperty("fontFamily", fontFamily, null);
    }

    /**
     *  See the font-size property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getFontSize()
    {
	return getPropertyValue("fontSize");
    }

    public void setFontSize(String fontSize)
                                             throws DOMException
    {
	setProperty("fontSize", fontSize, null);
    }

    /**
     *  See the font-size-adjust property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getFontSizeAdjust()
    {
	return getPropertyValue("fontSizeAdjust");
    }

    public void setFontSizeAdjust(String fontSizeAdjust)
                                             throws DOMException
    {
	setProperty("fontSizeAdjust", fontSizeAdjust, null);
    }

    /**
     *  See the font-stretch property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getFontStretch()
    {
	return getPropertyValue("fontStretch");
    }

    public void setFontStretch(String fontStretch)
                                             throws DOMException
    {
	setProperty("fontStretch", fontStretch, null);
    }

    /**
     *  See the font-style property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getFontStyle()
    {
	return getPropertyValue("fontStyle");
    }

    public void setFontStyle(String fontStyle)
                                             throws DOMException
    {
	setProperty("fontStyle", fontStyle, null);
    }

    /**
     *  See the font-variant property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getFontVariant()
    {
	return getPropertyValue("fontVariant");
    }

    public void setFontVariant(String fontVariant)
                                             throws DOMException
    {
	setProperty("fontVariant", fontVariant, null);
    }

    /**
     *  See the font-weight property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getFontWeight()
    {
	return getPropertyValue("fontWeight");
    }

    public void setFontWeight(String fontWeight)
                                             throws DOMException
    {
	setProperty("fontWeight", fontWeight, null);
    }

    /**
     *  See the height property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getHeight()
    {
	return getPropertyValue("height");
    }

    public void setHeight(String height)
                                             throws DOMException
    {
	setProperty("height", height, null);
    }

    /**
     *  See the left property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getLeft()
    {
	return getPropertyValue("left");
    }

    public void setLeft(String left)
                                             throws DOMException
    {
	setProperty("left", left, null);
    }

    /**
     *  See the letter-spacing property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getLetterSpacing()
    {
	return getPropertyValue("letterSpacing");
    }

    public void setLetterSpacing(String letterSpacing)
                                             throws DOMException
    {
	setProperty("letterSpacing", letterSpacing, null);
    }

    /**
     *  See the line-height property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getLineHeight()
    {
	return getPropertyValue("lineHeight");
    }

    public void setLineHeight(String lineHeight)
                                             throws DOMException
    {
	setProperty("lineHeight", lineHeight, null);
    }

    /**
     *  See the list-style property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getListStyle()
    {
	return getPropertyValue("listStyle");
    }

    public void setListStyle(String listStyle)
                                             throws DOMException
    {
	setProperty("listStyle", listStyle, null);
    }

    /**
     *  See the list-style-image property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getListStyleImage()
    {
	return getPropertyValue("listStyleImage");
    }

    public void setListStyleImage(String listStyleImage)
                                             throws DOMException
    {
	setProperty("listStyleImage", listStyleImage, null);
    }

    /**
     *  See the list-style-position property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getListStylePosition()
    {
	return getPropertyValue("listStylePosition");
    }

    public void setListStylePosition(String listStylePosition)
                                             throws DOMException
    {
	setProperty("listStylePosition", listStylePosition, null);
    }

    /**
     *  See the list-style-type property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getListStyleType()
    {
	return getPropertyValue("listStyleType");
    }

    public void setListStyleType(String listStyleType)
                                             throws DOMException
    {
	setProperty("listStyleType", listStyleType, null);
    }

    /**
     *  See the margin property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getMargin()
    {
	return getPropertyValue("margin");
    }

    public void setMargin(String margin)
                                             throws DOMException
    {
	setProperty("margin", margin, null);
    }

    /**
     *  See the margin-top property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getMarginTop()
    {
	return getPropertyValue("marginTop");
    }

    public void setMarginTop(String marginTop)
                                             throws DOMException
    {
	setProperty("marginTop", marginTop, null);
    }

    /**
     *  See the margin-right property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getMarginRight()
    {
	return getPropertyValue("marginRight");
    }

    public void setMarginRight(String marginRight)
                                             throws DOMException
    {
	setProperty("marginRight", marginRight, null);
    }

    /**
     *  See the margin-bottom property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getMarginBottom()
    {
	return getPropertyValue("marginBottom");
    }

    public void setMarginBottom(String marginBottom)
                                             throws DOMException
    {
	setProperty("marginBottom", marginBottom, null);
    }

    /**
     *  See the margin-left property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getMarginLeft()
    {
	return getPropertyValue("marginLeft");
    }

    public void setMarginLeft(String marginLeft)
                                             throws DOMException
    {
	setProperty("marginLeft", marginLeft, null);
    }

    /**
     *  See the marker-offset property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getMarkerOffset()
    {
	return getPropertyValue("markerOffset");
    }

    public void setMarkerOffset(String markerOffset)
                                             throws DOMException
    {
	setProperty("markerOffset", markerOffset, null);
    }

    /**
     *  See the marks property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getMarks()
    {
	return getPropertyValue("marks");
    }

    public void setMarks(String marks)
                                             throws DOMException
    {
	setProperty("marks", marks, null);
    }

    /**
     *  See the max-height property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getMaxHeight()
    {
	return getPropertyValue("maxHeight");
    }

    public void setMaxHeight(String maxHeight)
                                             throws DOMException
    {
	setProperty("maxHeight", maxHeight, null);
    }

    /**
     *  See the max-width property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getMaxWidth()
    {
	return getPropertyValue("maxWidth");
    }

    public void setMaxWidth(String maxWidth)
                                             throws DOMException
    {
	setProperty("maxWidth", maxWidth, null);
    }

    /**
     *  See the min-height property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getMinHeight()
    {
	return getPropertyValue("minHeight");
    }

    public void setMinHeight(String minHeight)
                                             throws DOMException
    {
	setProperty("minHeight", minHeight, null);
    }

    /**
     *  See the min-width property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getMinWidth()
    {
	return getPropertyValue("minWidth");
    }

    public void setMinWidth(String minWidth)
                                             throws DOMException
    {
	setProperty("minWidth", minWidth, null);
    }

    /**
     *  See the orphans property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getOrphans()
    {
	return getPropertyValue("orphans");
    }

    public void setOrphans(String orphans)
                                             throws DOMException
    {
	setProperty("orphans", orphans, null);
    }

    /**
     *  See the outline property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getOutline()
    {
	return getPropertyValue("outline");
    }

    public void setOutline(String outline)
                                             throws DOMException
    {
	setProperty("outline", outline, null);
    }

    /**
     *  See the outline-color property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getOutlineColor()
    {
	return getPropertyValue("outlineColor");
    }

    public void setOutlineColor(String outlineColor)
                                             throws DOMException
    {
	setProperty("outlineColor", outlineColor, null);
    }

    /**
     *  See the outline-style property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getOutlineStyle()
    {
	return getPropertyValue("outlineStyle");
    }

    public void setOutlineStyle(String outlineStyle)
                                             throws DOMException
    {
	setProperty("outlineStyle", outlineStyle, null);
    }

    /**
     *  See the outline-width property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getOutlineWidth()
    {
	return getPropertyValue("outlineWidth");
    }

    public void setOutlineWidth(String outlineWidth)
                                             throws DOMException
    {
	setProperty("outlineWidth", outlineWidth, null);
    }

    /**
     *  See the overflow property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getOverflow()
    {
	return getPropertyValue("overflow");
    }

    public void setOverflow(String overflow)
                                             throws DOMException
    {
	setProperty("overflow", overflow, null);
    }

    /**
     *  See the padding property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getPadding()
    {
	return getPropertyValue("padding");
    }

    public void setPadding(String padding)
                                             throws DOMException
    {
	setProperty("padding", padding, null);
    }

    /**
     *  See the padding-top property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getPaddingTop()
    {
	return getPropertyValue("paddingTop");
    }

    public void setPaddingTop(String paddingTop)
                                             throws DOMException
    {
	setProperty("paddingTop", paddingTop, null);
    }

    /**
     *  See the padding-right property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getPaddingRight()
    {
	return getPropertyValue("paddingRight");
    }

    public void setPaddingRight(String paddingRight)
                                             throws DOMException
    {
	setProperty("paddingRight", paddingRight, null);
    }

    /**
     *  See the padding-bottom property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getPaddingBottom()
    {
	return getPropertyValue("paddingBottom");
    }

    public void setPaddingBottom(String paddingBottom)
                                             throws DOMException
    {
	setProperty("paddingBottom", paddingBottom, null);
    }

    /**
     *  See the padding-left property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getPaddingLeft()
    {
	return getPropertyValue("paddingLeft");
    }

    public void setPaddingLeft(String paddingLeft)
                                             throws DOMException
    {
	setProperty("paddingLeft", paddingLeft, null);
    }

    /**
     *  See the page property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getPage()
    {
	return getPropertyValue("page");
    }

    public void setPage(String page)
                                             throws DOMException
    {
	setProperty("page", page, null);
    }

    /**
     *  See the page-break-after property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getPageBreakAfter()
    {
	return getPropertyValue("pageBreakAfter");
    }

    public void setPageBreakAfter(String pageBreakAfter)
                                             throws DOMException
    {
	setProperty("pageBreakAfter", pageBreakAfter, null);
    }

    /**
     *  See the page-break-before property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getPageBreakBefore()
    {
	return getPropertyValue("pageBreakBefore");
    }

    public void setPageBreakBefore(String pageBreakBefore)
                                             throws DOMException
    {
	setProperty("pageBreakBefore", pageBreakBefore, null);
    }

    /**
     *  See the page-break-inside property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getPageBreakInside()
    {
	return getPropertyValue("pageBreakInside");
    }

    public void setPageBreakInside(String pageBreakInside)
                                             throws DOMException
    {
	setProperty("pageBreakInside", pageBreakInside, null);
    }

    /**
     *  See the pause property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getPause()
    {
	return getPropertyValue("pause");
    }

    public void setPause(String pause)
                                             throws DOMException
    {
	setProperty("pause", pause, null);
    }

    /**
     *  See the pause-after property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getPauseAfter()
    {
	return getPropertyValue("pauseAfter");
    }

    public void setPauseAfter(String pauseAfter)
                                             throws DOMException
    {
	setProperty("pauseAfter", pauseAfter, null);
    }

    /**
     *  See the pause-before property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getPauseBefore()
    {
	return getPropertyValue("pauseBefore");
    }

    public void setPauseBefore(String pauseBefore)
                                             throws DOMException
    {
	setProperty("pauseBefore", pauseBefore, null);
    }

    /**
     *  See the pitch property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getPitch()
    {
	return getPropertyValue("pitch");
    }

    public void setPitch(String pitch)
                                             throws DOMException
    {
	setProperty("pitch", pitch, null);
    }

    /**
     *  See the pitch-range property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getPitchRange()
    {
	return getPropertyValue("pitchRange");
    }

    public void setPitchRange(String pitchRange)
                                             throws DOMException
    {
	setProperty("pitchRange", pitchRange, null);
    }

    /**
     *  See the play-during property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getPlayDuring()
    {
	return getPropertyValue("playDuring");
    }

    public void setPlayDuring(String playDuring)
                                             throws DOMException
    {
	setProperty("playDuring", playDuring, null);
    }

    /**
     *  See the position property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getPosition()
    {
	return getPropertyValue("position");
    }

    public void setPosition(String position)
                                             throws DOMException
    {
	setProperty("position", position, null);
    }

    /**
     *  See the quotes property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getQuotes()
    {
	return getPropertyValue("quotes");
    }

    public void setQuotes(String quotes)
                                             throws DOMException
    {
	setProperty("quotes", quotes, null);
    }

    /**
     *  See the richness property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getRichness()
    {
	return getPropertyValue("richness");
    }

    public void setRichness(String richness)
                                             throws DOMException
    {
	setProperty("richness", richness, null);
    }

    /**
     *  See the right property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getRight()
    {
	return getPropertyValue("right");
    }

    public void setRight(String right)
                                             throws DOMException
    {
	setProperty("right", right, null);
    }

    /**
     *  See the size property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getSize()
    {
	return getPropertyValue("size");
    }

    public void setSize(String size)
                                             throws DOMException
    {
	setProperty("size", size, null);
    }

    /**
     *  See the speak property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getSpeak()
    {
	return getPropertyValue("speak");
    }

    public void setSpeak(String speak)
                                             throws DOMException
    {
	setProperty("speak", speak, null);
    }

    /**
     *  See the speak-header property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getSpeakHeader()
    {
	return getPropertyValue("speakHeader");
    }

    public void setSpeakHeader(String speakHeader)
                                             throws DOMException
    {
	setProperty("speakHeader", speakHeader, null);
    }

    /**
     *  See the speak-numeral property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getSpeakNumeral()
    {
	return getPropertyValue("speakNumeral");
    }

    public void setSpeakNumeral(String speakNumeral)
                                             throws DOMException
    {
	setProperty("speakNumeral", speakNumeral, null);
    }

    /**
     *  See the speak-punctuation property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getSpeakPunctuation()
    {
	return getPropertyValue("speakPunctuation");
    }

    public void setSpeakPunctuation(String speakPunctuation)
                                             throws DOMException
    {
	setProperty("speakPunctuation", speakPunctuation, null);
    }

    /**
     *  See the speech-rate property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getSpeechRate()
    {
	return getPropertyValue("speechRate");
    }

    public void setSpeechRate(String speechRate)
                                             throws DOMException
    {
	setProperty("speechRate", speechRate, null);
    }

    /**
     *  See the stress property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getStress()
    {
	return getPropertyValue("stress");
    }

    public void setStress(String stress)
                                             throws DOMException
    {
	setProperty("stress", stress, null);
    }

    /**
     *  See the table-layout property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getTableLayout()
    {
	return getPropertyValue("tableLayout");
    }

    public void setTableLayout(String tableLayout)
                                             throws DOMException
    {
	setProperty("tableLayout", tableLayout, null);
    }

    /**
     *  See the text-align property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getTextAlign()
    {
	return getPropertyValue("textAlign");
    }

    public void setTextAlign(String textAlign)
                                             throws DOMException
    {
	setProperty("textAlign", textAlign, null);
    }

    /**
     *  See the text-decoration property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getTextDecoration()
    {
	return getPropertyValue("textDecoration");
    }

    public void setTextDecoration(String textDecoration)
                                             throws DOMException
    {
	setProperty("textDecoration", textDecoration, null);
    }

    /**
     *  See the text-indent property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getTextIndent()
    {
	return getPropertyValue("textIndent");
    }

    public void setTextIndent(String textIndent)
                                             throws DOMException
    {
	setProperty("textIndent", textIndent, null);
    }

    /**
     *  See the text-shadow property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getTextShadow()
    {
	return getPropertyValue("textShadow");
    }

    public void setTextShadow(String textShadow)
                                             throws DOMException
    {
	setProperty("textShadow", textShadow, null);
    }

    /**
     *  See the text-transform property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getTextTransform()
    {
	return getPropertyValue("textTransform");
    }

    public void setTextTransform(String textTransform)
                                             throws DOMException
    {
	setProperty("textTransform", textTransform, null);
    }

    /**
     *  See the top property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getTop()
    {
	return getPropertyValue("top");
    }

    public void setTop(String top)
                                             throws DOMException
    {
	setProperty("top", top, null);
    }

    /**
     *  See the unicode-bidi property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getUnicodeBidi()
    {
	return getPropertyValue("unicodeBidi");
    }

    public void setUnicodeBidi(String unicodeBidi)
                                             throws DOMException
    {
	setProperty("unicodeBidi", unicodeBidi, null);
    }

    /**
     *  See the vertical-align property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getVerticalAlign()
    {
	return getPropertyValue("verticalAlign");
    }

    public void setVerticalAlign(String verticalAlign)
                                             throws DOMException
    {
	setProperty("verticalAlign", verticalAlign, null);
    }

    /**
     *  See the visibility property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getVisibility()
    {
	return getPropertyValue("visibility");
    }

    public void setVisibility(String visibility)
                                             throws DOMException
    {
	setProperty("visibility", visibility, null);
    }

    /**
     *  See the voice-family property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getVoiceFamily()
    {
	return getPropertyValue("voiceFamily");
    }

    public void setVoiceFamily(String voiceFamily)
                                             throws DOMException
    {
	setProperty("voiceFamily", voiceFamily, null);
    }

    /**
     *  See the volume property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getVolume()
    {
	return getPropertyValue("volumn");
    }

    public void setVolume(String volume)
                                             throws DOMException
    {
	setProperty("volume", volume, null);
    }

    /**
     *  See the white-space property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getWhiteSpace()
    {
	return getPropertyValue("whiteSpace");
    }

    public void setWhiteSpace(String whiteSpace)
                                             throws DOMException
    {
	setProperty("whiteSpace", whiteSpace, null);
    }

    /**
     *  See the widows property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getWidows()
    {
	return getPropertyValue("widows");
    }

    public void setWidows(String widows)
                                             throws DOMException
    {
	setProperty("widows", widows, null);
    }

    /**
     *  See the width property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getWidth()
    {
	return getPropertyValue("width");
    }

    public void setWidth(String width)
                                             throws DOMException
    {
	setProperty("width", width, null);
    }

    /**
     *  See the word-spacing property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getWordSpacing()
    {
	return getPropertyValue("wordSpacing");
    }

    public void setWordSpacing(String wordSpacing)
                                             throws DOMException
    {
	setProperty("wordSpacing", wordSpacing, null);
    }

    /**
     *  See the z-index property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getZIndex()
    {
	return getPropertyValue("zIndex");
    }

    public void setZIndex(String zIndex)
                                             throws DOMException
    {
	setProperty("zIndex", zIndex, null);
    }
}




