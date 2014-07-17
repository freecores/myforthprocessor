/*
 * @(#)FontSupport.java	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d;

import sun.awt.FontProperties;

/**
 * Font support for graphics environment
 */
public interface FontSupport {

    /**
     * Maps fontName and style to the face name of a logical font or an
     * available physical font. fontName can be a family name or a
     * face name.
     */
    public String mapFontName(String fontName, int style);

    /**
     * Returns the current font properties.
     */
    public FontProperties getFontProperties();
}
