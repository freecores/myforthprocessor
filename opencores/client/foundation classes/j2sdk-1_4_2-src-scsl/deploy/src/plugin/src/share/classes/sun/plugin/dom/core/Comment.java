/*
 * @(#)Comment.java	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.core;

import org.w3c.dom.*;
import sun.plugin.dom.*;


/**
 *  This interface inherits from <code>CharacterData</code> and represents the 
 * content of a comment, i.e., all the characters between the starting '
 * <code>&lt;!--</code> ' and ending '<code>--&gt;</code> '. Note that this 
 * is the definition of a comment in XML, and, in practice, HTML, although 
 * some HTML tools may implement the full SGML comment structure.
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
final class Comment extends sun.plugin.dom.core.CharacterData 
		     implements org.w3c.dom.Comment
{
    /**
     * Construct a Comment object.
     */
    public Comment(DOMObject obj)
    {
	super(DOMObjectType.Comment, obj);
    }

    /**
     *  The name of this node, depending on its type; see the table above. 
     */
    public String getNodeName()
    {
	return "#comment";
    }

    /**
     *  A code representing the type of the underlying object, as defined 
     * above.
     */
    public short getNodeType()
    {
	return COMMENT_NODE;
    }
}

