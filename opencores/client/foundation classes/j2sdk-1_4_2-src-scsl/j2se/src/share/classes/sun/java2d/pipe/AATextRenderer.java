/*
 * @(#)AATextRenderer.java	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d.pipe;

import java.awt.font.GlyphVector;
import sun.java2d.SunGraphics2D;
import sun.awt.font.GlyphList;
import sun.awt.font.NativeFontWrapper;


/**
 * A delegate pipe of SG2D for drawinng anti-aliased text with 
 * a solid source colour to an opaque destination.
 */

public class AATextRenderer extends GlyphListPipe {

   /*
    * Override super class method to call the non-AA pipe if
    * AA is not specified in the GlyphVector's FontRenderContext
    */
   public void drawGlyphVector(SunGraphics2D sg2d, GlyphVector g,
                               float x, float y) {

        if (!g.getFontRenderContext().isAntiAliased()) {
            sg2d.surfaceData.solidTextRenderer.drawGlyphVector(sg2d, g, x, y);
        } else {
            super.drawGlyphVector(sg2d, g, x, y);
        }
   }

   protected void drawGlyphList(SunGraphics2D sg2d, GlyphList gl) {
       synchronized (NativeFontWrapper.class) {
	   sg2d.loops.drawGlyphListAALoop.DrawGlyphListAA(sg2d, 
							  sg2d.surfaceData,
							  gl, gl.getPeer());
       }
   }

}
