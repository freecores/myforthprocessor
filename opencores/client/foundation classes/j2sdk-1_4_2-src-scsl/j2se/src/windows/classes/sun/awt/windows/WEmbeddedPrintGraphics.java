/*
 * @(#)WEmbeddedPrintGraphics.java	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.windows;

import java.awt.*;

/**
 * WEmbeddedPrintGraphics is a WPrintGraphics that encapsulates an HDC
 * passed to an EmbeddedFrame.
 */

public class WEmbeddedPrintGraphics extends WPrintGraphics {

    WEmbeddedPrintGraphics(int hdc) {
        super(hdc);
    }

    protected void disposeImpl() {
        // Don't release the external HDC.
    }
}
