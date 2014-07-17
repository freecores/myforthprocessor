/*
 * @(#)PixelStore8.java	1.12 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.image;

import java.awt.image.*;

public class PixelStore8 extends PixelStore {
    public PixelStore8() {
    }

    public PixelStore8(int w, int h) {
	super(w, h);
    }

    public PixelStore8(int w, int h, ColorModel cm) {
	setDimensions(w, h);
	setColorModel(cm);
    }

    public void setDimensions(int w, int h) {
	super.setDimensions(w, h);
    }

    Object allocateLines(int num) {
	if (num * width > 1000000) {
	    return null;
	} else {
	    return (Object) new byte[num * width];
	}
    }

    void replayLines(ImageConsumer ic, int i, int cnt, Object line) {
	ic.setPixels(0, i, width, cnt, colormodel,
		     (byte[]) line, offsets[i], width);
    }
}
