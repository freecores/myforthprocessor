/*
 * @(#)GlyphCacheOverFlowException.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.font;


/**
 * This exception is thrown if the underlying font rasterisation engine
 * determines a particular text string when rasterised in the requested
 * render context is too large for the glyph cache
 */
public class GlyphCacheOverFlowException extends java.lang.RuntimeException {
    public GlyphCacheOverFlowException(String s) {
        super (s);
    }
}
